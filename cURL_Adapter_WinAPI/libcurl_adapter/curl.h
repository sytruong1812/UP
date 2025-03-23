#pragma once
#include "setup.h"

#ifdef  __cplusplus
extern "C" {
#endif

	typedef void CURL;

#ifdef CURL_STATICLIB
#  define CURL_EXTERN
#elif defined(_WIN32)
#  if defined(BUILDING_LIBCURL)
#    define CURL_EXTERN  __declspec(dllexport)
#  else
#    define CURL_EXTERN  __declspec(dllimport)
#  endif
#endif

/* Compile-time deprecation macros. */
#define CURL_DEPRECATED(version, message)
#define CURL_IGNORE_DEPRECATION(statements) statements

#ifndef CURL_MAX_READ_SIZE
#define CURL_MAX_READ_SIZE (10*1024*1024)
#endif

#ifndef CURL_MAX_WRITE_SIZE
#define CURL_MAX_WRITE_SIZE (10*1024*1024)
#endif

#ifndef CURL_MAX_HTTP_HEADER
#define CURL_MAX_HTTP_HEADER (100*1024)
#endif

/* The CURLPROTO_ defines below are for the **deprecated** CURLOPT_*PROTOCOLS options. Do not use. */
#define CURLPROTO_HTTP   (1<<0)
#define CURLPROTO_HTTPS  (1<<1)
#define CURLPROTO_FTP    (1<<2)
#define CURLPROTO_FTPS   (1<<3)
#define CURLPROTO_ALL    (~0) /* enable everything */

/*=============================[ Callback Function ]=================================*/
#define CURL_READFUNC_ABORT 0x10000000
#define CURL_READFUNC_PAUSE 0x10000001
    typedef size_t(*curl_read_callback)(char* buffer,
                                        size_t size,
                                        size_t nitems,
                                        void* instream);


#define CURL_WRITEFUNC_PAUSE 0x10000001
#define CURL_WRITEFUNC_ERROR 0xFFFFFFFF
    typedef size_t(*curl_write_callback)(char* buffer,
                                         size_t size,
                                         size_t nitems,
                                         void* outstream);

#define CURL_FNMATCHFUNC_MATCH    0 /* string corresponds to the pattern */
#define CURL_FNMATCHFUNC_NOMATCH  1 /* pattern does not match the string */
#define CURL_FNMATCHFUNC_FAIL     2 /* an error occurred */

    typedef int (*curl_fnmatch_callback)(void* ptr,
                                         const char* pattern,
                                         const char* string);

#define CURL_SEEKFUNC_OK                0
#define CURL_SEEKFUNC_FAIL              1 /* fail the entire transfer */
#define CURL_SEEKFUNC_CANTSEEK          2 /* tell libcurl seeking cannot be done, so libcurl might try other means instead */
    typedef int (*curl_seek_callback)(void* instream,
                                      curl_off_t offset,
                                      int origin);  /*'whence'*/

    /* The kind of data that is passed to information_callback */
    typedef enum {
        CURLINFO_TEXT = 0,
        CURLINFO_HEADER_IN,    /* 1 */
        CURLINFO_HEADER_OUT,   /* 2 */
        CURLINFO_DATA_IN,      /* 3 */
        CURLINFO_DATA_OUT,     /* 4 */
        CURLINFO_SSL_DATA_IN,  /* 5 */
        CURLINFO_SSL_DATA_OUT, /* 6 */
        CURLINFO_END
    } curl_infotype;

    typedef int (*curl_debug_callback)(CURL* handle,       /* the handle/transfer this concerns */
                                       curl_infotype type, /* what kind of data */
                                       char* data,         /* points to the data */
                                       size_t size,        /* size of the data pointed to */
                                       void* userptr);     /* whatever the user please */

/*=================================[ CURLOPT ]=====================================*/

#define CURLOPTTYPE_LONG                0
#define CURLOPTTYPE_OBJECTPOINT         10000
#define CURLOPTTYPE_FUNCTIONPOINT       20000
#define CURLOPTTYPE_OFF_T               30000
#define CURLOPTTYPE_BLOB                40000

#define CURLOPT(na,t,nu)                na = t + nu
#define CURLOPTDEPRECATED(na,t,nu,v,m)  na CURL_DEPRECATED(v,m) = t + nu

/* CURLOPT aliases that make no runtime difference */
#define CURLOPTTYPE_STRINGPOINT         CURLOPTTYPE_OBJECTPOINT
#define CURLOPTTYPE_SLISTPOINT          CURLOPTTYPE_OBJECTPOINT
#define CURLOPTTYPE_CBPOINT             CURLOPTTYPE_OBJECTPOINT
#define CURLOPTTYPE_VALUES              CURLOPTTYPE_LONG

    typedef enum {
        /* Common options for all protocols */
        CURLOPT(CURLOPT_WRITEDATA, CURLOPTTYPE_CBPOINT, 1),
        CURLOPT(CURLOPT_URL, CURLOPTTYPE_STRINGPOINT, 2),
        CURLOPT(CURLOPT_PORT, CURLOPTTYPE_LONG, 3),
        CURLOPT(CURLOPT_PROXY, CURLOPTTYPE_STRINGPOINT, 4),
        CURLOPT(CURLOPT_USERPWD, CURLOPTTYPE_STRINGPOINT, 5),
        CURLOPT(CURLOPT_PROXYUSERPWD, CURLOPTTYPE_STRINGPOINT, 6),
        CURLOPT(CURLOPT_RANGE, CURLOPTTYPE_STRINGPOINT, 7),
        CURLOPT(CURLOPT_STDERR, CURLOPTTYPE_OBJECTPOINT, 37),
        CURLOPT(CURLOPT_READDATA, CURLOPTTYPE_CBPOINT, 9),
        CURLOPT(CURLOPT_ERRORBUFFER, CURLOPTTYPE_OBJECTPOINT, 10),
        CURLOPT(CURLOPT_WRITEFUNCTION, CURLOPTTYPE_FUNCTIONPOINT, 11),
        CURLOPT(CURLOPT_READFUNCTION, CURLOPTTYPE_FUNCTIONPOINT, 12),
        CURLOPT(CURLOPT_TIMEOUT, CURLOPTTYPE_LONG, 13),
        CURLOPT(CURLOPT_TIMEOUT_MS, CURLOPTTYPE_LONG, 155),
        CURLOPT(CURLOPT_CONNECTTIMEOUT_MS, CURLOPTTYPE_LONG, 156),
        CURLOPT(CURLOPT_LOW_SPEED_LIMIT, CURLOPTTYPE_LONG, 19),
        CURLOPT(CURLOPT_LOW_SPEED_TIME, CURLOPTTYPE_LONG, 20),
        CURLOPT(CURLOPT_VERBOSE, CURLOPTTYPE_LONG, 41),
        CURLOPT(CURLOPT_NOPROGRESS, CURLOPTTYPE_LONG, 43),
        CURLOPT(CURLOPT_MAXCONNECTS, CURLOPTTYPE_LONG, 71),
        CURLOPT(CURLOPT_MAXFILESIZE, CURLOPTTYPE_LONG, 114),
        CURLOPT(CURLOPT_CONNECTTIMEOUT, CURLOPTTYPE_LONG, 78),
        CURLOPT(CURLOPT_CONNECT_ONLY, CURLOPTTYPE_LONG, 141),
        CURLOPT(CURLOPT_BUFFERSIZE, CURLOPTTYPE_LONG, 98),
        CURLOPT(CURLOPT_UPLOAD_BUFFERSIZE, CURLOPTTYPE_LONG, 280),
        CURLOPT(CURLOPT_PRIVATE, CURLOPTTYPE_OBJECTPOINT, 103),
        CURLOPT(CURLOPT_MAXFILESIZE_LARGE, CURLOPTTYPE_OFF_T, 117),
        CURLOPT(CURLOPT_POSTFIELDSIZE_LARGE, CURLOPTTYPE_OFF_T, 120),
        CURLOPT(CURLOPT_SEEKFUNCTION, CURLOPTTYPE_FUNCTIONPOINT, 167),
        CURLOPT(CURLOPT_SEEKDATA, CURLOPTTYPE_CBPOINT, 168),
        CURLOPT(CURLOPT_DEFAULT_PROTOCOL, CURLOPTTYPE_STRINGPOINT, 238),
        CURLOPT(CURLOPT_PROTOCOLS_STR, CURLOPTTYPE_STRINGPOINT, 318),
        CURLOPT(CURLOPT_REDIR_PROTOCOLS_STR, CURLOPTTYPE_STRINGPOINT, 319),

        /* HTTP/HTTPS specific options */
        CURLOPT(CURLOPT_PROXYPORT, CURLOPTTYPE_LONG, 59),
        CURLOPT(CURLOPT_POSTFIELDS, CURLOPTTYPE_OBJECTPOINT, 15),
        CURLOPT(CURLOPT_REFERER, CURLOPTTYPE_STRINGPOINT, 16),
        CURLOPT(CURLOPT_USERAGENT, CURLOPTTYPE_STRINGPOINT, 18),
        CURLOPT(CURLOPT_HTTPHEADER, CURLOPTTYPE_SLISTPOINT, 23),
        CURLOPT(CURLOPT_HTTPPOST, CURLOPTTYPE_OBJECTPOINT, 24),
        CURLOPT(CURLOPT_HEADERDATA, CURLOPTTYPE_CBPOINT, 29),
        CURLOPT(CURLOPT_CUSTOMREQUEST, CURLOPTTYPE_STRINGPOINT, 36),
        CURLOPT(CURLOPT_HEADER, CURLOPTTYPE_LONG, 42),
        CURLOPT(CURLOPT_NOBODY, CURLOPTTYPE_LONG, 44),
        CURLOPT(CURLOPT_FAILONERROR, CURLOPTTYPE_LONG, 45),
        CURLOPT(CURLOPT_POST, CURLOPTTYPE_LONG, 47),
        CURLOPT(CURLOPT_FOLLOWLOCATION, CURLOPTTYPE_LONG, 52),
        CURLOPT(CURLOPT_PUT, CURLOPTTYPE_LONG, 54),
        CURLOPT(CURLOPT_AUTOREFERER, CURLOPTTYPE_LONG, 58),
        CURLOPT(CURLOPT_POSTFIELDSIZE, CURLOPTTYPE_LONG, 60),
        CURLOPT(CURLOPT_HTTPPROXYTUNNEL, CURLOPTTYPE_LONG, 61),
        CURLOPT(CURLOPT_MAXREDIRS, CURLOPTTYPE_LONG, 68),
        CURLOPT(CURLOPT_HEADERFUNCTION, CURLOPTTYPE_FUNCTIONPOINT, 79),
        CURLOPT(CURLOPT_HTTPGET, CURLOPTTYPE_LONG, 80),
        CURLOPT(CURLOPT_HTTP_VERSION, CURLOPTTYPE_VALUES, 84),
        CURLOPT(CURLOPT_HTTPAUTH, CURLOPTTYPE_VALUES, 107),
        CURLOPT(CURLOPT_PATH_AS_IS, CURLOPTTYPE_LONG, 234),
        CURLOPT(CURLOPT_TRANSFER_ENCODING, CURLOPTTYPE_LONG, 207),
        CURLOPT(CURLOPT_HEADEROPT, CURLOPTTYPE_VALUES, 229),
        CURLOPT(CURLOPT_IGNORE_CONTENT_LENGTH, CURLOPTTYPE_LONG, 136),
        CURLOPT(CURLOPT_HTTP_TRANSFER_DECODING, CURLOPTTYPE_LONG, 157),
        CURLOPT(CURLOPT_HTTP_CONTENT_DECODING, CURLOPTTYPE_LONG, 158),
        CURLOPT(CURLOPT_HTTP200ALIASES, CURLOPTTYPE_SLISTPOINT, 104),
        CURLOPT(CURLOPT_ACCEPT_ENCODING, CURLOPTTYPE_STRINGPOINT, 102),

        /* SSL/TLS related options (for HTTPS) */
        CURLOPT(CURLOPT_SSL_VERIFYPEER, CURLOPTTYPE_LONG, 64),
        CURLOPT(CURLOPT_SSL_VERIFYHOST, CURLOPTTYPE_LONG, 81),
        

        /* WebSocket specific options */
        CURLOPT(CURLOPT_WS_OPTIONS, CURLOPTTYPE_LONG, 320),

        /* FTP/FTPS specific options */
        CURLOPT(CURLOPT_FTPPORT, CURLOPTTYPE_STRINGPOINT, 17),
        CURLOPT(CURLOPT_RESUME_FROM, CURLOPTTYPE_LONG, 21),
        CURLOPT(CURLOPT_TIMECONDITION, CURLOPTTYPE_VALUES, 33),
        CURLOPT(CURLOPT_TIMEVALUE, CURLOPTTYPE_LONG, 34),
        CURLOPT(CURLOPT_UPLOAD, CURLOPTTYPE_LONG, 46),
        CURLOPT(CURLOPT_TRANSFERTEXT, CURLOPTTYPE_LONG, 53),
        CURLOPT(CURLOPT_FILETIME, CURLOPTTYPE_LONG, 69),
        CURLOPT(CURLOPT_FRESH_CONNECT, CURLOPTTYPE_LONG, 74),
        CURLOPT(CURLOPT_FORBID_REUSE, CURLOPTTYPE_LONG, 75),
        CURLOPT(CURLOPT_FTP_USE_EPSV, CURLOPTTYPE_LONG, 85),
        CURLOPT(CURLOPT_FTP_USE_EPRT, CURLOPTTYPE_LONG, 106),
        CURLOPT(CURLOPT_FTP_USE_PRET, CURLOPTTYPE_LONG, 188),
        CURLOPT(CURLOPT_INFILESIZE, CURLOPTTYPE_LONG, 14),
        CURLOPT(CURLOPT_INFILESIZE_LARGE, CURLOPTTYPE_OFF_T, 115),
        CURLOPT(CURLOPT_FTP_FILEMETHOD, CURLOPTTYPE_VALUES, 138),
        CURLOPT(CURLOPT_FTP_SSL_CCC, CURLOPTTYPE_LONG, 154),
        CURLOPT(CURLOPT_FTP_ACCOUNT, CURLOPTTYPE_STRINGPOINT, 134),
        CURLOPT(CURLOPT_FTP_ALTERNATIVE_TO_USER, CURLOPTTYPE_STRINGPOINT, 147),
        CURLOPT(CURLOPT_FTP_SKIP_PASV_IP, CURLOPTTYPE_LONG, 137),
        CURLOPT(CURLOPT_FTPSSLAUTH, CURLOPTTYPE_VALUES, 129),
        CURLOPT(CURLOPT_ACCEPTTIMEOUT_MS, CURLOPTTYPE_LONG, 212),
        CURLOPT(CURLOPT_WILDCARDMATCH, CURLOPTTYPE_LONG, 197),
        CURLOPT(CURLOPT_FTP_CREATE_MISSING_DIRS, CURLOPTTYPE_LONG, 110),
        CURLOPT(CURLOPT_NEW_FILE_PERMS, CURLOPTTYPE_LONG, 159),
        CURLOPT(CURLOPT_NEW_DIRECTORY_PERMS, CURLOPTTYPE_LONG, 160),
        CURLOPT(CURLOPT_POSTQUOTE, CURLOPTTYPE_SLISTPOINT, 39),
        CURLOPT(CURLOPT_PREQUOTE, CURLOPTTYPE_SLISTPOINT, 93),
        CURLOPT(CURLOPT_QUOTE, CURLOPTTYPE_SLISTPOINT, 28),
        CURLOPT(CURLOPT_CHUNK_BGN_FUNCTION, CURLOPTTYPE_FUNCTIONPOINT, 198),
        CURLOPT(CURLOPT_CHUNK_END_FUNCTION, CURLOPTTYPE_FUNCTIONPOINT, 199),
        CURLOPT(CURLOPT_FNMATCH_FUNCTION, CURLOPTTYPE_FUNCTIONPOINT, 200),
        CURLOPT(CURLOPT_CHUNK_DATA, CURLOPTTYPE_CBPOINT, 201),
        CURLOPT(CURLOPT_FNMATCH_DATA, CURLOPTTYPE_CBPOINT, 202),


        CURLOPT_LASTENTRY
    } CURLoption;


    typedef enum {
        /* General/Common errors */
        CURLE_OK = 0,
        CURLE_UNSUPPORTED_PROTOCOL,
        CURLE_FAILED_INIT,
        CURLE_URL_MALFORMAT,
        CURLE_NOT_BUILT_IN,
        CURLE_COULDNT_RESOLVE_HOST,
        CURLE_COULDNT_CONNECT,
        CURLE_WRITE_ERROR,
        CURLE_READ_ERROR,
        CURLE_OUT_OF_MEMORY,
        CURLE_OPERATION_TIMEDOUT,
        CURLE_ABORTED_BY_CALLBACK,
        CURLE_BAD_FUNCTION_ARGUMENT,
        CURLE_INTERFACE_FAILED,
        CURLE_UNKNOWN_OPTION,
        CURLE_SETOPT_OPTION_SYNTAX,
        CURLE_GOT_NOTHING,
        CURLE_SEND_ERROR,
        CURLE_RECV_ERROR,
        CURLE_BAD_CONTENT_ENCODING,
        CURLE_FILESIZE_EXCEEDED,
        CURLE_SEND_FAIL_REWIND,
        CURLE_LOGIN_DENIED,
        CURLE_TFTP_NOTFOUND,          
        CURLE_TFTP_PERM,              
        CURLE_REMOTE_DISK_FULL,       
        CURLE_TFTP_ILLEGAL,           
        CURLE_TFTP_UNKNOWNID,         
        CURLE_REMOTE_FILE_EXISTS,     
        CURLE_TFTP_NOSUCHUSER,        
        CURLE_REMOTE_FILE_NOT_FOUND,
        CURLE_SSH,
        CURLE_AGAIN,
        CURLE_CHUNK_FAILED,
        CURLE_NO_CONNECTION_AVAILABLE,
        CURLE_RECURSIVE_API_CALL,
        CURLE_AUTH_ERROR,
        CURLE_TOO_LARGE,
        CURLE_FILE_COULDNT_READ_FILE,
        CURLE_FUNCTION_NOT_FOUND,

        /* HTTP/HTTPS specific errors */
        CURLE_WEIRD_SERVER_REPLY,
        CURLE_REMOTE_ACCESS_DENIED,
        CURLE_HTTP2,
        CURLE_HTTP_RETURNED_ERROR,
        CURLE_HTTP_POST_ERROR,
        CURLE_TOO_MANY_REDIRECTS,
        CURLE_RANGE_ERROR,
        CURLE_HTTP2_STREAM,
        CURLE_HTTP3,
        CURLE_PROXY,
        CURLE_ECH_REQUIRED,

        /* SSL/TLS related errors */
        CURLE_SSL_CONNECT_ERROR,
        CURLE_SSL_ENGINE_NOTFOUND,
        CURLE_SSL_ENGINE_SETFAILED,
        CURLE_SSL_CERTPROBLEM,
        CURLE_SSL_CIPHER,
        CURLE_PEER_FAILED_VERIFICATION,
        CURLE_USE_SSL_FAILED,
        CURLE_SSL_ENGINE_INITFAILED,
        CURLE_SSL_CACERT_BADFILE,
        CURLE_SSL_SHUTDOWN_FAILED,
        CURLE_SSL_CRL_BADFILE,
        CURLE_SSL_ISSUER_ERROR,
        CURLE_SSL_PINNEDPUBKEYNOTMATCH,
        CURLE_SSL_INVALIDCERTSTATUS,
        CURLE_SSL_CLIENTCERT,

        /* FTP/FTPS specific errors */
        CURLE_FTP_ACCEPT_FAILED,
        CURLE_FTP_WEIRD_PASS_REPLY,
        CURLE_FTP_ACCEPT_TIMEOUT,
        CURLE_FTP_WEIRD_PASV_REPLY,
        CURLE_FTP_WEIRD_227_FORMAT,
        CURLE_FTP_CANT_GET_HOST,
        CURLE_FTP_COULDNT_SET_TYPE,
        CURLE_FTP_COULDNT_RETR_FILE,
        CURLE_PARTIAL_FILE,
        CURLE_QUOTE_ERROR,
        CURLE_UPLOAD_FAILED,
        CURLE_FTP_PORT_FAILED,
        CURLE_FTP_COULDNT_USE_REST,
        CURLE_BAD_DOWNLOAD_RESUME,
        CURLE_FTP_PRET_FAILED,
        CURLE_FTP_BAD_FILE_LIST,

        /* WinHTTP specific errors */
        CURLE_WINHTTP_OPEN_A_SESSION_ERROR,          /* Failed to initialize WinHTTP session handle via WinHttpOpen() */
        CURLE_WINHTTP_CANNOT_CONNECT_SERVER,         /* Unable to establish connection to server via WinHttpConnect() */
        CURLE_WINHTTP_CANNOT_SET_TIMEOUT,            /* Failed to set timeout options using WinHttpSetTimeouts() */
        CURLE_WINHTTP_OPEN_REQUEST_ERROR,            /* Failed to create HTTP request handle via WinHttpOpenRequest() */
        CURLE_WINHTTP_SET_CREDENTIALS_ERROR,         /* Authentication failed when setting credentials via WinHttpSetCredentials() */
        CURLE_WINHTTP_SET_OPTION_ERROR,              /* Failed to set WinHTTP option using WinHttpSetOption() */
        CURLE_WINHTTP_ADD_REQUEST_HEADER_ERROR,      /* Failed to add headers to request via WinHttpAddRequestHeaders() */
        CURLE_WINHTTP_SEND_REQUEST_ERROR,            /* Failed to send HTTP request using WinHttpSendRequest() */
        CURLE_WINHTTP_RECV_RESPONSE_ERROR,           /* Failed to receive server response via WinHttpReceiveResponse() */
        CURLE_WINHTTP_QUERY_DATA_ERROR,              /* Failed to query HTTP headers via WinHttpQueryHeaders() */
        CURLE_WINHTTP_READ_DATA_ERROR,               /* Failed to read response data via WinHttpReadData() */
        CURLE_WINHTTP_WRITE_DATA_ERROR,              /* Failed to write request data via WinHttpWriteData() */
        CURLE_WINHTTP_WS_UPGRADE_ERROR,              /* Failed to upgrade to WebSocket protocol via WinHttpWebSocketCompleteUpgrade() */
        CURLE_WINHTTP_WS_CLOSE_ERROR,                /* Error closing WebSocket connection via WinHttpWebSocketClose() */
        CURLE_WINHTTP_WS_SEND_DATA_ERROR,            /* Failed to send WebSocket data via WinHttpWebSocketSend() */
        CURLE_WINHTTP_WS_RECV_DATA_ERROR,            /* Failed to receive WebSocket data via WinHttpWebSocketReceive() */
        CURLE_FAILED_TO_ALLOCATE,                    /* Failed to allocate memory for internal operations */
        CURLE_URL_GET_ERROR,                         /* Failed to parse or process URL */
        CURLE_SET_CURLU_ERROR,                       /* Failed to set URL parameters or components */

        /* WinINet FTP specific errors */
        CURLE_WININET_FTP_SESSION_ERROR = 90,        /* Failed to create WinInet session */
        CURLE_WININET_FTP_CONNECT_ERROR,             /* Failed to connect to FTP server */
        CURLE_WININET_FTP_SSL_ERROR,                 /* SSL configuration failed */
        CURLE_WININET_FTP_TIMEOUT_ERROR,             /* Operation timed out */
        CURLE_WININET_FTP_LOGIN_ERROR,               /* Authentication failed */
        CURLE_WININET_FTP_ACCESS_DENIED,             /* Access denied to resource */
        CURLE_WININET_FTP_CWD_ERROR,                 /* Failed to change directory */
        CURLE_WININET_FTP_PWD_ERROR,                 /* Failed to get working directory */
        CURLE_WININET_FTP_MKD_ERROR,                 /* Failed to create directory */
        CURLE_WININET_FTP_RMD_ERROR,                 /* Failed to remove directory */
        CURLE_WININET_FTP_DIR_LIST_ERROR,            /* Failed to list directory */
        CURLE_WININET_FTP_TRANSFER_TYPE_ERROR,       /* Failed to set transfer type */
        CURLE_WININET_FTP_PASSIVE_ERROR,             /* Failed to enter passive mode */
        CURLE_WININET_FTP_PORT_ERROR,                /* Failed to set port */
        CURLE_WININET_FTP_FILE_NOT_FOUND,            /* File not found on server */
        CURLE_WININET_FTP_FILE_ACCESS_ERROR,         /* File access denied */
        CURLE_WININET_FTP_FILE_SIZE_ERROR,           /* Failed to get file size */
        CURLE_WININET_FTP_FILE_TRANSFER_ERROR,       /* File transfer failed */
        CURLE_WININET_FTP_COMMAND_ERROR,             /* Command execution failed */
        CURLE_WININET_FTP_RESPONSE_ERROR,            /* Invalid server response */
        CURLE_WININET_FTP_TRANSFER_IN_PROGRESS,      /* Transfer already in progress */
        CURLE_WININET_FTP_TRANSFER_ABORTED,          /* Transfer was aborted */
        CURLE_WININET_FTP_TRANSFER_TIMEOUT,          /* Transfer timed out */
        CURLE_WININET_FTP_OUT_OF_MEMORY,             /* Memory allocation failed */
        CURLE_WININET_FTP_HANDLE_ERROR,              /* Invalid handle operation */

        /* Other protocol errors */
        CURLE_QUIC_CONNECT_ERROR,
        CURLE_UNRECOVERABLE_POLL,

        CURL_LAST
    } CURLcode;

/*============================[ CURLOPT_HTTP_VERSION ]==============================*/
    enum {
        CURL_HTTP_VERSION_NONE,
        CURL_HTTP_VERSION_1_0,
        CURL_HTTP_VERSION_1_1,
        CURL_HTTP_VERSION_2_0,
        CURL_HTTP_VERSION_2TLS,
        CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE,
        CURL_HTTP_VERSION_3 = 30,
        CURL_HTTP_VERSION_3ONLY = 31,
        CURL_HTTP_VERSION_LAST
    };

#define CURL_HTTP_VERSION_2             CURL_HTTP_VERSION_2_0
#define CURL_DEFAULT_PROXY_PORT         1080    /* default proxy port unless specified */
#define CURL_DEFAULT_HTTPS_PROXY_PORT   443     /* default https proxy port unless specified */

    CURL_EXTERN char* curl_getenv(const char* variable);
    CURL_EXTERN const char* curl_easy_strerror(CURLcode);

    CURL_EXTERN int curl_strequal(const char* s1, const char* s2);
    CURL_EXTERN int curl_strnequal(const char* s1, const char* s2, size_t n);

    CURL_EXTERN char* curl_easy_escape(CURL* handle, const char* string, int length);
    CURL_EXTERN char* curl_escape(const char* string, int length);
    CURL_EXTERN char* curl_easy_unescape(CURL* handle, const char* string, int length, int* outlength);
    CURL_EXTERN char* curl_unescape(const char* string, int length);

/*=================================[ CURLINFO ]=====================================*/

#define CURLINFO_STRING   0x100000
#define CURLINFO_LONG     0x200000
#define CURLINFO_DOUBLE   0x300000
#define CURLINFO_SLIST    0x400000
#define CURLINFO_PTR      0x400000
#define CURLINFO_SOCKET   0x500000
#define CURLINFO_OFF_T    0x600000
#define CURLINFO_MASK     0x0fffff
#define CURLINFO_TYPEMASK 0xf00000

    typedef enum 
{
    CURLINFO_NONE, /* first, never use this */
    CURLINFO_EFFECTIVE_URL = CURLINFO_STRING + 1,
    CURLINFO_RESPONSE_CODE = CURLINFO_LONG + 2,
    CURLINFO_TOTAL_TIME = CURLINFO_DOUBLE + 3,
    CURLINFO_NAMELOOKUP_TIME = CURLINFO_DOUBLE + 4,
    CURLINFO_CONNECT_TIME = CURLINFO_DOUBLE + 5,
    CURLINFO_PRETRANSFER_TIME = CURLINFO_DOUBLE + 6,
    CURLINFO_SIZE_UPLOAD CURL_DEPRECATED(7.55.0, "Use CURLINFO_SIZE_UPLOAD_T") = CURLINFO_DOUBLE + 7,
    CURLINFO_SIZE_UPLOAD_T = CURLINFO_OFF_T + 7,
    CURLINFO_SIZE_DOWNLOAD CURL_DEPRECATED(7.55.0, "Use CURLINFO_SIZE_DOWNLOAD_T") = CURLINFO_DOUBLE + 8,
    CURLINFO_SIZE_DOWNLOAD_T = CURLINFO_OFF_T + 8, 
    CURLINFO_SPEED_DOWNLOAD CURL_DEPRECATED(7.55.0, "Use CURLINFO_SPEED_DOWNLOAD_T") = CURLINFO_DOUBLE + 9,
    CURLINFO_SPEED_DOWNLOAD_T = CURLINFO_OFF_T + 9,
    CURLINFO_SPEED_UPLOAD CURL_DEPRECATED(7.55.0, "Use CURLINFO_SPEED_UPLOAD_T") = CURLINFO_DOUBLE + 10,
    CURLINFO_SPEED_UPLOAD_T = CURLINFO_OFF_T + 10,
    CURLINFO_HEADER_SIZE = CURLINFO_LONG + 11,
    CURLINFO_REQUEST_SIZE = CURLINFO_LONG + 12,
    CURLINFO_SSL_VERIFYRESULT = CURLINFO_LONG + 13,
    CURLINFO_FILETIME = CURLINFO_LONG + 14,
    CURLINFO_FILETIME_T = CURLINFO_OFF_T + 14,
    CURLINFO_CONTENT_LENGTH_DOWNLOAD CURL_DEPRECATED(7.55.0, "Use CURLINFO_CONTENT_LENGTH_DOWNLOAD_T") = CURLINFO_DOUBLE + 15,
    CURLINFO_CONTENT_LENGTH_DOWNLOAD_T = CURLINFO_OFF_T + 15,
    CURLINFO_CONTENT_LENGTH_UPLOAD CURL_DEPRECATED(7.55.0, "Use CURLINFO_CONTENT_LENGTH_UPLOAD_T") = CURLINFO_DOUBLE + 16,
    CURLINFO_CONTENT_LENGTH_UPLOAD_T = CURLINFO_OFF_T + 16,
    CURLINFO_STARTTRANSFER_TIME = CURLINFO_DOUBLE + 17,
    CURLINFO_CONTENT_TYPE = CURLINFO_STRING + 18,
    CURLINFO_REDIRECT_TIME = CURLINFO_DOUBLE + 19,
    CURLINFO_REDIRECT_COUNT = CURLINFO_LONG + 20,
    CURLINFO_PRIVATE = CURLINFO_STRING + 21,
    CURLINFO_HTTP_CONNECTCODE = CURLINFO_LONG + 22,
    CURLINFO_HTTPAUTH_AVAIL = CURLINFO_LONG + 23,
    CURLINFO_PROXYAUTH_AVAIL = CURLINFO_LONG + 24,
    CURLINFO_OS_ERRNO = CURLINFO_LONG + 25,
    CURLINFO_NUM_CONNECTS = CURLINFO_LONG + 26,
    CURLINFO_SSL_ENGINES = CURLINFO_SLIST + 27,
    CURLINFO_COOKIELIST = CURLINFO_SLIST + 28,
    CURLINFO_LASTSOCKET  CURL_DEPRECATED(7.45.0, "Use CURLINFO_ACTIVESOCKET") = CURLINFO_LONG + 29,
    CURLINFO_FTP_ENTRY_PATH = CURLINFO_STRING + 30,
    CURLINFO_REDIRECT_URL = CURLINFO_STRING + 31,
    CURLINFO_PRIMARY_IP = CURLINFO_STRING + 32,
    CURLINFO_APPCONNECT_TIME = CURLINFO_DOUBLE + 33,
    CURLINFO_CERTINFO = CURLINFO_PTR + 34,
    CURLINFO_CONDITION_UNMET = CURLINFO_LONG + 35,
    CURLINFO_RTSP_SESSION_ID = CURLINFO_STRING + 36,
    CURLINFO_RTSP_CLIENT_CSEQ = CURLINFO_LONG + 37,
    CURLINFO_RTSP_SERVER_CSEQ = CURLINFO_LONG + 38,
    CURLINFO_RTSP_CSEQ_RECV = CURLINFO_LONG + 39,
    CURLINFO_PRIMARY_PORT = CURLINFO_LONG + 40,
    CURLINFO_LOCAL_IP = CURLINFO_STRING + 41,
    CURLINFO_LOCAL_PORT = CURLINFO_LONG + 42,
    CURLINFO_TLS_SESSION CURL_DEPRECATED(7.48.0, "Use CURLINFO_TLS_SSL_PTR") = CURLINFO_PTR + 43,
    CURLINFO_ACTIVESOCKET = CURLINFO_SOCKET + 44,
    CURLINFO_TLS_SSL_PTR = CURLINFO_PTR + 45,
    CURLINFO_HTTP_VERSION = CURLINFO_LONG + 46,
    CURLINFO_PROXY_SSL_VERIFYRESULT = CURLINFO_LONG + 47,
    CURLINFO_PROTOCOL    CURL_DEPRECATED(7.85.0, "Use CURLINFO_SCHEME") = CURLINFO_LONG + 48,
    CURLINFO_SCHEME = CURLINFO_STRING + 49,
    CURLINFO_TOTAL_TIME_T = CURLINFO_OFF_T + 50,
    CURLINFO_NAMELOOKUP_TIME_T = CURLINFO_OFF_T + 51,
    CURLINFO_CONNECT_TIME_T = CURLINFO_OFF_T + 52,
    CURLINFO_PRETRANSFER_TIME_T = CURLINFO_OFF_T + 53,
    CURLINFO_STARTTRANSFER_TIME_T = CURLINFO_OFF_T + 54,
    CURLINFO_REDIRECT_TIME_T = CURLINFO_OFF_T + 55,
    CURLINFO_APPCONNECT_TIME_T = CURLINFO_OFF_T + 56,
    CURLINFO_RETRY_AFTER = CURLINFO_OFF_T + 57,
    CURLINFO_EFFECTIVE_METHOD = CURLINFO_STRING + 58,
    CURLINFO_PROXY_ERROR = CURLINFO_LONG + 59,
    CURLINFO_REFERER = CURLINFO_STRING + 60,
    CURLINFO_CAINFO = CURLINFO_STRING + 61,
    CURLINFO_CAPATH = CURLINFO_STRING + 62,
    CURLINFO_XFER_ID = CURLINFO_OFF_T + 63,
    CURLINFO_CONN_ID = CURLINFO_OFF_T + 64,
    CURLINFO_QUEUE_TIME_T = CURLINFO_OFF_T + 65,
    CURLINFO_USED_PROXY = CURLINFO_LONG + 66,
    CURLINFO_POSTTRANSFER_TIME_T = CURLINFO_OFF_T + 67,
    CURLINFO_EARLYDATA_SENT_T = CURLINFO_OFF_T + 68,
    CURLINFO_LASTONE = 68
} CURLINFO;

/*=================================[ GLOBAL INIT ]=====================================*/
#define CURL_GLOBAL_SSL (1<<0)
#define CURL_GLOBAL_WIN32 (1<<1)
#define CURL_GLOBAL_ALL (CURL_GLOBAL_SSL|CURL_GLOBAL_WIN32)
#define CURL_GLOBAL_NOTHING 0
#define CURL_GLOBAL_DEFAULT CURL_GLOBAL_ALL
#define CURL_GLOBAL_ACK_EINTR (1<<2)

    CURL_EXTERN void curl_free(void* p);
    CURL_EXTERN CURLcode curl_global_init(long flags);
    CURL_EXTERN void curl_global_cleanup(void);
    CURL_EXTERN CURLcode curl_global_trace(const char* config);



/*=================================[ FTP Protocol ]=====================================*/

    /* parameter for the CURLOPT_FTPSSLAUTH option */
    typedef enum 
    {
        CURLFTPAUTH_DEFAULT, /* let libcurl decide */
        CURLFTPAUTH_SSL,     /* use "AUTH SSL" */
        CURLFTPAUTH_TLS,     /* use "AUTH TLS" */
        CURLFTPAUTH_LAST /* not an option, never use */
    } curl_ftpauth;
    /* parameter for the CURLOPT_FTP_CREATE_MISSING_DIRS option */
    typedef enum 
    {
        CURLFTP_CREATE_DIR_NONE,  /* do NOT create missing dirs! */
        CURLFTP_CREATE_DIR,       /* (FTP/SFTP) if CWD fails, try MKD and then CWD again if MKD succeeded, for SFTP this does similar magic */
        CURLFTP_CREATE_DIR_RETRY, /* (FTP only) if CWD fails, try MKD and then CWD again even if MKD failed! */
        CURLFTP_CREATE_DIR_LAST   /* not an option, never use */
    } curl_ftpcreatedir;

    /* parameter for the CURLOPT_FTP_FILEMETHOD option */
    typedef enum {
        CURLFTPMETHOD_DEFAULT,   /* let libcurl pick */
        CURLFTPMETHOD_MULTICWD,  /* single CWD operation for each path part */
        CURLFTPMETHOD_NOCWD,     /* no CWD at all */
        CURLFTPMETHOD_SINGLECWD, /* one CWD to full dir, then work on file */
        CURLFTPMETHOD_LAST       /* not an option, never use */
    } curl_ftpmethod;


#ifdef  __cplusplus
} /* end of extern "C" */
#endif

#include "easy.h"
#include "multi.h"
#include "urlapi.h"