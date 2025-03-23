#pragma once
#include "curl.h"
#include "hash.h"

#ifdef  __cplusplus
extern "C" {
#endif

    typedef void CURLM;

    typedef enum {
        CURLM_CALL_MULTI_PERFORM = -1,  /* please call curl_multi_perform() or curl_multi_socket*() soon */
        CURLM_OK,
        CURLM_BAD_HANDLE,               /* the passed-in handle is not a valid CURLM handle */
        CURLM_BAD_EASY_HANDLE,          /* an easy handle was not good/valid */
        CURLM_OUT_OF_MEMORY,            /* if you ever get this, you are in deep sh*t */
        CURLM_INTERNAL_ERROR,           /* this is a libcurl bug */
        CURLM_BAD_SOCKET,               /* the passed in socket argument did not match */
        CURLM_UNKNOWN_OPTION,           /* curl_multi_setopt() with unsupported option */
        CURLM_ADDED_ALREADY,            /* an easy handle already added to a multi handle was attempted to get added - again */
        CURLM_RECURSIVE_API_CALL,       /* an api function was called from inside a callback */
        CURLM_WAKEUP_FAILURE,           /* wakeup is unavailable or failed */
        CURLM_BAD_FUNCTION_ARGUMENT,    /* function called with a bad parameter */
        CURLM_ABORTED_BY_CALLBACK,
        CURLM_UNRECOVERABLE_POLL,
        CURLM_LAST
    } CURLMcode;

    typedef enum {
        /* This is the socket callback function pointer */
        CURLOPT(CURLMOPT_SOCKETFUNCTION, CURLOPTTYPE_FUNCTIONPOINT, 1),
        /* This is the argument passed to the socket callback */
        CURLOPT(CURLMOPT_SOCKETDATA, CURLOPTTYPE_OBJECTPOINT, 2),
        /* set to 1 to enable pipelining for this multi handle */
        CURLOPT(CURLMOPT_PIPELINING, CURLOPTTYPE_LONG, 3),
        /* This is the timer callback function pointer */
        CURLOPT(CURLMOPT_TIMERFUNCTION, CURLOPTTYPE_FUNCTIONPOINT, 4),
        /* This is the argument passed to the timer callback */
        CURLOPT(CURLMOPT_TIMERDATA, CURLOPTTYPE_OBJECTPOINT, 5),
        /* maximum number of entries in the connection cache */
        CURLOPT(CURLMOPT_MAXCONNECTS, CURLOPTTYPE_LONG, 6),
        /* maximum number of (pipelining) connections to one host */
        CURLOPT(CURLMOPT_MAX_HOST_CONNECTIONS, CURLOPTTYPE_LONG, 7),
        /* maximum number of requests in a pipeline */
        CURLOPT(CURLMOPT_MAX_PIPELINE_LENGTH, CURLOPTTYPE_LONG, 8),
        /* a connection with a content-length longer than this will not be considered for pipelining */
        CURLOPT(CURLMOPT_CONTENT_LENGTH_PENALTY_SIZE, CURLOPTTYPE_OFF_T, 9),
        /* a connection with a chunk length longer than this will not be considered for pipelining */
        CURLOPT(CURLMOPT_CHUNK_LENGTH_PENALTY_SIZE, CURLOPTTYPE_OFF_T, 10),
        /* a list of site names(+port) that are blocked from pipelining */
        CURLOPT(CURLMOPT_PIPELINING_SITE_BL, CURLOPTTYPE_OBJECTPOINT, 11),
        /* a list of server types that are blocked from pipelining */
        CURLOPT(CURLMOPT_PIPELINING_SERVER_BL, CURLOPTTYPE_OBJECTPOINT, 12),
        /* maximum number of open connections in total */
        CURLOPT(CURLMOPT_MAX_TOTAL_CONNECTIONS, CURLOPTTYPE_LONG, 13),
        /* This is the server push callback function pointer */
        CURLOPT(CURLMOPT_PUSHFUNCTION, CURLOPTTYPE_FUNCTIONPOINT, 14),
        /* This is the argument passed to the server push callback */
        CURLOPT(CURLMOPT_PUSHDATA, CURLOPTTYPE_OBJECTPOINT, 15),
        /* maximum number of concurrent streams to support on a connection */
        CURLOPT(CURLMOPT_MAX_CONCURRENT_STREAMS, CURLOPTTYPE_LONG, 16),
        CURLMOPT_LASTENTRY /* the last unused */
    } CURLMoption;

    typedef enum {
        MSTATE_INIT,            /* 0 - start in this state */
        MSTATE_PENDING,         /* 1 - no connections, waiting for one */
        MSTATE_SETUP,           /* 2 - start a new transfer */
        MSTATE_CONNECT,         /* 3 - resolve/connect has been sent off */
        MSTATE_RESOLVING,       /* 4 - awaiting the resolve to finalize */
        MSTATE_CONNECTING,      /* 5 - awaiting the TCP connect to finalize */
        MSTATE_TUNNELING,       /* 6 - awaiting HTTPS proxy SSL initialization to complete and/or proxy CONNECT to finalize */
        MSTATE_PROTOCONNECT,    /* 7 - initiate protocol connect procedure */
        MSTATE_PROTOCONNECTING, /* 8 - completing the protocol-specific connect phase */
        MSTATE_DO,              /* 9 - start send off the request (part 1) */
        MSTATE_DOING,           /* 10 - sending off the request (part 1) */
        MSTATE_DOING_MORE,      /* 11 - send off the request (part 2) */
        MSTATE_DID,             /* 12 - done sending off request */
        MSTATE_PERFORMING,      /* 13 - transfer data */
        MSTATE_RATELIMITING,    /* 14 - wait because limit-rate exceeded */
        MSTATE_DONE,            /* 15 - post data transfer operation */
        MSTATE_COMPLETED,       /* 16 - operation complete */
        MSTATE_MSGSENT,         /* 17 - the operation complete message is sent */
        MSTATE_LAST             /* 18 - not a true state, never use this */
    } CURLMstate;

    CURL_EXTERN CURLM* curl_multi_init(void);

    CURL_EXTERN CURLMcode curl_multi_add_handle(CURLM* multi_handle, CURL* curl_handle);

    CURL_EXTERN CURLMcode curl_multi_remove_handle(CURLM* multi_handle, CURL* curl_handle);

    CURL_EXTERN CURLMcode curl_multi_setopt(CURLM* multi_handle, CURLMoption option, ...);

    CURL_EXTERN CURLMcode curl_multi_perform(CURLM* multi_handle, int* running_handles);

    CURL_EXTERN CURLMcode curl_multi_cleanup(CURLM* m);


#ifdef __cplusplus
} /* end of extern "C" */
#endif