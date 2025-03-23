#pragma once
#pragma once
#include "urldata.h"

#define STRERROR_LEN 256

const char* Curl_strerror(int err, char* buf, size_t buflen);

#if defined(_WIN32) || defined(_WIN32_WCE)

const char* Curl_winapi_strerror(DWORD err, char* buf, size_t buflen);

#endif