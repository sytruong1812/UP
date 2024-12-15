#include "curl.h"

CURL* curl_easy_init() {
    CURL* handle = new CURL();
    if (handle) {
        handle->timeout = 0;
    }
    return handle;
}

void curl_easy_cleanup(CURL* handle) {
    if (handle) {
        delete handle;
    }
}

CURLcode curl_easy_setopt(CURL* handle, CURLoption option, void* value) {
    if (!handle || !value) return CURL_ERROR_NULL_POINTER;

    switch (option) {
    case CURLOPT_URL:
        handle->url = std::wstring(static_cast<wchar_t*>(value));
        break;
    case CURLOPT_TIMEOUT:
        handle->timeout = *(static_cast<DWORD*>(value));
        break;
    case CURLOPT_HTTPHEADER:
        handle->custom_headers = std::wstring(static_cast<wchar_t*>(value));
        break;
    default:
        return CURL_ERROR_INVALID_OPTION;
    }
    return CURL_OK;
}

CURLcode curl_easy_perform(CURL* handle) {
    if (!handle || handle->url.empty()) {
        std::wcerr << L"Error: URL not set.\n";
        return CURL_ERROR_NULL_POINTER;
    }

    HINTERNET hSession = WinHttpOpen(L"CustomUserAgent/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    if (!hSession) {
        std::wcerr << L"WinHttpOpen failed.\n";
        return CURL_ERROR_WINHTTP_FAILURE;
    }

    HINTERNET hConnect = WinHttpConnect(hSession, handle->url.c_str(), INTERNET_DEFAULT_HTTP_PORT, 0);
    if (!hConnect) {
        std::wcerr << L"WinHttpConnect failed.\n";
        WinHttpCloseHandle(hSession);
        return CURL_ERROR_WINHTTP_FAILURE;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", NULL,
        NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        0);
    if (!hRequest) {
        std::wcerr << L"WinHttpOpenRequest failed.\n";
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return CURL_ERROR_WINHTTP_FAILURE;
    }

    // Set custom headers if provided
    if (!handle->custom_headers.empty()) {
        WinHttpAddRequestHeaders(hRequest, handle->custom_headers.c_str(),
            (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD);
    }

    // Send the request
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        std::wcerr << L"WinHttpSendRequest failed.\n";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return CURL_ERROR_WINHTTP_FAILURE;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        std::wcerr << L"WinHttpReceiveResponse failed.\n";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return CURL_ERROR_WINHTTP_FAILURE;
    }

    std::wcout << L"Request to " << handle->url << L" completed successfully.\n";

    // Clean up
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return CURL_OK;
}
