#include "utils.h"
#include "logger.h"
#include "http_client.h"
#include "file_handle.h"
#include "json/json_parser.h"

namespace NetworkOperations 
{
	HttpClient::HttpClient()
	{
#ifdef WININET
		hSession = InternetOpenW(USER_AGENT,
			INTERNET_OPEN_TYPE_DIRECT,
			NULL,
			NULL,
			0);
#else 
		hSession = WinHttpOpen(USER_AGENT,
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0);
#endif
	}

	HttpClient::~HttpClient()
	{
		if (hSession)
		{
#ifdef WININET
			InternetCloseHandle(hSession);
#else 
			WinHttpCloseHandle(hSession);
#endif
		}
	}

	BOOL HttpClient::Connect(const std::wstring& url)
	{
		return Connect(url, NULL);
	}

	BOOL HttpClient::Connect(const std::wstring& url, PCCERT_CONTEXT cert)
	{
		if (!hSession)
		{
			LOG_ERROR_W(L"Failed to initialize an Internet session.");
			return FALSE;
		}
		// Initialize the URL_COMPONENTS structure.
		URL_COMPONENTSW urlComp;
		ZeroMemory(&urlComp, sizeof(urlComp));
		urlComp.dwStructSize = sizeof(urlComp);

		// Set required component lengths to non-zero so that they are cracked.
		urlComp.dwSchemeLength = (DWORD)-1;
		urlComp.dwHostNameLength = (DWORD)-1;
		urlComp.dwUrlPathLength = (DWORD)-1;
		urlComp.dwUserNameLength = (DWORD)-1;
		urlComp.dwPasswordLength = (DWORD)-1;
		urlComp.dwExtraInfoLength = (DWORD)-1;

#ifdef WININET
		if (!InternetCrackUrlW(url.c_str(), (DWORD)url.length(), 0, &urlComp))
		{
#else
		if (!WinHttpCrackUrl(url.c_str(), (DWORD)url.length(), 0, &urlComp))
		{
#endif
			LOG_ERROR_W(L"Crack Url return error code: %d", (int)GetLastError());
			return FALSE;
		}

		// Copy cracked URL hostName & UrlPath to buffers so they are separated
		portNumber = urlComp.nPort;
		wcsncpy_s(scheme, 0x20, urlComp.lpszScheme, urlComp.dwSchemeLength);
		wcsncpy_s(hostName, 0x100, urlComp.lpszHostName, urlComp.dwHostNameLength);

		// Close any existing connection.
		if (hConnect)
		{
			Disconnect();
		}
		return Connect(hostName, portNumber, (scheme == L"https"), cert);
		}

	BOOL HttpClient::Connect(const std::wstring & host, WORD port, BOOL security)
	{
		return Connect(host, port, security, NULL);
	}

	BOOL HttpClient::Connect(const std::wstring & host, WORD port, BOOL security, PCCERT_CONTEXT cert)
	{
		this->pCertContext = cert;
		if (host.empty() || port == 0)
		{
			return FALSE;
		}
#ifdef WININET
		DWORD dwFlags = security ? INTERNET_FLAG_SECURE : 0;
		hConnect = InternetConnectW(hSession, host.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, dwFlags, 0);
#else 
		DWORD dwFlags = security ? WINHTTP_FLAG_SECURE : 0;
		hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
#endif
		return hConnect != NULL;
	}

	void HttpClient::OptionKeepConnect(BOOL enable)
	{
		keepConnect = enable;
	}

	void HttpClient::OptionRecvTimeOut(DWORD time)
	{
#ifdef WININET
		InternetSetOptionW(hConnect, INTERNET_OPTION_RECEIVE_TIMEOUT, &time, sizeof(time));
#else 
		WinHttpSetOption(hConnect, WINHTTP_OPTION_RECEIVE_TIMEOUT, &time, sizeof(time));
#endif
	}

	void HttpClient::OptionSendTimeOut(DWORD time)
	{
#ifdef WININET
		InternetSetOptionW(hConnect, INTERNET_OPTION_SEND_TIMEOUT, &time, sizeof(time));
#else 
		WinHttpSetOption(hConnect, WINHTTP_OPTION_SEND_TIMEOUT, &time, sizeof(time));
#endif
	}

	void HttpClient::OptionConnectTimeOut(DWORD time)
	{
#ifdef WININET
		InternetSetOptionW(hConnect, INTERNET_OPTION_CONNECT_TIMEOUT, &time, sizeof(time));
#else 
		WinHttpSetOption(hConnect, WINHTTP_OPTION_CONNECT_TIMEOUT, &time, sizeof(time));
#endif
	}

	BOOL HttpClient::Disconnect()
	{
		BOOL result = TRUE;
		if (hConnect)
		{
#ifdef WININET
			result = InternetCloseHandle(hConnect);
#else 
			result = WinHttpCloseHandle(hConnect);
#endif
		}
		if (pCertContext)
		{
			result = CertFreeCertificateContext(pCertContext);
		}
		return result;
	}

	LPVOID HttpClient::OpenRequest(const std::wstring & verb, const std::wstring & path, const std::wstring & headers)
	{
		if (!hConnect)
		{
			LOG_ERROR_W(L"You have not opened a connection to the server, call Connect() to create a connection!");
			return NULL;
		}
		HINTERNET hRequest = NULL;
		std::wstring fullPath = path.empty() ? L"" : path;

#ifdef WININET

		// Keep-Alive
		DWORD dwFlags = keepConnect ? INTERNET_FLAG_KEEP_CONNECTION : INTERNET_FLAG_RELOAD;

		// Open request
		hRequest = HttpOpenRequestW(hConnect, verb.c_str(), fullPath.c_str(),
			NULL, NULL, NULL,
			dwFlags,
			0);
		if (!hRequest)
		{
			LOG_ERROR_W(L"Failed to open request. Error code = %d", GetLastError());
			return NULL;
		}
		// Set the HTTP header request
		if (!HttpAddRequestHeadersW(hRequest,
			headers.c_str(), (DWORD)headers.length(),
			HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
		{
			LOG_ERROR_W(L"Failed to add headers. Error code = %d ", GetLastError());
			InternetCloseHandle(hRequest);
			return NULL;
		}
#else 
		// Open request
		hRequest = WinHttpOpenRequest(hConnect, verb.c_str(), fullPath.c_str(),
			NULL,
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_REFRESH);
		if (!hRequest)
		{
			LOG_ERROR_W(L"Failed to open request. Error code = %d", GetLastError());
			return NULL;
		}
		// Disable Keep-Alive
		if (!keepConnect)
		{
			DWORD disableKeepAlive = WINHTTP_DISABLE_KEEP_ALIVE;
			WinHttpSetOption(hRequest, WINHTTP_OPTION_DISABLE_FEATURE, &disableKeepAlive, sizeof(disableKeepAlive));
		}
		// Set the HTTP header request
		if (!WinHttpAddRequestHeaders(hRequest,
			headers.c_str(), (DWORD)headers.length(),
			WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE))
		{
			LOG_ERROR_W(L"Failed to add headers. Error code = %d ", GetLastError());
			WinHttpCloseHandle(hRequest);
			return NULL;
		}
#endif
		return hRequest;
	}

	BOOL HttpClient::SendRequest(LPVOID hRequest, const void* data, const size_t & length)
	{
		if (!hRequest)
		{
			LOG_ERROR_W(L"You have not opened a request, call OpenRequest() to create a request!");
			return FALSE;
		}
resend:
		//Send the HTTP request
#ifdef WININET
		if (!HttpSendRequestW(hRequest, NULL, 0, (LPVOID)data, (DWORD)length))
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED)
			{
				//Set the client certificate context
				LOG_WARNING_W(L"The server is requesting client authentication.");
				if (!InternetSetOptionW(hRequest,
					INTERNET_OPTION_CLIENT_CERT_CONTEXT,
					(LPVOID)pCertContext, sizeof(CERT_CONTEXT)))
				{
					LOG_ERROR_W(L"Failed to set client certificate context: Error code = %d", dwError);
					CertFreeCertificateContext(pCertContext);
					CloseRequest(hRequest);
					return FALSE;
				}
				goto resend;
			}
#else
		if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)data, (DWORD)length, (DWORD)length, 0))
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED)
			{
				//Set the client certificate context
				LOG_WARNING_W(L"The server is requesting client authentication.");
				if (!WinHttpSetOption(hRequest,
					WINHTTP_OPTION_CLIENT_CERT_CONTEXT,
					(LPVOID)pCertContext, sizeof(CERT_CONTEXT)))
				{
					LOG_ERROR_W(L"Failed to set client certificate context: Error code = %d", dwError);
					CertFreeCertificateContext(pCertContext);
					CloseRequest(hRequest);
					return FALSE;
				}
				goto resend;
			}
