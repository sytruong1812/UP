#include "http.h"

CURLcode Curl_http_setup_conn(struct Curl_easy* data, struct connectdata* conn)
{
	CURLcode result = CURLE_OK;
	struct UserDefined set = data->set;
	struct SingleRequest* req = &data->req;

	if (!data->set.uh)
	{
		data->set.uh = curl_url();
		CURLUcode rc = curl_url_set(data->set.uh, CURLUPART_URL, data->state.url, 0);
		if (rc != CURLUE_OK)
		{
			return CURLE_SET_CURLU_ERROR;
		}
	}
	// Open a session
	req->hSession = WinHttpOpen(curlx_convert_UTF8_to_wchar(data->set.str[STRING_USERAGENT]),
					WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
					WINHTTP_NO_PROXY_NAME,
					WINHTTP_NO_PROXY_BYPASS, 0);

	// Specify an HTTP server.
	if (req->hSession)
	{
		// Use WinHttpSetTimeouts to set a new time-out values.
		if (!WinHttpSetTimeouts(req->hSession,
			0,
			data->set.connecttimeout,
			data->set.timeout,
			data->set.timeout))
		{
			return CURLE_WINHTTP_CANNOT_SET_TIMEOUT;
		}
	}
	else
	{
		return CURLE_WINHTTP_CANNOT_CONNECT_SERVER;
	}
	return result;
}

CURLcode Curl_http_connect(struct Curl_easy* data, bool* done)
{
	CURLcode rc;
	CURLUcode uc;
	struct Curl_URL* h = data->set.uh;
	struct SingleRequest* req = &data->req;

	/* extract hostname from the parsed URL */
	char* host = NULL;
	uc = curl_url_get(h, CURLUPART_HOST, &host, 0);
	if (uc == CURLUE_BAD_HANDLE)
	{
		rc = CURLE_URL_GET_ERROR;
		goto exit;
	}
	infof(data, "Host name: %s", host);

	/* extract post from the parsed URL */
	INTERNET_PORT port;
	if (data->set.use_port)
	{
		port = data->set.use_port;
	}
	else
	{
		char* str_port = NULL;
		uc = curl_url_get(h, CURLUPART_PORT, &str_port, 0);
		if (uc == CURLUE_OK)
		{
			port = atoi(str_port);
		}
		else
		{
			if (uc == CURLUE_BAD_PORT_NUMBER)
			{
				port = INTERNET_DEFAULT_PORT;
			}
			else
			{
				rc = CURLE_URL_GET_ERROR;
				goto exit;
			}
		}
		if (str_port) free(str_port);
	}
	infof(data, "Port: %d", port);

	// Prepare and connect
	req->hConnect = WinHttpConnect(req->hSession,
									(LPCWSTR)curlx_convert_UTF8_to_wchar(host),
									port,
									0);
	if (req->hConnect == NULL)
	{
		rc = CURLE_WINHTTP_CANNOT_CONNECT_SERVER;
		goto exit;
	}
	*done = TRUE;
exit:
	if (host) {
		free(host);
	}
	return CURLE_OK;
}

static DWORD Choose_Auth_Scheme(DWORD dwSupportedSchemes)
{
	if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NEGOTIATE)
		return WINHTTP_AUTH_SCHEME_NEGOTIATE;
	else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NTLM)
		return WINHTTP_AUTH_SCHEME_NTLM;
	else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_PASSPORT)
		return WINHTTP_AUTH_SCHEME_PASSPORT;
	else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_DIGEST)
		return WINHTTP_AUTH_SCHEME_DIGEST;
	else
		return 0;
}

