using System;
using System.IO;
using System.Data;
using System.Text;
using System.Data.SqlClient;
using System.Threading.Tasks;
using System.Collections.Generic;

namespace CloudServer
{
    public class UserInfo
    {
        public int? user_id;
        public string first_name;
        public string last_name;
        public string user_name;
        public string password;
        public string email;
        public DateTime? birthday;
        public DateTime? create_at;
    }
    public class FileInfo
    {
        public int? file_id;
        public int? user_id;
        public string file_name;
        public long? file_size;
        public string folder;
        public int? attribute;
        public DateTime? create_time;
        public DateTime? last_write_time;
        public DateTime? last_access_time;
        public string storage_on;
        public DateTime? create_at;
    }

    sealed class LocalDatabase
    {
        private static readonly Lazy<LocalDatabase> lazy = new Lazy<LocalDatabase>(() => new LocalDatabase());
        public static LocalDatabase Instance { get { return lazy.Value; } }
        private LocalDatabase() { }

        private static string _storageDirectory = Path.Combine(Directory.GetCurrentDirectory(), "local");
        public string GetStorageDirectory()
        {
            return _storageDirectory;
        }
        public void SetStorageDirectory(string path)
        {
            _storageDirectory = path;
        }
    }

    sealed class SqlDatabase
    {
        private static SqlConnection _con;

        private static readonly Lazy<SqlDatabase> lazy = new Lazy<SqlDatabase>(() => new SqlDatabase());
        public static SqlDatabase Instance { get { return lazy.Value; } }
        private SqlDatabase() { }

        public void OpenDatabase(string data_source, string path_db)
        {
            _con = new SqlConnection($"Data Source={data_source};AttachDbFilename={path_db};Integrated Security=True");
            _con.Open();
        }
        public async Task OpenDatabaseAsync(string data_source, string path_db)
        {
            _con = new SqlConnection($"Data Source={data_source};AttachDbFilename={path_db};Integrated Security=True");
            await _con.OpenAsync();
        }
        public int ExecuteNonQuery(string query)
        {
            int nRow = 0;
            using (SqlCommand _cmd = new SqlCommand(query, _con))
            {
                nRow = _cmd.ExecuteNonQuery();
            }
            return nRow;
        }
        public async Task<int> ExecuteNonQueryAsync(string query)
        {
            int nRow = 0;
            using (SqlCommand _cmd = new SqlCommand(query, _con))
            {
                nRow = await _cmd.ExecuteNonQueryAsync();
            }
            return nRow;
        }
        public void CloseDatabase()
        {
            if (_con != null && _con.State == ConnectionState.Open)
            {
                _con.Close();
            }
        }
        public async Task<bool> CreateUserInfoTable()
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return false;
            }

            string queryUserInfo = @"CREATE TABLE UserInfo 
            (
                UserId INT PRIMARY KEY IDENTITY(1,1),
                FirstName NVARCHAR(100) NOT NULL,
                LastName NVARCHAR(100) NOT NULL,
                UserName NVARCHAR(100) NOT NULL UNIQUE,
                Password NVARCHAR(255) NOT NULL,
                Email NVARCHAR(255) NOT NULL UNIQUE,
                Birthday DATETIME NOT NULL,
                CreatedAt DATETIME DEFAULT GETDATE()
            );";

