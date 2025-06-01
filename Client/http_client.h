#pragma once
#include <map>
#include <vector>
#include <sstream>
#define WININET
#include <windows.h>
#ifdef WININET
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#else
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#endif
#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")

#include "data_transform.h"

#define KB              1024
#define MB              (KB * KB)  // 1 MB = 1024 * 1024 = 1.048.576
#define GB              (MB * MB)  // 1 GB = 1.048.576 * 1.048.576 = 1.073.741.824
#define BUFFER_SIZE		10 * KB
#define USER_AGENT      L"File storage client"

namespace NetworkOperations 
{
    class HttpHeaders;
    class HttpResponse;

    class HttpClient 
    {
    public:
        HttpClient();
        ~HttpClient();
        BOOL Connect(const std::wstring& url);
        BOOL Connect(const std::wstring& url, PCCERT_CONTEXT cert);
        BOOL Connect(const std::wstring& host, WORD port, BOOL security);
        BOOL Connect(const std::wstring& host, WORD port, BOOL security, PCCERT_CONTEXT cert);
        void OptionKeepConnect(BOOL enable);
        void OptionRecvTimeOut(DWORD milliseconds);
        void OptionSendTimeOut(DWORD milliseconds);
        void OptionConnectTimeOut(DWORD milliseconds);
        BOOL Disconnect();
        LPVOID OpenRequest(const std::wstring& verb, const std::wstring& path, const std::wstring& headers);
        BOOL SendRequest(LPVOID hRequest, const void* data, const size_t& length);
        BOOL CloseRequest(LPVOID hRequest);
        //Basic HTTP/HTTPS verb methods
        HttpResponse Head(const std::wstring& path, const HttpHeaders& headers);
        HttpResponse Get(const std::wstring& path, const HttpHeaders& headers);
        HttpResponse Post(const std::wstring& path, const HttpHeaders& headers, const std::string& data);
        HttpResponse Post(const std::wstring& path, const HttpHeaders& headers, const void* data, size_t length);
        HttpResponse Post(const std::wstring& path, const HttpHeaders& headers, IDataTransform* transform, const void* data, size_t length);
        HttpResponse Put(const std::wstring& path, const HttpHeaders& headers, const std::string& data);
        HttpResponse Put(const std::wstring& path, const HttpHeaders& headers, const void* data, size_t length);
        HttpResponse Put(const std::wstring& path, const HttpHeaders& headers, IDataTransform* transform, const void* data, size_t length);
        HttpResponse Patch(const std::wstring& path, const HttpHeaders& headers);
        HttpResponse Delete(const std::wstring& path, const HttpHeaders& headers);
        HttpResponse Select(const std::wstring& path, const HttpHeaders& headers);
        HttpResponse Options(const std::wstring& path, const HttpHeaders& headers);
        HttpResponse Trace(const std::wstring& path, const HttpHeaders& headers);
        //Advance HTTP/HTTPS methods
        HttpResponse DownloadFile(const std::wstring& path);
        HttpResponse UploadFile(const std::wstring& path);

    private:
        DWORD state = 0;
        WCHAR scheme[0x20];
        WCHAR hostName[0x100] = L"localhost";
        WORD portNumber = INTERNET_DEFAULT_HTTP_PORT;
        BOOL keepConnect = FALSE;
        PCCERT_CONTEXT pCertContext = NULL;

        HINTERNET hSession = NULL;
        HINTERNET hConnect = NULL;
    };

    class HttpHeaders 
    {
    public:
        HttpHeaders() = default;
        HttpHeaders(const std::string& name, const std::string& value) {
            SetHeader(name, value);
        }
        HttpHeaders(const std::wstring& name, const std::wstring& value) {
            SetHeader(name, value);
        }
        void SetHeader(const std::string& name, const std::string& value) {
            pairs[name] = value;
        }
        void SetHeader(const std::wstring& name, const std::wstring& value) {
            pairs[Helper::StringHelper::convertWideStringToString(name)] = Helper::StringHelper::convertWideStringToString(value);
        }
        void RemoveHeader(const std::string& name) {
            pairs.erase(name);
        }
        void RemoveHeader(const std::wstring& name) {
            pairs.erase(Helper::StringHelper::convertWideStringToString(name));
        }
        std::string GetFormatString() const {
            return Format();
        }
        std::wstring GetFormatWstring() const {
            return Helper::StringHelper::convertStringToWideString(Format());
        }
        std::string GetHeader(const std::string& name) const {
            auto it = pairs.find(name);
            return it != pairs.end() ? it->second : "";
        }
        std::wstring GetHeader(const std::wstring& name) const {
            auto it = pairs.find(Helper::StringHelper::convertWideStringToString(name));
            return it != pairs.end() ? Helper::StringHelper::convertStringToWideString(it->second) : L"";
        }
        const std::map<std::string, std::string>& GetHeaders() const {
            return pairs;
        }
    private:
        std::string Format() const 
        {
            std::ostringstream stream;
            for (const auto& pair : pairs)
            {
                stream << pair.first << ":" << pair.second << "\r\n";
            }
            return stream.str();
        }
        std::map<std::string, std::string> pairs;
    };

    class HttpResponse 
    {
    public:
        HttpResponse() : statusCode(0), contentString("Server is currently unavailable! Please try again later.") {}
        HttpResponse(DWORD statusCode, const std::string& content, const std::string& headers)
            : statusCode(statusCode), contentString(content), headerString(headers) {}
        HttpResponse(HINTERNET hRequest, BOOL closeRequest = TRUE)
            : statusCode(0)
        {
            if (hRequest)
            {
                statusCode = GetStatusCode(hRequest);
                headerString = ReadResponseHeader(hRequest);
                if (!headerString.empty())
                {
                    headerPairs = ParseResponseHeaders(headerString);
                }
                contentString = ReadResponseContent(hRequest);
                if (closeRequest)
                {
#ifdef WININET
                    InternetCloseHandle(hRequest);
#else
                    WinHttpCloseHandle(hRequest);
#endif
                }
            }
        }
        DWORD GetStatusCode() { return statusCode; }
        std::string GetHeaderString() { return headerString; }
        std::wstring GetHeaderWString() { return Helper::StringHelper::convertStringToWideString(headerString); }
        std::string GetContentString() { return contentString; }
        std::wstring GetContentWString() { return Helper::StringHelper::convertStringToWideString(contentString); }
        std::map<std::string, std::string> GetHeaderPairs() { return headerPairs; }

        BOOL CheckContentIsJson();
        std::string GetResponseHeader(const std::string& key);
        std::wstring GetResponseHeader(const std::wstring& key);
    private:
        DWORD statusCode;
        std::string headerString;
        std::string contentString;
        std::map<std::string, std::string> headerPairs;

        DWORD GetStatusCode(HINTERNET hRequest);
        std::string ReadResponseHeader(HINTERNET hRequest);
        std::string ReadResponseContent(HINTERNET hRequest);
        std::map<std::string, std::string> ParseResponseHeaders(const std::string& headerStr);
    };

    class CertificateManager 
    {
    public:
        static CertificateManager* Instance()
        {
            if (!s_instance)
            {
                s_instance = new CertificateManager;
            }
            return s_instance;
        }
        BOOL RemoveCertificateFromStore(PCCERT_CONTEXT pCertContext);
        PCCERT_CONTEXT GetCertificateFromStore(const std::wstring& subjectName, const std::wstring& store, DWORD branch);
        PCCERT_CONTEXT ImportLoadCertFromFile(const std::wstring& path, const std::wstring& password, const std::wstring& subjectName);
    private:
        static CertificateManager* s_instance;
    };

}
