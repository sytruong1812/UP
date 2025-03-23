#pragma once

#include <windows.h>
#include <wininet.h>
#include <iostream>
#include <string>

class WinINetCurl {
private:
    HINTERNET hInternet;
public:
    WinINetCurl() {
        hInternet = InternetOpenW(L"WinINetCurl", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    }

    ~WinINetCurl() {
        if (hInternet) {
            InternetCloseHandle(hInternet);
        }
    }

    std::string get(const std::string& url) {
        HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hConnect) {
            return "Error opening URL";
        }

        char buffer[4096];
        DWORD bytesRead;
        std::string response;

        while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead != 0) {
            buffer[bytesRead] = '\0';
            response += buffer;
        }

        InternetCloseHandle(hConnect);
        return response;
    }

    bool post(const std::string& url, const std::string& data) {
        HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hConnect) {
            return false;
        }

        DWORD bytesWritten;
        BOOL result = InternetWriteFile(hConnect, data.c_str(), data.size(), &bytesWritten);
        InternetCloseHandle(hConnect);
        return result;
    }
};


int main() {
    WinINetCurl curl;

    // Example GET request
    std::string response = curl.get("http://www.example.com");
    std::cout << "GET Response: " << response << std::endl;

    // Example POST request
    bool success = curl.post("http://www.example.com/post", "param1=value1&param2=value2");
    std::cout << "POST Success: " << success << std::endl;

    return 0;
}