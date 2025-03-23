#pragma once
#include "utils.h"

#define GLOBAL_INIT_IS_THREADSAFE
/* true globals -- for curl_global_init() and curl_global_cleanup() */
static unsigned int  initialized;
static long          easy_init_flags;

#ifdef GLOBAL_INIT_IS_THREADSAFE

#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x600

static SRWLOCK s_lock = SRWLOCK_INIT;
#define global_init_lock() AcquireSRWLockExclusive(&s_lock)
#define global_init_unlock() ReleaseSRWLockExclusive(&s_lock)

#else

#define global_init_lock()
#define global_init_unlock()

#endif	/* _WIN32_WINNT */
#endif	/* GLOBAL_INIT_IS_THREADSAFE */

#ifdef  __cplusplus
extern "C" {
#endif

#define CURL_BLOB_COPY   1 /* tell libcurl to copy the data */
#define CURL_BLOB_NOCOPY 0 /* tell libcurl to NOT copy the data */

	struct curl_blob 
	{
		void* data;
		size_t len;
		unsigned int flags;
	};

	CURL_EXTERN CURL* curl_easy_init(void);
	CURL_EXTERN CURLcode curl_easy_setopt(CURL* curl, CURLoption option, ...);
	CURL_EXTERN CURLcode curl_easy_perform(CURL* curl);
	CURL_EXTERN void curl_easy_cleanup(CURL* curl);

	CURL_EXTERN void curl_easy_reset(CURL* curl);
	CURL_EXTERN CURL* curl_easy_duphandle(CURL* curl);
	CURL_EXTERN CURLcode curl_easy_upkeep(CURL* curl);
	CURL_EXTERN CURLcode curl_easy_getinfo(CURL* curl, CURLINFO info, ...);
	CURL_EXTERN CURLcode curl_easy_recv(CURL* curl, void* buffer, size_t buflen, size_t* n);
	CURL_EXTERN CURLcode curl_easy_send(CURL* curl, const void* buffer, size_t buflen, size_t* n);

#ifdef  __cplusplus
} /* end of extern "C" */
#endif