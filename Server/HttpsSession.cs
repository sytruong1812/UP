using System;
using System.Net;
using System.Net.Sockets;


namespace CloudServer
{
    public class HttpsSession : SslSession
    {
        public HttpsSession(HttpsServer server) : base(server)
        {
            cache = server.Cache;
            request = new Request();
            response = new Response();
        }

        /// <summary>
        /// Get the static content cache
        /// </summary>
        public FileCache cache { get; }

        /// <summary>
        /// Get the HTTP request
        /// </summary>
        public Request request { get; }

        /// <summary>
        /// Get the HTTP response
        /// </summary>
        public Response response { get; }

        #region Session handlers

        /// <summary>
        /// Base on SslSession
        /// </summary>
        protected override void OnConnected() => Console.WriteLine($"\nHTTPS session with Id {Id} connected!\n");

        /// <summary>
        /// Base on SslSession
        /// </summary>
        protected override void OnHandshaked() => Console.WriteLine($"\nHTTPS session with Id {Id} handshaked!\n");

        /// <summary>
        /// Base on SslSession
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="offset"></param>
        /// <param name="size"></param>
        protected override void OnReceived(byte[] buffer, long offset, long size)
        {
            // Receive HTTP request header
            if (request.IsPendingHeader())
            {
                if (request.ReceiveHeader(buffer, (int)offset, (int)size))
                {
                    OnReceivedRequestHeader(request);
                }
                size = 0;
            }
            // Check for HTTP request error
            if (request.IsErrorSet)
            {
                OnReceivedRequestError(request, "Invalid HTTPS request!");
                request.Clear();
                Disconnect();
                return;
            }
            // Receive HTTP request body
            if (request.ReceiveBody(buffer, (int)offset, (int)size))
            {
                OnReceivedCachedRequest(request);

                // Process the request!
                OnReceivedRequest(request);

                request.Clear();
                return;
            }
            // Check for HTTP request error
            if (request.IsErrorSet)
            {
                OnReceivedRequestError(request, "Invalid HTTPS request!");
                request.Clear();
                Disconnect();
                return;
            }
        }
        /// <summary>
        /// Base on SslSession
        /// </summary>
        protected override void OnDisconnected()
        {
            // Receive HTTP request body
            Console.WriteLine($"\nHTTPS session with Id {Id} disconnected!\n");
            if (request.IsPendingBody())
            {
                OnReceivedCachedRequest(request);
                request.Clear();
                return;
            }
        }

        /// <summary>
        /// base on SslSession
        /// </summary>
        /// <param name="error"></param>
        protected override void OnError(SocketError error) => Console.WriteLine($"HTTPS session caught an error: {error}");

        /// <summary>
        /// Handle HTTP cached request received notification
        /// </summary>
        /// <remarks>
        /// Notification is called when HTTP request was received
        /// from the client and the corresponding cached content
        /// was found.
        ///
        /// Default behavior is just send cached response content
        /// to the client.
        /// </remarks>
        /// <param name="request">HTTP request</param>
        /// <param name="content">Cached response content</param>
        private void OnReceivedCachedRequest(Request request)
        {
            // Try to get the cached response
            if (request.Method == "GET")
            {
                var index = request.Url.IndexOf('?');
                var response = cache.Find((index < 0) ? request.Url : request.Url.Substring(0, index));
                if (response.Item1)
                {
                    // Process the request with the cached response
                    SendAsync(response.Item2);
                    return;
                }
            }
        }

        /// <summary>
        /// Handle HTTP request error notification
        /// </summary>
        /// <remarks>Notification is called when HTTP request error was received from the client.</remarks>
        /// <param name="request">HTTP request</param>
        /// <param name="error">HTTP request error</param>
        private void OnReceivedRequestError(Request request, string error) => Console.WriteLine($"Request error: {error}");

