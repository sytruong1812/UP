#pragma once
#include "curl.h"
#include "urldata.h"

#define CURL_ERROR_SIZE 256

#define CURL_LOG_LVL_NONE  0
#define CURL_LOG_LVL_INFO  1

struct Curl_cfilter;

CURLcode Curl_trc_init(void);
CURLcode Curl_trc_opt(const char* config);

void Curl_debug(struct Curl_easy* data, curl_infotype type, char* ptr, size_t size);
void Curl_failf(struct Curl_easy* data,const char* fmt, ...);

#define infof Curl_infof
#define failf Curl_failf
#define CURL_TRC_WRITE  Curl_trc_write
#define CURL_TRC_READ   Curl_trc_read

#ifndef CURL_DISABLE_FTP
#define CURL_TRC_FTP   Curl_trc_ftp
#endif
#if !defined(CURL_DISABLE_WEBSOCKETS) && !defined(CURL_DISABLE_HTTP)
#define CURL_TRC_WS    Curl_trc_ws
#endif

#ifndef CURL_DISABLE_VERBOSE_STRINGS

struct curl_trc_feat 
{
    const char* name;
    int log_level;
};
extern struct curl_trc_feat Curl_trc_feat_read;
extern struct curl_trc_feat Curl_trc_feat_write;

#define Curl_trc_is_verbose(data) \
            ((data) && (data)->set.verbose && \
            (!(data)->state.feat || \
             ((data)->state.feat->log_level >= CURL_LOG_LVL_INFO)))
#define Curl_trc_cf_is_verbose(cf, data) \
            (Curl_trc_is_verbose(data) && \
             (cf) && (cf)->cft->log_level >= CURL_LOG_LVL_INFO)
#define Curl_trc_ft_is_verbose(data, ft) \
            (Curl_trc_is_verbose(data) && \
             (ft)->log_level >= CURL_LOG_LVL_INFO)

void Curl_set_in_callback(struct Curl_easy* data, bool value);

bool Curl_is_in_callback(struct Curl_easy* data);

/**
 * Output an informational message when transfer's verbose logging is enabled.
 */
void Curl_infof(struct Curl_easy* data, const char* fmt, ...);

/**
 * Output an informational message when both transfer's verbose logging 
 * and connection filters verbose logging are enabled.
 */
void Curl_trc_write(struct Curl_easy* data, const char* fmt, ...);
void Curl_trc_read(struct Curl_easy* data, const char* fmt, ...);

#ifndef CURL_DISABLE_FTP
extern struct curl_trc_feat Curl_trc_feat_ftp;
void Curl_trc_ftp(struct Curl_easy* data, const char* fmt, ...);
#endif
#if !defined(CURL_DISABLE_WEBSOCKETS) && !defined(CURL_DISABLE_HTTP)
extern struct curl_trc_feat Curl_trc_feat_ws;
void Curl_trc_ws(struct Curl_easy* data, const char* fmt, ...);
#endif


#else /* defined(CURL_DISABLE_VERBOSE_STRINGS) */

/* All informational messages are not compiled in for size savings */
#define Curl_trc_is_verbose(d)        (FALSE)
#define Curl_trc_cf_is_verbose(x,y)   (FALSE)
#define Curl_trc_ft_is_verbose(x,y)   (FALSE)

static void Curl_infof(struct Curl_easy* data, const char* fmt, ...)
{
    (void)data; (void)fmt;
}

struct curl_trc_feat;

static void Curl_trc_write(struct Curl_easy* data, const char* fmt, ...)
{
    (void)data; (void)fmt;
}

static void Curl_trc_read(struct Curl_easy* data, const char* fmt, ...)
{
    (void)data; (void)fmt;
}

#ifndef CURL_DISABLE_FTP
static void Curl_trc_ftp(struct Curl_easy* data, const char* fmt, ...)
{
    (void)data; (void)fmt;
}
#endif

#if !defined(CURL_DISABLE_WEBSOCKETS) || !defined(CURL_DISABLE_HTTP)
static void Curl_trc_ws(struct Curl_easy* data, const char* fmt, ...)
{
    (void)data; (void)fmt;
}
#endif

#endif /* !defined(CURL_DISABLE_VERBOSE_STRINGS) */