void Check_Authentication(struct Curl_easy* data) {

	DWORD dwTarget;
	DWORD dwFirstScheme;
	DWORD dwLastStatus = 0;
	DWORD dwStatusCode = 0;
	DWORD dwSelectedScheme;
	DWORD dwSupportedSchemes;
	DWORD dwProxyAuthScheme = 0;
	DWORD dwSize = sizeof(DWORD);
	struct SingleRequest* req = &data->req;

	BOOL bResults = FALSE;
	while (!req->req_good)
	{
		//  If a proxy authentication challenge was responded to, reset
		//  those credentials before each SendRequest, because the proxy  
		//  may require re-authentication after responding to a 401 or  
		//  to a redirect. If you don't, you can get into a 
		//  407-401-407-401- loop.
		if (dwProxyAuthScheme != 0)
			bResults = WinHttpSetCredentials(req->hRequest,
				WINHTTP_AUTH_TARGET_PROXY,
				dwProxyAuthScheme,
				curlx_convert_UTF8_to_wchar(data->set.str[STRING_USERNAME]),
				curlx_convert_UTF8_to_wchar(data->set.str[STRING_PASSWORD]),
				NULL);
		// Send a request.
		bResults = WinHttpSendRequest(req->hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0,
			WINHTTP_NO_REQUEST_DATA,
			0,
			0,
			0);

		// End the request.
		if (bResults)
		{
			bResults = WinHttpReceiveResponse(req->hRequest, NULL);
		}
		// Resend the request in case of ERROR_WINHTTP_RESEND_REQUEST error.
		if (!bResults && GetLastError() == ERROR_WINHTTP_RESEND_REQUEST)
		{
			continue;
		}
		// Check the status code.
		if (bResults)
		{
			bResults = WinHttpQueryHeaders(req->hRequest,
				WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
				NULL,
				&dwStatusCode,
				&dwSize,
				NULL);
		}
		if (bResults)
		{
			switch (dwStatusCode)
			{
				case 200:
					// The resource was successfully retrieved.
					// You can use WinHttpReadData to read the 
					// contents of the server's response.
					infof(data, "The resource was successfully retrieved.\n");
					req->req_good = TRUE;
					break;

				case 401:
					// The server requires authentication.
					infof(data, " The server requires authentication. Sending credentials...\n");
					// Obtain the supported and preferred schemes.
					bResults = WinHttpQueryAuthSchemes(req->hRequest,
						&dwSupportedSchemes,
						&dwFirstScheme,
						&dwTarget);
					// Set the credentials before resending the request.
					if (bResults)
					{
						dwSelectedScheme = Choose_Auth_Scheme(dwSupportedSchemes);

						if (dwSelectedScheme == 0)
						{
							req->req_good = TRUE;
						}
						else
						{
							bResults = WinHttpSetCredentials(req->hRequest,
								dwTarget,
								dwSelectedScheme,
								curlx_convert_UTF8_to_wchar(data->set.str[STRING_USERNAME]),
								curlx_convert_UTF8_to_wchar(data->set.str[STRING_PASSWORD]),
								NULL);
						}
					}
					// If the same credentials are requested twice, abort the
					// request.  For simplicity, this sample does not check
					// for a repeated sequence of status codes.
					if (dwLastStatus == 401)
					{
						req->req_good = TRUE;
					}
					break;

				case 407:
					// The proxy requires authentication.
					infof(data, "The proxy requires authentication.  Sending credentials...\n");
					// Obtain the supported and preferred schemes.
					bResults = WinHttpQueryAuthSchemes(req->hRequest,
						&dwSupportedSchemes,
						&dwFirstScheme,
						&dwTarget);
					// Set the credentials before resending the request.
					if (bResults)
					{
						dwProxyAuthScheme = Choose_Auth_Scheme(dwSupportedSchemes);
					}
					// If the same credentials are requested twice, abort the
					// request.  For simplicity, this sample does not check 
					// for a repeated sequence of status codes.
					if (dwLastStatus == 407)
					{
						req->req_good = TRUE;
					}
					break;
				default:
					// The status code does not indicate success.
					infof(data, "Error. Status code %d returned.\n", dwStatusCode);
					req->req_good = TRUE;
			}
		}
		// Keep track of the last status code.
		dwLastStatus = dwStatusCode;
		// If there are any errors, break out of the loop.
		if (!bResults)
		{
			req->req_good = TRUE;
		}
	}
}