        /// <summary>
        /// Handle HTTP request header received notification
        /// </summary>
        /// <remarks>Notification is called when HTTP request header was received from the client.</remarks>
        /// <param name="request">HTTP request</param>
        private void OnReceivedRequestHeader(Request request) => Console.WriteLine($"Request header: {request.Headers}");

        /// <summary>
        /// Handle HTTP request received notification
        /// </summary>
        /// <remarks>Notification is called when HTTP request was received from the client.</remarks>
        /// <param name="request">HTTP request</param>
        private void OnReceivedRequest(Request request)
        {
            switch (request.Method)
            {
                case "HEAD":
                    Console.WriteLine(request);
                    SendResponse(response.MakeHeadResponse());
                    break;
                case "GET":
                    OnGetMethod(this.request);
                    break;
                case "POST":
                    OnPostMethod(request);
                    break;
                case "PUT":
                    OnPutMethod(request);
                    break;
                case "DELETE":
                    OnDeleteMethod(request);
                    break;
                case "OPTIONS":
                    Console.WriteLine(request);
                    SendResponse(response.MakeOptionsResponse());
                    break;
                case "TRACE":
                    Console.WriteLine(request);
                    SendResponse(response.MakeTraceResponse($"Hello client[{Id}]"));
                    break;
                default:
                    Console.WriteLine(request);
                    SendResponse(response.MakeErrorResponse($"Unsupported HTTP method: {request.Method}"));
                    break;
            }
        }

        #endregion

