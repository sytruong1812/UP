#include "ws.h"
#include "http.h"

const struct Curl_handler Curl_handler_ws = {
  "WS",                                 /* scheme */
  Curl_http_setup_conn,                 /* setup_connection */
  Curl_ws,                              /* do_it */
  Curl_http_done,                       /* done */
  ZERO_NULL,                            /* do_more */
  Curl_http_connect,                    /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  ws_disconnect,                        /* disconnect */
  Curl_http_write_resp,                 /* write_resp */
  Curl_http_write_resp_hd,              /* write_resp_hd */
  ZERO_NULL,                            /* connection_check */
  ZERO_NULL,                            /* attach connection */
  PORT_HTTP,                            /* defport */
  CURLPROTO_WS,                         /* protocol */
  CURLPROTO_HTTP,                       /* family */
  PROTOPT_CREDSPERREQUEST |             /* flags */
  PROTOPT_USERPWDCTRL
};

#ifdef USE_SSL
const struct Curl_handler Curl_handler_wss = {
  "WSS",                                /* scheme */
  Curl_http_setup_conn,                 /* setup_connection */
  Curl_ws,                              /* do_it */
  Curl_http_done,                       /* done */
  ZERO_NULL,                            /* do_more */
  Curl_http_connect,                    /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  ws_disconnect,                        /* disconnect */
  Curl_http_write_resp,                 /* write_resp */
  Curl_http_write_resp_hd,              /* write_resp_hd */
  ZERO_NULL,                            /* connection_check */
  ZERO_NULL,                            /* attach connection */
  PORT_HTTPS,                           /* defport */
  CURLPROTO_WSS,                        /* protocol */
  CURLPROTO_HTTP,                       /* family */
  PROTOPT_SSL | PROTOPT_CREDSPERREQUEST | /* flags */
  PROTOPT_USERPWDCTRL
};
#endif

