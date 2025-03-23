#pragma once
#include "curl.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*================================[memdebug.h]====================================*/
#undef DEBUGASSERT
#if defined(DEBUGBUILD)
#define DEBUGASSERT(x) assert(x)
#else
#define DEBUGASSERT(x) do { } while(0)
#endif

#define Curl_safefree(ptr) do { free((ptr)); (ptr) = NULL;} while(0)

/*================================[curl_ctype.h]====================================*/

#define ISLOWHEXALHA(x) (((x) >= 'a') && ((x) <= 'f'))
#define ISUPHEXALHA(x) (((x) >= 'A') && ((x) <= 'F'))

#define ISLOWCNTRL(x) ((unsigned char)(x) <= 0x1f)
#define IS7F(x) ((x) == 0x7f)

#define ISLOWPRINT(x) (((x) >= 9) && ((x) <= 0x0d))

#define ISPRINT(x)  (ISLOWPRINT(x) || (((x) >= ' ') && ((x) <= 0x7e)))
#define ISGRAPH(x)  (ISLOWPRINT(x) || (((x) > ' ') && ((x) <= 0x7e)))
#define ISCNTRL(x) (ISLOWCNTRL(x) || IS7F(x))
#define ISALPHA(x) (ISLOWER(x) || ISUPPER(x))
#define ISXDIGIT(x) (ISDIGIT(x) || ISLOWHEXALHA(x) || ISUPHEXALHA(x))
#define ISALNUM(x)  (ISDIGIT(x) || ISLOWER(x) || ISUPPER(x))
#define ISUPPER(x)  (((x) >= 'A') && ((x) <= 'Z'))
#define ISLOWER(x)  (((x) >= 'a') && ((x) <= 'z'))
#define ISDIGIT(x)  (((x) >= '0') && ((x) <= '9'))
#define ISBLANK(x)  (((x) == ' ') || ((x) == '\t'))
#define ISSPACE(x)  (ISBLANK(x) || (((x) >= 0xa) && ((x) <= 0x0d)))
#define ISURLPUNTCS(x) (((x) == '-') || ((x) == '.') || ((x) == '_') || \
                        ((x) == '~'))
#define ISUNRESERVED(x) (ISALNUM(x) || ISURLPUNTCS(x))

/*================================[strdup.h]====================================*/

char* Curl_strdup(const char* str);
wchar_t* Curl_wcsdup(const wchar_t* src);
void* Curl_memdup(const void* src, size_t buffer_length);
void* Curl_memdup0(const char* src, size_t length);
void* Curl_saferealloc(void* ptr, size_t size);

/*================================[strtoofft.h]====================================*/
#define strtooff strtoll

typedef enum 
{
    CURL_OFFT_OK,    /* parsed fine */
    CURL_OFFT_FLOW,  /* over or underflow */
    CURL_OFFT_INVAL  /* nothing was parsed */
} CURLofft;

CURLofft curlx_strtoofft(const char* str, char** endp, int base, curl_off_t* num);

/*================================[strtok.h]====================================*/
#include <stddef.h>

#ifndef HAVE_STRTOK_R
char* Curl_strtok_r(char* s, const char* delim, char** last);
#define strtok_r Curl_strtok_r
#else
#include <string.h>
#endif

/*================================[strcase.h]====================================*/
#define STRCONST(x) x, sizeof(x)-1
#define strcasecompare(a,b) curl_strequal(a,b)
#define strncasecompare(a,b,c) curl_strnequal(a,b,c)
#define checkprefix(a,b) curl_strnequal(b, STRCONST(a))

char Curl_raw_toupper(char in);
char Curl_raw_tolower(char in);

void Curl_strntoupper(char* dest, const char* src, size_t n);
void Curl_strntolower(char* dest, const char* src, size_t n);

bool Curl_safecmp(char* a, char* b);
int Curl_timestrcmp(const char* first, const char* second);

/*================================[warnless.h]====================================*/
#define CURLX_FUNCTION_CAST(target_type, func) (target_type)(void (*) (void))(func)

unsigned short curlx_ultous(unsigned long ulnum);
unsigned char curlx_ultouc(unsigned long ulnum);
int curlx_uztosi(size_t uznum);
curl_off_t curlx_uztoso(size_t uznum);
unsigned long curlx_uztoul(size_t uznum);
unsigned int curlx_uztoui(size_t uznum);
int curlx_sltosi(long slnum);
unsigned int curlx_sltoui(long slnum);
unsigned short curlx_sltous(long slnum);
ssize_t curlx_uztosz(size_t uznum);
size_t curlx_sotouz(curl_off_t sonum);
int curlx_sztosi(ssize_t sznum);
unsigned short curlx_uitous(unsigned int uinum);
size_t curlx_sitouz(int sinum);

#if defined(_WIN32)
ssize_t curlx_read(int fd, void* buf, size_t count);
ssize_t curlx_write(int fd, const void* buf, size_t count);
#endif /* _WIN32 */

