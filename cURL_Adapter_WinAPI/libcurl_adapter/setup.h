#pragma once

#ifdef _WIN32
#  if defined(UNICODE) && !defined(_UNICODE)
#    error "UNICODE is defined but _UNICODE is not defined"
#  endif
#  if defined(_UNICODE) && !defined(UNICODE)
#    error "_UNICODE is defined but UNICODE is not defined"
#  endif

#  ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#  endif

#	include <WinSock2.h>
#	include <windows.h>
#	include <Wininet.h>
# pragma comment (lib, "wininet.lib")
#	include <winhttp.h>
# pragma comment(lib, "Winhttp.lib")
#   include <wincrypt.h>
# pragma comment(lib, "Crypt32.lib")
#	include <winerror.h>

#   include <limits>
#	include <tchar.h>
#   include <stdlib.h>
#	include <corecrt_malloc.h>
#	include <vcruntime_string.h>
#endif

#ifndef SIZE_T_MAX	/* some limits.h headers have this defined, some do not */
#if defined(SIZEOF_SIZE_T) && (SIZEOF_SIZE_T > 4)
#define SIZE_T_MAX 18446744073709551615U
#else
#define SIZE_T_MAX 4294967295U
#endif
#endif

#ifndef SSIZE_T_MAX	/* some limits.h headers have this defined, some do not */
#if defined(SIZEOF_SIZE_T) && (SIZEOF_SIZE_T > 4)
#define SSIZE_T_MAX 9223372036854775807
#else
#define SSIZE_T_MAX 2147483647
#endif
#endif

#if defined(_MSC_VER) && defined(_DLL)
#  pragma warning(push)
#  pragma warning(disable:4232) /* MSVC extension, dllimport identity */
#endif

#ifdef HTTP_ONLY
#  ifndef CURL_DISABLE_FTP
#    define CURL_DISABLE_FTP
#  endif
#  ifndef CURL_DISABLE_TFTP
#    define CURL_DISABLE_TFTP
#  endif
#endif

#ifdef _MSC_VER
#  if (_MSC_VER >= 1800)
#    include <inttypes.h>
#    define CURL_TYPEOF_CURL_OFF_T     __int64
#    define CURL_FORMAT_CURL_OFF_T     PRId64
#    define CURL_FORMAT_CURL_OFF_TU    PRIu64
#    define CURL_SUFFIX_CURL_OFF_T     i64
#    define CURL_SUFFIX_CURL_OFF_TU    ui64
#  elif (_MSC_VER >= 900) && (_INTEGRAL_MAX_BITS >= 64)
#    define CURL_TYPEOF_CURL_OFF_T     __int64
#    define CURL_FORMAT_CURL_OFF_T     "I64d"
#    define CURL_FORMAT_CURL_OFF_TU    "I64u"
#    define CURL_SUFFIX_CURL_OFF_T     i64
#    define CURL_SUFFIX_CURL_OFF_TU    ui64
#  else
#    define CURL_TYPEOF_CURL_OFF_T     long
#    define CURL_FORMAT_CURL_OFF_T     "ld"
#    define CURL_FORMAT_CURL_OFF_TU    "lu"
#    define CURL_SUFFIX_CURL_OFF_T     L
#    define CURL_SUFFIX_CURL_OFF_TU    UL
#  endif
#  define CURL_TYPEOF_CURL_SOCKLEN_T int
#endif

#if defined(_MSC_VER) && !defined(_WIN32_WCE)
#  if (_MSC_VER >= 900) && (_INTEGRAL_MAX_BITS >= 64)
#    define USE_WIN32_LARGE_FILES
#  else
#    define USE_WIN32_SMALL_FILES
#  endif
#endif

/*
 * Large file (>2Gb) support using Win32 functions.
 */

#ifdef USE_WIN32_LARGE_FILES
#  include <io.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  undef  lseek
#  define lseek(fdes,offset,whence)  _lseeki64(fdes, offset, whence)
#  undef  fstat
#  define fstat(fdes,stp)            _fstati64(fdes, stp)
#  undef  stat
#  define stat(fname,stp)            curlx_win32_stat(fname, stp)
#  define struct_stat                struct _stati64
#  define LSEEK_ERROR                (__int64)-1
#  define open                       curlx_win32_open
#  define fopen(fname,mode)          curlx_win32_fopen(fname, mode)
int curlx_win32_open(const char* filename, int oflag, ...);
int curlx_win32_stat(const char* path, struct_stat* buffer);
FILE* curlx_win32_fopen(const char* filename, const char* mode);
#endif

/*
 * Small file (<2Gb) support using Win32 functions.
 */

#ifdef USE_WIN32_SMALL_FILES
#  include <io.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  ifndef _WIN32_WCE
#    undef  lseek
#    define lseek(fdes,offset,whence)  _lseek(fdes, (long)offset, whence)
#    define fstat(fdes,stp)            _fstat(fdes, stp)
#    define stat(fname,stp)            curlx_win32_stat(fname, stp)
#    define struct_stat                struct _stat
#    define open                       curlx_win32_open
#    define fopen(fname,mode)          curlx_win32_fopen(fname, mode)
int curlx_win32_stat(const char* path, struct_stat* buffer);
int curlx_win32_open(const char* filename, int oflag, ...);
FILE* curlx_win32_fopen(const char* filename, const char* mode);
#  endif
#  define LSEEK_ERROR                (long)-1
#endif


#ifdef _MSC_VER
typedef unsigned int bit;
#define BIT(x) bit x:1
#endif

#if defined(_WIN32)
typedef SOCKET curl_socket_t;
#endif

#ifdef CURL_TYPEOF_CURL_OFF_T
typedef CURL_TYPEOF_CURL_OFF_T curl_off_t;
#endif

#ifdef DEBUGBUILD
#define DEBUGF(x) x
#else
#define DEBUGF(x) do { } while(0)
#endif

#undef DEBUGASSERT
#if defined(DEBUGBUILD)
#define DEBUGASSERT(x) assert(x)
#else
#define DEBUGASSERT(x) do { } while(0)
#endif

#if !defined(FALLTHROUGH)
#if (defined(__GNUC__) && __GNUC__ >= 7) || \
    (defined(__clang__) && __clang_major__ >= 10)
#  define FALLTHROUGH()  __attribute__((fallthrough))
#else
#  define FALLTHROUGH()  do {} while (0)
#endif
#endif

#ifdef __VMS
#define argv_item_t  __char_ptr32
#elif defined(_UNICODE)
#define argv_item_t wchar_t *
#else
#define argv_item_t  char *
#endif

#define ZERO_NULL 0

#ifndef _SSIZE_T_DEFINED
#  if defined(__MINGW32__)
#  elif defined(_WIN64)
#    define _SSIZE_T_DEFINED
#    define ssize_t __int64
#  else
#    define _SSIZE_T_DEFINED
#    define ssize_t int
#  endif
#endif