CURLcode Curl_http(struct Curl_easy* data, bool* done)
{
	DWORD keep_alive;
	Curl_HttpReq httpreq;
	CURLcode result = CURLE_OK;
	struct SingleRequest* req = &data->req;

	// HTTP method, could be GET, POST, PUT, etc.
	const char* request = NULL;
	Curl_http_method(data, &request, &httpreq);
	infof(data, "Request: %s", request);

	CURLUcode uc;
	char* path = NULL;
	/* extract the path from the parsed URL */
	uc = curl_url_get(data->set.uh, CURLUPART_PATH, &path, 0);
	if (uc == CURLUE_BAD_HANDLE)
	{
		return CURLE_URL_GET_ERROR;
	}
	infof(data, "Path: %s", path);
	/* extract query from the parsed URL */
	char* query = NULL;
	uc = curl_url_get(data->set.uh, CURLUPART_QUERY, &query, 0);
	if (uc == CURLUE_BAD_HANDLE)
	{
		return CURLE_URL_GET_ERROR;
	}
	infof(data, "Query: %s", query);

	char* full_path = NULL;
	if (query != NULL)
	{
		full_path = (char*)malloc(strlen(path) + strlen(query) + 2); // 1='?' and 1='\0'
		if (full_path)
		{
			sprintf(full_path, "%s?%s", path, query);
		}
	}
	else
	{
		full_path = (char*)malloc(strlen(path) + 1); // 1='\0'
		if (full_path)
		{
			strcpy(full_path, path);
		}
	}
	char* schema = NULL;
	DWORD dwFlag = WINHTTP_FLAG_REFRESH;
	uc = curl_url_get(data->set.uh, CURLUPART_SCHEME, &schema, 0);
	if (uc == CURLUE_BAD_HANDLE)
	{
		return CURLE_URL_GET_ERROR;
	}
	else
	{
		if (strcasecompare(schema, "https") && data->set.using_ssl)
		{
			dwFlag = WINHTTP_FLAG_SECURE;
		}
	}
	infof(data, "Schema: %s", schema);
	req->req_good = FALSE;
	req->hRequest = WinHttpOpenRequest(req->hConnect,
		(LPCWSTR)curlx_convert_UTF8_to_wchar(request),
		(LPCWSTR)curlx_convert_UTF8_to_wchar(full_path),
		NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		dwFlag);
	if (!req->hRequest)
	{
		result = CURLE_WINHTTP_OPEN_REQUEST_ERROR;
		goto exit;
	}
	else
	{
		req->req_good = *done = TRUE;	//Continue to send a request until status code is not 401 or 407.
	}

	// Set HTTP Version
	DWORD version;
	switch (data->set.httpwant)
	{
		case CURL_HTTP_VERSION_NONE:
		case CURL_HTTP_VERSION_1_0:
		case CURL_HTTP_VERSION_1_1:
			version = 0;
			break;
		case CURL_HTTP_VERSION_2_0:
		case CURL_HTTP_VERSION_2TLS:
		case CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE:
			version = WINHTTP_PROTOCOL_FLAG_HTTP2;
			break;
		case CURL_HTTP_VERSION_3:
		case CURL_HTTP_VERSION_3ONLY:
			version = WINHTTP_PROTOCOL_FLAG_HTTP3;
			break;
		default:
			version = 0;
			break;
	}
	if (!WinHttpSetOption(req->hRequest,
		WINHTTP_OPTION_ENABLE_HTTP_PROTOCOL,
		&version, sizeof(version)))
	{
		result = CURLE_WINHTTP_SET_OPTION_ERROR;
		goto exit;
	}

	//Disable Keep-Alive for HTTP 1.0
	keep_alive = WINHTTP_DISABLE_KEEP_ALIVE;
	if (!WinHttpSetOption(req->hRequest,
		WINHTTP_OPTION_DISABLE_FEATURE,
		&keep_alive, sizeof(keep_alive)))
	{
		result = CURLE_WINHTTP_SET_OPTION_ERROR;
		goto exit;
	}

	if (data->set.str[STRING_ENCODING] && !data->set.http_ce_skip)
	{
		DWORD content_encoded;
		if (strcasecompare(data->set.str[STRING_ENCODING], "gzip"))
		{
			content_encoded = WINHTTP_DECOMPRESSION_FLAG_GZIP;
		}
		else if (strcasecompare(data->set.str[STRING_ENCODING], "deflate"))
		{
			content_encoded = WINHTTP_DECOMPRESSION_FLAG_DEFLATE;
		}
		else
		{
			content_encoded = WINHTTP_DECOMPRESSION_FLAG_ALL;
		}
		if (!WinHttpSetOption(req->hRequest,
			WINHTTP_OPTION_DECOMPRESSION,
			&content_encoded, sizeof(content_encoded)))
		{
			DWORD r = GetLastError();
			result = CURLE_WINHTTP_SET_OPTION_ERROR;
			goto exit;
		}
	}

	//Some HTTP servers and proxies require authentication before allowing access to resources on the Internet.
	Check_Authentication(data);

	//Send the request
	infof(data, "Body size: %d", data->set.postfieldsize);
	if (data->set.postfields != NULL && data->set.postfieldsize > 0)
	{
		if (!WinHttpSendRequest(req->hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS, 0,
			(LPVOID)data->set.postfields, (DWORD)data->set.postfieldsize,
			(DWORD)data->set.postfieldsize, 0))
		{
			result = CURLE_WINHTTP_SEND_REQUEST_ERROR;
			goto exit;
		}
	}
exit:
	if (path) free(path);
	if (query) free(query);
	if (full_path) free(full_path);
	return result;
}