#ifndef HEADER_CURL_WARNLESS_H_REDEFS
#define HEADER_CURL_WARNLESS_H_REDEFS

#if defined(_WIN32)
#undef  read
#define read(fd, buf, count)  curlx_read(fd, buf, count)
#undef  write
#define write(fd, buf, count) curlx_write(fd, buf, count)
#endif

#endif /* HEADER_CURL_WARNLESS_H_REDEFS */

/*================================[escape.h]====================================*/
enum urlreject {
    REJECT_NADA = 2,
    REJECT_CTRL = 3,
    REJECT_ZERO = 4
};
CURLcode Curl_urldecode(const char* string, size_t length, char** ostring, size_t* olen, enum urlreject ctrl);
void Curl_hexencode(const unsigned char* src, size_t len, unsigned char* out, size_t olen);

/*================================[mprintf.h]====================================*/
#include <stdarg.h>
#include <stdio.h>

#define MERR_OK        0
#define MERR_MEM       1
#define MERR_TOO_LARGE 2

CURL_EXTERN int curl_mprintf(const char* format, ...);
CURL_EXTERN int curl_mfprintf(FILE* fd, const char* format, ...);
CURL_EXTERN int curl_msprintf(char* buffer, const char* format, ...);
CURL_EXTERN int curl_msnprintf(char* buffer, size_t maxlength, const char* format, ...);
CURL_EXTERN int curl_mvprintf(const char* format, va_list args);
CURL_EXTERN int curl_mvfprintf(FILE* fd, const char* format, va_list args);
CURL_EXTERN int curl_mvsprintf(char* buffer, const char* format, va_list args);
CURL_EXTERN int curl_mvsnprintf(char* buffer, size_t maxlength, const char* format, va_list args);
CURL_EXTERN char* curl_maprintf(const char* format, ...);
CURL_EXTERN char* curl_mvaprintf(const char* format, va_list args);

# define printf curl_mprintf
# define fprintf curl_mfprintf
# define msnprintf curl_msnprintf
# define vprintf curl_mvprintf
# define vfprintf curl_mvfprintf
# define mvsnprintf curl_mvsnprintf
# define aprintf curl_maprintf
# define vaprintf curl_mvaprintf

/*================================[dynbuf.h]====================================*/
#ifndef BUILDING_LIBCURL

#define curlx_dynbuf dynbuf /* for the struct name */

void Curl_dyn_init(struct dynbuf* s, size_t toobig);
void Curl_dyn_free(struct dynbuf* s);
CURLcode Curl_dyn_addn(struct dynbuf* s, const void* mem, size_t len);
CURLcode Curl_dyn_add(struct dynbuf* s, const char* str);
CURLcode Curl_dyn_addf(struct dynbuf* s, const char* fmt, ...);
CURLcode Curl_dyn_vaddf(struct dynbuf* s, const char* fmt, va_list ap);
void Curl_dyn_reset(struct dynbuf* s);
CURLcode Curl_dyn_tail(struct dynbuf* s, size_t trail);
CURLcode Curl_dyn_setlen(struct dynbuf* s, size_t set);
char* Curl_dyn_ptr(const struct dynbuf* s);
unsigned char* Curl_dyn_uptr(const struct dynbuf* s);
size_t Curl_dyn_len(const struct dynbuf* s);

#endif

struct dynbuf {
    char* bufr;    /* point to a null-terminated allocated buffer */
    size_t leng;   /* number of bytes *EXCLUDING* the null-terminator */
    size_t allc;   /* size of the current allocation */
    size_t toobig; /* size limit for the buffer */
#ifdef DEBUGBUILD
    int init;     /* detect API usage mistakes */
#endif
};

/* The implementation of this function exists in mprintf.c 
   Returns 0 on success, -1 on error */
int Curl_dyn_vprintf(struct dynbuf* dyn, const char* format, va_list ap_save);

/* Dynamic buffer max sizes */
#define DYN_DOH_RESPONSE    3000
#define DYN_DOH_CNAME       256
#define DYN_PAUSE_BUFFER    (64 * 1024 * 1024)
#define DYN_HAXPROXY        2048
#define DYN_HTTP_REQUEST    (1024*1024)
#define DYN_APRINTF         8000000
#define DYN_RTSP_REQ_HEADER (64*1024)
#define DYN_TRAILERS        (64*1024)
#define DYN_PROXY_CONNECT_HEADERS 16384
#define DYN_QLOG_NAME       1024
#define DYN_H1_TRAILER      4096
#define DYN_PINGPPONG_CMD   (64*1024)
#define DYN_IMAP_CMD        (64*1024)
#define DYN_MQTT_RECV       (64*1024)


/*============================[curl_multibyte.h]===================================*/

#define curlx_unicodefree(ptr)  do { if(ptr) { (free)(ptr);(ptr) = NULL; } } while(0)