            try
            {
                using (SqlCommand command = new SqlCommand(queryUserInfo, _con))
                {
                    await command.ExecuteNonQueryAsync();
                }
                return true; // Tables created successfully
            }
            catch (Exception ex)
            {
                Console.WriteLine($"An error occurred: {ex.Message}");
                return false;
            }
        }
        public async Task<bool> CreateFileInfoTable()
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return false;
            }

            string queryFileInfo = @"CREATE TABLE FileInfo 
            (
                FileId INT PRIMARY KEY IDENTITY(1,1),
                UserId INT NOT NULL,
                FilePath NVARCHAR(255) NOT NULL,
                FileName NVARCHAR(255) NOT NULL,
                FileSize BIGINT NOT NULL,
                Attribute INT NOT NULL,
                CreateTime DATETIME DEFAULT GETDATE(),
                LastWriteTime DATETIME DEFAULT GETDATE(),
                LastAccessTime DATETIME DEFAULT GETDATE(),
                StorageOn NVARCHAR(100) NOT NULL,
                CreatedAt DATETIME DEFAULT GETDATE(),
                FOREIGN KEY (UserId) REFERENCES UserInfo(UserId)
            );";

            try
            {
                using (SqlCommand command = new SqlCommand(queryFileInfo, _con))
                {
                    await command.ExecuteNonQueryAsync();
                }
                return true; // Tables created successfully
            }
            catch (Exception ex)
            {
                Console.WriteLine($"An error occurred: {ex.Message}");
                return false;
            }
        }

        public async Task<bool> UserExistAsync(int user_id)
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return false;
            }
            string query = $"SELECT COUNT(1) FROM [UserInfo] WHERE [user_id] = @userId";
            using (var _cmd = new SqlCommand(query, _con))
            {
                // Add parameters to the command
                _cmd.Parameters.AddWithValue("@userId", user_id);

                // Execute the command and retrieve the count
                int count = (int)(await _cmd.ExecuteScalarAsync());
                return count > 0;
            }
        }
        public async Task<bool> AccountExistAsync(UserInfo user_info)
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return false;
            }
            // Corrected SQL query with AND instead of commas
            string query = "SELECT COUNT(1) FROM [UserInfo] " +
                           "WHERE first_name = @firstName AND last_name = @lastName " +
                           "AND password = @password AND email = @email " +
                           "AND birthday = @birthday";

            using (var _cmd = new SqlCommand(query, _con))
            {
                // Add parameters to the command
                _cmd.Parameters.AddWithValue("@firstName", user_info.first_name);
                _cmd.Parameters.AddWithValue("@lastName", user_info.last_name);
                _cmd.Parameters.AddWithValue("@password", user_info.password);
                _cmd.Parameters.AddWithValue("@email", user_info.email);
                _cmd.Parameters.AddWithValue("@birthday", user_info.birthday);

                // Execute the command and retrieve the count
                int count = (int)(await _cmd.ExecuteScalarAsync());
                return count > 0; // Return true if count is greater than 0
            }
        }
        public async Task<UserInfo> RegisterAccountAsync(UserInfo info)
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return null;
            }
            // Prepare the insert statement with OUTPUT clause
            string query = $"INSERT INTO [UserInfo] (first_name, last_name, user_name, password, email, birthday) " +
                           $"OUTPUT INSERTED.user_id, INSERTED.create_at " +
                           $"VALUES (@firstName, @lastName, @userName, @password, @email, @birthday)";
            // Start a transaction
            using (var transaction = _con.BeginTransaction())
            {
                try
                {
                    // Execute the query using a database connection
                    using (var _cmd = new SqlCommand(query, _con, transaction))
                    {
                        // Add parameters for the key value and create_at timestamp
                        _cmd.Parameters.AddWithValue("@firstName", info.first_name);
                        _cmd.Parameters.AddWithValue("@lastName", info.last_name);
                        _cmd.Parameters.AddWithValue("@userName", info.user_name);
                        _cmd.Parameters.AddWithValue("@password", info.password);
                        _cmd.Parameters.AddWithValue("@email", info.email);
                        _cmd.Parameters.AddWithValue("@birthday", info.birthday);

                        // Execute the command and get the user_id and create_at
                        using (var reader = await _cmd.ExecuteReaderAsync())
                        {
                            if (await reader.ReadAsync())
                            {
                                info.user_id = reader.GetInt32(0);
                                info.create_at = reader.GetDateTime(1);
                            }
                        }
                    }
                    transaction.Commit();
                }
                catch (Exception ex)
                {
                    // Rollback the transaction in case of an error
                    transaction.Rollback();
                    Console.WriteLine($"An error occurred: {ex.Message}");
                    return null;
                }
            }
            return info;
        }
        public async Task<UserInfo> GetUserProfileAsync(int user_id)
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return null;
            }

            string query = "SELECT * FROM [UserInfo] WHERE [user_id] = @userId";
            using (var _cmd = new SqlCommand(query, _con))
            {
                // Add parameters to the command with explicit type
                _cmd.Parameters.Add("@userId", SqlDbType.NVarChar).Value = user_id;

                // Execute the command and read the data
                using (var reader = await _cmd.ExecuteReaderAsync())
                {
                    if (await reader.ReadAsync()) // Check if any record was returned
                    {
                        // Create a new UserInfo object and populate it with data from the database
                        var info = new UserInfo
                        {
                            user_id = reader.GetInt32(reader.GetOrdinal("user_id")),
                            first_name = reader.GetString(reader.GetOrdinal("first_name")),
                            last_name = reader.GetString(reader.GetOrdinal("last_name")),
                            user_name = reader.GetString(reader.GetOrdinal("user_name")),
                            password = reader.GetString(reader.GetOrdinal("password")),
                            email = reader.GetString(reader.GetOrdinal("email")),
                            birthday = reader.GetDateTime(reader.GetOrdinal("birthday")),
                            create_at = reader.GetDateTime(reader.GetOrdinal("create_at"))
                        };

                        return info; // Return the populated UserInfo object
                    }
                }
            }
            return null; // User profile not found
        }
        public async Task<int?> LoginAccountAsync(string user_name, string password)
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return null;
            }
            // Prepare the SQL query using parameterized queries to prevent SQL injection
            string query = $"SELECT user_id FROM [UserInfo] WHERE user_name = @username AND password = @password";
            using (var _cmd = new SqlCommand(query, _con))
            {
                // Add parameters to the command
                _cmd.Parameters.AddWithValue("@username", user_name);
                _cmd.Parameters.AddWithValue("@password", password);

                // Execute the command and retrieve the count
                var result = await _cmd.ExecuteScalarAsync();
                if (result == null)
                {
                    // User not found
                    return null;
                }
                return (int)result;
            }
        }
        public async Task<bool> DeleteAccountAsync(int user_id)
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return false;
            }
            // Start a transaction
            using (var transaction = _con.BeginTransaction())
            {
                try
                {
                    // Step 1: Retrieve the user_id
                    int userId;
                    string getUserIdQuery = "SELECT user_id FROM [UserInfo] WHERE user_id = @userId";
                    using (var getUserIdCmd = new SqlCommand(getUserIdQuery, _con, transaction))
                    {
                        getUserIdCmd.Parameters.AddWithValue("@userId", user_id);

                        var result = await getUserIdCmd.ExecuteScalarAsync();
                        if (result == null)
                        {
                            // User not found
                            return false;
                        }
                        userId = (int)result;
                    }

                    // Step 2: Delete related records in FileInfo
                    string deleteFileInfoQuery = "DELETE FROM [FileInfo] WHERE user_id = @userId";
                    using (var deleteFileInfoCmd = new SqlCommand(deleteFileInfoQuery, _con, transaction))
                    {
                        deleteFileInfoCmd.Parameters.AddWithValue("@userId", userId);
                        await deleteFileInfoCmd.ExecuteNonQueryAsync();
                    }

                    // Step 3: Delete the user from UserInfo
                    string deleteUserQuery = "DELETE FROM [UserInfo] WHERE user_id = @userId";
                    using (var deleteUserCmd = new SqlCommand(deleteUserQuery, _con, transaction))
                    {
                        deleteUserCmd.Parameters.AddWithValue("@userId", userId);
                        int rowsAffected = await deleteUserCmd.ExecuteNonQueryAsync();

                        // Commit the transaction if the user was deleted
                        if (rowsAffected > 0)
                        {
                            transaction.Commit();
                            return true; // User deleted successfully
                        }
                    }
                }
                catch (Exception ex)
                {
                    // Rollback the transaction in case of an error
                    transaction.Rollback();
                    Console.WriteLine($"An error occurred: {ex.Message}");
                }
            }
            return false;
        }
        public async Task<bool> UpdateProfileAsync(int user_id, UserInfo info)
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return false;
            }

            // Start building the SQL query
            var query = new StringBuilder("UPDATE [UserInfo] SET ");
            var parameters = new List<string>();

            // Check each field and add to the query if it's not null or empty
            if (!string.IsNullOrEmpty(info.first_name))
            {
                parameters.Add("first_name = @firstName");
            }
            if (!string.IsNullOrEmpty(info.last_name))
            {
                parameters.Add("last_name = @lastName");
            }
            if (!string.IsNullOrEmpty(info.user_name))
            {
                parameters.Add("user_name = @userName");
            }
            if (!string.IsNullOrEmpty(info.password))
            {
                parameters.Add("password = @password");
            }
            if (!string.IsNullOrEmpty(info.email))
            {
                parameters.Add("email = @email");
            }
            if (info.birthday.HasValue)
            {
                parameters.Add("birthday = @birthday");
            }

            // If no fields are provided, return false
            if (parameters.Count == 0)
            {
                return false; // No updates to perform
            }

            // Join the parameters to form the complete query
            query.Append(string.Join(", ", parameters));
            query.Append(" WHERE user_id = @userId");

            // Start a transaction
            using (var transaction = _con.BeginTransaction())
            {
                // Execute the update command
                using (var _cmd = new SqlCommand(query.ToString(), _con, transaction))
                {
                    _cmd.Parameters.AddWithValue("@userId", user_id);
                    // Add parameters for the update
                    if (parameters.Contains("first_name = @firstName"))
                        _cmd.Parameters.AddWithValue("@firstName", info.first_name);
                    if (parameters.Contains("last_name = @lastName"))
                        _cmd.Parameters.AddWithValue("@lastName", info.last_name);
                    if (parameters.Contains("user_name = @userName"))
                        _cmd.Parameters.AddWithValue("@userName", info.user_name);
                    if (parameters.Contains("password = @password"))
                        _cmd.Parameters.AddWithValue("@password", info.password);
                    if (parameters.Contains("email = @email"))
                        _cmd.Parameters.AddWithValue("@email", info.email);
                    if (parameters.Contains("birthday = @birthday"))
                        _cmd.Parameters.AddWithValue("@birthday", info.birthday);

                    int rowsAffected = await _cmd.ExecuteNonQueryAsync();
                    if (rowsAffected > 0)
                    {
                        transaction.Commit();
                        return true;
                    }
                    else
                    {
                        transaction.Rollback();
                        return false;
                    }
                }
            }
        }
        public async Task<bool> ChangePasswordAsync(int user_id, string old_password, string new_password)
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return false;
            }
            // Start a transaction
            using (var transaction = _con.BeginTransaction())
            {
                try
                {
                    // Step 1: Check if the old password matches
                    string checkPasswordQuery = "SELECT COUNT(*) FROM [UserInfo] WHERE user_id = @userId AND password = @oldPassword";
                    using (var checkCmd = new SqlCommand(checkPasswordQuery, _con, transaction))
                    {
                        checkCmd.Parameters.AddWithValue("@userId", user_id);
                        checkCmd.Parameters.AddWithValue("@oldPassword", old_password);

                        int count = (int)(await checkCmd.ExecuteScalarAsync());
                        if (count == 0)
                        {
                            // Old password does not match
                            transaction.Rollback();
                            return false; // Indicate failure
                        }
                    }

                    // Step 2: Update the password
                    string updateQuery = "UPDATE [UserInfo] SET password = @newPassword WHERE user_id = @userId";
                    using (var updateCmd = new SqlCommand(updateQuery, _con, transaction))
                    {
                        updateCmd.Parameters.AddWithValue("@userId", user_id);
                        updateCmd.Parameters.AddWithValue("@newPassword", new_password);

                        int rowsAffected = await updateCmd.ExecuteNonQueryAsync();
                        if (rowsAffected > 0)
                        {
                            transaction.Commit();
                            return true; // Password changed successfully
                        }
                        else
                        {
                            transaction.Rollback();
                            return false; // Indicate failure
                        }
                    }
                }
                catch (Exception ex)
                {
                    // Rollback the transaction in case of an error
                    transaction.Rollback();
                    Console.WriteLine($"An error occurred: {ex.Message}");
                    return false; // Indicate failure
                }
            }
        }
        public async Task<FileInfo> UploadFileInfoAsync(int user_id, FileInfo file)
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return null;
            }
            // Prepare the SQL query with a parameter for user_name
            string query = @"INSERT INTO [FileInfo] (user_id, file_name, file_size, folder, attribute, create_time, last_write_time, last_access_time, storage_on) 
                            OUTPUT INSERTED.file_id, INSERTED.create_at 
                            VALUES (@userId, @fileName, @fileSize, @folderName, @attribute, @createTime, @lastWriteTime, @lastAccessTime, @storageOn)";
            // Start a transaction
            using (var transaction = _con.BeginTransaction())
            {
                try
                {
                    // Execute the query using a database connection
                    using (var _cmd = new SqlCommand(query, _con, transaction))
                    {
                        // Add parameters for the insert
                        _cmd.Parameters.AddWithValue("@userId", user_id);
                        _cmd.Parameters.AddWithValue("@fileName", file.file_name);
                        _cmd.Parameters.AddWithValue("@fileSize", SqlDbType.BigInt).Value = file.file_size;
                        _cmd.Parameters.AddWithValue("@folderName", file.folder);
                        _cmd.Parameters.AddWithValue("@attribute", file.attribute);
                        _cmd.Parameters.AddWithValue("@createTime", file.create_time);
                        _cmd.Parameters.AddWithValue("@lastWriteTime", file.last_write_time);
                        _cmd.Parameters.AddWithValue("@lastAccessTime", file.last_access_time);
                        _cmd.Parameters.AddWithValue("@storageOn", file.storage_on);

                        // Execute the command and get the user_id and create_at
                        using (var reader = await _cmd.ExecuteReaderAsync())
                        {
                            if (await reader.ReadAsync())
                            {
                                file.file_id = reader.GetInt32(0);
                                file.create_at = reader.GetDateTime(1);
                            }
                        }
                    }
                    transaction.Commit();
                    return file;
                }
                catch (Exception ex)
                {
                    // Rollback the transaction in case of an error
                    transaction.Rollback();
                    Console.WriteLine($"An error occurred: {ex.Message}");
                }
            }
            return null;
        }
        public async Task<bool> UpdateFileInfoAsync(int user_id, FileInfo file)
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return false;
            }
            // Start a transaction
            using (var transaction = _con.BeginTransaction())
            {
                string updateQuery = "UPDATE [FileInfo] " +
                "SET file_name = @fileName, file_size = @fileSize, folder = @folderName, attribute = @attribute, create_time = @createTime, last_write_time = @lastWriteTime, last_access_time = @lastAccessTime, create_at = @createAt " +
                "WHERE user_id = @userId AND file_id = @fileId";

                // Execute the update command
                using (var updateCmd = new SqlCommand(updateQuery, _con, transaction))
                {
                    // Add parameters for the update
                    updateCmd.Parameters.AddWithValue("@userId", user_id);
                    updateCmd.Parameters.AddWithValue("@fileId", file.file_id);
                    updateCmd.Parameters.AddWithValue("@fileName", file.file_name);
                    updateCmd.Parameters.AddWithValue("@fileSize", file.file_size);
                    updateCmd.Parameters.AddWithValue("@folderName", file.folder);
                    updateCmd.Parameters.AddWithValue("@attribute", file.attribute);
                    updateCmd.Parameters.AddWithValue("@createTime", file.create_time);
                    updateCmd.Parameters.AddWithValue("@lastWriteTime", file.last_write_time);
                    updateCmd.Parameters.AddWithValue("@lastAccessTime", file.last_access_time);
                    updateCmd.Parameters.AddWithValue("@createAt", DateTime.Now);

                    int rowsAffected = await updateCmd.ExecuteNonQueryAsync();
                    if (rowsAffected > 0)
                    {
                        transaction.Commit();
                        return true;
                    }
                    else
                    {
                        transaction.Rollback();
                        return false;
                    }
                }
            }
        }
        public async Task<string> RenameFileAsync(int user_id, int file_id, string new_filename)
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return null;
            }
            // Start a transaction
            using (var transaction = _con.BeginTransaction())
            {
                try
                {
                    // Step 1: Retrieve the storage path
                    string storage_path;
                    string getFileNameQuery = "SELECT storage_on FROM [FileInfo] WHERE file_id = @fileId";
                    using (var getFileNameCmd = new SqlCommand(getFileNameQuery, _con, transaction))
                    {
                        getFileNameCmd.Parameters.AddWithValue("@fileId", file_id);
                        var result = await getFileNameCmd.ExecuteScalarAsync();
                        if (result == null)
                        {
                            return null; // File not found
                        }
                        storage_path = (string)result;
                    }

                    // Step 2: Update the file name in FileInfo
                    string updateFileInfoQuery = "UPDATE [FileInfo] SET file_name = @newFileName WHERE user_id = @userId AND file_id = @fileId";
                    using (var updateFileInfoCmd = new SqlCommand(updateFileInfoQuery, _con, transaction))
                    {
                        updateFileInfoCmd.Parameters.AddWithValue("@userId", user_id);
                        updateFileInfoCmd.Parameters.AddWithValue("@fileId", file_id);
                        updateFileInfoCmd.Parameters.AddWithValue("@newFileName", new_filename);
                        int rowsAffected = await updateFileInfoCmd.ExecuteNonQueryAsync();

                        // Commit the transaction if the file name was updated
                        if (rowsAffected > 0)
                        {
                            transaction.Commit();   // File renamed successfully 
                            return storage_path;
                        }
                    }
                }
                catch (Exception ex)
                {
                    // Rollback the transaction in case of an error
                    transaction.Rollback();
                    Console.WriteLine($"An error occurred: {ex.Message}");
                }
            }
            return null; // File not renamed
        }
        public async Task<string> RemoveFileAsync(int user_id, int file_id)
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return null;
            }
            // Start a transaction
            using (var transaction = _con.BeginTransaction())
            {
                try
                {
                    // Step 1: Retrieve the storage path
                    string storage_path;
                    string getFileNameQuery = "SELECT storage_on FROM [FileInfo] WHERE file_id = @fileId";
                    using (var getFileNameCmd = new SqlCommand(getFileNameQuery, _con, transaction))
                    {
                        getFileNameCmd.Parameters.AddWithValue("@fileId", file_id);
                        var result = await getFileNameCmd.ExecuteScalarAsync();
                        if (result == null)
                        {
                            return null; // File not found
                        }
                        storage_path = (string)result;
                    }

                    // Step 2: Delete related records in FileInfo
                    string deleteFileInfoQuery = "DELETE FROM [FileInfo] WHERE user_id = @userId AND file_id = @fileId ";
                    using (var deleteFileInfoCmd = new SqlCommand(deleteFileInfoQuery, _con, transaction))
                    {
                        deleteFileInfoCmd.Parameters.AddWithValue("@userId", user_id);
                        deleteFileInfoCmd.Parameters.AddWithValue("@fileId", file_id);
                        int rowsAffected = await deleteFileInfoCmd.ExecuteNonQueryAsync();

                        // Commit the transaction if the file was deleted
                        if (rowsAffected > 0)
                        {
                            transaction.Commit();   // File deleted successfully
                            return storage_path;
                        }
                    }
                }
                catch (Exception ex)
                {
                    // Rollback the transaction in case of an error
                    transaction.Rollback();
                    Console.WriteLine($"An error occurred: {ex.Message}");
                }
            }
            return null; // File not deleted
        }
        public async Task<List<FileInfo>> RetrieveUploadAsync(int user_id, List<FileInfo> files)
        {
            if (_con is null)
            {
                Console.WriteLine("Database connection is not available.");
                return new List<FileInfo>(); // Return an empty list instead of null
            }

            var files_miss = new List<FileInfo>();

            // Start a transaction
            using (var transaction = _con.BeginTransaction())
            {
                string retrieveQuery = "SELECT file_name, file_size, folder, attribute, create_time, last_write_time, last_access_time " +
                                       "FROM FileInfo " +
                                       "WHERE " +
                                       "file_name = @fileName AND " +
                                       "file_size = @fileSize AND " +
                                       "folder = @folderName AND " +
                                       "attribute = @attribute AND " +
                                       "create_time = @createTime AND " +
                                       "last_write_time = @lastWriteTime AND " +
                                       "last_access_time = @lastAccessTime";

                try
                {
                    foreach (var file in files)
                    {
                        using (var _cmd = new SqlCommand(retrieveQuery, _con, transaction))
                        {
                            _cmd.Parameters.AddWithValue("@userId", user_id);
                            _cmd.Parameters.AddWithValue("@fileName", file.file_name);
                            _cmd.Parameters.AddWithValue("@fileSize", file.file_size); // Corrected
                            _cmd.Parameters.AddWithValue("@folderName", file.folder);
                            _cmd.Parameters.AddWithValue("@attribute", file.attribute);
                            _cmd.Parameters.AddWithValue("@createTime", file.create_time);
                            _cmd.Parameters.AddWithValue("@lastWriteTime", file.last_write_time);
                            _cmd.Parameters.AddWithValue("@lastAccessTime", file.last_access_time);

                            using (var reader = await _cmd.ExecuteReaderAsync())
                            {
                                if (!await reader.ReadAsync())
                                {
                                    // If no record is found, add the file to the missing files list
                                    files_miss.Add(file);
                                }
                            }
                        }
                    }

                    // Commit the transaction if everything is successful
                    transaction.Commit();
                }
                catch (Exception ex)
                {
                    // Rollback the transaction in case of an error
                    transaction.Rollback();
                    Console.WriteLine($"An error occurred: {ex.Message}");
                }
            }

            return files_miss;
        }


    }
}