#endif
			else
			{
				LOG_ERROR_W(L"Failed to send request. Error code = %d", dwError);
				CloseRequest(hRequest);
				return FALSE;
			}
		}
		return TRUE;
		}

	BOOL HttpClient::CloseRequest(LPVOID hRequest)
	{
		if (hRequest)
		{
#ifdef WININET
			return InternetCloseHandle(hRequest);
#else 
			return WinHttpCloseHandle(hRequest);
#endif
		}
		return FALSE;
	}

	HttpResponse HttpClient::Head(const std::wstring & path, const HttpHeaders & headers)
	{
		HINTERNET hRequest = OpenRequest(L"HEAD", path, headers.GetFormatWstring());
		if (!hRequest)
		{
			LOG_ERROR_W(L"Failed to initialize an request!");
			return HttpResponse();
		}
		if (!SendRequest(hRequest, NULL, 0))
		{
			return HttpResponse();
		}
		// Read the response content, status code, and headers
		return HttpResponse(hRequest);
	}

	HttpResponse HttpClient::Get(const std::wstring & path, const HttpHeaders & headers)
	{
		HINTERNET hRequest = OpenRequest(L"GET", path, headers.GetFormatWstring());
		if (!hRequest)
		{
			LOG_ERROR_W(L"Failed to initialize an request!");
			return HttpResponse();
		}
		if (!SendRequest(hRequest, NULL, 0))
		{
			return HttpResponse();
		}
		// Read the response content, status code, and headers
		return HttpResponse(hRequest);
	}

	HttpResponse HttpClient::Post(const std::wstring & path, const HttpHeaders & headers, const std::string & data)
	{
		return Post(path, headers, data.c_str(), data.length());
	}

	HttpResponse HttpClient::Post(const std::wstring & path, const HttpHeaders & headers, const void* data, size_t length)
	{
		HINTERNET hRequest = OpenRequest(L"POST", path, headers.GetFormatWstring());
		if (!hRequest)
		{
			LOG_ERROR_W(L"Failed to initialize an request!");
			return HttpResponse();
		}
		if (!SendRequest(hRequest, data, length))
		{
			return HttpResponse();
		}
		// Read the response content, status code, and headers
		return HttpResponse(hRequest);
	}

	HttpResponse HttpClient::Post(const std::wstring & path, const HttpHeaders & headers, IDataTransform * transform, const void* data, size_t length)
	{
		BYTE* buffer = NULL;
		DWORD buffer_size = 0;
		transform->TransformData((BYTE*)data, (DWORD)length, buffer, buffer_size);
		return Post(path, headers, buffer, buffer_size);
	}


	HttpResponse HttpClient::Put(const std::wstring & path, const HttpHeaders & headers, const std::string & data)
	{
		return Put(path, headers, data.c_str(), data.length());
	}

	HttpResponse HttpClient::Put(const std::wstring & path, const HttpHeaders & headers, const void* data, size_t length)
	{
		HINTERNET hRequest = OpenRequest(L"PUT", path, headers.GetFormatWstring());
		if (!hRequest)
		{
			LOG_ERROR_W(L"Failed to initialize an request!");
			return HttpResponse();
		}
		if (!SendRequest(hRequest, data, length))
		{
			return HttpResponse();
		}
		// Read the response content, status code, and headers
		return HttpResponse(hRequest);
	}

	HttpResponse HttpClient::Put(const std::wstring & path, const HttpHeaders & headers, IDataTransform * transform, const void* data, size_t length)
	{
		BYTE* buffer = NULL;
		DWORD buffer_size = 0;
		transform->TransformData((BYTE*)data, (DWORD)length, buffer, buffer_size);
		return Put(path, headers, buffer, buffer_size);
	}


	HttpResponse HttpClient::Patch(const std::wstring & path, const HttpHeaders & headers)
	{
		HINTERNET hRequest = OpenRequest(L"PATCH", path, headers.GetFormatWstring());
		if (!hRequest)
		{
			LOG_ERROR_W(L"Failed to initialize an request!");
			return HttpResponse();
		}
		if (!SendRequest(hRequest, NULL, 0))
		{
			return HttpResponse();
		}
		// Read the response content, status code, and headers
		return HttpResponse(hRequest);
	}

	HttpResponse HttpClient::Delete(const std::wstring & path, const HttpHeaders & headers)
	{
		HINTERNET hRequest = OpenRequest(L"DELETE", path, headers.GetFormatWstring());
		if (!hRequest)
		{
			LOG_ERROR_W(L"Failed to initialize an request!");
			return HttpResponse();
		}
		if (!SendRequest(hRequest, NULL, 0))
		{
			return HttpResponse();
		}
		// Read the response content, status code, and headers
		return HttpResponse(hRequest);
	}

	HttpResponse HttpClient::Select(const std::wstring & path, const HttpHeaders & headers)
	{
		HttpResponse response;
		HINTERNET hRequest = OpenRequest(L"SELECT", path, headers.GetFormatWstring());
		if (!hRequest)
		{
			LOG_ERROR_W(L"Failed to initialize an request!");
			return HttpResponse();
		}
		if (!SendRequest(hRequest, NULL, 0))
		{
			return HttpResponse();
		}
		// Read the response content, status code, and headers
		return HttpResponse(hRequest);
	}

	HttpResponse HttpClient::Options(const std::wstring & path, const HttpHeaders & headers)
	{
		HINTERNET hRequest = OpenRequest(L"OPTIONS", path, headers.GetFormatWstring());
		if (!hRequest)
		{
			LOG_ERROR_W(L"Failed to initialize an request!");
			return HttpResponse();
		}
		if (!SendRequest(hRequest, NULL, 0))
		{
			return HttpResponse();
		}
		// Read the response content, status code, and headers
		return HttpResponse(hRequest);
	}

	HttpResponse HttpClient::Trace(const std::wstring & path, const HttpHeaders & headers)
	{
		HINTERNET hRequest = OpenRequest(L"TRACE", path, headers.GetFormatWstring());
		if (!hRequest)
		{
			LOG_ERROR_W(L"Failed to initialize an request!");
			return HttpResponse();
		}
		if (!SendRequest(hRequest, NULL, 0))
		{
			return HttpResponse();
		}
		// Read the response content, status code, and headers
		return HttpResponse(hRequest);
	}

