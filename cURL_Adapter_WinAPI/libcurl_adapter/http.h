#pragma once
#include "curl.h"
#include "urlapi.h"
#include "urldata.h"

extern const struct Curl_handler Curl_handler_http;
#ifdef USE_SSL
extern const struct Curl_handler Curl_handler_https;
#endif

typedef enum {
	HTTPREQ_GET,
	HTTPREQ_POST,
	HTTPREQ_POST_FORM, /* we make a difference internally */
	HTTPREQ_POST_MIME, /* we make a difference internally */
	HTTPREQ_PUT,
	HTTPREQ_HEAD
} Curl_HttpReq;

CURLcode Curl_http_setup_conn(struct Curl_easy* data, struct connectdata* conn);

CURLcode Curl_http_connect(struct Curl_easy* data, bool* done);

CURLcode Curl_http(struct Curl_easy* data, bool* done);

CURLcode Curl_http_write_resp(struct Curl_easy* data, const char* buf, size_t blen, bool is_eos);

CURLcode Curl_http_write_resp_hd(struct Curl_easy* data, const char* hd, size_t hdlen, bool is_eos);

CURLcode Curl_http_done(struct Curl_easy* data, CURLcode status, bool premature);

void Curl_http_method(struct Curl_easy* data, const char** method, Curl_HttpReq* reqp);

void Check_Authentication(struct Curl_easy* data);