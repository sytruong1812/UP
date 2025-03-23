#include "curl.h"
#include "error.h"
#include "utils.h"

#if defined(_WIN32) || defined(_WIN32_WCE)
#define PRESERVE_WINDOWS_ERROR_CODE
#endif

const char* curl_easy_strerror(CURLcode error)
{
#ifndef CURL_DISABLE_VERBOSE_STRINGS
    switch (error)
    {
        case CURLE_OK:
            return "No error";

        case CURLE_UNSUPPORTED_PROTOCOL:
            return "Unsupported protocol";

        case CURLE_FAILED_INIT:
            return "Failed initialization";

        case CURLE_URL_MALFORMAT:
            return "URL using bad/illegal format or missing URL";

        case CURLE_NOT_BUILT_IN:
            return "A requested feature, protocol or option was not found built-in in"
                " this libcurl due to a build-time decision.";

        case CURLE_COULDNT_RESOLVE_HOST:
            return "Could not resolve hostname";

        case CURLE_COULDNT_CONNECT:
            return "Could not connect to server";

        case CURLE_WEIRD_SERVER_REPLY:
            return "Weird server reply";

        case CURLE_REMOTE_ACCESS_DENIED:
            return "Access denied to remote resource";

        case CURLE_FTP_ACCEPT_FAILED:
            return "FTP: The server failed to connect to data port";

        case CURLE_FTP_ACCEPT_TIMEOUT:
            return "FTP: Accepting server connect has timed out";

        case CURLE_FTP_PRET_FAILED:
            return "FTP: The server did not accept the PRET command.";

        case CURLE_FTP_WEIRD_PASS_REPLY:
            return "FTP: unknown PASS reply";

        case CURLE_FTP_WEIRD_PASV_REPLY:
            return "FTP: unknown PASV reply";

        case CURLE_FTP_WEIRD_227_FORMAT:
            return "FTP: unknown 227 response format";

        case CURLE_FTP_CANT_GET_HOST:
            return "FTP: cannot figure out the host in the PASV response";

        case CURLE_HTTP2:
            return "Error in the HTTP2 framing layer";

        case CURLE_FTP_COULDNT_SET_TYPE:
            return "FTP: could not set file type";

        case CURLE_PARTIAL_FILE:
            return "Transferred a partial file";

        case CURLE_FTP_COULDNT_RETR_FILE:
            return "FTP: could not retrieve (RETR failed) the specified file";

        case CURLE_QUOTE_ERROR:
            return "Quote command returned error";

        case CURLE_HTTP_RETURNED_ERROR:
            return "HTTP response code said error";

        case CURLE_WRITE_ERROR:
            return "Failed writing received data to disk/application";

        case CURLE_UPLOAD_FAILED:
            return "Upload failed (at start/before it took off)";

        case CURLE_READ_ERROR:
            return "Failed to open/read local data from file/application";

        case CURLE_OUT_OF_MEMORY:
            return "Out of memory";

        case CURLE_OPERATION_TIMEDOUT:
            return "Timeout was reached";

        case CURLE_FTP_PORT_FAILED:
            return "FTP: command PORT failed";

        case CURLE_FTP_COULDNT_USE_REST:
            return "FTP: command REST failed";

        case CURLE_RANGE_ERROR:
            return "Requested range was not delivered by the server";

        case CURLE_HTTP_POST_ERROR:
            return "Internal problem setting up the POST";

        case CURLE_SSL_CONNECT_ERROR:
            return "SSL connect error";

        case CURLE_BAD_DOWNLOAD_RESUME:
            return "Could not resume download";

        case CURLE_FILE_COULDNT_READ_FILE:
            return "Could not read a file:// file";

        case CURLE_FUNCTION_NOT_FOUND:
            return "A required function in the library was not found";

        case CURLE_ABORTED_BY_CALLBACK:
            return "Operation was aborted by an application callback";

        case CURLE_BAD_FUNCTION_ARGUMENT:
            return "A libcurl function was given a bad argument";

        case CURLE_INTERFACE_FAILED:
            return "Failed binding local connection end";

        case CURLE_TOO_MANY_REDIRECTS:
            return "Number of redirects hit maximum amount";

        case CURLE_UNKNOWN_OPTION:
            return "An unknown option was passed in to libcurl";

        case CURLE_SETOPT_OPTION_SYNTAX:
            return "Malformed option provided in a setopt";

        case CURLE_GOT_NOTHING:
            return "Server returned nothing (no headers, no data)";

        case CURLE_SSL_ENGINE_NOTFOUND:
            return "SSL crypto engine not found";

        case CURLE_SSL_ENGINE_SETFAILED:
            return "Can not set SSL crypto engine as default";

        case CURLE_SSL_ENGINE_INITFAILED:
            return "Failed to initialise SSL crypto engine";

        case CURLE_SEND_ERROR:
            return "Failed sending data to the peer";

        case CURLE_RECV_ERROR:
            return "Failure when receiving data from the peer";

        case CURLE_SSL_CERTPROBLEM:
            return "Problem with the local SSL certificate";

        case CURLE_SSL_CIPHER:
            return "Could not use specified SSL cipher";

        case CURLE_PEER_FAILED_VERIFICATION:
            return "SSL peer certificate or SSH remote key was not OK";

        case CURLE_SSL_CACERT_BADFILE:
            return "Problem with the SSL CA cert (path? access rights?)";

        case CURLE_BAD_CONTENT_ENCODING:
            return "Unrecognized or bad HTTP Content or Transfer-Encoding";

        case CURLE_FILESIZE_EXCEEDED:
            return "Maximum file size exceeded";

        case CURLE_USE_SSL_FAILED:
            return "Requested SSL level failed";

        case CURLE_SSL_SHUTDOWN_FAILED:
            return "Failed to shut down the SSL connection";

        case CURLE_SSL_CRL_BADFILE:
            return "Failed to load CRL file (path? access rights?, format?)";

        case CURLE_SSL_ISSUER_ERROR:
            return "Issuer check against peer certificate failed";

        case CURLE_SEND_FAIL_REWIND:
            return "Send failed since rewinding of the data stream failed";

        case CURLE_LOGIN_DENIED:
            return "Login denied";

        case CURLE_TFTP_NOTFOUND:
            return "TFTP: File Not Found";

        case CURLE_TFTP_PERM:
            return "TFTP: Access Violation";

        case CURLE_REMOTE_DISK_FULL:
            return "Disk full or allocation exceeded";

        case CURLE_TFTP_ILLEGAL:
            return "TFTP: Illegal operation";

        case CURLE_TFTP_UNKNOWNID:
            return "TFTP: Unknown transfer ID";

        case CURLE_REMOTE_FILE_EXISTS:
            return "Remote file already exists";

        case CURLE_TFTP_NOSUCHUSER:
            return "TFTP: No such user";

        case CURLE_REMOTE_FILE_NOT_FOUND:
            return "Remote file not found";

        case CURLE_SSH:
            return "Error in the SSH layer";

        case CURLE_AGAIN:
            return "Socket not ready for send/recv";

        case CURLE_FTP_BAD_FILE_LIST:
            return "Unable to parse FTP file list";

        case CURLE_CHUNK_FAILED:
            return "Chunk callback failed";

        case CURLE_NO_CONNECTION_AVAILABLE:
            return "The max connection limit is reached";

        case CURLE_SSL_PINNEDPUBKEYNOTMATCH:
            return "SSL public key does not match pinned public key";

        case CURLE_SSL_INVALIDCERTSTATUS:
            return "SSL server certificate status verification FAILED";

        case CURLE_HTTP2_STREAM:
            return "Stream error in the HTTP/2 framing layer";

        case CURLE_RECURSIVE_API_CALL:
            return "API function called from within callback";

        case CURLE_AUTH_ERROR:
            return "An authentication function returned an error";

        case CURLE_HTTP3:
            return "HTTP/3 error";

        case CURLE_QUIC_CONNECT_ERROR:
            return "QUIC connection error";

        case CURLE_PROXY:
            return "proxy handshake error";

        case CURLE_SSL_CLIENTCERT:
            return "SSL Client Certificate required";

        case CURLE_UNRECOVERABLE_POLL:
            return "Unrecoverable error in select/poll";

        case CURLE_TOO_LARGE:
            return "A value or data field grew larger than allowed";

        case CURLE_ECH_REQUIRED:
            return "ECH attempted but failed";

        case CURL_LAST:
            break;
    }
  
    return "Unknown error";
#else
    if (!error)
        return "No error";
    else
        return "Error";
#endif
}