#ifdef WININET

	HttpResponse HttpClient::DownloadFile(const std::wstring & path)
	{
		HttpHeaders headers;
		headers.SetHeader("Content-Type", "application/octet-stream");
		headers.SetHeader("Content-Transfer-Encoding", "binary");

		std::vector<BYTE> buffer(BUFFER_SIZE);
		DWORD bytesRead, bytesWrite, dwBytesAvailable;

		// Initialize the HTTP request with the GET method
		HINTERNET hRequest = OpenRequest(L"GET", L"downloadfile", headers.GetFormatWstring());
		if (!hRequest)
		{
			LOG_ERROR_W(L"Failed to initialize an request!");
			return HttpResponse();
		}
		if (!SendRequest(hRequest, NULL, 0))
		{
			return HttpResponse();
		}

		HANDLE hOutputFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutputFile == INVALID_HANDLE_VALUE)
		{
			return HttpResponse();
		}
		// Query the response data length
		if (!InternetQueryDataAvailable(hRequest, &dwBytesAvailable, 0, 0))
		{
			return HttpResponse();
		}
		if (dwBytesAvailable < BUFFER_SIZE)
		{
			buffer.resize(dwBytesAvailable);
		}
		// Read the response data
		while (InternetReadFile(hRequest, buffer.data(), (DWORD)buffer.size(), &bytesRead) && bytesRead > 0)
		{
			if (!WriteFile(hOutputFile, buffer.data(), (DWORD)buffer.size(), &bytesWrite, NULL))
			{
				LOG_ERROR_W(L"Error writing data: %lu", GetLastError());
				break;
			}
			buffer.clear();
		}

		if (hOutputFile != NULL)
		{
			CloseHandle(hOutputFile);
		}
		// Get the response status code and headers
		return HttpResponse(hRequest, true);
	}

	HttpResponse HttpClient::UploadFile(const std::wstring & path)
	{
		DWORD bytesRead, bytesWrite;
		std::vector<BYTE> buffer(BUFFER_SIZE);

		HttpHeaders headers;
		headers.SetHeader("Content-Type", "application/octet-stream");
		headers.SetHeader("Content-Transfer-Encoding", "binary");

		// Open file handle
		HANDLE hInputFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInputFile == INVALID_HANDLE_VALUE)
		{
			return HttpResponse();
		}
		// Query the file data length
		DWORD fileSize = GetFileSize(hInputFile, NULL);
		if (fileSize < BUFFER_SIZE)
		{
			buffer.resize(fileSize);
		}

		headers.SetHeader("Content-length", std::to_string(fileSize));
		// Initialize the HTTP request with the POST method
		HINTERNET hRequest = OpenRequest(L"POST", L"uploadfile", headers.GetFormatWstring());
		if (!hRequest)
		{
			LOG_ERROR_W(L"Failed to initialize a request!");
			return HttpResponse();
		}
