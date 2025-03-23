#pragma once
#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <string>

class WinHTTPCurl
{
private:
    HINTERNET hSession = NULL;
public:
    WinHTTPCurl() {
        hSession = WinHttpOpen(L"WinHTTPCurl", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    }

    ~WinHTTPCurl() {
        if (hSession) {
            WinHttpCloseHandle(hSession);
        }
    }

    std::string get(const std::wstring& url) {
        URL_COMPONENTS urlComp = { sizeof(URL_COMPONENTS) };
        urlComp.lpszHostName = new wchar_t[256];
        urlComp.dwHostNameLength = 256;
        urlComp.lpszUrlPath = new wchar_t[256];
        urlComp.dwUrlPathLength = 256;

        WinHttpCrackUrl(url.c_str(), 0, 0, &urlComp);

        HINTERNET hConnect = WinHttpConnect(hSession, urlComp.lpszHostName, urlComp.nPort, 0);
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComp.lpszUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

        WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
        WinHttpReceiveResponse(hRequest, NULL);

        std::string response;
        char buffer[4096];
        DWORD bytesRead;

        while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
            response.append(buffer, bytesRead);
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        delete[] urlComp.lpszHostName;
        delete[] urlComp.lpszUrlPath;

        return response;
    }

    bool post(const std::wstring& url, const std::string& data) {

    }
};