const char* curl_multi_strerror(CURLMcode error)
{
#ifndef CURL_DISABLE_VERBOSE_STRINGS
    switch (error)
    {
        case CURLM_CALL_MULTI_PERFORM:
            return "Please call curl_multi_perform() soon";

        case CURLM_OK:
            return "No error";

        case CURLM_BAD_HANDLE:
            return "Invalid multi handle";

        case CURLM_BAD_EASY_HANDLE:
            return "Invalid easy handle";

        case CURLM_OUT_OF_MEMORY:
            return "Out of memory";

        case CURLM_INTERNAL_ERROR:
            return "Internal error";

        case CURLM_BAD_SOCKET:
            return "Invalid socket argument";

        case CURLM_UNKNOWN_OPTION:
            return "Unknown option";

        case CURLM_ADDED_ALREADY:
            return "The easy handle is already added to a multi handle";

        case CURLM_RECURSIVE_API_CALL:
            return "API function called from within callback";

        case CURLM_WAKEUP_FAILURE:
            return "Wakeup is unavailable or failed";

        case CURLM_BAD_FUNCTION_ARGUMENT:
            return "A libcurl function was given a bad argument";

        case CURLM_ABORTED_BY_CALLBACK:
            return "Operation was aborted by an application callback";

        case CURLM_UNRECOVERABLE_POLL:
            return "Unrecoverable error in select/poll";

        case CURLM_LAST:
            break;
    }

    return "Unknown error";
#else
    if (error == CURLM_OK)
        return "No error";
    else
        return "Error";
#endif
}

