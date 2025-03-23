#pragma once

#include "setup.h"
#include "urldata.h"

#if !defined(CURL_DISABLE_WEBSOCKETS) && !defined(CURL_DISABLE_HTTP)

extern const struct Curl_handler Curl_handler_ws;
#ifdef USE_SSL
extern const struct Curl_handler Curl_handler_wss;
#endif
#else
#define Curl_ws_request(x,y) CURLE_OK
#define Curl_ws_free(x) Curl_nop_stmt
#endif

struct websocket 
{
	struct Curl_easy* data; /* used for write callback handling */
};

CURLcode Curl_ws(struct Curl_easy* data, bool* done);

CURL_EXTERN CURLcode curl_ws_recv(CURL* curl, void* buffer, size_t buflen, size_t* nread);

CURL_EXTERN CURLcode curl_ws_send(CURL* curl, const void* buffer, size_t buflen, size_t* nsent);

CURLcode ws_disconnect(struct Curl_easy* data, struct connectdata* conn, bool dead_connection);