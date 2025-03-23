#include "ftp.h"
#include "urldata.h"

static CURLcode ftp_setup_connection(struct Curl_easy* data, struct connectdata* conn) 
{
	return CURLE_OK;
}

static CURLcode ftp_do(struct Curl_easy* data, bool* done) 
{
	return CURLE_OK;
}

static CURLcode ftp_done(struct Curl_easy* data, CURLcode, bool premature) 
{
	return CURLE_OK;
}

static CURLcode ftp_connect(struct Curl_easy* data, bool* done) 
{
	return CURLE_OK;
}

static CURLcode ftp_disconnect(struct Curl_easy* data, struct connectdata* conn, bool dead_connection) 
{
	return CURLE_OK;
}

static CURLcode ftp_do_more(struct Curl_easy* data, int* completed) 
{
	return CURLE_OK;
}

static CURLcode ftp_multi_statemach(struct Curl_easy* data, bool* done) 
{
	return CURLE_OK;
}

static CURLcode ftp_doing(struct Curl_easy* data, bool* dophase_done) 
{
	return CURLE_OK;
}

const struct Curl_handler Curl_handler_ftp = {
  "ftp",                           /* scheme */
  ftp_setup_connection,            /* setup_connection */
  ftp_do,                          /* do_it */
  ftp_done,                        /* done */
  ftp_do_more,                     /* do_more */
  ftp_connect,                     /* connect_it */
  ftp_multi_statemach,             /* connecting */
  ftp_doing,                       /* doing */
  ftp_disconnect,                  /* disconnect */
  ZERO_NULL,                       /* write_resp */
  ZERO_NULL,                       /* write_resp_hd */
  ZERO_NULL,                       /* connection_check */
  ZERO_NULL,                       /* attach connection */
  PORT_FTP,                        /* defport */
  CURLPROTO_FTP,                   /* protocol */
  CURLPROTO_FTP,                   /* family */
  PROTOPT_DUAL | PROTOPT_CLOSEACTION | PROTOPT_NEEDSPWD |
  PROTOPT_NOURLQUERY | PROTOPT_PROXY_AS_HTTP |
  PROTOPT_WILDCARD /* flags */
};

#ifdef USE_SSL
const struct Curl_handler Curl_handler_ftps = {
  "ftps",                          /* scheme */
  ftp_setup_connection,            /* setup_connection */
  ftp_do,                          /* do_it */
  ftp_done,                        /* done */
  ftp_do_more,                     /* do_more */
  ftp_connect,                     /* connect_it */
  ftp_multi_statemach,             /* connecting */
  ftp_doing,                       /* doing */
  ftp_disconnect,                  /* disconnect */
  ZERO_NULL,                       /* write_resp */
  ZERO_NULL,                       /* write_resp_hd */
  ZERO_NULL,                       /* connection_check */
  ZERO_NULL,                       /* attach connection */
  PORT_FTPS,                       /* defport */
  CURLPROTO_FTPS,                  /* protocol */
  CURLPROTO_FTP,                   /* family */
  PROTOPT_SSL | PROTOPT_DUAL | PROTOPT_CLOSEACTION |
  PROTOPT_NEEDSPWD | PROTOPT_NOURLQUERY | PROTOPT_WILDCARD /* flags */
};
#endif