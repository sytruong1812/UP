#include "trace.h"
#include "utils.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(A) (sizeof(A)/sizeof((A)[0]))
#endif

void Curl_set_in_callback(struct Curl_easy* data, bool value)
{
    if (data && data->multi)
    {
        data->multi->in_callback = value;
    }
}

bool Curl_is_in_callback(struct Curl_easy* data)
{
    return (data && data->multi && data->multi->in_callback);
}

void Curl_debug(struct Curl_easy* data, curl_infotype type, char* ptr, size_t size)
{
    if (data->set.verbose)
    {
        static const char s_infotype[CURLINFO_END][3] = { "* ", "< ", "> ", "{ ", "} ", "{ ", "} " };
        if (data->set.fdebug)
        {
            bool inCallback = Curl_is_in_callback(data);
            Curl_set_in_callback(data, TRUE);
            (void)(*data->set.fdebug)(data, type, ptr, size, data->set.debug_data);
            Curl_set_in_callback(data, inCallback);
        }
        else
        {
            switch (type)
            {
                case CURLINFO_TEXT:
                case CURLINFO_HEADER_OUT:
                case CURLINFO_HEADER_IN:
                    fwrite(s_infotype[type], 2, 1, data->set.err);
                    fwrite(ptr, size, 1, data->set.err);
                    break;
                default: /* nada */
                    break;
            }
        }
    }
}

/* Curl_failf() is for messages stating why we failed.
 * The message SHALL NOT include any LF or CR.
 */
void Curl_failf(struct Curl_easy* data, const char* fmt, ...)
{
    DEBUGASSERT(!strchr(fmt, '\n'));
    if (data->set.verbose || data->set.error_buffer)
    {
        va_list ap;
        int len;
        char error[CURL_ERROR_SIZE + 2];
        va_start(ap, fmt);
        len = mvsnprintf(error, CURL_ERROR_SIZE, fmt, ap);

        if (data->set.error_buffer && !data->state.error_buf)
        {
            strcpy(data->set.error_buffer, error);
            data->state.error_buf = TRUE; /* wrote error string */
        }
        error[len++] = '\n';
        error[len] = '\0';
        Curl_debug(data, CURLINFO_TEXT, error, len);
        va_end(ap);
    }
}

#if !defined(CURL_DISABLE_VERBOSE_STRINGS)

#define MAXINFO 2048    /* Curl_infof() is for info message along the way */

static void trc_infof(struct Curl_easy* data, struct curl_trc_feat* feat, const char* const fmt, va_list ap)
{
    int len = 0;
    char buffer[MAXINFO + 5];
    if (feat)
    {
        len = msnprintf(buffer, (MAXINFO + 1), "[%s] ", feat->name);
    }
    len += mvsnprintf(buffer + len, (MAXINFO + 1) - len, fmt, ap);
    /* too long, shorten with '...' */
    if (len >= MAXINFO)
    { 
        --len;
        buffer[len++] = '.';
        buffer[len++] = '.';
        buffer[len++] = '.';
    }
    buffer[len++] = '\n';
    buffer[len] = '\0';
    Curl_debug(data, CURLINFO_TEXT, buffer, len);
}

void Curl_infof(struct Curl_easy* data, const char* fmt, ...)
{
    DEBUGASSERT(!strchr(fmt, '\n'));
    if (Curl_trc_is_verbose(data))
    {
        va_list ap;
        va_start(ap, fmt);
        trc_infof(data, data->state.feat, fmt, ap);
        va_end(ap);
    }
}

struct curl_trc_feat Curl_trc_feat_read = 
{
  "READ",
  CURL_LOG_LVL_NONE,
};

void Curl_trc_read(struct Curl_easy* data, const char* fmt, ...)
{
    DEBUGASSERT(!strchr(fmt, '\n'));
    if (Curl_trc_ft_is_verbose(data, &Curl_trc_feat_read))
    {
        va_list ap;
        va_start(ap, fmt);
        trc_infof(data, &Curl_trc_feat_read, fmt, ap);
        va_end(ap);
    }
}

struct curl_trc_feat Curl_trc_feat_write = 
{
  "WRITE",
  CURL_LOG_LVL_NONE,
};

void Curl_trc_write(struct Curl_easy* data, const char* fmt, ...)
{
    DEBUGASSERT(!strchr(fmt, '\n'));
    if (Curl_trc_ft_is_verbose(data, &Curl_trc_feat_write))
    {
        va_list ap;
        va_start(ap, fmt);
        trc_infof(data, &Curl_trc_feat_write, fmt, ap);
        va_end(ap);
    }
}

#ifndef CURL_DISABLE_FTP
struct curl_trc_feat Curl_trc_feat_ftp = 
{
  "FTP",
  CURL_LOG_LVL_NONE,
};