#if defined(_WIN32)
wchar_t* curlx_convert_UTF8_to_wchar(const char* str_utf8);
char* curlx_convert_wchar_to_UTF8(const wchar_t* str_w);
#endif /* WIN32 */

#if defined(UNICODE) && defined(_WIN32)

#define curlx_convert_UTF8_to_tchar(ptr) curlx_convert_UTF8_to_wchar((ptr))
#define curlx_convert_tchar_to_UTF8(ptr) curlx_convert_wchar_to_UTF8((ptr))
typedef union 
{
    unsigned short* tchar_ptr;
    const unsigned short* const_tchar_ptr;
    unsigned short* tbyte_ptr;
    const unsigned short* const_tbyte_ptr;
} xcharp_u;

#else

#define curlx_convert_UTF8_to_tchar(ptr) (strdup)(ptr)
#define curlx_convert_tchar_to_UTF8(ptr) (strdup)(ptr)
typedef union 
{
    char* tchar_ptr;
    const char* const_tchar_ptr;
    unsigned char* tbyte_ptr;
    const unsigned char* const_tbyte_ptr;
} xcharp_u;

#endif /* UNICODE && _WIN32 */

/*================================[idn.h]====================================*/
#define IDN_MAX_LENGTH 255

struct hostname 
{
    char* rawalloc;         /* allocated "raw" version of the name */
    char* encalloc;         /* allocated IDN-encoded version of the name */
    char* name;             /* name to use internally, might be encoded, might be raw */
    const char* dispname;   /* name to display, as 'name' might be encoded */
};

bool Curl_is_ASCII_name(const char* hostname);
CURLcode Curl_idnconvert_hostname(struct hostname* host);

#if defined(USE_WIN32_IDN)
#define USE_IDN
void Curl_free_idnconverted_hostname(struct hostname* host);
CURLcode Curl_idn_decode(const char* input, char** output);
CURLcode Curl_idn_encode(const char* input, char** output);
#else
#define Curl_free_idnconverted_hostname(x)
#define Curl_idn_decode(x) NULL
#endif

/*=======================[inet_pton.h & inet_nton.h]=========================*/
int Curl_inet_pton(int, const char*, void*);

#ifdef HAVE_INET_PTON
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#define Curl_inet_pton(x,y,z) inet_pton(x,y,z)
#endif

char* Curl_inet_ntop(int af, const void* addr, char* buf, size_t size);

#ifdef HAVE_INET_NTOP
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef _WIN32
#define Curl_inet_ntop(af,addr,buf,size) inet_ntop(af, addr, buf, size)
#else
#define Curl_inet_ntop(af,addr,buf,size) inet_ntop(af, addr, buf, (curl_socklen_t)(size))
#endif
#endif

/*==========================[timeval.h & timediff.h ]==============================*/
#if defined(__STDC__) || defined(_MSC_VER) || defined(__cplusplus)
  /* This compiler is believed to have an ISO compatible preprocessor */
#define CURL_ISOCPP
#else
  /* This compiler is believed NOT to have an ISO compatible preprocessor */
#undef CURL_ISOCPP
#endif

#  ifdef CURL_ISOCPP
#    define CURLINC_OFF_T_C_HLPR2(Val,Suffix) Val ## Suffix
#  else
#    define CURLINC_OFF_T_C_HLPR2(Val,Suffix) Val/**/Suffix
#  endif
#  define CURLINC_OFF_T_C_HLPR1(Val,Suffix) CURLINC_OFF_T_C_HLPR2(Val,Suffix)
#  define CURL_OFF_T_C(Val)  CURLINC_OFF_T_C_HLPR1(Val,CURL_SUFFIX_CURL_OFF_T)
#  define CURL_OFF_TU_C(Val) CURLINC_OFF_T_C_HLPR1(Val,CURL_SUFFIX_CURL_OFF_TU)

#define CURL_OFF_T_MAX CURL_OFF_T_C(0x7FFFFFFFFFFFFFFF)
#define CURL_OFF_T_MIN (-CURL_OFF_T_MAX - CURL_OFF_T_C(1))

/* Use a larger type even for 32-bit time_t systems so that we can keep microsecond accuracy in it */
typedef curl_off_t timediff_t;
#define FMT_TIMEDIFF_T FMT_OFF_T

#define TIMEDIFF_T_MAX CURL_OFF_T_MAX
#define TIMEDIFF_T_MIN CURL_OFF_T_MIN

struct timeval* curlx_mstotv(struct timeval* tv, timediff_t ms);
timediff_t curlx_tvtoms(struct timeval* tv);

struct curltime {
    time_t tv_sec; /* seconds */
    int tv_usec;   /* microseconds */
};

struct curltime Curl_now(void);
timediff_t Curl_timediff(struct curltime newer, struct curltime older);
timediff_t Curl_timediff_ceil(struct curltime newer, struct curltime older);
timediff_t Curl_timediff_us(struct curltime newer, struct curltime older);


#ifdef  __cplusplus
} /* end of extern "C" */
#endif