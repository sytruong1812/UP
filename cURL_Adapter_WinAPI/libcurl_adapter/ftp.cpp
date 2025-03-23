#include "ftp.h"
#include "utils.h"
#include "urldata.h"

static void CALLBACK ftp_status_callback(HINTERNET, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD)
{
    struct Curl_easy* data = (struct Curl_easy*)dwContext;
    struct connectdata* conn = data->conn;
    struct ftp_conn* ftp = conn->proto.ftpc;
    if (!ftp) {
        return;
    }

    switch (dwInternetStatus) 
    {
    case INTERNET_STATUS_SENDING_REQUEST:
        ftp->transfer.inProgress = true;
        break;
    case INTERNET_STATUS_REQUEST_COMPLETE:
        ftp->transfer.inProgress = false;
        ftp->result = CURLE_OK;
        break;
    case INTERNET_STATUS_RESPONSE_RECEIVED:
        if (lpvStatusInformation) {
            DWORD* bytes = (DWORD*)lpvStatusInformation;
            ftp->transfer.position += *bytes;
        }
        break;
    }
}

static CURLcode ftp_setup_connection(struct Curl_easy* data, struct connectdata* conn)
{
    // Verify parameters
    if (!data || !conn)
    {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    struct ftp_conn* ftp = (struct ftp_conn*)calloc(1, sizeof(struct ftp_conn));
    if (!ftp)
    {
        return CURLE_OUT_OF_MEMORY;
    }
    // Initialize WinInet
    ftp->hInternet = InternetOpenW(curlx_convert_UTF8_to_wchar(data->set.str[STRING_USERAGENT]),
                                    INTERNET_OPEN_TYPE_DIRECT,
                                    NULL, NULL,
                                    0); // Changed from INTERNET_FLAG_ASYNC to 0 for synchronous ops
    if (!ftp->hInternet) 
    {
        free(ftp);
        return CURLE_FAILED_INIT;
    }

    // Set default options
    ftp->options.passive = true;
    ftp->options.ascii = false;
    ftp->options.method = FTPFILE_MULTICWD;
    ftp->state = FTP_STOP;

    // Configure timeouts
    DWORD timeout = 60000; // 60 seconds
    InternetSetOptionW(ftp->hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionW(ftp->hInternet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionW(ftp->hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

    conn->proto.ftpc = ftp;
    return CURLE_OK;
}

static CURLcode ftp_connect(struct Curl_easy* data, bool* done)
{
    if (!data || !done || !data->conn) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    struct connectdata* conn = data->conn;
    struct ftp_conn* ftp = conn->proto.ftpc;

    if (!ftp) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    DWORD flags = ftp->options.passive ? INTERNET_FLAG_PASSIVE : 0;

    // Add SSL if required
    if (conn->handler->flags & PROTOPT_SSL) {
        flags |= INTERNET_FLAG_SECURE;
        ftp->options.ssl = true;
    }

    // Establish connection
    ftp->hFtp = InternetConnectW(ftp->hInternet,
                                curlx_convert_UTF8_to_wchar(conn->host.name),
                                (INTERNET_PORT)conn->conn_to_port,
                                curlx_convert_UTF8_to_wchar(conn->user),
                                curlx_convert_UTF8_to_wchar(conn->passwd),
                                INTERNET_SERVICE_FTP,
                                flags,
                                (DWORD_PTR)data);
    if (!ftp->hFtp) 
    {
        return CURLE_WININET_FTP_CONNECT_ERROR;
    }

    // Configure SSL if needed
    if (ftp->options.ssl) {
        DWORD securityFlags = SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
         InternetSetOption(ftp->hFtp,
                            INTERNET_OPTION_SECURITY_FLAGS,
                            &securityFlags,
                            sizeof(securityFlags));
    }

    *done = true;
    return CURLE_OK;
}

static CURLcode ftp_do(struct Curl_easy* data, bool* done)
{
    if (!data || !done || !data->conn)
    {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    struct connectdata* conn = data->conn;
    struct ftp_conn* ftp = conn->proto.ftpc;

    if (!ftp) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    CURLcode result = CURLE_OK;

    // Set transfer type using FTP TYPE command
    const wchar_t* typeCmd;
    if (ftp->options.ascii) {
        typeCmd = L"TYPE A\r\n";
    }
    else {
        typeCmd = L"TYPE I\r\n";
    }

    BOOL typeResult;
    DWORD bytesReturned = 0;

    // Send FTP command directly using InternetWriteFile and InternetReadFile
    typeResult = InternetWriteFile(
        ftp->hFtp,
        typeCmd,
        wcslen(typeCmd) * sizeof(wchar_t),
        &bytesReturned
    );

    if (!typeResult)
    {
        return CURLE_WININET_FTP_TRANSFER_TYPE_ERROR;
    }

    // Read response if needed
    wchar_t response[1024];
    DWORD bytesRead;
    if (InternetReadFile(ftp->hFtp, response, sizeof(response), &bytesRead))
    {
        // Check response if needed
    }

    // Use ftp->state instead of data->set.ftp_command
    switch (ftp->state)
    {
    case FTP_CWD:
        if (!FtpSetCurrentDirectoryW(ftp->hFtp, ftp->currentPath))
        {
            result = CURLE_WININET_FTP_CWD_ERROR;
        }
        ftp->state = FTP_STOP;
        break;

    case FTP_LIST:
    {
        WIN32_FIND_DATAW findData;
        HINTERNET hFind = FtpFindFirstFileW(ftp->hFtp,
            L"*",
            &findData,
            INTERNET_FLAG_RELOAD, // Force refresh
            (DWORD_PTR)data);
        if (!hFind)
        {
            result = CURLE_WININET_FTP_DIR_LIST_ERROR;
        }
        else
        {
            // Process directory listing
            do {
                // Here you can handle each file entry
                // For example, add to your result buffer
            } while (InternetFindNextFileW(hFind, &findData));

            InternetCloseHandle(hFind);
        }
        ftp->state = FTP_STOP;
    }
    break;

    case FTP_STOP:
        // Nothing to do
        break;

    default:
        result = CURLE_BAD_FUNCTION_ARGUMENT;
        break;
    }

    *done = true;
    return result;
}

static CURLcode ftp_doing(struct Curl_easy* data, bool* dophase_done)
{
    if (!data || !dophase_done || !data->conn) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    struct connectdata* conn = data->conn;
    struct ftp_conn* ftp = conn->proto.ftpc;

    if (!ftp) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }

    *dophase_done = !ftp->transfer.inProgress;
    return ftp->result;
}

static CURLcode ftp_done(struct Curl_easy* data, CURLcode status, bool premature)
{
    if (!data || !data->conn) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    struct connectdata* conn = data->conn;
    struct ftp_conn* ftp = conn->proto.ftpc;

    if (ftp) {
        ftp->transfer.inProgress = false;
        ftp->result = status;
    }

    return CURLE_OK;
}

static CURLcode ftp_disconnect(struct Curl_easy* data, struct connectdata* conn, bool dead_connection)
{
    if (!conn) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }

    struct ftp_conn* ftp = conn->proto.ftpc;

    if (ftp) 
    {
        if (ftp->hFtp) {
            InternetCloseHandle(ftp->hFtp);
            ftp->hFtp = NULL;
        }
        if (ftp->hInternet) {
            InternetCloseHandle(ftp->hInternet);
            ftp->hInternet = NULL;
        }

        free(ftp->currentPath);
        free(ftp);
        conn->proto.ftpc = NULL;
    }

    return CURLE_OK;
}

static CURLcode ftp_do_more(struct Curl_easy* data, int* completed)
{
    if (!data || !completed || !data->conn) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }

    struct connectdata* conn = data->conn;
    struct ftp_conn* ftp = conn->proto.ftpc;

    if (!ftp) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }

    *completed = !ftp->transfer.inProgress;
    return ftp->result;
}

static CURLcode ftp_multi_statemach(struct Curl_easy* data, bool* done)
{
    if (!data || !done || !data->conn) {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    struct connectdata* conn = data->conn;
    struct ftp_conn* ftp = conn->proto.ftpc;

    if (!ftp) 
    {
        return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    *done = !ftp->transfer.inProgress;
    return ftp->result;
}

// Curl handler definitions
const struct Curl_handler Curl_handler_ftp = {
    "ftp",                          /* scheme */
    ftp_setup_connection,           /* setup_connection */
    ftp_do,                         /* do_it */
    ftp_done,                       /* done */
    ftp_do_more,                    /* do_more */
    ftp_connect,                    /* connect_it */
    ftp_multi_statemach,            /* connecting */
    ftp_doing,                      /* doing */
    ftp_disconnect,                 /* disconnect */
    ZERO_NULL,                      /* write_resp */
    ZERO_NULL,                      /* write_resp_hd */
    ZERO_NULL,                      /* connection_check */
    ZERO_NULL,                      /* attach connection */
    PORT_FTP,                       /* defport */
    CURLPROTO_FTP,                  /* protocol */
    CURLPROTO_FTP,                  /* family */
    PROTOPT_DUAL | PROTOPT_CLOSEACTION | PROTOPT_NEEDSPWD |
    PROTOPT_NOURLQUERY | PROTOPT_PROXY_AS_HTTP |
    PROTOPT_WILDCARD                /* flags */
};

#ifdef USE_SSL
const struct Curl_handler Curl_handler_ftps = {
    "ftps",                         /* scheme */
    ftp_setup_connection,           /* setup_connection */
    ftp_do,                         /* do_it */
    ftp_done,                       /* done */
    ftp_do_more,                    /* do_more */
    ftp_connect,                    /* connect_it */
    ftp_multi_statemach,            /* connecting */
    ftp_doing,                      /* doing */
    ftp_disconnect,                 /* disconnect */
    ZERO_NULL,                      /* write_resp */
    ZERO_NULL,                      /* write_resp_hd */
    ZERO_NULL,                      /* connection_check */
    ZERO_NULL,                      /* attach connection */
    PORT_FTPS,                      /* defport */
    CURLPROTO_FTPS,                 /* protocol */
    CURLPROTO_FTP,                  /* family */
    PROTOPT_SSL | PROTOPT_DUAL | PROTOPT_CLOSEACTION |
    PROTOPT_NEEDSPWD | PROTOPT_NOURLQUERY | PROTOPT_WILDCARD /* flags */
};
#endif