const char* curl_url_strerror(CURLUcode error)
{
#ifndef CURL_DISABLE_VERBOSE_STRINGS
    switch (error)
    {
        case CURLUE_OK:
            return "No error";

        case CURLUE_BAD_HANDLE:
            return "An invalid CURLU pointer was passed as argument";

        case CURLUE_BAD_PARTPOINTER:
            return "An invalid 'part' argument was passed as argument";

        case CURLUE_MALFORMED_INPUT:
            return "Malformed input to a URL function";

        case CURLUE_BAD_PORT_NUMBER:
            return "Port number was not a decimal number between 0 and 65535";

        case CURLUE_UNSUPPORTED_SCHEME:
            return "Unsupported URL scheme";

        case CURLUE_URLDECODE:
            return "URL decode error, most likely because of rubbish in the input";

        case CURLUE_OUT_OF_MEMORY:
            return "A memory function failed";

        case CURLUE_USER_NOT_ALLOWED:
            return "Credentials was passed in the URL when prohibited";

        case CURLUE_UNKNOWN_PART:
            return "An unknown part ID was passed to a URL API function";

        case CURLUE_NO_SCHEME:
            return "No scheme part in the URL";

        case CURLUE_NO_USER:
            return "No user part in the URL";

        case CURLUE_NO_PASSWORD:
            return "No password part in the URL";

        case CURLUE_NO_OPTIONS:
            return "No options part in the URL";

        case CURLUE_NO_HOST:
            return "No host part in the URL";

        case CURLUE_NO_PORT:
            return "No port part in the URL";

        case CURLUE_NO_QUERY:
            return "No query part in the URL";

        case CURLUE_NO_FRAGMENT:
            return "No fragment part in the URL";

        case CURLUE_NO_ZONEID:
            return "No zoneid part in the URL";

        case CURLUE_BAD_LOGIN:
            return "Bad login part";

        case CURLUE_BAD_IPV6:
            return "Bad IPv6 address";

        case CURLUE_BAD_HOSTNAME:
            return "Bad hostname";

        case CURLUE_BAD_FILE_URL:
            return "Bad file:// URL";

        case CURLUE_BAD_SLASHES:
            return "Unsupported number of slashes following scheme";

        case CURLUE_BAD_SCHEME:
            return "Bad scheme";

        case CURLUE_BAD_PATH:
            return "Bad path";

        case CURLUE_BAD_FRAGMENT:
            return "Bad fragment";

        case CURLUE_BAD_QUERY:
            return "Bad query";

        case CURLUE_BAD_PASSWORD:
            return "Bad password";

        case CURLUE_BAD_USER:
            return "Bad user";

        case CURLUE_LACKS_IDN:
            return "libcurl lacks IDN support";

        case CURLUE_TOO_LARGE:
            return "A value or data field is larger than allowed";

        case CURLUE_LAST:
            break;
    }

    return "CURLUcode unknown";
#else
    if (error == CURLUE_OK)
        return "No error";
    else
        return "Error";
#endif
}

#if defined(_WIN32) || defined(_WIN32_WCE)
/* This is a helper function for Curl_strerror that converts Windows API error
 * codes (GetLastError) to error messages.
 * Returns NULL if no error message was found for error code.
 */