CURLcode Curl_ws(struct Curl_easy* data, bool* done)
{
	CURLUcode uc;
	CURLcode result = CURLE_OK;
	char* path = NULL;
	wchar_t* sub_protocol = NULL;
	struct SingleRequest* req = &data->req;

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
	req->hRequest = WinHttpOpenRequest(req->hConnect,
		(LPCWSTR)(L"GET"),
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
	/*=========================[Add WebSocket upgrade]=========================*/
#pragma prefast(suppress:6387, "WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET does not take any arguments.")
	if (!WinHttpSetOption(req->hRequest,
		WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,	// Add WebSocket upgrade to our HTTP request
		0,
		0))
	{
		result = CURLE_WINHTTP_SET_OPTION_ERROR;
		goto exit;
	}
	// Allocate temporary memory for header string

	if (sub_protocol != NULL)
	{
		size_t headerLen = wcslen(sub_protocol) + 0x100;
		wchar_t* ws_protocol = (wchar_t*)malloc(headerLen * sizeof(wchar_t));
		if (ws_protocol == NULL)
		{
			result = CURLE_FAILED_TO_ALLOCATE;
			goto exit;
		}
		// Build the "Sec-WebSocket-Protocol" header
		swprintf_s(ws_protocol, headerLen, L"%ls %ls", L"Sec-WebSocket-Protocol:", sub_protocol);
		// Add to the request handle
		if (!WinHttpAddRequestHeaders(req->hRequest,
			ws_protocol, -1L,
			WINHTTP_ADDREQ_FLAG_ADD))
		{
			result = CURLE_WINHTTP_ADD_REQUEST_HEADER_ERROR;
			goto exit;
		}
		if (ws_protocol)
		{
			free(ws_protocol);
		}
	}
	/*===================[Send the WebSocket upgrade request]======================*/
	if (!WinHttpSendRequest(req->hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, 0, 0, 0, 0))
	{
		result = CURLE_WINHTTP_SEND_REQUEST_ERROR;
		goto exit;
	}
	/*=====================[Receive response from the server]=======================*/
	while (TRUE)
	{
		Check_Authentication(data);
		if (WinHttpReceiveResponse(req->hRequest, 0))
		{
			break;
		}
		req->req_good = FALSE;	// Continue to send a request until status code is not 401 or 407
		result = CURLE_WINHTTP_RECV_RESPONSE_ERROR;
		goto exit;
	}
	/*=======================[Finally complete the upgrade]==========================*/
	req->hWebSocket = WinHttpWebSocketCompleteUpgrade(req->hRequest, (DWORD_PTR)0);
	if (req->hWebSocket == 0)
	{
		result = CURLE_WINHTTP_WS_UPGRADE_ERROR;
		goto exit;
	}

exit:
	if (path) free(path);
	if (query) free(query);
	if (full_path) free(full_path);
	return result;
}

CURLcode ws_disconnect(struct Curl_easy* data, struct connectdata* conn, bool dead_connection) {
	CURLcode result = CURLE_OK;
	struct SingleRequest* req = &data->req;

	int last_error = WinHttpWebSocketClose(req->hWebSocket,
											WINHTTP_WEB_SOCKET_SERVER_ERROR_CLOSE_STATUS,
											NULL, 
											0);
	if (last_error != NO_ERROR)
	{
		result = CURLE_WINHTTP_WS_CLOSE_ERROR;
	}
	return result;
}

static bool ws_close(struct Curl_easy* data, WINHTTP_WEB_SOCKET_CLOSE_STATUS status, CHAR* reason)
{
	BOOL result = TRUE;
	struct SingleRequest* req = &data->req;
	// Length of reason in bytes
	DWORD reason_len = (reason == NULL ? 0 : (DWORD)strlen(reason));

	//Note: WinHttpWebSocketClose does not close this handle. 
	DWORD errorCode = WinHttpWebSocketClose(req->hWebSocket, status, reason, reason_len);
	if (errorCode != NO_ERROR)
	{
		result = FALSE;
	}
	// Close the connection handles
	WinHttpCloseHandle(req->hWebSocket);
	WinHttpCloseHandle(req->hRequest);
	WinHttpCloseHandle(req->hConnect);

	// Set class variables to NULL
	req->hWebSocket = NULL;
	req->hRequest = NULL;
	req->hConnect = NULL;

	return result;
}

CURL_EXTERN CURLcode curl_ws_recv(CURL* curl, void* buffer, size_t buflen, size_t* nread)
{
	CURLcode result = CURLE_OK;
	struct Curl_easy* data = (Curl_easy*)curl;
	struct SingleRequest* req = &data->req;

	char* pbuffer = (char*)malloc(READBUFFER_SIZE);
	if (pbuffer == NULL)
	{
		return CURLE_FAILED_TO_ALLOCATE;
	}
	while (true)
	{
		DWORD dwBytesReceived = 0;
		DWORD dwTotalBytesReceived = 0;
		WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;

		/*==================[Get data from the server]===================*/
		do
		{
			if (WinHttpWebSocketReceive(req->hWebSocket,
				pbuffer,
				READBUFFER_SIZE - dwTotalBytesReceived,
				&dwBytesReceived,
				&bufferType) != NO_ERROR)
			{
				ws_close(data, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL);
				result = CURLE_WINHTTP_WS_RECV_DATA_ERROR;
				goto exit;
			}
			dwTotalBytesReceived += dwBytesReceived;

		} while ((bufferType == WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE)
			|| (bufferType == WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE));

		/*=====================[Check buffer type]=======================*/
		if (bufferType == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE)
		{
			// Server wants to close the connection
			unsigned short statusCode = 0;
			char* pCloseReason = NULL;
			DWORD dwReasonLength = 0;
			DWORD dwReasonLengthNeeded = 0;

			// Get the length of the close reason
			WinHttpWebSocketQueryCloseStatus(req->hWebSocket, &statusCode, 0, 0, &dwReasonLengthNeeded);
			if (dwReasonLengthNeeded != 0)
			{
				dwReasonLength = dwReasonLengthNeeded;
				pCloseReason = (char*)malloc(dwReasonLength);
				WinHttpWebSocketQueryCloseStatus(req->hWebSocket, &statusCode, pCloseReason, dwReasonLength, &dwReasonLengthNeeded);
			}
			// NOTE: Close reason is not guaranteed to be human readable
			infof(data, "%s%d%s", "Received: (CLOSE) - Status Code: ", statusCode, ", Reason: ");

			// Gracefully close the connection
			ws_close(data, (WINHTTP_WEB_SOCKET_CLOSE_STATUS)statusCode, pCloseReason);
			if (pCloseReason)
			{
				infof(data, "%s", pCloseReason);
				free(pCloseReason);
			}
			goto exit;
		}
		else if (bufferType == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE)
		{
			// We got a UTF8 message
			// NOTE: The WebSocket does not set a terminating NULL character for a string
			pbuffer[dwBytesReceived] = '\0';
			buffer = pbuffer;
			buflen = dwTotalBytesReceived;
			break;
		}
	}
exit:
	if (pbuffer)
	{
		free(pbuffer);
	}
	return result;
}

CURL_EXTERN CURLcode curl_ws_send(CURL* curl, const void* buffer, size_t buflen, size_t* nsent)
{
	struct Curl_easy* data = (Curl_easy*)curl;
	struct SingleRequest* req = &data->req;

	if (WinHttpWebSocketSend(req->hWebSocket,
							WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
							(PVOID)buffer,
							(DWORD)buflen) != NO_ERROR)
	{
		nsent = 0;
		return CURLE_WINHTTP_WS_SEND_DATA_ERROR;
	}
	nsent = &buflen;
	return CURLE_OK;
}

