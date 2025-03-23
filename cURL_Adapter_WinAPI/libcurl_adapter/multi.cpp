#include "hash.h"
#include "multi.h"
#include "utils.h"
#include "urldata.h"

void Curl_init_connect(struct Curl_easy* data)
{
	data->state.in = data->set.in_set;
	data->state.fread_func = data->set.fread_func_set;
	data->state.upload = (data->state.httpreq == HTTPREQ_PUT);
}
static void before_perform(struct Curl_easy* data)
{

}
static void init_completed(struct Curl_easy* data)
{
	struct connectdata* conn = data->conn;
	if (conn)
	{
		Curl_node_remove(&data->conn_queue);
	}
	data->conn = NULL;
}

typedef void (*init_multistate_func)(struct Curl_easy* data);
static void mstate(struct Curl_easy* data, CURLMstate state)	
{
	CURLMstate oldstate = data->mstate;
	static const init_multistate_func finit[MSTATE_LAST] = {
	  NULL,					/* INIT */
	  NULL,					/* PENDING */
	  Curl_init_connect,    /* SETUP */
	  NULL,					/* CONNECT */
	  NULL,					/* RESOLVING */
	  NULL,					/* CONNECTING */
	  NULL,					/* TUNNELING */
	  NULL,					/* PROTOCONNECT */
	  NULL,					/* PROTOCONNECTING */
	  NULL,					/* DO */
	  NULL,					/* DOING */
	  NULL,					/* DOING_MORE */
	  before_perform,		/* DID */
	  NULL,					/* PERFORMING */
	  NULL,					/* RATELIMITING */
	  NULL,					/* DONE */
	  init_completed,		/* COMPLETED */
	  NULL					/* MSGSENT */
	};

	if (oldstate == state)
	{
		return;	/* do not bother when the new state is the same as the old state */
	}
	data->mstate = state;

	if (state == MSTATE_COMPLETED)
	{
		DEBUGASSERT(data->multi->num_alive > 0);	/* changing to COMPLETED means there is one less easy handle 'alive' */
		data->multi->num_alive--;
	}
	if (finit[state])
	{
		finit[state](data);		/* if this state has an init-function, run it */
	}
}
#define multistate(x,y) mstate(x,y)

/*================================[ Transfer ]===============================*/
static CURLcode Curl_setstropt(char** charp, const char* s)
{
	/* Release the previous storage at `charp' and replace by a dynamic storage copy of `s'.
	   Return CURLE_OK or CURLE_OUT_OF_MEMORY. */
	if (s)
	{
		if (strlen(s) > CURL_MAX_INPUT_LENGTH)
		{
			return CURLE_BAD_FUNCTION_ARGUMENT;
		}
		*charp = _strdup(s);
		if (!*charp)
		{
			return CURLE_OUT_OF_MEMORY;
		}
	}

	return CURLE_OK;
}

CURLcode Curl_pretransfer(struct Curl_easy* data)
{
	CURLcode result = CURLE_OK;
	if (!data->state.url)
	{
		return CURLE_URL_MALFORMAT;	        /* we cannot do anything without URL */
	}
	if (!data->state.url && data->set.uh)
	{
		CURLUcode uc;
		free(data->set.str[STRING_SET_URL]);
		uc = curl_url_get(data->set.uh, CURLUPART_URL, &data->set.str[STRING_SET_URL], 0);
		if (uc)
		{
			return CURLE_URL_MALFORMAT;
		}
	}
	if (data->set.str[STRING_USERAGENT])
	{
		Curl_safefree(data->state.aptr.uagent);
		data->state.aptr.uagent = aprintf("User-Agent: %s\r\n", data->set.str[STRING_USERAGENT]);
		if (!data->state.aptr.uagent)
		{
			return CURLE_OUT_OF_MEMORY;
		}
	}
	if (!result)
	{
		result = Curl_setstropt(&data->state.aptr.user, data->set.str[STRING_USERNAME]);
		result = Curl_setstropt(&data->state.aptr.passwd, data->set.str[STRING_PASSWORD]);
	}
	data->state.httpreq = data->set.method;
	data->state.url = data->set.str[STRING_SET_URL];
	data->state.httpwant = data->set.httpwant;
	data->state.httpversion = 0;

	return result;
}

/*=================================[ State ]===============================*/
static CURLMcode state_connect(struct Curl_multi* multi, struct Curl_easy* data, struct curltime* nowp, CURLcode* resultp) 
{

}