static const char* get_winapi_error(int err, char* buf, size_t buflen)
{
    char* p;
    wchar_t wbuf[256];
    if (!buflen) {
        return NULL;
    }

    *buf = '\0';
    *wbuf = L'\0';

    /* We return the local codepage version of the error string because if it is
       output to the user's terminal it will likely be with functions which
       expect the local codepage (eg fprintf, failf, infof).
       FormatMessageW -> wcstombs is used for Windows CE compatibility. */
    if (FormatMessageW((FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS), NULL, (DWORD)err,
        LANG_NEUTRAL, wbuf, sizeof(wbuf) / sizeof(wchar_t), NULL))
    {
        size_t written = wcstombs(buf, wbuf, buflen - 1);
        if (written != (size_t)-1)
            buf[written] = '\0';
        else
            *buf = '\0';
    }

    /* Truncate multiple lines */
    p = strchr(buf, '\n');
    if (p)
    {
        if (p > buf && *(p - 1) == '\r')
            *(p - 1) = '\0';
        else
            *p = '\0';
    }

    return (*buf ? buf : NULL);
}
#endif /* _WIN32 || _WIN32_WCE */

/*
 * Our thread-safe and smart strerror() replacement.
 *
 * The 'err' argument passed in to this function MUST be a true errno number
 * as reported on this system. We do no range checking on the number before
 * we pass it to the "number-to-message" conversion function and there might
 * be systems that do not do proper range checking in there themselves.
 *
 * We do not do range checking (on systems other than Windows) since there is
 * no good reliable and portable way to do it.
 *
 * On Windows different types of error codes overlap. This function has an
 * order of preference when trying to match error codes:
 * CRT (errno), Winsock (WSAGetLastError), Windows API (GetLastError).
 *
 * It may be more correct to call one of the variant functions instead:
 * Call Curl_sspi_strerror if the error code is definitely Windows SSPI.
 * Call Curl_winapi_strerror if the error code is definitely Windows API.
 */
const char* Curl_strerror(int err, char* buf, size_t buflen)
{
#ifdef PRESERVE_WINDOWS_ERROR_CODE
    DWORD old_win_err = GetLastError();
#endif
    int old_errno = errno;
    char* p;

    if (!buflen)
        return NULL;

#ifndef _WIN32
    DEBUGASSERT(err >= 0);
#endif

    * buf = '\0';

#if defined(_WIN32) || defined(_WIN32_WCE)
#if defined(_WIN32)
    /* 'sys_nerr' is the maximum errno number, it is not widely portable */
    if (err >= 0 && err < sys_nerr)
        msnprintf(buf, buflen, "%s", sys_errlist[err]);
    else
#endif
    {
        if (!get_winapi_error(err, buf, buflen))
            msnprintf(buf, buflen, "Unknown error %d (%#x)", err, err);
    }
#else /* not Windows coming up */

#endif /* end of not Windows */

    /* strip trailing '\r\n' or '\n'. */
    p = strrchr(buf, '\n');
    if (p && (p - buf) >= 2)
        *p = '\0';
    p = strrchr(buf, '\r');
    if (p && (p - buf) >= 1)
        *p = '\0';

    if (errno != old_errno)
        errno = old_errno;

#ifdef PRESERVE_WINDOWS_ERROR_CODE
    if (old_win_err != GetLastError())
        SetLastError(old_win_err);
#endif

    return buf;
}

/*
 * Curl_winapi_strerror:
 * Variant of Curl_strerror if the error code is definitely Windows API.
 */
#if defined(_WIN32) || defined(_WIN32_WCE)
const char* Curl_winapi_strerror(DWORD err, char* buf, size_t buflen)
{
#ifdef PRESERVE_WINDOWS_ERROR_CODE
    DWORD old_win_err = GetLastError();
#endif
    int old_errno = errno;

    if (!buflen)
        return NULL;

    *buf = '\0';

#ifndef CURL_DISABLE_VERBOSE_STRINGS
    if (!get_winapi_error((int)err, buf, buflen))
    {
        msnprintf(buf, buflen, "Unknown error %lu (0x%08lX)", err, err);
    }
#else
    {
        const char* txt = (err == ERROR_SUCCESS) ? "No error" : "Error";
        if (strlen(txt) < buflen)
            strcpy(buf, txt);
    }
#endif

    if (errno != old_errno)
        errno = old_errno;

#ifdef PRESERVE_WINDOWS_ERROR_CODE
    if (old_win_err != GetLastError())
        SetLastError(old_win_err);
#endif

    return buf;
}
#endif /* _WIN32 || _WIN32_WCE */