resend:
		//Send the HTTP request
		if (!HttpSendRequestExW(hRequest, NULL, 0, HSR_INITIATE, NULL))
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED)
			{
				//Set the client certificate context
				LOG_WARNING_W(L"The server is requesting client authentication.");
				if (!InternetSetOptionW(hRequest, INTERNET_OPTION_CLIENT_CERT_CONTEXT, (LPVOID)pCertContext, sizeof(CERT_CONTEXT)))
				{
					LOG_ERROR_W(L"Failed to set client certificate context: Error code = %d", dwError);
					CertFreeCertificateContext(pCertContext);
					CloseRequest(hRequest);
					return HttpResponse();
				}
				goto resend;
			}
			else
			{
				LOG_ERROR_W(L"Failed to send request. Error code = %d", dwError);
				CloseRequest(hRequest);
				return HttpResponse();
			}
		}

		// Read and write the file in chunks
		while (ReadFile(hInputFile, buffer.data(), (DWORD)buffer.size(), &bytesRead, NULL) && bytesRead > 0)
		{
			if (!InternetWriteFile(hRequest, buffer.data(), (DWORD)buffer.size(), &bytesWrite) && bytesWrite > 0)
			{
				LOG_ERROR_W(L"Error writing data: %lu", GetLastError());
				break;
			}
			buffer.clear();
		}
		// End the request
		if (!HttpEndRequestW(hRequest, NULL, 0, 0))
		{
			LOG_ERROR_W(L"HttpEndRequest failed: %lu", GetLastError());
		}
		if (hInputFile != NULL)
		{
			CloseHandle(hInputFile);
		}
		// Get the response status code and headers
		return HttpResponse(hRequest, true);
	}

