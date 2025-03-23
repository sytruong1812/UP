#include "url.h"
#include "curl.h"
#include "setopt.h"
#include "urldata.h"
#include "request.h"

CURLcode curl_global_init(long flags)
{
	CURLcode result = CURLE_OK;
	global_init_lock();
	if (initialized++)
	{
		return CURLE_OK;
	}
	if (Curl_trc_init())
	{
		initialized--; /* undo the increase */
		DEBUGF(fprintf(stderr, "Error: Curl_trc_init failed\n"));
		return CURLE_FAILED_INIT;
	}
	global_init_unlock();
	return CURLE_OK;
}

void curl_global_cleanup(void)
{
	global_init_lock();
	if (!initialized)
	{
		global_init_unlock();
		return;
	}
	if (--initialized)
	{
		global_init_unlock();
		return;
	}
	easy_init_flags = 0;
	global_init_unlock();
}

CURLcode curl_global_trace(const char* config)
{
#ifndef CURL_DISABLE_VERBOSE_STRINGS
	CURLcode result;
	global_init_lock();

	result = Curl_trc_opt(config);

	global_init_unlock();

	return result;
#else
	(void)config;
	return CURLE_OK;
#endif
}

CURL* curl_easy_init(void) 
{
	CURLcode result = CURLE_OK;
	Curl_easy* data = (Curl_easy*)calloc(1, sizeof(struct Curl_easy));
	if (!data)
	{
		DEBUGF(fprintf(stderr, "Error: Calloc of Curl_easy failed!\n"));
		return NULL;
	}
	data->magic = CURLEASY_MAGIC_NUMBER;

	Curl_req_init(&data->req);

	result = Curl_init_userdefined(data);
	if (result)
	{
		DEBUGF(fprintf(stderr, "Error: Curl_open failed!\n"));
		Curl_req_free(&data->req);
		Curl_freeset(data);
		return NULL;
	}
	return data;
}

CURLcode curl_easy_setopt(CURL* curl, CURLoption tag, ...)
{
	va_list arg;
	CURLcode result;
	struct Curl_easy* data = (struct Curl_easy*)curl;
	if (!data) {
		return CURLE_BAD_FUNCTION_ARGUMENT;
	}
	va_start(arg, tag);
	result = Curl_vsetopt(data, tag, arg);
	va_end(arg);
	return result;
}

static CURLcode easy_transfer(struct Curl_multi* multi)
{
	bool done = FALSE;
	CURLMcode mcode = CURLM_OK;
	CURLcode result = CURLE_OK;

	while (!done && !mcode)
	{
		int still_running = 0;
		mcode = curl_multi_perform(multi, &still_running);
		if (!mcode && !still_running)
		{

		}
	}
	/* Make sure to return some kind of error if there was a multi problem */
	if (mcode)
	{
		result = (mcode == CURLM_OUT_OF_MEMORY) ? CURLE_OUT_OF_MEMORY : CURLE_BAD_FUNCTION_ARGUMENT;
	}
	return result;
}

CURLcode curl_easy_perform(CURL* curl)
{
	CURLcode result = CURLE_OK;
	struct Curl_easy* data = (struct Curl_easy*)curl;
	if (!data)
	{
		return CURLE_BAD_FUNCTION_ARGUMENT;
	}
	if (data->set.error_buffer)
	{
		data->set.error_buffer[0] = 0;
	}
	if (data->multi)
	{
		failf(data, "Easy handle already used in multi handle!");
		return CURLE_FAILED_INIT;
	}

	struct Curl_multi* multi;
	if (data->multi_easy)
	{
		multi = data->multi_easy;
	}
	else
	{
		multi = (Curl_multi*)curl_multi_init();
		if (!multi)
		{
			return CURLE_OUT_OF_MEMORY;
		}
	}
	if (multi->in_callback)
	{
		return CURLE_RECURSIVE_API_CALL;
	}

	/* Copy the MAXCONNECTS option to the multi handle */
	curl_multi_setopt(multi, CURLMOPT_MAXCONNECTS, (long)data->set.maxconnects);
	
	/* Pretend it does not exist */
	data->multi_easy = NULL;

	CURLMcode mcode = curl_multi_add_handle(multi, data);
	if (mcode)
	{
		curl_multi_cleanup(multi);
		if (mcode == CURLM_OUT_OF_MEMORY)
		{
			return CURLE_OUT_OF_MEMORY;
		}
		return CURLE_FAILED_INIT;
	}
	data->multi_easy = multi;

	result = easy_transfer(multi);

	(void)curl_multi_remove_handle(multi, data);

	return result;
}

void curl_easy_cleanup(CURL* ptr) 
{
	struct Curl_easy* data = (Curl_easy*)ptr;
	if (GOOD_EASY_HANDLE(data))
	{
		if (data->multi) {
			curl_multi_remove_handle(data->multi, data);
		}
		if (data->multi_easy) {
			curl_multi_cleanup(data->multi_easy);
			data->multi_easy = NULL;
		}
		data->magic = 0;

		for (int i = (enum dupstring)0; i < STRING_LAST; i++)
		{
			Curl_safefree(data->set.str[i]);
		}

		for (int j = (enum dupblob)0; j < BLOB_LAST; j++)
		{
			Curl_safefree(data->set.blobs[j]);
		}

		if (data->state.url_alloc)
		{
			Curl_safefree(data->state.url);
			data->state.url_alloc = FALSE;
		}
		data->state.url = NULL;
	}
}