CURLcode Curl_http_write_resp(struct Curl_easy* data, const char* buf, size_t blen, bool is_eos /* End of Stream */)
{
	CURLcode result = CURLE_OK;

	DWORD dwSize = 0;
	DWORD dwTotal = 0;
	DWORD bytesRead = 0;
	DWORD content_length = 0;
	struct SingleRequest* req = &data->req;

	char* buffer = NULL;
	DWORD buffer_size = READBUFFER_SIZE;

	if (!WinHttpReceiveResponse(req->hRequest, NULL))
	{
		WinHttpCloseHandle(req->hRequest);
		return CURLE_WINHTTP_RECV_RESPONSE_ERROR;
	}

	// Add query header in to response
	if (data->set.include_header)
	{
		DWORD dwHeaderSize;
		// First, use WinHttpQueryHeaders to obtain the size of the buffer.
		if (!WinHttpQueryHeaders(req->hRequest,
			WINHTTP_QUERY_RAW_HEADERS_CRLF | WINHTTP_QUERY_FLAG_REQUEST_HEADERS,
			WINHTTP_HEADER_NAME_BY_INDEX,
			NULL, &dwHeaderSize,
			WINHTTP_NO_HEADER_INDEX))
		{
			WinHttpCloseHandle(req->hRequest);
			return CURLE_WINHTTP_QUERY_DATA_ERROR;
		}
		// Allocate memory for the buffer.
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			wchar_t* header = (wchar_t*)malloc(dwHeaderSize);
			if (header == NULL)
			{
				return CURLE_FAILED_TO_ALLOCATE;
			}
			// Now, use WinHttpQueryHeaders to retrieve the header.
			if (!WinHttpQueryHeaders(req->hRequest,
				WINHTTP_QUERY_RAW_HEADERS_CRLF | WINHTTP_QUERY_FLAG_REQUEST_HEADERS,
				WINHTTP_HEADER_NAME_BY_INDEX,
				header, &dwHeaderSize,
				WINHTTP_NO_HEADER_INDEX))
			{
				WinHttpCloseHandle(req->hRequest);
				return CURLE_WINHTTP_QUERY_DATA_ERROR;
			}
			if (data->set.fwrite_func)
			{
				if (data->set.fwrite_func(curlx_convert_wchar_to_UTF8(header),
					dwHeaderSize / sizeof(wchar_t), 1, data->set.out_get) == 0)
				{
					result = CURLE_WINHTTP_WRITE_DATA_ERROR;
				}
			}
			free(header);
		}
	}

	if (!data->set.ignorecl)
	{
		DWORD dwContentLength = 64;
		wchar_t szContentLength[32] = { 0 };
		DWORD dwHeaderIndex = WINHTTP_NO_HEADER_INDEX;
		if (WinHttpQueryHeaders(req->hRequest,
			WINHTTP_QUERY_CONTENT_LENGTH,
			NULL,
			&szContentLength,
			&dwContentLength,
			&dwHeaderIndex))
		{
			content_length = _wtoi(szContentLength);
		}
		buffer_size = content_length;
	}
	else
	{
		if (data->set.buffer_size > 0)
		{
			buffer_size = data->set.buffer_size;
		}
	}

	buffer = (char*)malloc(buffer_size);
	if (buffer == NULL)
	{
		return CURLE_FAILED_TO_ALLOCATE;
	}

	while (WinHttpQueryDataAvailable(req->hRequest, &dwSize) && dwSize)
	{
		if (dwSize > buffer_size)
		{
			dwSize = buffer_size;
		}
		if (!WinHttpReadData(req->hRequest, buffer, dwSize, &dwSize))
		{
			result = CURLE_WINHTTP_READ_DATA_ERROR;
			break;
		}
		dwTotal += dwSize;

		if (data->set.fwrite_func)
		{
			if (data->set.fwrite_func(buffer, dwSize, 1, data->set.out_get) == 0)
			{
				result = CURLE_WINHTTP_WRITE_DATA_ERROR;
				break;
			}
		}
	}
	if (buffer) free(buffer);
	return result;
}