static CURLMcode state_resolving(struct Curl_multi* multi, struct Curl_easy* data, bool* stream_errorp, CURLcode* resultp)
{

}

static CURLMcode state_do(struct Curl_easy* data, bool* stream_errorp, CURLcode* resultp)
{

}

static CURLMcode state_ratelimiting(struct Curl_easy* data, struct curltime* nowp, CURLcode* resultp) 
{

}

/*=================================[ Multi Handle ]===============================*/
static CURLcode multi_done(struct Curl_easy* data, CURLcode status, bool premature)
{
	CURLcode result = CURLE_OK;
	struct connectdata* conn = data->conn;

#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
	DEBUGF(infof(data, "multi_done[%s]: status: %d prem: %d done: %d",  multi_statename[data->mstate], (int)status, (int)premature, data->state.done));
#else
	DEBUGF(infof(data, "multi_done: status: %d prem: %d done: %d",  (int)status, (int)premature, data->state.done));
#endif

	if (data->state.done)
	{
		return CURLE_OK;
	}
	/* Cleanup possible redirect junk */
	Curl_safefree(data->req.newurl);
	Curl_safefree(data->req.location);

	/* This calls the protocol-specific function pointer previously set */
	if (conn->handler->done)
	{
		result = conn->handler->done(data, status, premature);
	}
	else
	{
		result = status;
	}
	return result;
}

static bool multi_ischanged(struct Curl_multi* multi, bool clear)
{
	bool retval = multi->recheckstate;
	if (clear)
	{
		multi->recheckstate = FALSE;
	}
	return retval;
}

static CURLMcode multi_runsingle(struct Curl_multi* multi, struct Curl_easy* data)
{
	CURLMcode rc;
	CURLcode result = CURLE_OK;

	if (!GOOD_EASY_HANDLE(data))
	{
		return CURLM_BAD_EASY_HANDLE;
	}
	if (multi->dead)
	{
		result = CURLE_ABORTED_BY_CALLBACK;
		multi_done(data, result, FALSE);
		multistate(data, MSTATE_COMPLETED);
	}

	do
	{
		rc = CURLM_OK;
		bool stream_error = FALSE;
		if (multi_ischanged(multi, TRUE))
		{
			DEBUGF(infof(data, "multi changed, check CONNECT_PEND queue"));
		}
		if (data->mstate > MSTATE_CONNECT && data->mstate < MSTATE_COMPLETED)
		{
			DEBUGASSERT(data->conn);	/* Make sure we set the connection's current owner */
			if (!data->conn)
			{
				return CURLM_INTERNAL_ERROR;
			}
		}
		if ((data->mstate >= MSTATE_CONNECT) && (data->mstate < MSTATE_COMPLETED))
		{
			goto statemachine_end;		/* Skip the statemachine and go directly to error handling section. */
		}

		switch (data->mstate)
		{
			case MSTATE_INIT:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_INIT");
				result = Curl_pretransfer(data);
				if (result) {
					break;
				}
				multistate(data, MSTATE_SETUP);
				FALLTHROUGH();
			case MSTATE_SETUP:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_SETUP");
				if (data->set.timeout)
				{
					//Curl_expire(data, data->set.timeout, EXPIRE_TIMEOUT);
				}				
				if (data->set.connecttimeout)
				{
					//Curl_expire(data, data->set.connecttimeout, EXPIRE_CONNECTTIMEOUT);
				}
				multistate(data, MSTATE_CONNECT);
				FALLTHROUGH();
			case MSTATE_CONNECT:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_CONNECT");


				break;
			case MSTATE_RESOLVING:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_RESOLVING");


				break;

#ifndef CURL_DISABLE_HTTP
			case MSTATE_TUNNELING:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_TUNNELING");


				break;
#endif
			case MSTATE_CONNECTING:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_CONNECTING");


				break;
			case MSTATE_PROTOCONNECT:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_PROTOCONNECT");


				break;
			case MSTATE_PROTOCONNECTING:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_PROTOCONNECTING");


				break;
			case MSTATE_DO:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_DO");



				break;
			case MSTATE_DOING:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_DOING");




				break;
			case MSTATE_DOING_MORE:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_DOING_MORE");


				break;
			case MSTATE_DID:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_DID");


				break;
			case MSTATE_RATELIMITING:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_RATELIMITING");


				break;
			case MSTATE_PERFORMING:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_PERFORMING");


				multistate(data, MSTATE_DONE);
				break;
			case MSTATE_DONE:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_DONE");


				multistate(data, MSTATE_COMPLETED);
				break;
			case MSTATE_COMPLETED:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_COMPLETED");



				break;
			case MSTATE_PENDING:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_PENDING");



				DEBUGASSERT(0);
				break;
			case MSTATE_MSGSENT:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> MSTATE_MSGSENT");



				DEBUGASSERT(0);
				break;
			default:
				infof(data, "\t\t\t\t\t\t\t\t\t\t-----> CURLM_INTERNAL_ERROR");

				return CURLM_INTERNAL_ERROR;
		}

statemachine_end:

		if (data->mstate < MSTATE_COMPLETED)
		{

		}
		if (MSTATE_COMPLETED == data->mstate)
		{

		}

	} while ((rc == CURLM_CALL_MULTI_PERFORM) || multi_ischanged(multi, FALSE));

	data->result = result;
	return rc;
}