        #region Request handlers
        private async void OnGetMethod(Request request)
        {
            // Decode the URL and remove leading/trailing 
            string url_path = Uri.UnescapeDataString(request.Url.Trim('/'));
            if (string.IsNullOrEmpty(url_path))
            {
                SendResponseAsync(response.MakeErrorResponse((int)HttpStatusCode.BadRequest, $"Required value was not found for the key: {request.Url}"));
                return;
            }

            /* GET /account/action      : /teddy/profile */
            /* GET /account/action      : /teddy/download/file_id */
            string action, user_name;
            string[] segments = url_path.Split('/');
            {
                user_name = segments[0];
                action = segments[1];
            }

            switch (action.ToLower())
            {
                case "profile":
                    Console.WriteLine("\n=================[ Get profile ]================");
                    Console.WriteLine(request);
                    await UserOperations.ProcessGetProfile(this, request, response, user_name);
                    break;
                default:
                    Console.WriteLine(request);
                    SendResponseAsync(response.MakeErrorResponse((int)HttpStatusCode.BadRequest, $"Required value was not found for the key: {url_path}"));
                    break;
            }
        }
        private async void OnPostMethod(Request request)
        {
            // Decode the URL and remove leading/trailing 
            string url_path = Uri.UnescapeDataString(request.Url.Trim('/'));
            if (string.IsNullOrEmpty(url_path))
            {
                SendResponseAsync(response.MakeErrorResponse((int)HttpStatusCode.BadRequest, $"Required value was not found for the key: {request.Url}"));
                return;
            }

            /* POST /action                          : /register or /login */
            /* POST /user_name/files/action          : /teddy/files/upload */
            /* POST /user_name/files/action/endpoint : /teddy/files/upload/init */
            string action, account;
            string[] segments = url_path.Split('/');
            {
                if (segments.Length == 1)
                {
                    account = null;
                    action = segments[0];
                }
                else
                {
                    account = segments[0];
                    action = segments[1];
                }
            }

            switch (action.ToLower())
            {
                case "register":
                    {
                        Console.WriteLine("\n=================[ Register ]===================");
                        Console.WriteLine(request);
                        await UserOperations.ProcessRegister(this, Id, request, response);

                    }
                    break;
                case "login":
                    {
                        Console.WriteLine("\n===================[ Login ]====================");
                        Console.WriteLine(request);
                        await UserOperations.ProcessLogin(this, Id, request, response);
                    }
                    break;
                case "logout":
                    {
                        Console.WriteLine("\n===================[ Logout ]===================");
                        Console.WriteLine(request);
                        await UserOperations.ProcessLogout(this, Id, request, response, account);
                    }
                    break;
                case "files":
                    {
                        if (!string.IsNullOrEmpty(segments[2]))
                        {
                            string file_action = segments[2].ToLower();
                            if (file_action == "upload")
                            {
                                if (segments.Length > 3)
                                {
                                    string endpoint = segments[3];
                                    await UserOperations.ProcessUploadFileLarge(this, Id, request, response, account, endpoint);
                                }
                                else
                                {
                                    Console.WriteLine("\n=================[ Upload file ]================");
                                    Console.WriteLine(request);
                                    await UserOperations.ProcessUploadFile(this, Id, request, response, account);
                                }
                            }
                        }
                    }
                    break;
                default:
                    Console.WriteLine(request);
                    SendResponseAsync(response.MakeErrorResponse((int)HttpStatusCode.NotFound, $"Required value was not found for the key: {url_path}"));
                    break;
            }
        }
        private async void OnPutMethod(Request request)
        {
            // Decode the URL and remove leading/trailing 
            string url_path = Uri.UnescapeDataString(request.Url.Trim('/'));
            if (string.IsNullOrEmpty(url_path))
            {
                SendResponseAsync(response.MakeErrorResponse((int)HttpStatusCode.BadRequest, $"Required value was not found for the key: {request.Url}"));
                return;
            }

            /* PUT /user_name/action                : /teddy/password */
            /* PUT /user_name/file/action/endpoint  : /teddy/files/upload/init */
            string account, action;
            string[] segments = url_path.Split('/');
            {
                account = segments[0];
                action = segments[1];
            }

            switch (action.ToLower())
            {
                case "password":
                    {
                        Console.WriteLine("\n================[ Change password ]=============");
                        Console.WriteLine(request);
                        await UserOperations.ProcessChangePassword(this, request, response, account);
                    }
                    break;
                case "profile":
                    {
                        Console.WriteLine("\n================[ Update profile ]==============");
                        Console.WriteLine(request);
                        await UserOperations.ProcessUpdateProfile(this, request, response, account);
                    }
                    break;
                case "files":
                    {
                        if (!string.IsNullOrEmpty(segments[2]))
                        {
                            string file_action = segments[2].ToLower();
                            if (file_action == "rename")
                            {
                                Console.WriteLine("\n=================[ Rename file ]================");
                                Console.WriteLine(request);
                                int file_id = Convert.ToInt32(segments[3]);
                                await UserOperations.ProcessRenameFile(this, request, response, account, file_id);
                            }
                            else if (file_action == "update")
                            {
                                if (segments.Length > 3)
                                {

                                    string endpoint = segments[3];
                                    await UserOperations.ProcessUpdateFileLarge(this, Id, request, response, account, endpoint);
                                }
                                else
                                {
                                    Console.WriteLine("\n=================[ Update file ]================");
                                    Console.WriteLine(request);
                                    await UserOperations.ProcessUpdateFile(this, Id, request, response, account);
                                }
                            }
                        }
                    }
                    break;
                default:
                    {
                        Console.WriteLine(request);
                        SendResponseAsync(response.MakeErrorResponse((int)HttpStatusCode.BadRequest, $"Required value was not found for the key: {url_path}"));
                    }
                    break;
            }
        }
        private async void OnDeleteMethod(Request request)
        {
            // Decode the URL and remove leading/trailing 
            string url_path = Uri.UnescapeDataString(request.Url.Trim('/'));
            if (string.IsNullOrEmpty(url_path))
            {
                SendResponseAsync(response.MakeErrorResponse((int)HttpStatusCode.BadRequest, $"Required value was not found for the key: {request.Url}"));
                return;
            }

            /* DELETE /user_name         :  /teddy */
            /* DELETE /user_name/file_id :  /teddy/123 */
            string account, action;
            string[] segments = url_path.Split('/');

            if (segments.Length == 1)
            {
                account = segments[0].ToLower();
                Console.WriteLine("\n=================[ Delete account ]=============");
                Console.WriteLine(request);
                await UserOperations.ProcessDeleteAccount(this, request, response, account);
            }
            else
            {
                account = segments[0].ToLower();
                action = segments[1].ToLower();
                if (action == "files")
                {
                    Console.WriteLine("\n=================[ Remove file ]================");
                    Console.WriteLine(request);
                    int file_id = Convert.ToInt32(segments[2]);
                    await UserOperations.ProcessRemoveFile(this, request, response, account, file_id);
                }
            }
        }

