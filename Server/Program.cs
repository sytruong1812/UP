using System;
using System.Net;
using System.Threading;
using System.Net.Security;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;
using Newtonsoft.Json.Linq;

namespace CloudServer
{
    class Program
    {
        static void Main(string[] args)
        {
            int port_number = 8080;
            string host_name = string.Empty;
            string protocol = string.Empty;
            string cert_path = string.Empty;
            string cert_key = string.Empty;

            string database_path = string.Empty;
            string storage_path = string.Empty;

            if (args.Length > 0)
            {
                string message = System.IO.File.ReadAllText(args[0]);
                JObject json_config = JObject.Parse(message);
                port_number = (int)json_config.SelectToken("port_number");
                host_name = (string)json_config.SelectToken("host_name");
                protocol = (string)json_config.SelectToken("protocol");
                cert_path = (string)json_config.SelectToken("cert_path");
                cert_key = (string)json_config.SelectToken("cert_key");
                database_path = (string)json_config.SelectToken("database_path");
                storage_path = (string)json_config.SelectToken("storage_path");

                if (string.IsNullOrWhiteSpace(database_path))
                {
                    Console.WriteLine("SqlDatabase path cannot be empty. Please try again.");
                    return;
                }
                SqlDatabase.Instance.OpenDatabase("(LocalDB)\\MSSQLLocalDB", database_path);
                if (string.IsNullOrWhiteSpace(storage_path))
                {
                    Console.WriteLine("Storage path cannot be empty. Please try again.");
                    return;
                }
                LocalDatabase.Instance.SetStorageDirectory(storage_path);

            }
            else
            {
                do
                {
                    // Get host from user input
                    Console.Write("Enter host (eg: localhost): ");
                    host_name = Console.ReadLine();
                    if (string.IsNullOrWhiteSpace(host_name))
                    {
                        Console.WriteLine("Invalid input! Please enter a valid host name.");
                        continue;
                    }
                    // Get port from user input
                    Console.Write("Enter port (eg: 8080, 8443): ");
                    string input = Console.ReadLine();
                    if (!string.IsNullOrWhiteSpace(input))
                    {
                        if (int.TryParse(input, out int port))
                        {
                            port_number = port;
                        }
                        else
                        {
                            Console.WriteLine("Invalid input! Please enter a valid port number.");
                            continue;
                        }
                    }

                    Console.Write("Choose protocol (HTTP, HTTPS): ");
                    protocol = Console.ReadLine().ToLower();
                    if (protocol != "http" && protocol != "https")
                    {
                        Console.WriteLine("Invalid protocol selected. Please choose either 'http' or 'https'.");
                        continue;
                    }

                    if (protocol == "https")
                    {
                        Console.Write("Enter the path to the certificate file: ");
                        cert_path = Console.ReadLine();
                        if (string.IsNullOrWhiteSpace(cert_path))
                        {
                            Console.WriteLine("Certificate path cannot be empty. Please try again.");
                            continue;
                        }

                        Console.Write("Enter the certificate password: ");
                        cert_key = Console.ReadLine();
                        if (string.IsNullOrWhiteSpace(cert_key))
                        {
                            Console.WriteLine("Certificate password cannot be empty. Please try again.");
                            continue;
                        }
                    }

                    Console.Write("Enter the database path: ");
                    database_path = Console.ReadLine();
                    if (string.IsNullOrWhiteSpace(database_path))
                    {
                        Console.WriteLine("SqlDatabase path cannot be empty. Please try again.");
                        continue;
                    }
                    SqlDatabase.Instance.OpenDatabase("(LocalDB)\\MSSQLLocalDB", database_path);

                    Console.Write("Enter the storage path: ");
                    storage_path = Console.ReadLine();
                    if (string.IsNullOrWhiteSpace(storage_path))
                    {
                        Console.WriteLine("Storage path cannot be empty. Please try again.");
                        continue;
                    }
                    LocalDatabase.Instance.SetStorageDirectory(storage_path);

                    break;

                } while (true);
            }

            // Start the appropriate server based on the protocol
            if (protocol.ToLower() == "http")
            {
                Thread http = new Thread(() => HttpServerMain(host_name, port_number));
                http.Start();
                http.Join();
            }
            else if (protocol.ToLower() == "https")
            {
                Thread https = new Thread(() => HttpsServerMain(host_name, port_number, true, cert_path, cert_key));
                https.Start();
                https.Join();
            }
        }
        static void HttpServerMain(string host, int port)
        {
            Console.WriteLine($"\nFile Storage Server: http://{host}:{port}");

            HttpServer server = new HttpServer(IPAddress.Any, port);
            server.OptionKeepAlive = true;
            server.OptionAcceptorBacklog = 5;
            //server.OptionNoDelay = true;

            Console.Write("Server starting... ");
            server.Start();
            Console.WriteLine("Done!");

            Console.WriteLine("Press Enter to stop the server or '!' to restart the server...");
            while (true)
            {
                string line = Console.ReadLine();
                if (string.IsNullOrEmpty(line))
                {
                    break;
                }
                if (line == "!")
                {
                    Console.Write("Server restarting... ");
                    server.Restart();
                    Console.WriteLine("Done!");
                }
            }
            Console.WriteLine("Server stopping...");
            SqlDatabase.Instance.CloseDatabase();
            server.Stop();
        }
        static void HttpsServerMain(string host, int port, bool cert_required, string cert_path, string password)
        {
            Console.WriteLine($"\nFile Storage Server: https://{host}:{port}");

            bool ValidateClientCertificate(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors)
            {
                Console.WriteLine("Validating certificate {0}", certificate.Issuer);
                if (sslPolicyErrors == SslPolicyErrors.None && certificate.Subject == "CN=localhost")
                {
                    return true;
                }
                Console.WriteLine("Certificate error: " + sslPolicyErrors);
                return false;
            }

            SslContext context = cert_required
                ? new SslContext(SslProtocols.Tls13, new X509Certificate2(cert_path, password), ValidateClientCertificate)
                : new SslContext(SslProtocols.Tls13, new X509Certificate2(cert_path, password));

            context.ClientCertificateRequired = cert_required;

            HttpsServer server = new HttpsServer(context, IPAddress.Any, port);
            server.OptionKeepAlive = true;
            server.OptionAcceptorBacklog = 5;
            //server.OptionNoDelay = true;

            Console.Write("Server starting... ");
            server.Start();
            Console.WriteLine("Done!");

            Console.WriteLine("Press Enter to stop the server or '!' to restart the server...");
            while (true)
            {
                string line = Console.ReadLine();
                if (string.IsNullOrEmpty(line))
                {
                    break;
                }
                if (line == "!")
                {
                    Console.Write("Server restarting... ");
                    server.Restart();
                    Console.WriteLine("Done!");
                }
            }
            Console.WriteLine("Server stopping...");
            SqlDatabase.Instance.CloseDatabase();
            server.Stop();
        }
    }
}
