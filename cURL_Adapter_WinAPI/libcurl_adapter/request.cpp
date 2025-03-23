#include "request.h"
#include "urldata.h"

void Curl_req_init(struct SingleRequest* req)
{
	memset(req, 0, sizeof(*req));
}

void Curl_req_free(struct SingleRequest* req)
{
	Curl_safefree(req->location);
	Curl_safefree(req->newurl);
}

void Curl_req_hard_reset(struct SingleRequest* req, struct Curl_easy* data)
{
	Curl_safefree(req->location);
	Curl_safefree(req->newurl);
	req->req_good = FALSE;
	req->httpcode = 0;
	req->upgr101 = UPGR101_INIT;
	req->location = NULL;
	req->newurl = NULL;
	req->header = FALSE;
	req->content_range = FALSE;
	req->download_done = FALSE;
	req->eos_written = FALSE;
	req->eos_read = FALSE;
	req->eos_sent = FALSE;
	req->upload_done = FALSE;
	req->upload_aborted = FALSE;
	req->ignorebody = FALSE;
	req->http_bodyless = FALSE;
	req->chunk = FALSE;
	req->ignore_cl = FALSE;
	req->upload_chunky = FALSE;
	req->getheader = FALSE;
	req->no_body = data->set.opt_no_body;
	req->authneg = FALSE;
	req->shutdown = FALSE;
}

CURLcode Curl_req_start(SingleRequest* req, Curl_easy* data)
{
	req->done = FALSE;
	req->upload_done = FALSE;
	req->upload_aborted = FALSE;
	req->download_done = FALSE;
	req->eos_written = FALSE;
	req->eos_read = FALSE;
	req->eos_sent = FALSE;
	req->ignorebody = FALSE;
	req->shutdown = FALSE;
	req->header = TRUE; /* assume header */

	return CURLE_OK;
}