        #endregion

        #region Send response / Send response body

        /// <summary>
        /// Send the current HTTP response (synchronous)
        /// </summary>
        /// <returns>Size of sent data</returns>
        public long SendResponse() => SendResponse(response);
        /// <summary>
        /// Send the HTTP response (synchronous)
        /// </summary>
        /// <param name="response">HTTP response</param>
        /// <returns>Size of sent data</returns>
        public long SendResponse(Response response) => Send(response.Cache.Data, response.Cache.Offset, response.Cache.Size);
        /// <summary>
        /// Send the HTTP response body (synchronous)
        /// </summary>
        /// <param name="body">HTTP response body</param>
        /// <returns>Size of sent data</returns>
        public long SendResponseBody(string body) => Send(body);
        /// <summary>
        /// Send the HTTP response body (synchronous)
        /// </summary>
        /// <param name="buffer">HTTP response body buffer</param>
        /// <returns>Size of sent data</returns>
        public long SendResponseBody(byte[] buffer) => Send(buffer);
        /// <summary>
        /// Send the HTTP response body (synchronous)
        /// </summary>
        /// <param name="buffer">HTTP response body buffer</param>
        /// <param name="offset">HTTP response body buffer offset</param>
        /// <param name="size">HTTP response body size</param>
        /// <returns>Size of sent data</returns>
        public long SendResponseBody(byte[] buffer, long offset, long size) => Send(buffer, offset, size);
        /// <summary>
        /// Send the current HTTP response (asynchronous)
        /// </summary>
        /// <returns>'true' if the current HTTP response was successfully sent, 'false' if the session is not connected</returns>
        public bool SendResponseAsync() => SendResponseAsync(response);
        /// <summary>
        /// Send the HTTP response (asynchronous)
        /// </summary>
        /// <param name="response">HTTP response</param>
        /// <returns>'true' if the current HTTP response was successfully sent, 'false' if the session is not connected</returns>
        public bool SendResponseAsync(Response response) => SendAsync(response.Cache.Data, response.Cache.Offset, response.Cache.Size);
        /// <summary>
        /// Send the HTTP response body (asynchronous)
        /// </summary>
        /// <param name="body">HTTP response body</param>
        /// <returns>'true' if the HTTP response body was successfully sent, 'false' if the session is not connected</returns>
        public bool SendResponseBodyAsync(string body) => SendAsync(body);
        /// <summary>
        /// Send the HTTP response body (asynchronous)
        /// </summary>
        /// <param name="buffer">HTTP response body buffer</param>
        /// <returns>'true' if the HTTP response body was successfully sent, 'false' if the session is not connected</returns>
        public bool SendResponseBodyAsync(byte[] buffer) => SendAsync(buffer);
        /// <summary>
        /// Send the HTTP response body (asynchronous)
        /// </summary>
        /// <param name="buffer">HTTP response body buffer</param>
        /// <param name="offset">HTTP response body buffer offset</param>
        /// <param name="size">HTTP response body size</param>
        /// <returns>'true' if the HTTP response body was successfully sent, 'false' if the session is not connected</returns>
        public bool SendResponseBodyAsync(byte[] buffer, long offset, long size) => SendAsync(buffer, offset, size);

        #endregion

    }
}