CURLcode Curl_http_write_resp_hd(struct Curl_easy* data, const char* hd, size_t hdlen, bool is_eos /* End of Stream */)
{
	struct SingleRequest* req = &data->req;
	CURLcode result = CURLE_OK;

	DWORD dwSize = 0;
	wchar_t* buffer = NULL;

	if (!WinHttpReceiveResponse(req->hRequest, NULL))
	{
		WinHttpCloseHandle(req->hRequest);
		return CURLE_WINHTTP_RECV_RESPONSE_ERROR;
	}

	// First, use WinHttpQueryHeaders to obtain the size of the buffer.
	if (!WinHttpQueryHeaders(req->hRequest,
		WINHTTP_QUERY_RAW_HEADERS_CRLF | WINHTTP_QUERY_FLAG_REQUEST_HEADERS,
		WINHTTP_HEADER_NAME_BY_INDEX,
		NULL, &dwSize,
		WINHTTP_NO_HEADER_INDEX))
	{
		WinHttpCloseHandle(req->hRequest);
		return CURLE_WINHTTP_QUERY_DATA_ERROR;
	}

	// Allocate memory for the buffer.
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		buffer = (wchar_t*)malloc(dwSize);
		if (buffer == NULL)
		{
			return CURLE_FAILED_TO_ALLOCATE;
		}
		// Now, use WinHttpQueryHeaders to retrieve the header.
		if (!WinHttpQueryHeaders(req->hRequest,
			WINHTTP_QUERY_RAW_HEADERS_CRLF | WINHTTP_QUERY_FLAG_REQUEST_HEADERS,
			WINHTTP_HEADER_NAME_BY_INDEX,
			buffer, &dwSize,
			WINHTTP_NO_HEADER_INDEX))
		{
			WinHttpCloseHandle(req->hRequest);
			return CURLE_WINHTTP_QUERY_DATA_ERROR;
		}

		if (data->set.fwrite_header)
		{
			if (data->set.fwrite_header(curlx_convert_wchar_to_UTF8(buffer),
				dwSize / sizeof(wchar_t),
				1,
				data->set.header_get) == 0)
			{
				free(buffer);
				return CURLE_WINHTTP_WRITE_DATA_ERROR;
			}
		}
		free(buffer);
	}
	return result;
}

