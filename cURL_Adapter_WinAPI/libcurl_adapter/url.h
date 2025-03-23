#pragma once
#include "curl.h"
#include "utils.h"

#define MAX_URL_LEN 0xffff

#define CURL_DEFAULT_USER "anonymous"
#define CURL_DEFAULT_PASSWORD "ftp@example.com"

void Curl_freeset(struct Curl_easy* data);
CURLcode Curl_open(struct Curl_easy** curl);
CURLcode Curl_init_do(struct Curl_easy* data, struct connectdata* conn);
CURLcode Curl_uc_to_curlcode(CURLUcode uc);
CURLcode Curl_close(struct Curl_easy** datap);
CURLcode Curl_setup_conn(struct Curl_easy* data, bool* protocol_done);
CURLcode Curl_connect(struct Curl_easy*, bool* async, bool* protocol_connect);
bool Curl_on_disconnect(struct Curl_easy* data, struct connectdata* conn, bool aborted);
CURLcode Curl_init_userdefined(struct Curl_easy* data);
CURLcode Curl_parse_login_details(const char* login, const size_t len, char** userptr, char** passwdptr, char** optionsptr);
const struct Curl_handler* Curl_get_scheme_handler(const char* scheme);
const struct Curl_handler* Curl_getn_scheme_handler(const char* scheme, size_t len);
