#ifndef CURL_WINHTTP_H
#define CURL_WINHTTP_H

#include <windows.h>
#include <winhttp.h>
#include <string>
#include <iostream>

// Define error codes
typedef enum {
    CURL_OK = 0,
    CURL_ERROR_INVALID_OPTION = 1,
    CURL_ERROR_NULL_POINTER = 2,
    CURL_ERROR_WINHTTP_FAILURE = 3
} CURLcode;

// Define CURL handle structure
typedef struct {
    std::wstring url;
    DWORD timeout;
    std::wstring custom_headers;
} CURL;

// Define CURLOPT options
typedef enum {
    CURLOPT_URL,
    CURLOPT_TIMEOUT,
    CURLOPT_HTTPHEADER
} CURLoption;

// Function prototypes
CURL* curl_easy_init();
void curl_easy_cleanup(CURL* handle);
CURLcode curl_easy_setopt(CURL* handle, CURLoption option, void* value);
CURLcode curl_easy_perform(CURL* handle);

#endif // CURL_WINHTTP_H