#else

	HttpResponse HttpClient::DownloadFile(const std::wstring & path)
	{

	}

	HttpResponse HttpClient::UploadFile(const std::wstring & path)
	{

	}

#endif

#pragma region HttpResponse

	BOOL HttpResponse::CheckContentIsJson()
	{
		BOOL result = FALSE;
		if (!headerPairs.empty() && !headerPairs["Content-Type"].empty())
		{
			if (headerPairs["Content-Type"].compare("application/json") == 0)
			{
				result = TRUE;
			}
			result = (JsonParser::Parse(contentString.c_str())->CountChildren() > 0) ? TRUE : FALSE;
		}
		return result;
	}

	std::string HttpResponse::GetResponseHeader(const std::string& key)
	{
		return headerPairs[key];
	}

	std::wstring HttpResponse::GetResponseHeader(const std::wstring& key)
	{
		std::string key_str = Helper::StringHelper::convertWideStringToString(key);
		return Helper::StringHelper::convertStringToWideString(headerPairs[key_str]);
	}

	DWORD HttpResponse::GetStatusCode(HINTERNET hRequest)
	{
		DWORD statusCode = 0;
		DWORD length = sizeof(statusCode);
#ifdef WININET
		if (!HttpQueryInfoW(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &length, NULL))
		{
#else	
		if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &statusCode, &length, NULL))
		{
#endif
			LOG_ERROR_W(L"Failed to query status code. Error code = %ld", GetLastError());
			return 0;
		}
		return statusCode;
		}

	std::string HttpResponse::ReadResponseHeader(HINTERNET hRequest)
	{
		if (!hRequest)
		{
			LOG_ERROR_W(L"Handle request is null!");
			return std::string();
		}
		DWORD bufferSize = 0;
		std::vector<wchar_t> buffer(BUFFER_SIZE);
#ifdef WININET
		HttpQueryInfoW(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &bufferSize, NULL);
#else
		WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, NULL, NULL, &bufferSize, NULL);