CURLM* curl_multi_init(void) 
{
	struct Curl_multi* multi = (Curl_multi*)calloc(1, sizeof(struct Curl_multi));
	if (!multi)
	{
		return NULL;
	}
	multi->magic = CURL_MULTI_HANDLE;

	Curl_llist_init(&multi->process, NULL);
	Curl_llist_init(&multi->pending, NULL);
	Curl_llist_init(&multi->msgsent, NULL);

	return multi;
}

CURLMcode curl_multi_add_handle(CURLM* m, CURL* d) 
{
	CURLMcode rc = CURLM_OK;
	struct Curl_multi* multi = (Curl_multi*)m;
	struct Curl_easy* data = (Curl_easy*)d;

	/* First, make some basic checks that the CURLM handle is a good handle */
	if (!GOOD_MULTI_HANDLE(multi))
	{
		return CURLM_BAD_HANDLE;
	}
	if (data->multi)
	{
		return CURLM_ADDED_ALREADY;
	}
	if (data->multi_easy)
	{
		curl_multi_cleanup(data->multi_easy);
		data->multi_easy = NULL;
	}

	/* make the Curl_easy refer back to this multi handle - before Curl_expire() is called. */
	data->multi = multi;
	/* set the easy handle */
	multistate(data, MSTATE_INIT);
	/* add the easy handle to the process list */
	Curl_llist_append(&multi->process, data, &data->multi_queue);
	/* increase the node-counter */
	multi->num_easy++;
	/* increase the alive-counter */
	multi->num_alive++;

	return CURLM_OK;
}

CURLMcode curl_multi_remove_handle(CURLM* m, CURL* d) 
{
	struct Curl_multi* multi = (Curl_multi*)m;
	struct Curl_easy* data = (Curl_easy*)d;

	CURLMcode rc = CURLM_OK;
	struct Curl_llist_node* e = NULL;
	bool removed_timer = FALSE;

	/* First, make some basic checks that the CURLM handle is a good handle */
	if (!GOOD_MULTI_HANDLE(multi))
	{
		return CURLM_BAD_HANDLE;
	}
	/* Verify that we got a somewhat good easy handle too */
	if (!GOOD_EASY_HANDLE(data) || !multi->num_easy)
	{
		return CURLM_BAD_EASY_HANDLE;
	}
	/* Prevent users from trying to remove same easy handle more than once */
	if (!data->multi)
	{
		return CURLM_OK; /* it is already removed so let's say it is fine! */
	}
	/* Prevent users from trying to remove an easy handle from the wrong multi */
	if (data->multi != multi)
	{
		return CURLM_BAD_EASY_HANDLE;
	}

	bool premature = (data->mstate < MSTATE_COMPLETED);

	/* If the 'state' is not INIT or COMPLETED, we might need to do something
	   nice to put the easy_handle in a good known state when this returns. */
	if (premature)
	{
		/* this handle is "alive" so we need to count down 
		   the total number of alive connections when this is removed */
		multi->num_alive--;
	}
	if (data->conn && data->mstate > MSTATE_DO && data->mstate < MSTATE_COMPLETED)
	{
		/* Set connection owner so that the DONE function closes it. We can
		   safely do this here since connection is killed. */
	}
	if (data->conn)
	{
		(void)multi_done(data, data->result, premature);
	}

	/* the handle is in a list, remove it from whichever it is */
	Curl_node_remove(&data->multi_queue);

	data->mstate = MSTATE_COMPLETED;
	data->multi = NULL; /* clear the association to this multi handle */
	multi->num_easy--;	/* one less to care about now */

	return CURLM_OK;
}

