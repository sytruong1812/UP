#pragma once
#include "setup.h"
#include "utils.h"

enum expect100 
{
    EXP100_SEND_DATA,           /* enough waiting, just send the body now */
    EXP100_AWAITING_CONTINUE,   /* waiting for the 100 Continue header */
    EXP100_SENDING_REQUEST,     /* still sending the request but will wait for the 100 header once done with the request */
    EXP100_FAILED               /* used on 417 Expectation Failed */
};

enum upgrade101 
{
    UPGR101_INIT,               /* default state */
    UPGR101_WS,                 /* upgrade to WebSockets requested */
    UPGR101_H2,                 /* upgrade to HTTP/2 requested */
    UPGR101_RECEIVED,           /* 101 response received */
    UPGR101_WORKING             /* talking upgraded protocol */
};

struct SingleRequest 
{
    HINTERNET hSession;		    /* WinAPI - WinHttp.h */
    HINTERNET hConnect;		    /* WinAPI - WinHttp.h */
    HINTERNET hRequest;		    /* WinAPI - WinHttp.h */
    HINTERNET hWebSocket;	    /* WinAPI - WinHttp.h */

    int httpversion;            /* Version in response (09, 10, 11, etc.) */
    int httpcode;               /* error code from the 'HTTP/1.? XXX' or 'RTSP/1.? XXX' line */

    enum upgrade101 upgr101;    /* 101 upgrade state */

    char* newurl;               /* Set to the new URL to use when a redirect or a retry is wanted */
    char* location;             /* This points to an allocated version of the Location: header data */

    BIT(done);                  /* request is done */
    BIT(req_good);			    /* TODO: teddy set */
    BIT(header);                /* incoming data has HTTP header */
    BIT(content_range);         /* set TRUE if Content-Range: was found */
    BIT(download_done);         /* set to TRUE when download is complete */
    BIT(eos_written);           /* iff EOS has been written to client */
    BIT(eos_read);              /* iff EOS has been read from the client */
    BIT(eos_sent);              /* iff EOS has been sent to the server */
    BIT(rewind_read);           /* iff reader needs rewind at next start */
    BIT(upload_done);           /* set to TRUE when all request data has been sent */
    BIT(upload_aborted);        /* set to TRUE when upload was aborted. Will also show `upload_done` as TRUE. */
    BIT(ignorebody);            /* we read a response-body but we ignore it! */
    BIT(http_bodyless);         /* HTTP response status code is between 100 and 199, 204 or 304 */
    BIT(chunk);                 /* if set, this is a chunked transfer-encoding */
    BIT(resp_trailer);          /* response carried 'Trailer:' header field */
    BIT(ignore_cl);             /* ignore content-length */
    BIT(upload_chunky);         /* set TRUE if we are doing chunked transfer-encoding on upload */

    BIT(getheader);             /* TRUE if header parsing is wanted */
    BIT(no_body);               /* The response has no body */
    BIT(authneg);               /* TRUE when the auth phase has started, which means that we are creating a request with an auth header,
                                    but it is not the final request in the auth negotiation. */

    BIT(sendbuf_init);          /* sendbuf is initialized */
    BIT(shutdown);              /* request end will shutdown connection */
    BIT(shutdown_err_ignore);   /* errors in shutdown will not fail request */

};

void Curl_req_init(struct SingleRequest* req);

void Curl_req_free(struct SingleRequest* req);

void Curl_req_hard_reset(struct SingleRequest* req, struct Curl_easy* data);

CURLcode Curl_req_start(struct SingleRequest* req, struct Curl_easy* data);
CURLcode Curl_req_soft_reset(struct SingleRequest* req, struct Curl_easy* data);
CURLcode Curl_req_done(struct SingleRequest* req, struct Curl_easy* data, bool aborted);
CURLcode Curl_req_send(struct Curl_easy* data, struct dynbuf* buf);
bool Curl_req_done_sending(struct Curl_easy* data);
CURLcode Curl_req_send_more(struct Curl_easy* data);
bool Curl_req_want_send(struct Curl_easy* data);
bool Curl_req_sendbuf_empty(struct Curl_easy* data);
CURLcode Curl_req_abort_sending(struct Curl_easy* data);
CURLcode Curl_req_stop_send_recv(struct Curl_easy* data);
CURLcode Curl_req_set_upload_done(struct Curl_easy* data);