using System;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Threading.Tasks;
using MultipartFormData;
using System.Collections.Generic;
using System.Web.Configuration;

namespace CloudServer
{
    public class UserOperations
    {
        static public async Task<bool> ProcessRegister(Object session, Guid session_id, Request request, Response response)
        {
            try
            {
                // Deserializing json string to UserInfo
                UserInfo info = JObject.Parse(request.Body).ToObject<UserInfo>();

                // Check if the account already exists
                if (await SqlDatabase.Instance.AccountExistAsync(info))
                {
                    SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.BadRequest, "Account exists!"));
                    return false;
                }

                // Insert account info to SqlDatabase
                info = await SqlDatabase.Instance.RegisterAccountAsync(info);
                if (info is null)
                {
                    SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "Registration failed!"));
                    return false;
                }
                UserManager.Instance.AddUserSession(session_id, STATES.REGISTED, (int)info.user_id, info.user_name);
                
                // Create directory local for handling file uploads and updates
                string storagePath = Path.Combine(LocalDatabase.Instance.GetStorageDirectory(), info.user_name);
                Directory.CreateDirectory(storagePath);

                // Create response json
                string json_response = JsonConvert.SerializeObject(info, Formatting.Indented);
                SendResponseAsync(session, response.MakeResponse((int)HttpStatusCode.Created, json_response, "application/json"));
            }
            catch (Exception ex)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "An error occurred: " + ex.Message));
                return false;
            }
            return true;
        }
        static public async Task<bool> ProcessLogin(Object session, Guid session_id, Request request, Response response)
        {
            try
            {
                // Parse json request
                JObject json_request = JObject.Parse(request.Body);
                string user_name = (string)json_request.SelectToken("user_name");
                string password = (string)json_request.SelectToken("password");
                // Validate credentials asynchronously
                var user_id = await SqlDatabase.Instance.LoginAccountAsync(user_name, password);
                if (user_id is null)
                {
                    SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.Unauthorized, "Invalid username or password."));
                    return false;
                }
                // Check user is active
                if (UserManager.Instance.GetUserStatus((int)user_id) == STATES.LOGGED_IN)
                {
                    SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.BadRequest, "User session is already inactive."));
                    return false;
                }
                // TODO: Create TokenId = user_name + password + DateTime
                string token_id = Convert.ToBase64String(Encoding.ASCII.GetBytes(user_name + password + DateTime.Now.ToString()));
                if (token_id is null)
                {
                    SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "Can't create token_id for user login."));
                    return false;
                }
                if(UserManager.Instance.GetUserStatus((int)user_id) == STATES.REGISTED)
                {
                    UserManager.Instance.UpdateUserSession(session_id, STATES.LOGGED_IN, token_id, TimeSpan.FromMinutes(5));
                }
                else
                {
                    UserManager.Instance.AddUserSession(session_id, STATES.LOGGED_IN, (int)user_id, user_name, token_id, TimeSpan.FromMinutes(5));
                }

                // Create local directory for save file upload
                string storagePath = Path.Combine(LocalDatabase.Instance.GetStorageDirectory(), user_name);
                if (!Directory.Exists(storagePath))
                {
                    Directory.CreateDirectory(storagePath);
                }
                // Create and send json response 
                JObject json_response = new JObject
                {
                    { "token", token_id },
                    { "message", $"Welcome {user_name}, you have logged in successfully!" }
                };
                SendResponseAsync(session, response.MakeOkResponse(json_response.ToString(), "application/json"));
            }
            catch (Exception ex)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "An error occurred: " + ex.Message));
                return false;
            }
            return true;
        }
        static public async Task<bool> ProcessLogout(Object session, Guid session_id, Request request, Response response, string user_name)
        {
            var authorizationHeader = request.Header("Authorization");
            if (string.IsNullOrEmpty(authorizationHeader))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.Unauthorized, "Authorization header is missing."));
                return false;
            }
            var token_id = authorizationHeader.StartsWith("Bearer ")
                            ? authorizationHeader.Substring("Bearer ".Length)
                            : authorizationHeader; // Fallback to the full header if it doesn't start with "Bearer "

            var user_id = UserManager.Instance.GetUserID(token_id);
            if (user_id is null)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, $"User {user_name} does not exist."));
                return false;
            }
            var user_status = UserManager.Instance.GetUserStatus((int)user_id);
            if (user_status == STATES.LOGGED_OUT || user_status == STATES.DISCONNECTED)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, $"User {user_name} not logged in."));
                return false;
            }
            if (!await SqlDatabase.Instance.UserExistAsync((int)user_id))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "Account doesn't exists!"));
                return false;
            }
            try
            {
                // Create and send json response 
                JObject json_response = new JObject
                {
                    { "token", token_id },
                    { "message", $"Logout successful! Bye {user_name}." }
                };
                SendResponseAsync(session, response.MakeOkResponse(json_response.ToString(), "application/json"));
                UserManager.Instance.UpdateUserSession(session_id, STATES.LOGGED_OUT);
            }
            catch (Exception ex)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "An error occurred: " + ex.Message));
                return false;
            }
            return true;
        }
        
        static public async Task<bool> ProcessGetProfile(Object session, Request request, Response response, string user_name)
        {
            var authorizationHeader = request.Header("Authorization");
            if (string.IsNullOrEmpty(authorizationHeader))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.Unauthorized, "Authorization header is missing."));
                return false;
            }
            var token_id = authorizationHeader.StartsWith("Bearer ")
                           ? authorizationHeader.Substring("Bearer ".Length)
                           : authorizationHeader; // Fallback to the full header if it doesn't start with "Bearer "

            var user_id = UserManager.Instance.GetUserID(token_id);
            if (user_id is null)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, $"User {user_name} does not exist."));
                return false;
            }
            var user_status = UserManager.Instance.GetUserStatus((int)user_id);
            if (user_status == STATES.LOGGED_OUT || user_status == STATES.DISCONNECTED)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, $"User {user_name} not logged in."));
                return false;
            }
            UserInfo info = await SqlDatabase.Instance.GetUserProfileAsync((int)user_id);
            if (info is null)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, "Failed to retrieve profile!"));
                return false;
            }
            string json_response = JsonConvert.SerializeObject(info, Formatting.Indented);
            SendResponseAsync(session, response.MakeResponse((int)HttpStatusCode.OK, json_response, "application/json"));
            return true;

        }
        static public async Task<bool> ProcessUpdateProfile(Object session, Request request, Response response, string user_name)
        {
            string requestBody = request.Body;
            var authorizationHeader = request.Header("Authorization");
            if (string.IsNullOrEmpty(authorizationHeader))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.Unauthorized, "Authorization header is missing."));
                return false;
            }
            var token_id = authorizationHeader.StartsWith("Bearer ")
                           ? authorizationHeader.Substring("Bearer ".Length)
                           : authorizationHeader; // Fallback to the full header if it doesn't start with "Bearer "

            var user_id = UserManager.Instance.GetUserID(token_id);
            var result = await SqlDatabase.Instance.UserExistAsync((int)user_id);
            if (!(user_id is null) && !result)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, $"User {user_name} does not exist."));
                return false;
            }
            var user_status = UserManager.Instance.GetUserStatus((int)user_id);
            if (user_status == STATES.LOGGED_OUT || user_status == STATES.DISCONNECTED)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, $"User {user_name} not logged in."));
                return false;
            }

            UserInfo new_info = new UserInfo();
            try
            {
                JObject json_request = JObject.Parse(requestBody);

                new_info.first_name = json_request["first_name"]?.ToString();
                new_info.last_name = json_request["last_name"]?.ToString();
                new_info.user_name = json_request["user_name"]?.ToString();
                new_info.password = json_request["password"]?.ToString();
                new_info.email = json_request["email"]?.ToString();
                new_info.birthday = json_request["birthday"] != null ? (DateTime?)json_request["birthday"].ToObject<DateTime>() : null;
            }
            catch (JsonReaderException ex)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.BadRequest, "Invalid JSON format: " + ex.Message));
                return false;
            }
            if (!await SqlDatabase.Instance.UpdateProfileAsync((int)user_id, new_info))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "Failed to update profile!"));
                return false;
            }
            SendResponseAsync(session, response.MakeOkResponse("Profile updated successfully!"));
            return true;
        }
        static public async Task<bool> ProcessChangePassword(Object session, Request request, Response response, string user_name)
        {
            string requestBody = request.Body;
            var authorizationHeader = request.Header("Authorization");
            if (string.IsNullOrEmpty(authorizationHeader))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.Unauthorized, "Authorization header is missing."));
                return false;
            }
            var token_id = authorizationHeader.StartsWith("Bearer ")
                           ? authorizationHeader.Substring("Bearer ".Length)
                           : authorizationHeader; // Fallback to the full header if it doesn't start with "Bearer "

            var user_id = UserManager.Instance.GetUserID(token_id);
            var result = await SqlDatabase.Instance.UserExistAsync((int)user_id);
            if (!(user_id is null) && !result)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, $"User {user_name} does not exist."));
                return false;
            }
            var user_status = UserManager.Instance.GetUserStatus((int)user_id);
            if (user_status == STATES.LOGGED_OUT || user_status == STATES.DISCONNECTED)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "User not logged in."));
                return false;
            }

            string old_password, new_password;
            try
            {
                JObject json_request = JObject.Parse(requestBody);

                old_password = (string)json_request.SelectToken("old_password");
                new_password = (string)json_request.SelectToken("new_password");
            }
            catch (JsonReaderException ex)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "Invalid JSON format: " + ex.Message));
                return false;
            }
            if (!await SqlDatabase.Instance.ChangePasswordAsync((int)user_id, old_password, new_password))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "Failed to change password!"));
                return false;
            }

            SendResponseAsync(session, response.MakeOkResponse($"Account {user_name}. Password changed successfully!"));
            return true;
        }
        static public async Task<bool> ProcessDeleteAccount(Object session, Request request, Response response, string user_name)
        {
            var authorizationHeader = request.Header("Authorization");
            if (string.IsNullOrEmpty(authorizationHeader))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.Unauthorized, "Authorization header is missing."));
                return false;
            }
            var token_id = authorizationHeader.StartsWith("Bearer ")
                           ? authorizationHeader.Substring("Bearer ".Length)
                           : authorizationHeader; // Fallback to the full header if it doesn't start with "Bearer "

            var user_id = UserManager.Instance.GetUserID(token_id);
            var result = await SqlDatabase.Instance.UserExistAsync((int)user_id);
            if (!(user_id is null) && !result)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, $"User {user_name} does not exist."));
                return false;
            }
            var user_status = UserManager.Instance.GetUserStatus((int)user_id);
            if (user_status == STATES.LOGGED_OUT || user_status == STATES.DISCONNECTED)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, $"User {user_name} not logged in."));
                return false;
            }
            if (!await SqlDatabase.Instance.DeleteAccountAsync((int)user_id))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "Failed to delete account!"));
                return false;
            }
            // Delete folder storage
            string storagePath = Path.Combine(LocalDatabase.Instance.GetStorageDirectory(), user_name);
            if (Directory.Exists(storagePath))
            {
                Directory.Delete(storagePath, true);
            }
            SendResponseAsync(session, response.MakeOkResponse($"Account ID = {user_id} deleted successfully!"));
            return true;
        }

        static public async Task<bool> ProcessRenameFile(Object session, Request request, Response response, string user_name, int file_id)
        {
            string requestBody = request.Body;
            var authorizationHeader = request.Header("Authorization");
            if (string.IsNullOrEmpty(authorizationHeader))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.Unauthorized, "Authorization header is missing."));
                return false;
            }
            var token_id = authorizationHeader.StartsWith("Bearer ")
                           ? authorizationHeader.Substring("Bearer ".Length)
                           : authorizationHeader; // Fallback to the full header if it doesn't start with "Bearer "

            var user_id = UserManager.Instance.GetUserID(token_id);
            if (user_id is null)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, $"User {user_name} does not exist."));
                return false;
            }
            var user_status = UserManager.Instance.GetUserStatus((int)user_id);
            if (user_status == STATES.LOGGED_OUT || user_status == STATES.DISCONNECTED)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, $"User {user_name} not logged in."));
                return false;
            }

            //Parse json request
            JObject json_request = JObject.Parse(requestBody);
            string new_fileName = (string)json_request.SelectToken("new_file_name");

            string storage_path = await SqlDatabase.Instance.RenameFileAsync((int)user_id, file_id, new_fileName);
            if (storage_path is null)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, "File not found!"));
                return false;
            }

            string storage_dir_path = Path.GetDirectoryName(storage_path);
            string new_filePath = Path.Combine(storage_dir_path, new_fileName);
            File.Move(storage_path, new_filePath);

            SendResponseAsync(session, response.MakeOkResponse($"File ID = {file_id} renamed successfully!"));
            return true;
        }
        static public async Task<bool> ProcessRemoveFile(Object session, Request request, Response response, string user_name, int file_id)
        {
            var authorizationHeader = request.Header("Authorization");
            if (string.IsNullOrEmpty(authorizationHeader))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.Unauthorized, "Authorization header is missing."));
                return false;
            }
            var token_id = authorizationHeader.StartsWith("Bearer ")
                           ? authorizationHeader.Substring("Bearer ".Length)
                           : authorizationHeader; // Fallback to the full header if it doesn't start with "Bearer "

            var user_id = UserManager.Instance.GetUserID(token_id);
            if (user_id is null)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, $"User {user_name} does not exist."));
                return false;
            }
            var user_status = UserManager.Instance.GetUserStatus((int)user_id);
            if (user_status == STATES.LOGGED_OUT || user_status == STATES.DISCONNECTED)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, $"User {user_name} not logged in."));
                return false;
            }

            string storage_path = await SqlDatabase.Instance.RemoveFileAsync((int)user_id, file_id);
            if (storage_path is null)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "Failed to delete file!"));
                return false;
            }
            File.Delete(storage_path);
            SendResponseAsync(session, response.MakeOkResponse($"File ID = {file_id} deleted successfully!"));
            return true;
        }

        static public async Task<bool> ProcessUploadFileMiss(Object session, Request request, Response response, string user_name)
        {
            string requestBody = request.Body;
            var authorizationHeader = request.Header("Authorization");
            if (string.IsNullOrEmpty(authorizationHeader))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.Unauthorized, "Authorization header is missing."));
                return false;
            }
            var token_id = authorizationHeader.StartsWith("Bearer ")
                           ? authorizationHeader.Substring("Bearer ".Length)
                           : authorizationHeader; // Fallback to the full header if it doesn't start with "Bearer "

            var user_id = UserManager.Instance.GetUserID(token_id);
            if (user_id is null)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, $"User {user_name} does not exist."));
                return false;
            }
            var user_status = UserManager.Instance.GetUserStatus((int)user_id);
            if (user_status == STATES.LOGGED_OUT || user_status == STATES.DISCONNECTED)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, $"User {user_name} not logged in."));
                return false;
            }
            //Parse json request
            var files = JsonConvert.DeserializeObject<List<FileInfo>>(requestBody);
            var files_miss = await SqlDatabase.Instance.RetrieveUploadAsync((int)user_id, files);

            if (files_miss == null || files_miss.Count == 0)
            {
                SendResponseAsync(session, response.MakeOkResponse("No missing files found."));
            }
            else
            {
                JArray array = new JArray();
                for (int i = 0; i < files_miss.Count; i++)
                {
                    // Create a JObject for each missing file
                    JObject fileObject = new JObject
                    {
                        { "folder_path", files_miss[i].folder },
                        { "file_name", files_miss[i].file_name }
                    };

                    // Add the JObject to the JArray
                    array.Add(fileObject);
                }

                // Serialize JArray to JSON
                string jsonResult = JsonConvert.SerializeObject(array, Formatting.Indented);
                SendResponseAsync(session, response.MakeOkResponse(jsonResult, "application/json"));
            }
            return true;
        }

        static public async Task<bool> ProcessUploadFile(Object session, Guid session_id, Request request, Response response, string user_name)
        {
            var authorizationHeader = request.Header("Authorization");
            if (string.IsNullOrEmpty(authorizationHeader))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.Unauthorized, "Authorization header is missing."));
                return false;
            }
            var token_id = authorizationHeader.StartsWith("Bearer ")
                           ? authorizationHeader.Substring("Bearer ".Length)
                           : authorizationHeader; // Fallback to the full header if it doesn't start with "Bearer "

            var user_id = UserManager.Instance.GetUserID(token_id);
            if (user_id is null)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, $"User {user_name} does not exist."));
                return false;
            }
            var user_status = UserManager.Instance.GetUserStatus((int)user_id);
            if (user_status == STATES.LOGGED_OUT || user_status == STATES.DISCONNECTED)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, $"User {user_name} not logged in."));
                return false;
            }
            using (var stream = new MemoryStream(request.BodyBytes))
            {
                // Parse synchronously
                MultipartFormDataParser parser = MultipartFormDataParser.Parse(stream);
                string file_info = parser.GetParameterValue("fileinfo");    // Retrieve form data
                if (parser.Files.Count() == 0)
                {
                    Console.WriteLine($"No files were uploaded. Please attach a file and try again.");
                    return false;
                }

                // Add column SqlDatabase.StoragePath
                JObject obj = JObject.Parse(file_info);
                string folder = (string)obj.SelectToken("folder");
                string file_name = (string)obj.SelectToken("file_name");
                string storage_path = Path.Combine(LocalDatabase.Instance.GetStorageDirectory(), user_name, folder, file_name);

                if (!obj.ContainsKey("storage_on"))
                {
                    obj.Add("storage_on", storage_path);
                }

                // Convert JObject to JArray for deserialization to DataTable
                FileInfo info = obj.ToObject<FileInfo>();
                info = await SqlDatabase.Instance.UploadFileInfoAsync((int)user_id, info);
                if (info is null)
                {
                    SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "Unable to INSERT file information into the database."));
                    return false;
                }

                // Process the file (save it)
                Stream file_data = parser.Files.First().Data;
                using (var fileStream = new FileStream(storage_path, FileMode.Create, FileAccess.Write))
                {
                    file_data.CopyTo(fileStream);
                }

                JObject json_response = new JObject
                {
                    { "file_id", info.file_id },
                    { "upload_id", session_id.ToString() },
                    { "massager", $"File {file_name} uploaded successfully." },
                };
                SendResponseAsync(session, response.MakeResponse((int)HttpStatusCode.Created, json_response.ToString(), "application/json"));
                Console.WriteLine($"File {file_name} has been uploaded!");
            }
            return true;
        }
        static public async Task<bool> ProcessUploadFileLarge(Object session, Guid session_id, Request request, Response response, string user_name, string endpoint)
        {
            var authorizationHeader = request.Header("Authorization");
            if (string.IsNullOrEmpty(authorizationHeader))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.Unauthorized, "Authorization header is missing."));
                return false;
            }
            var token_id = authorizationHeader.StartsWith("Bearer ")
                           ? authorizationHeader.Substring("Bearer ".Length)
                           : authorizationHeader; // Fallback to the full header if it doesn't start with "Bearer "

            var user_id = UserManager.Instance.GetUserID(token_id);
            if (user_id is null)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, $"User {user_name} does not exist."));
                return false;
            }
            var user_status = UserManager.Instance.GetUserStatus((int)user_id);
            if (user_status == STATES.LOGGED_OUT || user_status == STATES.DISCONNECTED)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, $"User {user_name} not logged in."));
                return false;
            }

            if (endpoint.ToLower() == "init")   //step 1
            {
                Console.WriteLine(request);

                // Parse synchronously
                JObject json_request = JObject.Parse(request.Body);
                string folder = (string)json_request.SelectToken("folder");
                string storage_dir = Path.Combine(LocalDatabase.Instance.GetStorageDirectory(), user_name, folder);
                string file_name = (string)json_request.SelectToken("file_name");
                string storage_path = Path.Combine(storage_dir, file_name);
                if (!Directory.Exists(storage_dir))
                {
                    Directory.CreateDirectory(storage_dir);
                    File.Create(storage_path).Close();
                }

                // Add column SqlDatabase.StoragePath
                if (!json_request.ContainsKey("storage_on"))
                {
                    json_request.Add("storage_on", storage_path);
                }

                // Convert JObject to JArray for deserialization to DataTable
                FileInfo info = json_request.ToObject<FileInfo>();
                info = await SqlDatabase.Instance.UploadFileInfoAsync((int)user_id, info);
                if (info is null)
                {
                    SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "Unable to INSERT file information into the database."));
                    return false;
                }
                // Create json response
                JObject json_response = new JObject
                {
                    { "file_id", info.file_id },
                    { "upload_id", session_id },
                    { "massager", $"File {file_name} information inserted successfully!" },
                };
                SendResponseAsync(session, response.MakeResponse((int)HttpStatusCode.Created, json_response.ToString(), "application/json"));
            }
            else if (endpoint == session_id.ToString())        //step 2
            {
                MultipartFormDataParser parser = MultipartFormDataParser.Parse(new MemoryStream(request.BodyBytes));
                Stream file_data = parser.Files.First().Data;
                string file_name = parser.Files.First().FileName;
                string folder_name = parser.GetParameterValue("folder");    // Retrieve form data
                string storage_path = Path.Combine(LocalDatabase.Instance.GetStorageDirectory(), user_name, folder_name, file_name);

                // Process the file (save it)
                using (var fileStream = new FileStream(storage_path, FileMode.Append, FileAccess.Write))
                {
                    file_data.CopyTo(fileStream);
                }
                SendResponseAsync(session, response.MakeOkResponse($"File part has been uploaded {file_data.Length} bytes."));

                // Move the cursor to the beginning of the line
                Console.SetCursorPosition(0, Console.CursorTop);
                // Write the message, overwriting the previous one
                Console.Write($"[{user_name}]: Currently updating file {file_name} with the next part. Please wait... ");
            }
            else if (endpoint.ToLower() == "complete")  //step 3
            {
                Console.WriteLine("\n");
                Console.WriteLine(request);
                SendResponseAsync(session, response.MakeOkResponse("File upload completed successfully."));
            }
            else
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, $"Required value was not found for the key: {endpoint}"));
                return false;
            }
            return true;
        }

        static public async Task<bool> ProcessUpdateFile(Object session, Guid session_id, Request request, Response response, string user_name)
        {
            var authorizationHeader = request.Header("Authorization");
            if (string.IsNullOrEmpty(authorizationHeader))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.Unauthorized, "Authorization header is missing."));
                return false;
            }
            var token_id = authorizationHeader.StartsWith("Bearer ")
                           ? authorizationHeader.Substring("Bearer ".Length)
                           : authorizationHeader; // Fallback to the full header if it doesn't start with "Bearer "

            var user_id = UserManager.Instance.GetUserID(token_id);
            if (user_id is null)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, $"User {user_name} does not exist."));
                return false;
            }
            var user_status = UserManager.Instance.GetUserStatus((int)user_id);
            if (user_status == STATES.LOGGED_OUT || user_status == STATES.DISCONNECTED)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, $"User {user_name} not logged in."));
                return false;
            }
            using (var stream = new MemoryStream(request.BodyBytes))
            {
                // Parse synchronously
                MultipartFormDataParser parser = MultipartFormDataParser.Parse(stream);
                string file_info = parser.GetParameterValue("fileinfo");    // Retrieve form data
                if (parser.Files.Count() == 0)
                {
                    Console.WriteLine($"No files were updated. Please attach a file and try again.");
                    return false;
                }
                JObject obj = JObject.Parse(file_info);
                int file_id = (int)obj.SelectToken("file_id");
                string folder = (string)obj.SelectToken("folder");
                string file_name = (string)obj.SelectToken("file_name");
                string storage_path = Path.Combine(LocalDatabase.Instance.GetStorageDirectory(), user_name, folder, file_name);

                // Convert JObject to JArray for deserialization to DataTable
                FileInfo info = obj.ToObject<FileInfo>();
                if (!await SqlDatabase.Instance.UpdateFileInfoAsync((int)user_id, info))
                {
                    SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "Unable to UPDATE file information into the database."));
                    return false;
                }
                // Process the file (save it)
                Stream file_data = parser.Files.First().Data;
                using (var fileStream = new FileStream(storage_path, FileMode.Create, FileAccess.Write))
                {
                    file_data.CopyTo(fileStream);
                }
                // Create json response
                JObject json_response = new JObject
                {      
                    { "file_id", info.file_id },
                    { "update_id", session_id.ToString() },
                    { "massager", $"File ID {file_id} updated successfully." }
                };
                SendResponseAsync(session, response.MakeResponse((int)HttpStatusCode.Created, json_response.ToString(), "application/json"));
                Console.WriteLine($"File ID {file_id} has been updated!");
            }
            return true;
        }
        static public async Task<bool> ProcessUpdateFileLarge(Object session, Guid session_id, Request request, Response response, string user_name, string endpoint)
        {
            var authorizationHeader = request.Header("Authorization");
            if (string.IsNullOrEmpty(authorizationHeader))
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.Unauthorized, "Authorization header is missing."));
                return false;
            }
            var token_id = authorizationHeader.StartsWith("Bearer ")
                           ? authorizationHeader.Substring("Bearer ".Length)
                           : authorizationHeader; // Fallback to the full header if it doesn't start with "Bearer "

            var user_id = UserManager.Instance.GetUserID(token_id);
            if (user_id is null)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.NotFound, $"User {user_name} does not exist."));
                return false;
            }
            var user_status = UserManager.Instance.GetUserStatus((int)user_id);
            if (user_status == STATES.LOGGED_OUT || user_status == STATES.DISCONNECTED)
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, $"User {user_name} not logged in."));
                return false;
            }
            if (endpoint.ToLower() == "init")
            {
                Console.WriteLine(request);

                // Parse synchronously
                JObject json_request = JObject.Parse(request.Body);
                // Convert JObject to JArray for deserialization to DataTable
                FileInfo info = json_request.ToObject<FileInfo>();
                if (!await SqlDatabase.Instance.UpdateFileInfoAsync((int)user_id, info))
                {
                    SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, "Unable to UPDATE file information into the database."));
                    return false;
                }
                string storage_path = Path.Combine(LocalDatabase.Instance.GetStorageDirectory(), user_name, info.folder, info.file_name);
                if (File.Exists(storage_path))
                {
                    System.IO.File.WriteAllText(storage_path, string.Empty);
                }
                // Create json response
                JObject json_response = new JObject
                {      
                    { "file_id", info.file_id },
                    { "update_id", session_id },
                    { "massager", $"File ID {info.file_id} information updated successfully." }
                };
                SendResponseAsync(session, response.MakeResponse((int)HttpStatusCode.Created, json_response.ToString(), "application/json"));
            }
            else if (endpoint == session_id.ToString())
            {
                MultipartFormDataParser parser = MultipartFormDataParser.Parse(new MemoryStream(request.BodyBytes));
                Stream file_data = parser.Files.First().Data;
                string file_name = parser.Files.First().FileName;
                string folder_name = parser.GetParameterValue("folder");    // Retrieve form data
                string storage_path = Path.Combine(LocalDatabase.Instance.GetStorageDirectory(), user_name, folder_name, file_name);

                // Process the file (update it)
                using (var fileStream = new FileStream(storage_path, FileMode.Append, FileAccess.Write))
                {
                    file_data.CopyTo(fileStream);
                }
                SendResponseAsync(session, response.MakeOkResponse($"File part has been updated {file_data.Length} bytes."));

                // Move the cursor to the beginning of the line
                Console.SetCursorPosition(0, Console.CursorTop);
                // Write the message, overwriting the previous one
                Console.Write($"[{user_name}]: Currently updating file {file_name} with the next part. Please wait... ");
            }
            else if (endpoint.ToLower() == "complete")
            {
                Console.WriteLine("\n");
                Console.WriteLine(request);
                SendResponseAsync(session, response.MakeOkResponse("File update completed successfully."));
            }
            else
            {
                SendResponseAsync(session, response.MakeErrorResponse((int)HttpStatusCode.InternalServerError, $"Required value was not found for the key: {endpoint}"));
                return false;
            }
            return true;
        }

        #region Send response / Send response body
        static private long SendResponse(Object session, Response response)
        {
            if (session is HttpSession http)
            {
                return http.SendResponse(response);
            }
            else if (session is HttpsSession https)
            {
                return https.SendResponse(response);
            }
            else
            {
                return 0;
            }
        }
        static private long SendResponseBody(Object session, string body)
        {
            if (session is HttpSession http)
            {
                return http.SendResponseBody(body);
            }
            else if (session is HttpsSession https)
            {
                return https.SendResponseBody(body);
            }
            else
            {
                return 0;
            }
        }
        static private long SendResponseBody(Object session, byte[] buffer)
        {
            if (session is HttpSession http)
            {
                return http.SendResponseBody(buffer);
            }
            else if (session is HttpsSession https)
            {
                return https.SendResponseBody(buffer);
            }
            else
            {
                return 0;
            }
        }
        static private long SendResponseBody(Object session, byte[] buffer, long offset, long size)
        {
            if (session is HttpSession http)
            {
                return http.SendResponseBody(buffer, offset, size);
            }
            else if (session is HttpsSession https)
            {
                return https.SendResponseBody(buffer, offset, size);
            }
            else
            {
                return 0;
            }
        }
        static private bool SendResponseAsync(Object session, Response response)
        {
            if (session is HttpSession http)
            {
                return http.SendResponseAsync(response);
            }
            else if (session is HttpsSession https)
            {
                return https.SendResponseAsync(response);
            }
            else
            {
                return false;
            }
        }
        static private bool SendResponseBodyAsync(Object session, string body)
        {
            if (session is HttpSession http)
            {
                return http.SendResponseBodyAsync(body);
            }
            else if (session is HttpsSession https)
            {
                return https.SendResponseBodyAsync(body);
            }
            else
            {
                return false;
            }
        }
        static private bool SendResponseBodyAsync(Object session, byte[] buffer)
        {
            if (session is HttpSession http)
            {
                return http.SendResponseBodyAsync(buffer);
            }
            else if (session is HttpsSession https)
            {
                return https.SendResponseBodyAsync(buffer);
            }
            else
            {
                return false;
            }
        }
        static private bool SendResponseBodyAsync(Object session, byte[] buffer, long offset, long size)
        {
            if (session is HttpSession http)
            {
                return http.SendResponseBodyAsync(buffer, offset, size);
            }
            else if (session is HttpsSession https)
            {
                return https.SendResponseBodyAsync(buffer, offset, size);
            }
            else
            {
                return false;
            }
        }
        #endregion

    }
}