CURLMcode curl_multi_setopt(CURLM* m, CURLMoption option, ...) {
	va_list param;
	unsigned long uarg;

	CURLMcode res = CURLM_OK;
	struct Curl_multi* multi = (Curl_multi*)m;

	if (!GOOD_MULTI_HANDLE(multi))
	{
		return CURLM_BAD_HANDLE;
	}

	va_start(param, option);
	switch (option)
	{
		case CURLMOPT_SOCKETFUNCTION:
		case CURLMOPT_SOCKETDATA:
		case CURLMOPT_PUSHFUNCTION:
		case CURLMOPT_PUSHDATA:
		case CURLMOPT_PIPELINING:
		case CURLMOPT_TIMERFUNCTION:
		case CURLMOPT_TIMERDATA:
		case CURLMOPT_MAXCONNECTS:
			uarg = va_arg(param, unsigned long);
			if (uarg <= UINT_MAX) {
				multi->maxconnects = (unsigned int)uarg;
			}
			break;
		case CURLMOPT_MAX_HOST_CONNECTIONS:
			multi->max_host_connections = va_arg(param, long);
			break;
		case CURLMOPT_MAX_TOTAL_CONNECTIONS:
			multi->max_total_connections = va_arg(param, long);
			multi->max_shutdown_connections = va_arg(param, long);
			break;
			/* options formerly used for pipelining */
		case CURLMOPT_MAX_PIPELINE_LENGTH:
			break;
		case CURLMOPT_CONTENT_LENGTH_PENALTY_SIZE:
			break;
		case CURLMOPT_CHUNK_LENGTH_PENALTY_SIZE:
			break;
		case CURLMOPT_PIPELINING_SITE_BL:
			break;
		case CURLMOPT_PIPELINING_SERVER_BL:
			break;
		case CURLMOPT_MAX_CONCURRENT_STREAMS:
			break;
		default:
			res = CURLM_UNKNOWN_OPTION;
			break;
	}
	va_end(param);
	return res;
}

CURLMcode curl_multi_perform(CURLM* m, int* running_handles) 
{
	CURLMcode return_code = CURLM_OK;
	struct Curl_llist_node* e = NULL;
	struct Curl_llist_node* n = NULL;
	struct Curl_multi* multi = (Curl_multi*)m;

	if (!GOOD_MULTI_HANDLE(multi))
	{
		return CURLM_BAD_HANDLE;
	}
	if (multi->in_callback)
	{
		return CURLM_RECURSIVE_API_CALL;
	}

	for (e = Curl_llist_head(&multi->process); e; e = n)
	{
		struct Curl_easy* data = (Curl_easy*)Curl_node_elem(e);

		n = Curl_node_next(e);

		CURLMcode result = multi_runsingle(multi, data);
		if (result) {
			return_code = result;
		}
	}
	if (running_handles)
	{
		*running_handles = (int)multi->num_alive;
	}
	return return_code;
}

CURLMcode curl_multi_cleanup(CURLM* m) 
{
	struct Curl_multi* multi = (Curl_multi*)m;
	if (GOOD_MULTI_HANDLE(multi))
	{
		struct Curl_llist_node* e;
		struct Curl_llist_node* n;
		if (multi->in_callback)
		{
			return CURLM_RECURSIVE_API_CALL;
		}
		for (e = Curl_llist_head(&multi->process); e; e = n)	
		{
			/* First remove all remaining easy handles */
			struct Curl_easy* data = (Curl_easy*)Curl_node_elem(e);
			if (!GOOD_EASY_HANDLE(data))
			{
				return CURLM_BAD_HANDLE;
			}
			n = Curl_node_next(e);
			if (!data->state.done && data->conn)
			{
				(void)multi_done(data, CURLE_OK, TRUE);	/* if DONE was never called for this handle */
			}
			data->multi = NULL; /* clear the association */
		}

		multi->magic = 0; /* not good anymore */

		free(multi);

		return CURLM_OK;
	}
	return CURLM_BAD_HANDLE;
}