void Curl_trc_ftp(struct Curl_easy* data, const char* fmt, ...)
{
    DEBUGASSERT(!strchr(fmt, '\n'));
    if (Curl_trc_ft_is_verbose(data, &Curl_trc_feat_ftp))
    {
        va_list ap;
        va_start(ap, fmt);
        trc_infof(data, &Curl_trc_feat_ftp, fmt, ap);
        va_end(ap);
    }
}
#endif /* !CURL_DISABLE_FTP */

#if !defined(CURL_DISABLE_WEBSOCKETS) && !defined(CURL_DISABLE_HTTP)
struct curl_trc_feat Curl_trc_feat_ws = 
{
  "WS",
  CURL_LOG_LVL_NONE,
};

void Curl_trc_ws(struct Curl_easy* data, const char* fmt, ...)
{
    DEBUGASSERT(!strchr(fmt, '\n'));
    if (Curl_trc_ft_is_verbose(data, &Curl_trc_feat_ws))
    {
        va_list ap;
        va_start(ap, fmt);
        trc_infof(data, &Curl_trc_feat_ws, fmt, ap);
        va_end(ap);
    }
}
#endif /* !CURL_DISABLE_WEBSOCKETS && !CURL_DISABLE_HTTP */

#define TRC_CT_NONE        (0)
#define TRC_CT_PROTOCOL    (1<<(0))
#define TRC_CT_NETWORK     (1<<(1))
#define TRC_CT_PROXY       (1<<(2))

struct trc_feat_def {
    struct curl_trc_feat* feat;
    unsigned int category;
};

static struct trc_feat_def trc_feats[] = {
  { &Curl_trc_feat_read,      TRC_CT_NONE },
  { &Curl_trc_feat_write,     TRC_CT_NONE },
#ifndef CURL_DISABLE_FTP
  { &Curl_trc_feat_ftp,       TRC_CT_PROTOCOL },
#endif
#if !defined(CURL_DISABLE_WEBSOCKETS) && !defined(CURL_DISABLE_HTTP)
  { &Curl_trc_feat_ws,        TRC_CT_PROTOCOL },
#endif
};

static void trc_apply_level_by_name(const char* const token, int lvl)
{
    size_t i;
    for (i = 0; i < ARRAYSIZE(trc_feats); ++i)
    {
        if (strcasecompare(token, trc_feats[i].feat->name))
        {
            trc_feats[i].feat->log_level = lvl;
            break;
        }
    }
}

static void trc_apply_level_by_category(int category, int lvl)
{
    size_t i;
    for (i = 0; i < ARRAYSIZE(trc_feats); ++i)
    {
        if (!category || (trc_feats[i].category & category))
            trc_feats[i].feat->log_level = lvl;
    }
}

static CURLcode trc_opt(const char* config)
{
    int lvl;
    char* token, * tok_buf, * tmp;

    tmp = _strdup(config);
    if (!tmp)
    {
        return CURLE_OUT_OF_MEMORY;
    }

    token = strtok_r(tmp, ", ", &tok_buf);
    while (token)
    {
        switch (*token)
        {
            case '-':
                lvl = CURL_LOG_LVL_NONE;
                ++token;
                break;
            case '+':
                lvl = CURL_LOG_LVL_INFO;
                ++token;
                break;
            default:
                lvl = CURL_LOG_LVL_INFO;
                break;
        }
        if (strcasecompare(token, "all"))
            trc_apply_level_by_category(TRC_CT_NONE, lvl);
        else if (strcasecompare(token, "protocol"))
            trc_apply_level_by_category(TRC_CT_PROTOCOL, lvl);
        else if (strcasecompare(token, "network"))
            trc_apply_level_by_category(TRC_CT_NETWORK, lvl);
        else if (strcasecompare(token, "proxy"))
            trc_apply_level_by_category(TRC_CT_PROXY, lvl);
        else
            trc_apply_level_by_name(token, lvl);

        token = strtok_r(NULL, ", ", &tok_buf);
    }
    free(tmp);
    return CURLE_OK;
}

CURLcode Curl_trc_opt(const char* config)
{
    CURLcode result = config ? trc_opt(config) : CURLE_OK;
#ifdef DEBUGBUILD
    /* CURL_DEBUG can override anything */
    if (!result)
    {
        const char* dbg_config = getenv("CURL_DEBUG");
        if (dbg_config)
            result = trc_opt(dbg_config);
    }
#endif /* DEBUGBUILD */
    return result;
}

CURLcode Curl_trc_init(void)
{
#ifdef DEBUGBUILD
    return Curl_trc_opt(NULL);
#else
    return CURLE_OK;
#endif
}

#else /* defined(CURL_DISABLE_VERBOSE_STRINGS) */

CURLcode Curl_trc_init(void)
{
    return CURLE_OK;
}

#endif /* !defined(CURL_DISABLE_VERBOSE_STRINGS) */