#endif
		if (bufferSize == 0)
		{
			LOG_ERROR_W(L"Header size is zero!");
			return std::string();
		}
		else
		{
			if (bufferSize < BUFFER_SIZE)
			{
				buffer.resize(bufferSize / sizeof(wchar_t));
			}
		}
#ifdef WININET
		if (!HttpQueryInfoW(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, buffer.data(), &bufferSize, NULL))
		{
#else
		if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, NULL, buffer.data(), &bufferSize, NULL))
		{
#endif
			LOG_ERROR_W(L"Failed to query header information!");
			return std::string();
		}
		std::wstring headers(buffer.begin(), buffer.end());
		return Helper::StringHelper::convertWideStringToString(headers);
		}

	std::string HttpResponse::ReadResponseContent(HINTERNET hRequest)
	{
		std::string content;
		std::vector<BYTE> buffer(BUFFER_SIZE);
		DWORD bytesRead, dwBytesAvailable;
		if (!hRequest)
		{
			LOG_ERROR_W(L"Handle request is null!");
			return std::string();
		}
#ifdef WININET
		if (!InternetQueryDataAvailable(hRequest, &dwBytesAvailable, 0, 0))
		{
#else
		if (!WinHttpQueryDataAvailable(hRequest, &dwBytesAvailable))
		{
#endif
			LOG_ERROR_W(L"Failed to query data availability. Error code = %ld", GetLastError());
			return std::string();
		}
		if (dwBytesAvailable < BUFFER_SIZE)
		{
			buffer.resize(dwBytesAvailable);
		}
		while (TRUE)
		{
#ifdef WININET
			if (!InternetReadFile(hRequest, buffer.data(), (DWORD)buffer.size(), &bytesRead))
			{
#else
			if (!WinHttpReadData(hRequest, buffer.data(), (DWORD)buffer.size(), &bytesRead))
			{
#endif
				LOG_ERROR_W(L"Failed to read the HTTP response. Error code = %d", GetLastError());
				break;
			}
			if (bytesRead == 0)
			{
				break;	// EOF.
			}
			content.append(buffer.begin(), buffer.begin() + bytesRead);
			buffer.clear();
			}
		return content;
		}

	std::map<std::string, std::string> HttpResponse::ParseResponseHeaders(const std::string& headerStr)
	{
		std::string line;
		std::map<std::string, std::string> header;

		std::istringstream stream(headerStr);
		while (std::getline(stream, line))
		{
			size_t colonPos = line.find(":");
			if (colonPos != std::string::npos)
			{
				std::string name = line.substr(0, colonPos);
				std::string value = line.substr(colonPos + 2, line.length() - colonPos - 3); // Also remove \r
				header[name] = value;
			}
		}
		return header;
	}

#pragma endregion HttpResponse

#pragma region CertificateManager

	CertificateManager* CertificateManager::s_instance = 0;

	BOOL CertificateManager::RemoveCertificateFromStore(PCCERT_CONTEXT pCertContext)
	{
		if (pCertContext)
		{
			if (CertDeleteCertificateFromStore(pCertContext))
			{
				CertFreeCertificateContext(pCertContext);
				return true;
			}
		}
		return false;
	}

	PCCERT_CONTEXT CertificateManager::GetCertificateFromStore(const std::wstring & subjectName, const std::wstring & store, DWORD branch)
	{
		HCERTSTORE hStore = NULL;
		PCCERT_CONTEXT pCertContext = NULL;

		DWORD dwFlags = CERT_STORE_READONLY_FLAG | branch;
		if (!store.empty() && branch != 0)
		{
			hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, dwFlags, store.c_str());
			if (hStore)
			{
				pCertContext = CertFindCertificateInStore(hStore,
					X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
					0,
					CERT_FIND_SUBJECT_STR,
					(LPVOID)subjectName.c_str(),
					NULL);
			}
		}
		else
		{
			int i = 0;
			const WCHAR* stores[] = { L"My", L"WebHosting" };
			while ((pCertContext == NULL) && (i < ARRAYSIZE(stores)))
			{
				hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_STORE_READONLY_FLAG | CERT_SYSTEM_STORE_CURRENT_USER, stores[i]);
				if (hStore)
				{
					pCertContext = CertFindCertificateInStore(hStore,
						X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
						0,
						CERT_FIND_SUBJECT_STR,
						(LPVOID)subjectName.c_str(),
						NULL);
				}
				i++;
			}
		}
		if (hStore)
		{
			CertCloseStore(hStore, 0);
		}
		return pCertContext;
	}

	PCCERT_CONTEXT CertificateManager::ImportLoadCertFromFile(const std::wstring & path, const std::wstring & password, const std::wstring & subjectName)
	{
		BYTE* buffer = NULL;
		DWORD buffer_size = 0;
		if (!ResourceOperations::FileHandle::ReadFileData(path, buffer, buffer_size))
		{
			LOG_ERROR_A("%s", ResourceOperations::FileHandle::GetLastErrorString().c_str());
			return NULL;
		}
		// Prepare the CRYPT_DATA_BLOB structure
		CRYPT_DATA_BLOB blob;
		blob.pbData = buffer;
		blob.cbData = buffer_size;

		// Import the certificate store from the PFX file
		HCERTSTORE hStore = PFXImportCertStore(&blob, password.c_str(), 0);
		if (!hStore)
		{
			LOG_ERROR_W(L"Failed to load certificate from the file.pfx");
			return NULL;
		}

		// Find the first certificate in the store
		PCCERT_CONTEXT pCertContext = CertFindCertificateInStore(hStore,
			X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
			0,
			CERT_FIND_SUBJECT_STR, subjectName.c_str(),
			NULL);
		if (!pCertContext)
		{
			LOG_ERROR_W(L"Failed to find a certificate in the store.");
		}

		// Clean up
		if (buffer)
		{
			delete[] buffer;
		}
		CertCloseStore(hStore, 0);
		return pCertContext;
	}
#pragma endregion CertificateManager

}	// NetworkOperations