CURLcode Curl_http_done(struct Curl_easy* data, CURLcode status, bool premature)
{
	struct SingleRequest* req = &data->req;
	if (req->hRequest)
	{
		WinHttpCloseHandle(req->hRequest);
	}
	if (req->hConnect)
	{
		WinHttpCloseHandle(req->hConnect);
	}
	if (req->hSession)
	{
		WinHttpCloseHandle(req->hSession);
	}
	if (status)
	{
		return status;
	}
	return CURLE_OK;
}

void Curl_http_method(struct Curl_easy* data, const char** method, Curl_HttpReq* reqp)
{
	const char* request;
	Curl_HttpReq httpreq = (Curl_HttpReq)data->set.method;

	/* Now set the 'request' pointer to the proper request string */
	if (data->set.str[STRING_CUSTOMREQUEST])
	{
		request = data->set.str[STRING_CUSTOMREQUEST];
	}
	else
	{
		if (data->set.opt_no_body)
		{
			request = "HEAD";
		}
		else
		{
			DEBUGASSERT((httpreq >= HTTPREQ_GET) && (httpreq <= HTTPREQ_HEAD));
			switch (httpreq)
			{
				case HTTPREQ_POST:
				case HTTPREQ_POST_FORM:
				case HTTPREQ_POST_MIME:
					request = "POST";
					break;
				case HTTPREQ_PUT:
					request = "PUT";
					break;
				default: /* this should never happen */
				case HTTPREQ_GET:
					request = "GET";
					break;
				case HTTPREQ_HEAD:
					request = "HEAD";
					break;
			}
		}
	}
	*method = request;
	*reqp = httpreq;
}

/*
 * HTTP handler interface.
 */
const struct Curl_handler Curl_handler_http = {
  "http",													/* scheme */
  Curl_http_setup_conn,										/* setup_connection */
  Curl_http,												/* do_it */
  Curl_http_done,											/* done */
  ZERO_NULL,												/* do_more */
  Curl_http_connect,										/* connect_it */
  ZERO_NULL,												/* connecting */
  ZERO_NULL,												/* doing */
  ZERO_NULL,												/* disconnect */
  Curl_http_write_resp,										/* write_resp */
  Curl_http_write_resp_hd,									/* write_resp_hd */
  ZERO_NULL,												/* connection_check */
  ZERO_NULL,												/* attach connection */
  PORT_HTTP,												/* defport */
  CURLPROTO_HTTP,											/* protocol */
  CURLPROTO_HTTP,											/* family */
  PROTOPT_CREDSPERREQUEST | PROTOPT_USERPWDCTRL             /* flags */
};

#ifdef USE_SSL
/*
 * HTTPS handler interface.
 */
const struct Curl_handler Curl_handler_https = {
  "https",													/* scheme */
  Curl_http_setup_conn,										/* setup_connection */
  Curl_http,												/* do_it */
  Curl_http_done,											/* done */
  ZERO_NULL,												/* do_more */
  Curl_http_connect,										/* connect_it */
  NULL,														/* connecting */
  ZERO_NULL,												/* doing */
  ZERO_NULL,												/* disconnect */
  Curl_http_write_resp,										/* write_resp */
  Curl_http_write_resp_hd,									/* write_resp_hd */
  ZERO_NULL,												/* connection_check */
  ZERO_NULL,												/* attach connection */
  PORT_HTTPS,												/* defport */
  CURLPROTO_HTTPS,											/* protocol */
  CURLPROTO_HTTP,											/* family */
  PROTOPT_SSL | PROTOPT_CREDSPERREQUEST |					/* flags */
  PROTOPT_ALPN | PROTOPT_USERPWDCTRL
};
#endif