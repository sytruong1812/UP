#include "utils.h"
#include "llist.h"

#pragma region strdup.h

/***************************************************************************
 *
 * Curl_strdup(source)
 *
 * Copies the 'source' char string to a newly allocated buffer (that is
 * returned).
 *
 * Returns the new pointer or NULL on failure.
 *
 ***************************************************************************/
char* Curl_strdup(const char* str)
{
	size_t len;
	char* newstr;
	if (!str)
	{
		return (char*)NULL;
	}
	len = strlen(str) + 1;
	newstr = (char*)malloc(len);
	if (!newstr)
	{
		return (char*)NULL;
	}
	memcpy(newstr, str, len);
	return newstr;
}

/***************************************************************************
 *
 * Curl_wcsdup(source)
 *
 * Copies the 'source' wchar string to a newly allocated buffer (that is
 * returned).
 *
 * Returns the new pointer or NULL on failure.
 *
 ***************************************************************************/
wchar_t* Curl_wcsdup(const wchar_t* src)
{
	size_t length = wcslen(src);
	if (length > (SIZE_T_MAX / sizeof(wchar_t)) - 1)
	{
		return (wchar_t*)NULL; /* integer overflow */
	}
	return (wchar_t*)Curl_memdup(src, (length + 1) * sizeof(wchar_t));
}

/***************************************************************************
 *
 * Curl_memdup(source, length)
 *
 * Copies the 'source' data to a newly allocated buffer (that is
 * returned). Copies 'length' bytes.
 *
 * Returns the new pointer or NULL on failure.
 *
 ***************************************************************************/
void* Curl_memdup(const void* src, size_t length)
{
	void* buffer = malloc(length);
	if (!buffer)
	{
		return NULL; /* fail */
	}
	memcpy(buffer, src, length);
	return buffer;
}

/***************************************************************************
 *
 * Curl_memdup0(source, length)
 *
 * Copies the 'source' string to a newly allocated buffer (that is returned).
 * Copies 'length' bytes then adds a null terminator.
 *
 * Returns the new pointer or NULL on failure.
 *
 ***************************************************************************/
void* Curl_memdup0(const char* src, size_t length)
{
	char* buf = (char*)malloc(length + 1);
	if (!buf)
	{
		return NULL;
	}
	memcpy(buf, src, length);
	buf[length] = 0;
	return buf;
}

/***************************************************************************
 *
 * Curl_saferealloc(ptr, size)
 *
 * Does a normal realloc(), but will free the data pointer if the realloc
 * fails. If 'size' is non-zero, it will free the data and return a failure.
 *
 * This convenience function is provided and used to help us avoid a common
 * mistake pattern when we could pass in a zero, catch the NULL return and end
 * up free'ing the memory twice.
 *
 * Returns the new pointer or NULL on failure.
 *
 ***************************************************************************/
void* Curl_saferealloc(void* ptr, size_t size)
{
	void* datap = realloc(ptr, size);
	if (size && !datap)
	{
		free(ptr);	/* only free 'ptr' if size was non-zero */
	}
	return datap;
}

#pragma endregion

#pragma region strtofft.h

/*
 * Parse a *positive* up to 64-bit number written in ASCII.
 */
CURLofft curlx_strtoofft(const char* str, char** endp, int base, curl_off_t* num)
{
	*num = 0;	/* clear by default */
	errno = 0;	/* clear by default */
	char* end = NULL;
	curl_off_t number;
	DEBUGASSERT(base); /* starting now, avoid base zero */

	while (*str && ISBLANK(*str))
	{
		str++;
	}
	if (('-' == *str) || (ISSPACE(*str)))
	{
		if (endp)
		{
			*endp = (char*)str; /* did not actually move */
		}
		return CURL_OFFT_INVAL; /* nothing parsed */
	}
	number = strtooff(str, &end, base);
	if (endp)
	{
		*endp = end;
	}
	if (errno == ERANGE)
	{
		return CURL_OFFT_FLOW;	/* overflow/underflow */
	}
	else if (str == end)
	{
		return CURL_OFFT_INVAL;	/* nothing parsed */
	}
	*num = number;
	return CURL_OFFT_OK;
}

#pragma endregion

#pragma region strtok.h

char* Curl_strtok_r(char* ptr, const char* sep, char** end)
{
	if (!ptr)
	{
		/* we got NULL input so then we get our last position instead */
		ptr = *end;
	}
	/* pass all letters that are including in the separator string */
	while (*ptr && strchr(sep, *ptr))
	{
		++ptr;
	}

	if (*ptr)
	{
		/* so this is where the next piece of string starts */
		char* start = ptr;
		/* set the end pointer to the first byte after the start */
		*end = start + 1;
		/* scan through the string to find where it ends, it ends on a
		   null byte or a character that exists in the separator string */
		while (**end && !strchr(sep, **end))
		{
			++*end;
		}
		if (**end)
		{
			/* the end is not a null byte */
			**end = '\0';  /* null-terminate it! */
			++*end;        /* advance the last pointer to beyond the null byte */
		}
		return start; /* return the position where the string starts */
	}
	/* we ended up on a null byte, there are no more strings to find! */
	return NULL;
}

#pragma endregion

#pragma region strcase.h

/* Mapping table to go from lowercase to uppercase for plain ASCII.*/
static const unsigned char touppermap[256] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,
79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 65,
66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
85, 86, 87, 88, 89, 90, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133,
134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165,
166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213,
214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229,
230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245,
246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

/* Mapping table to go from uppercase to lowercase for plain ASCII.*/
static const unsigned char tolowermap[256] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
62, 63, 64, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 91, 92, 93, 94, 95,
96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

/* Portable, consistent toupper. Do not use toupper() because its behavior is
   altered by the current locale. */
char Curl_raw_toupper(char in)
{
	return (char)touppermap[(unsigned char)in];
}

/* Portable, consistent tolower. Do not use tolower() because its behavior is
   altered by the current locale. */
char Curl_raw_tolower(char in)
{
	return (char)tolowermap[(unsigned char)in];
}

/*
 * curl_strequal() is for doing "raw" case insensitive strings. This is meant
 * to be locale independent and only compare strings we know are safe for
 * this. See https://daniel.haxx.se/blog/2008/10/15/strcasecmp-in-turkish/ for
 * further explanations as to why this function is necessary.
 */

static int casecompare(const char* first, const char* second)
{
	while (*first && *second)
	{
		if (Curl_raw_toupper(*first) != Curl_raw_toupper(*second))
			/* get out of the loop as soon as they do not match */
			return 0;
		first++;
		second++;
	}
	/* If we are here either the strings are the same or the length is different.
	   We can just test if the "current" character is non-zero for one and zero
	   for the other. Note that the characters may not be exactly the same even
	   if they match, we only want to compare zero-ness. */
	return !*first == !*second;
}

/* --- public function --- */
int curl_strequal(const char* first, const char* second)
{
	if (first && second)
		/* both pointers point to something then compare them */
		return casecompare(first, second);

	/* if both pointers are NULL then treat them as equal */
	return (NULL == first && NULL == second);
}

static int ncasecompare(const char* first, const char* second, size_t max)
{
	while (*first && *second && max)
	{
		if (Curl_raw_toupper(*first) != Curl_raw_toupper(*second))
			return 0;
		max--;
		first++;
		second++;
	}
	if (0 == max)
		return 1; /* they are equal this far */

	return Curl_raw_toupper(*first) == Curl_raw_toupper(*second);
}

/* --- public function --- */
int curl_strnequal(const char* first, const char* second, size_t max)
{
	if (first && second)
		/* both pointers point to something then compare them */
		return ncasecompare(first, second, max);

	/* if both pointers are NULL then treat them as equal if max is non-zero */
	return (NULL == first && NULL == second && max);
}
/* Copy an upper case version of the string from src to dest. The
 * strings may overlap. No more than n characters of the string are copied
 * (including any NUL) and the destination string will NOT be
 * NUL-terminated if that limit is reached.
 */
void Curl_strntoupper(char* dest, const char* src, size_t n)
{
	if (n < 1)
		return;

	do
	{
		*dest++ = Curl_raw_toupper(*src);
	} while (*src++ && --n);
}

/* Copy a lower case version of the string from src to dest. The
 * strings may overlap. No more than n characters of the string are copied
 * (including any NUL) and the destination string will NOT be
 * NUL-terminated if that limit is reached.
 */
void Curl_strntolower(char* dest, const char* src, size_t n)
{
	if (n < 1)
		return;

	do
	{
		*dest++ = Curl_raw_tolower(*src);
	} while (*src++ && --n);
}

/* Compare case-sensitive NUL-terminated strings, taking care of possible
 * null pointers. Return true if arguments match.
 */
bool Curl_safecmp(char* a, char* b)
{
	if (a && b)
		return !strcmp(a, b);
	return !a && !b;
}

/*
 * Curl_timestrcmp() returns 0 if the two strings are identical. The time this
 * function spends is a function of the shortest string, not of the contents.
 */
int Curl_timestrcmp(const char* a, const char* b)
{
	int match = 0;
	int i = 0;

	if (a && b)
	{
		while (1)
		{
			match |= a[i] ^ b[i];
			if (!a[i] || !b[i])
				break;
			i++;
		}
	}
	else
		return a || b;
	return match;
}

#pragma endregion

#pragma region warnless.h
#include <limits.h>

#ifdef _WIN32
#undef read
#undef write
#endif

#define CURL_MASK_UCHAR   ((unsigned char)~0)
#define CURL_MASK_SCHAR   (CURL_MASK_UCHAR >> 1)

#define CURL_MASK_USHORT  ((unsigned short)~0)
#define CURL_MASK_SSHORT  (CURL_MASK_USHORT >> 1)

#define CURL_MASK_UINT    ((unsigned int)~0)
#define CURL_MASK_SINT    (CURL_MASK_UINT >> 1)

#define CURL_MASK_ULONG   ((unsigned long)~0)
#define CURL_MASK_SLONG   (CURL_MASK_ULONG >> 1)

#define CURL_MASK_UCOFFT  ((unsigned CURL_TYPEOF_CURL_OFF_T)~0)
#define CURL_MASK_SCOFFT  (CURL_MASK_UCOFFT >> 1)

#define CURL_MASK_USIZE_T ((size_t)~0)
#define CURL_MASK_SSIZE_T (CURL_MASK_USIZE_T >> 1)

/*
** unsigned long to unsigned short
*/

unsigned short curlx_ultous(unsigned long ulnum)
{
#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:810) /* conversion may lose significant bits */
#endif

	DEBUGASSERT(ulnum <= (unsigned long)CURL_MASK_USHORT);
	return (unsigned short)(ulnum & (unsigned long)CURL_MASK_USHORT);

#ifdef __INTEL_COMPILER
#  pragma warning(pop)
#endif
}

/*
** unsigned long to unsigned char
*/

unsigned char curlx_ultouc(unsigned long ulnum)
{
#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:810) /* conversion may lose significant bits */
#endif

	DEBUGASSERT(ulnum <= (unsigned long)CURL_MASK_UCHAR);
	return (unsigned char)(ulnum & (unsigned long)CURL_MASK_UCHAR);

#ifdef __INTEL_COMPILER
#  pragma warning(pop)
#endif
}

/*
** unsigned size_t to signed curl_off_t
*/

curl_off_t curlx_uztoso(size_t uznum)
{
#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:810) /* conversion may lose significant bits */
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable:4310) /* cast truncates constant value */
#endif

	DEBUGASSERT(uznum <= (size_t)CURL_MASK_SCOFFT);
	return (curl_off_t)(uznum & (size_t)CURL_MASK_SCOFFT);

#if defined(__INTEL_COMPILER) || defined(_MSC_VER)
#  pragma warning(pop)
#endif
}

/*
** unsigned size_t to signed int
*/

int curlx_uztosi(size_t uznum)
{
#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:810) /* conversion may lose significant bits */
#endif

	DEBUGASSERT(uznum <= (size_t)CURL_MASK_SINT);
	return (int)(uznum & (size_t)CURL_MASK_SINT);

#ifdef __INTEL_COMPILER
#  pragma warning(pop)
#endif
}

/*
** unsigned size_t to unsigned long
*/

unsigned long curlx_uztoul(size_t uznum)
{
#ifdef __INTEL_COMPILER
# pragma warning(push)
# pragma warning(disable:810) /* conversion may lose significant bits */
#endif

#if ULONG_MAX < SIZE_T_MAX
	DEBUGASSERT(uznum <= (size_t)CURL_MASK_ULONG);
#endif
	return (unsigned long)(uznum & (size_t)CURL_MASK_ULONG);

#ifdef __INTEL_COMPILER
# pragma warning(pop)
#endif
}

/*
** unsigned size_t to unsigned int
*/

unsigned int curlx_uztoui(size_t uznum)
{
#ifdef __INTEL_COMPILER
# pragma warning(push)
# pragma warning(disable:810) /* conversion may lose significant bits */
#endif

#if UINT_MAX < SIZE_T_MAX
	DEBUGASSERT(uznum <= (size_t)CURL_MASK_UINT);
#endif
	return (unsigned int)(uznum & (size_t)CURL_MASK_UINT);

#ifdef __INTEL_COMPILER
# pragma warning(pop)
#endif
}

/*
** signed long to signed int
*/

int curlx_sltosi(long slnum)
{
#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:810) /* conversion may lose significant bits */
#endif

	DEBUGASSERT(slnum >= 0);
#if INT_MAX < LONG_MAX
	DEBUGASSERT((unsigned long)slnum <= (unsigned long)CURL_MASK_SINT);
#endif
	return (int)(slnum & (long)CURL_MASK_SINT);

#ifdef __INTEL_COMPILER
#  pragma warning(pop)
#endif
}

/*
** signed long to unsigned int
*/

unsigned int curlx_sltoui(long slnum)
{
#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:810) /* conversion may lose significant bits */
#endif

	DEBUGASSERT(slnum >= 0);
#if UINT_MAX < LONG_MAX
	DEBUGASSERT((unsigned long)slnum <= (unsigned long)CURL_MASK_UINT);
#endif
	return (unsigned int)(slnum & (long)CURL_MASK_UINT);

#ifdef __INTEL_COMPILER
#  pragma warning(pop)
#endif
}

/*
** signed long to unsigned short
*/

unsigned short curlx_sltous(long slnum)
{
#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:810) /* conversion may lose significant bits */
#endif

	DEBUGASSERT(slnum >= 0);
	DEBUGASSERT((unsigned long)slnum <= (unsigned long)CURL_MASK_USHORT);
	return (unsigned short)(slnum & (long)CURL_MASK_USHORT);

#ifdef __INTEL_COMPILER
#  pragma warning(pop)
#endif
}

/*
** unsigned size_t to signed ssize_t
*/

ssize_t curlx_uztosz(size_t uznum)
{
#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:810) /* conversion may lose significant bits */
#endif

	DEBUGASSERT(uznum <= (size_t)CURL_MASK_SSIZE_T);
	return (ssize_t)(uznum & (size_t)CURL_MASK_SSIZE_T);

#ifdef __INTEL_COMPILER
#  pragma warning(pop)
#endif
}

/*
** signed curl_off_t to unsigned size_t
*/

size_t curlx_sotouz(curl_off_t sonum)
{
#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:810) /* conversion may lose significant bits */
#endif

	DEBUGASSERT(sonum >= 0);
	return (size_t)(sonum & (curl_off_t)CURL_MASK_USIZE_T);

#ifdef __INTEL_COMPILER
#  pragma warning(pop)
#endif
}

/*
** signed ssize_t to signed int
*/

int curlx_sztosi(ssize_t sznum)
{
#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:810) /* conversion may lose significant bits */
#endif

	DEBUGASSERT(sznum >= 0);
#if INT_MAX < SSIZE_T_MAX
	DEBUGASSERT((size_t)sznum <= (size_t)CURL_MASK_SINT);
#endif
	return (int)(sznum & (ssize_t)CURL_MASK_SINT);

#ifdef __INTEL_COMPILER
#  pragma warning(pop)
#endif
}

/*
** unsigned int to unsigned short
*/

unsigned short curlx_uitous(unsigned int uinum)
{
#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:810) /* conversion may lose significant bits */
#endif

	DEBUGASSERT(uinum <= (unsigned int)CURL_MASK_USHORT);
	return (unsigned short)(uinum & (unsigned int)CURL_MASK_USHORT);

#ifdef __INTEL_COMPILER
#  pragma warning(pop)
#endif
}

/*
** signed int to unsigned size_t
*/

size_t curlx_sitouz(int sinum)
{
#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:810) /* conversion may lose significant bits */
#endif

	DEBUGASSERT(sinum >= 0);
	return (size_t)sinum;

#ifdef __INTEL_COMPILER
#  pragma warning(pop)
#endif
}

#if defined(_WIN32)

ssize_t curlx_read(int fd, void* buf, size_t count)
{
	return (ssize_t)_read(fd, buf, curlx_uztoui(count));
}

ssize_t curlx_write(int fd, const void* buf, size_t count)
{
	return (ssize_t)_write(fd, buf, curlx_uztoui(count));
}

#endif /* _WIN32 */

/* Ensure that warnless.h redefinitions continue to have an effect in "unity" builds. */
#undef HEADER_CURL_WARNLESS_H_REDEFS

#pragma endregion

#pragma region dynbuf.h

#define MIN_FIRST_ALLOC 32
#define DYNINIT 0xbee51da /* random pattern */

/*
 * Init a dynbuf struct.
 */
void Curl_dyn_init(struct dynbuf* s, size_t toobig)
{
	DEBUGASSERT(s);
	DEBUGASSERT(toobig);
	s->bufr = NULL;
	s->leng = 0;
	s->allc = 0;
	s->toobig = toobig;
#ifdef DEBUGBUILD
	s->init = DYNINIT;
#endif
}

/*
 * free the buffer and re-init the necessary fields. It does not touch the
 * 'init' field and thus this buffer can be reused to add data to again.
 */
void Curl_dyn_free(struct dynbuf* s)
{
	DEBUGASSERT(s);
	Curl_safefree(s->bufr);
	s->leng = s->allc = 0;
}

/*
 * Store/append an chunk of memory to the dynbuf.
 */
static CURLcode dyn_nappend(struct dynbuf* s,
	const unsigned char* mem, size_t len)
{
	size_t indx = s->leng;
	size_t a = s->allc;
	size_t fit = len + indx + 1; /* new string + old string + zero byte */

	/* try to detect if there is rubbish in the struct */
	DEBUGASSERT(s->init == DYNINIT);
	DEBUGASSERT(s->toobig);
	DEBUGASSERT(indx < s->toobig);
	DEBUGASSERT(!s->leng || s->bufr);
	DEBUGASSERT(a <= s->toobig);
	DEBUGASSERT(!len || mem);

	if (fit > s->toobig)
	{
		Curl_dyn_free(s);
		return CURLE_TOO_LARGE;
	}
	else if (!a)
	{
		DEBUGASSERT(!indx);
		/* first invoke */
		if (MIN_FIRST_ALLOC > s->toobig)
			a = s->toobig;
		else if (fit < MIN_FIRST_ALLOC)
			a = MIN_FIRST_ALLOC;
		else
			a = fit;
	}
	else
	{
		while (a < fit)
			a *= 2;
		if (a > s->toobig)
			/* no point in allocating a larger buffer than this is allowed to use */
			a = s->toobig;
	}

	if (a != s->allc)
	{
		/* this logic is not using Curl_saferealloc() to make the tool not have to
		   include that as well when it uses this code */
		void* p = realloc(s->bufr, a);
		if (!p)
		{
			Curl_dyn_free(s);
			return CURLE_OUT_OF_MEMORY;
		}
		s->bufr = (char*)p;
		s->allc = a;
	}

	if (len)
		memcpy(&s->bufr[indx], mem, len);
	s->leng = indx + len;
	s->bufr[s->leng] = 0;
	return CURLE_OK;
}

/*
 * Clears the string, keeps the allocation. This can also be called on a
 * buffer that already was freed.
 */
void Curl_dyn_reset(struct dynbuf* s)
{
	DEBUGASSERT(s);
	DEBUGASSERT(s->init == DYNINIT);
	DEBUGASSERT(!s->leng || s->bufr);
	if (s->leng)
		s->bufr[0] = 0;
	s->leng = 0;
}

/*
 * Specify the size of the tail to keep (number of bytes from the end of the
 * buffer). The rest will be dropped.
 */
CURLcode Curl_dyn_tail(struct dynbuf* s, size_t trail)
{
	DEBUGASSERT(s);
	DEBUGASSERT(s->init == DYNINIT);
	DEBUGASSERT(!s->leng || s->bufr);
	if (trail > s->leng)
		return CURLE_BAD_FUNCTION_ARGUMENT;
	else if (trail == s->leng)
		return CURLE_OK;
	else if (!trail)
	{
		Curl_dyn_reset(s);
	}
	else
	{
		memmove(&s->bufr[0], &s->bufr[s->leng - trail], trail);
		s->leng = trail;
		s->bufr[s->leng] = 0;
	}
	return CURLE_OK;

}

/*
 * Appends a buffer with length.
 */
CURLcode Curl_dyn_addn(struct dynbuf* s, const void* mem, size_t len)
{
	DEBUGASSERT(s);
	DEBUGASSERT(s->init == DYNINIT);
	DEBUGASSERT(!s->leng || s->bufr);
	return dyn_nappend(s, (unsigned char*)mem, len);
}

/*
 * Append a null-terminated string at the end.
 */
CURLcode Curl_dyn_add(struct dynbuf* s, const char* str)
{
	size_t n;
	DEBUGASSERT(str);
	DEBUGASSERT(s);
	DEBUGASSERT(s->init == DYNINIT);
	DEBUGASSERT(!s->leng || s->bufr);
	n = strlen(str);
	return dyn_nappend(s, (unsigned char*)str, n);
}

/*
 * Append a string vprintf()-style
 */
CURLcode Curl_dyn_vaddf(struct dynbuf* s, const char* fmt, va_list ap)
{
#ifdef BUILDING_LIBCURL
	int rc;
	DEBUGASSERT(s);
	DEBUGASSERT(s->init == DYNINIT);
	DEBUGASSERT(!s->leng || s->bufr);
	DEBUGASSERT(fmt);
	rc = Curl_dyn_vprintf(s, fmt, ap);

	if (!rc)
		return CURLE_OK;
	else if (rc == MERR_TOO_LARGE)
		return CURLE_TOO_LARGE;
	return CURLE_OUT_OF_MEMORY;
#else
	char* str;
	str = vaprintf(fmt, ap); /* this allocs a new string to append */

	if (str)
	{
		CURLcode result = dyn_nappend(s, (unsigned char*)str, strlen(str));
		free(str);
		return result;
	}
	/* If we failed, we cleanup the whole buffer and return error */
	Curl_dyn_free(s);
	return CURLE_OUT_OF_MEMORY;
#endif
}

/*
 * Append a string printf()-style
 */
CURLcode Curl_dyn_addf(struct dynbuf* s, const char* fmt, ...)
{
	CURLcode result;
	va_list ap;
	DEBUGASSERT(s);
	DEBUGASSERT(s->init == DYNINIT);
	DEBUGASSERT(!s->leng || s->bufr);
	va_start(ap, fmt);
	result = Curl_dyn_vaddf(s, fmt, ap);
	va_end(ap);
	return result;
}

/*
 * Returns a pointer to the buffer.
 */
char* Curl_dyn_ptr(const struct dynbuf* s)
{
	DEBUGASSERT(s);
	DEBUGASSERT(s->init == DYNINIT);
	DEBUGASSERT(!s->leng || s->bufr);
	return s->bufr;
}

/*
 * Returns an unsigned pointer to the buffer.
 */
unsigned char* Curl_dyn_uptr(const struct dynbuf* s)
{
	DEBUGASSERT(s);
	DEBUGASSERT(s->init == DYNINIT);
	DEBUGASSERT(!s->leng || s->bufr);
	return (unsigned char*)s->bufr;
}

/*
 * Returns the length of the buffer.
 */
size_t Curl_dyn_len(const struct dynbuf* s)
{
	DEBUGASSERT(s);
	DEBUGASSERT(s->init == DYNINIT);
	DEBUGASSERT(!s->leng || s->bufr);
	return s->leng;
}

/*
 * Set a new (smaller) length.
 */
CURLcode Curl_dyn_setlen(struct dynbuf* s, size_t set)
{
	DEBUGASSERT(s);
	DEBUGASSERT(s->init == DYNINIT);
	DEBUGASSERT(!s->leng || s->bufr);
	if (set > s->leng)
		return CURLE_BAD_FUNCTION_ARGUMENT;
	s->leng = set;
	s->bufr[s->leng] = 0;
	return CURLE_OK;
}

#pragma endregion

#pragma region escape.h

/* for ABI-compatibility with previous versions */
char* curl_escape(const char* string, int inlength)
{
	return curl_easy_escape(NULL, string, inlength);
}

/* for ABI-compatibility with previous versions */
char* curl_unescape(const char* string, int length)
{
	return curl_easy_unescape(NULL, string, length, NULL);
}

/* Escapes for URL the given unescaped string of given length. 'data' is ignored since 7.82.0. */
char* curl_easy_escape(CURL* data, const char* string, int inlength)
{
	size_t length;
	struct dynbuf d;
	(void)data;

	if (!string || (inlength < 0))
		return NULL;

	length = (inlength ? (size_t)inlength : strlen(string));
	if (!length)
		return _strdup("");

	Curl_dyn_init(&d, length * 3 + 1);

	while (length--)
	{
		/* treat the characters unsigned */
		unsigned char in = (unsigned char)*string++;

		if (ISUNRESERVED(in))
		{
			/* append this */
			if (Curl_dyn_addn(&d, &in, 1))
				return NULL;
		}
		else
		{
			/* encode it */
			const char hex[] = "0123456789ABCDEF";
			char out[3] = { '%' };
			out[1] = hex[in >> 4];
			out[2] = hex[in & 0xf];
			if (Curl_dyn_addn(&d, out, 3))
				return NULL;
		}
	}

	return Curl_dyn_ptr(&d);
}

static const unsigned char hextable[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,       /* 0x30 - 0x3f */
  0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x40 - 0x4f */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       /* 0x50 - 0x5f */
  0, 10, 11, 12, 13, 14, 15                             /* 0x60 - 0x66 */
};

#define onehex2dec(x) hextable[x - '0']     /* the input is a single hex digit */

/*
 * Curl_urldecode() URL decodes the given string.
 *
 * Returns a pointer to a malloced string in *ostring with length given in
 * *olen. If length == 0, the length is assumed to be strlen(string).
 *
 * ctrl options:
 * - REJECT_NADA: accept everything
 * - REJECT_CTRL: rejects control characters (byte codes lower than 32) in
 *                the data
 * - REJECT_ZERO: rejects decoded zero bytes
 *
 * The values for the enum starts at 2, to make the assert detect legacy
 * invokes that used TRUE/FALSE (0 and 1).
 */
CURLcode Curl_urldecode(const char* string, size_t length, char** ostring, size_t* olen, enum urlreject ctrl)
{
	size_t alloc;
	char* ns;

	DEBUGASSERT(string);
	DEBUGASSERT(ctrl >= REJECT_NADA); /* crash on TRUE/FALSE */

	alloc = (length ? length : strlen(string));
	ns = (char*)malloc(alloc + 1);

	if (!ns)
		return CURLE_OUT_OF_MEMORY;

	/* store output string */
	*ostring = ns;

	while (alloc)
	{
		unsigned char in = (unsigned char)*string;
		if (('%' == in) && (alloc > 2) &&
			ISXDIGIT(string[1]) && ISXDIGIT(string[2]))
		{
			/* this is two hexadecimal digits following a '%' */
			in = (unsigned char)(onehex2dec(string[1]) << 4) | onehex2dec(string[2]);

			string += 3;
			alloc -= 3;
		}
		else
		{
			string++;
			alloc--;
		}

		if (((ctrl == REJECT_CTRL) && (in < 0x20)) ||
			((ctrl == REJECT_ZERO) && (in == 0)))
		{
			Curl_safefree(*ostring);
			return CURLE_URL_MALFORMAT;
		}

		*ns++ = (char)in;
	}
	*ns = 0; /* terminate it */

	if (olen)
		/* store output size */
		*olen = ns - *ostring;

	return CURLE_OK;
}

/*
 * Unescapes the given URL escaped string of given length. Returns a
 * pointer to a malloced string with length given in *olen.
 * If length == 0, the length is assumed to be strlen(string).
 * If olen == NULL, no output length is stored.
 * 'data' is ignored since 7.82.0.
 */
char* curl_easy_unescape(CURL* data, const char* string, int length, int* olen)
{
	char* str = NULL;
	(void)data;
	if (string && (length >= 0))
	{
		size_t inputlen = (size_t)length;
		size_t outputlen;
		CURLcode res = Curl_urldecode(string, inputlen, &str, &outputlen,
			REJECT_NADA);
		if (res)
			return NULL;

		if (olen)
		{
			if (outputlen <= (size_t)INT_MAX)
				*olen = curlx_uztosi(outputlen);
			else
				/* too large to return in an int, fail! */
				Curl_safefree(str);
		}
	}
	return str;
}

/* For operating systems/environments that use different malloc/free
   systems for the app and for this library, we provide a free that uses
   the library's memory system */
void curl_free(void* p)
{
	free(p);
}

/*
 * Curl_hexencode()
 *
 * Converts binary input to lowercase hex-encoded ASCII output.
 * Null-terminated.
 */
void Curl_hexencode(const unsigned char* src, size_t len, unsigned char* out, size_t olen)
{
	const char* hex = "0123456789abcdef";
	DEBUGASSERT(src && len && (olen >= 3));
	if (src && len && (olen >= 3))
	{
		while (len-- && (olen >= 3))
		{
			/* clang-tidy warns on this line without this comment: */
			/* NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult) */
			*out++ = (unsigned char)hex[(*src & 0xF0) >> 4];
			*out++ = (unsigned char)hex[*src & 0x0F];
			++src;
			olen -= 2;
		}
		*out = 0;
	}
	else if (olen)
	{
		*out = 0;
	}
}

#pragma endregion

#pragma region mprintf.h

/*
 * If SIZEOF_SIZE_T has not been defined, default to the size of long.
 */

#ifdef HAVE_LONGLONG
#  define LONG_LONG_TYPE long long
#  define HAVE_LONG_LONG_TYPE
#else
#  if defined(_MSC_VER) && (_MSC_VER >= 900) && (_INTEGRAL_MAX_BITS >= 64)
#    define LONG_LONG_TYPE __int64
#    define HAVE_LONG_LONG_TYPE
#  else
#    undef LONG_LONG_TYPE
#    undef HAVE_LONG_LONG_TYPE
#  endif
#endif

 /*
  * Max integer data types that mprintf.c is capable
  */

#ifdef HAVE_LONG_LONG_TYPE
#  define mp_intmax_t LONG_LONG_TYPE
#  define mp_uintmax_t unsigned LONG_LONG_TYPE
#else
#  define mp_intmax_t long
#  define mp_uintmax_t unsigned long
#endif

#define BUFFSIZE 326 /* buffer for long-to-str and float-to-str calcs, should
						fit negative DBL_MAX (317 letters) */
#define MAX_PARAMETERS 128 /* number of input arguments */
#define MAX_SEGMENTS   128 /* number of output segments */

#ifdef __AMIGA__
# undef FORMAT_INT
#endif

						/* Lower-case digits.  */
static const char lower_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

/* Upper-case digits.  */
static const char upper_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#define OUTCHAR(x)                                      \
  do {                                                  \
    if(!stream((unsigned char)x, userp))                \
      done++;                                           \
    else                                                \
      return done; /* return on failure */              \
  } while(0)

/* Data type to read from the arglist */
typedef enum {
	FORMAT_STRING,
	FORMAT_PTR,
	FORMAT_INTPTR,
	FORMAT_INT,
	FORMAT_LONG,
	FORMAT_LONGLONG,
	FORMAT_INTU,
	FORMAT_LONGU,
	FORMAT_LONGLONGU,
	FORMAT_DOUBLE,
	FORMAT_LONGDOUBLE,
	FORMAT_WIDTH,
	FORMAT_PRECISION
} FormatType;

/* conversion and display flags */
enum {
	FLAGS_SPACE = 1 << 0,
	FLAGS_SHOWSIGN = 1 << 1,
	FLAGS_LEFT = 1 << 2,
	FLAGS_ALT = 1 << 3,
	FLAGS_SHORT = 1 << 4,
	FLAGS_LONG = 1 << 5,
	FLAGS_LONGLONG = 1 << 6,
	FLAGS_LONGDOUBLE = 1 << 7,
	FLAGS_PAD_NIL = 1 << 8,
	FLAGS_UNSIGNED = 1 << 9,
	FLAGS_OCTAL = 1 << 10,
	FLAGS_HEX = 1 << 11,
	FLAGS_UPPER = 1 << 12,
	FLAGS_WIDTH = 1 << 13, /* '*' or '*<num>$' used */
	FLAGS_WIDTHPARAM = 1 << 14, /* width PARAMETER was specified */
	FLAGS_PREC = 1 << 15, /* precision was specified */
	FLAGS_PRECPARAM = 1 << 16, /* precision PARAMETER was specified */
	FLAGS_CHAR = 1 << 17, /* %c story */
	FLAGS_FLOATE = 1 << 18, /* %e or %E */
	FLAGS_FLOATG = 1 << 19, /* %g or %G */
	FLAGS_SUBSTR = 1 << 20  /* no input, only substring */
};

enum {
	DOLLAR_UNKNOWN,
	DOLLAR_NOPE,
	DOLLAR_USE
};

/*
 * Describes an input va_arg type and hold its value.
 */
struct va_input {
	FormatType type; /* FormatType */
	union {
		char* str;
		void* ptr;
		mp_intmax_t nums; /* signed */
		mp_uintmax_t numu; /* unsigned */
		double dnum;
	} val;
};

/*
 * Describes an output segment.
 */
struct outsegment {
	int width;     /* width OR width parameter number */
	int precision; /* precision OR precision parameter number */
	unsigned int flags;
	unsigned int input; /* input argument array index */
	char* start;      /* format string start to output */
	size_t outlen;     /* number of bytes from the format string to output */
};

struct nsprintf {
	char* buffer;
	size_t length;
	size_t max;
};

struct asprintf {
	struct dynbuf* b;
	char merr;
};

/* the provided input number is 1-based but this returns the number 0-based.

   returns -1 if no valid number was provided.
*/
static int dollarstring(char* input, char** end)
{
	if (ISDIGIT(*input))
	{
		int number = 0;
		do
		{
			if (number < MAX_PARAMETERS)
			{
				number *= 10;
				number += *input - '0';
			}
			input++;
		} while (ISDIGIT(*input));

		if (number && (number <= MAX_PARAMETERS) && ('$' == *input))
		{
			*end = ++input;
			return number - 1;
		}
	}
	return -1;
}

/*
 * Parse the format string.
 *
 * Create two arrays. One describes the inputs, one describes the outputs.
 *
 * Returns zero on success.
 */

#define PFMT_OK          0
#define PFMT_DOLLAR      1 /* bad dollar for main param */
#define PFMT_DOLLARWIDTH 2 /* bad dollar use for width */
#define PFMT_DOLLARPREC  3 /* bad dollar use for precision */
#define PFMT_MANYARGS    4 /* too many input arguments used */
#define PFMT_PREC        5 /* precision overflow */
#define PFMT_PRECMIX     6 /* bad mix of precision specifiers */
#define PFMT_WIDTH       7 /* width overflow */
#define PFMT_INPUTGAP    8 /* gap in arguments */
#define PFMT_WIDTHARG    9 /* attempted to use same arg twice, for width */
#define PFMT_PRECARG    10 /* attempted to use same arg twice, for prec */
#define PFMT_MANYSEGS   11 /* maxed out output segments */

static int parsefmt(const char* format,
	struct outsegment* out,
	struct va_input* in,
	int* opieces,
	int* ipieces, va_list arglist)
{
	char* fmt = (char*)format;
	int param_num = 0;
	int param;
	int width;
	int precision;
	unsigned int flags;
	FormatType type;
	int max_param = -1;
	int i;
	int ocount = 0;
	unsigned char usedinput[MAX_PARAMETERS / 8];
	size_t outlen = 0;
	struct outsegment* optr;
	int use_dollar = DOLLAR_UNKNOWN;
	char* start = fmt;

	/* clear, set a bit for each used input */
	memset(usedinput, 0, sizeof(usedinput));

	while (*fmt)
	{
		if (*fmt == '%')
		{
			struct va_input* iptr;
			bool loopit = TRUE;
			fmt++;
			outlen = (size_t)(fmt - start - 1);
			if (*fmt == '%')
			{
				/* this means a %% that should be output only as %. Create an output
				   segment. */
				if (outlen)
				{
					optr = &out[ocount++];
					if (ocount > MAX_SEGMENTS)
						return PFMT_MANYSEGS;
					optr->input = 0;
					optr->flags = FLAGS_SUBSTR;
					optr->start = start;
					optr->outlen = outlen;
				}
				start = fmt;
				fmt++;
				continue; /* while */
			}

			flags = 0;
			width = precision = 0;

			if (use_dollar != DOLLAR_NOPE)
			{
				param = dollarstring(fmt, &fmt);
				if (param < 0)
				{
					if (use_dollar == DOLLAR_USE)
						/* illegal combo */
						return PFMT_DOLLAR;

					/* we got no positional, just get the next arg */
					param = -1;
					use_dollar = DOLLAR_NOPE;
				}
				else
					use_dollar = DOLLAR_USE;
			}
			else
				param = -1;

			/* Handle the flags */
			while (loopit)
			{
				switch (*fmt++)
				{
					case ' ':
						flags |= FLAGS_SPACE;
						break;
					case '+':
						flags |= FLAGS_SHOWSIGN;
						break;
					case '-':
						flags |= FLAGS_LEFT;
						flags &= ~(unsigned int)FLAGS_PAD_NIL;
						break;
					case '#':
						flags |= FLAGS_ALT;
						break;
					case '.':
						if ('*' == *fmt)
						{
							/* The precision is picked from a specified parameter */
							flags |= FLAGS_PRECPARAM;
							fmt++;

							if (use_dollar == DOLLAR_USE)
							{
								precision = dollarstring(fmt, &fmt);
								if (precision < 0)
									/* illegal combo */
									return PFMT_DOLLARPREC;
							}
							else
								/* get it from the next argument */
								precision = -1;
						}
						else
						{
							bool is_neg = FALSE;
							flags |= FLAGS_PREC;
							precision = 0;
							if ('-' == *fmt)
							{
								is_neg = TRUE;
								fmt++;
							}
							while (ISDIGIT(*fmt))
							{
								if (precision > INT_MAX / 10)
									return PFMT_PREC;
								precision *= 10;
								precision += *fmt - '0';
								fmt++;
							}
							if (is_neg)
								precision = -precision;
						}
						if ((flags & (FLAGS_PREC | FLAGS_PRECPARAM)) ==
							(FLAGS_PREC | FLAGS_PRECPARAM))
							/* it is not permitted to use both kinds of precision for the same
							   argument */
							return PFMT_PRECMIX;
						break;
					case 'h':
						flags |= FLAGS_SHORT;
						break;
#if defined(_WIN32) || defined(_WIN32_WCE)
					case 'I':
						/* Non-ANSI integer extensions I32 I64 */
						if ((fmt[0] == '3') && (fmt[1] == '2'))
						{
							flags |= FLAGS_LONG;
							fmt += 2;
						}
						else if ((fmt[0] == '6') && (fmt[1] == '4'))
						{
							flags |= FLAGS_LONGLONG;
							fmt += 2;
						}
						else
						{
#if (SIZEOF_CURL_OFF_T > SIZEOF_LONG)
							flags |= FLAGS_LONGLONG;
#else
							flags |= FLAGS_LONG;
#endif
						}
						break;
#endif /* _WIN32 || _WIN32_WCE */
					case 'l':
						if (flags & FLAGS_LONG)
							flags |= FLAGS_LONGLONG;
						else
							flags |= FLAGS_LONG;
						break;
					case 'L':
						flags |= FLAGS_LONGDOUBLE;
						break;
					case 'q':
						flags |= FLAGS_LONGLONG;
						break;
					case 'z':
						/* the code below generates a warning if -Wunreachable-code is
						   used */
#if (SIZEOF_SIZE_T > SIZEOF_LONG)
						flags |= FLAGS_LONGLONG;
#else
						flags |= FLAGS_LONG;
#endif
						break;
					case 'O':
#if (SIZEOF_CURL_OFF_T > SIZEOF_LONG)
						flags |= FLAGS_LONGLONG;
#else
						flags |= FLAGS_LONG;
#endif
						break;
					case '0':
						if (!(flags & FLAGS_LEFT))
							flags |= FLAGS_PAD_NIL;
						FALLTHROUGH();
					case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						flags |= FLAGS_WIDTH;
						width = 0;
						fmt--;
						do
						{
							if (width > INT_MAX / 10)
								return PFMT_WIDTH;
							width *= 10;
							width += *fmt - '0';
							fmt++;
						} while (ISDIGIT(*fmt));
						break;
					case '*':  /* read width from argument list */
						flags |= FLAGS_WIDTHPARAM;
						if (use_dollar == DOLLAR_USE)
						{
							width = dollarstring(fmt, &fmt);
							if (width < 0)
								/* illegal combo */
								return PFMT_DOLLARWIDTH;
						}
						else
							/* pick from the next argument */
							width = -1;
						break;
					default:
						loopit = FALSE;
						fmt--;
						break;
				} /* switch */
			} /* while */

			switch (*fmt)
			{
				case 'S':
					flags |= FLAGS_ALT;
					FALLTHROUGH();
				case 's':
					type = FORMAT_STRING;
					break;
				case 'n':
					type = FORMAT_INTPTR;
					break;
				case 'p':
					type = FORMAT_PTR;
					break;
				case 'd':
				case 'i':
					if (flags & FLAGS_LONGLONG)
						type = FORMAT_LONGLONG;
					else if (flags & FLAGS_LONG)
						type = FORMAT_LONG;
					else
						type = FORMAT_INT;
					break;
				case 'u':
					if (flags & FLAGS_LONGLONG)
						type = FORMAT_LONGLONGU;
					else if (flags & FLAGS_LONG)
						type = FORMAT_LONGU;
					else
						type = FORMAT_INTU;
					flags |= FLAGS_UNSIGNED;
					break;
				case 'o':
					if (flags & FLAGS_LONGLONG)
						type = FORMAT_LONGLONGU;
					else if (flags & FLAGS_LONG)
						type = FORMAT_LONGU;
					else
						type = FORMAT_INTU;
					flags |= FLAGS_OCTAL | FLAGS_UNSIGNED;
					break;
				case 'x':
					if (flags & FLAGS_LONGLONG)
						type = FORMAT_LONGLONGU;
					else if (flags & FLAGS_LONG)
						type = FORMAT_LONGU;
					else
						type = FORMAT_INTU;
					flags |= FLAGS_HEX | FLAGS_UNSIGNED;
					break;
				case 'X':
					if (flags & FLAGS_LONGLONG)
						type = FORMAT_LONGLONGU;
					else if (flags & FLAGS_LONG)
						type = FORMAT_LONGU;
					else
						type = FORMAT_INTU;
					flags |= FLAGS_HEX | FLAGS_UPPER | FLAGS_UNSIGNED;
					break;
				case 'c':
					type = FORMAT_INT;
					flags |= FLAGS_CHAR;
					break;
				case 'f':
					type = FORMAT_DOUBLE;
					break;
				case 'e':
					type = FORMAT_DOUBLE;
					flags |= FLAGS_FLOATE;
					break;
				case 'E':
					type = FORMAT_DOUBLE;
					flags |= FLAGS_FLOATE | FLAGS_UPPER;
					break;
				case 'g':
					type = FORMAT_DOUBLE;
					flags |= FLAGS_FLOATG;
					break;
				case 'G':
					type = FORMAT_DOUBLE;
					flags |= FLAGS_FLOATG | FLAGS_UPPER;
					break;
				default:
					/* invalid instruction, disregard and continue */
					continue;
			} /* switch */

			if (flags & FLAGS_WIDTHPARAM)
			{
				if (width < 0)
					width = param_num++;
				else
				{
					/* if this identifies a parameter already used, this
					   is illegal */
					if (usedinput[width / 8] & (1 << (width & 7)))
						return PFMT_WIDTHARG;
				}
				if (width >= MAX_PARAMETERS)
					return PFMT_MANYARGS;
				if (width >= max_param)
					max_param = width;

				in[width].type = FORMAT_WIDTH;
				/* mark as used */
				usedinput[width / 8] |= (unsigned char)(1 << (width & 7));
			}

			if (flags & FLAGS_PRECPARAM)
			{
				if (precision < 0)
					precision = param_num++;
				else
				{
					/* if this identifies a parameter already used, this
					   is illegal */
					if (usedinput[precision / 8] & (1 << (precision & 7)))
						return PFMT_PRECARG;
				}
				if (precision >= MAX_PARAMETERS)
					return PFMT_MANYARGS;
				if (precision >= max_param)
					max_param = precision;

				in[precision].type = FORMAT_PRECISION;
				usedinput[precision / 8] |= (unsigned char)(1 << (precision & 7));
			}

			/* Handle the specifier */
			if (param < 0)
				param = param_num++;
			if (param >= MAX_PARAMETERS)
				return PFMT_MANYARGS;
			if (param >= max_param)
				max_param = param;

			iptr = &in[param];
			iptr->type = type;

			/* mark this input as used */
			usedinput[param / 8] |= (unsigned char)(1 << (param & 7));

			fmt++;
			optr = &out[ocount++];
			if (ocount > MAX_SEGMENTS)
				return PFMT_MANYSEGS;
			optr->input = (unsigned int)param;
			optr->flags = flags;
			optr->width = width;
			optr->precision = precision;
			optr->start = start;
			optr->outlen = outlen;
			start = fmt;
		}
		else
			fmt++;
	}

	/* is there a trailing piece */
	outlen = (size_t)(fmt - start);
	if (outlen)
	{
		optr = &out[ocount++];
		if (ocount > MAX_SEGMENTS)
			return PFMT_MANYSEGS;
		optr->input = 0;
		optr->flags = FLAGS_SUBSTR;
		optr->start = start;
		optr->outlen = outlen;
	}

	/* Read the arg list parameters into our data list */
	for (i = 0; i < max_param + 1; i++)
	{
		struct va_input* iptr = &in[i];
		if (!(usedinput[i / 8] & (1 << (i & 7))))
			/* bad input */
			return PFMT_INPUTGAP;

		/* based on the type, read the correct argument */
		switch (iptr->type)
		{
			case FORMAT_STRING:
				iptr->val.str = va_arg(arglist, char*);
				break;

			case FORMAT_INTPTR:
			case FORMAT_PTR:
				iptr->val.ptr = va_arg(arglist, void*);
				break;

			case FORMAT_LONGLONGU:
				iptr->val.numu = (mp_uintmax_t)va_arg(arglist, mp_uintmax_t);
				break;

			case FORMAT_LONGLONG:
				iptr->val.nums = (mp_intmax_t)va_arg(arglist, mp_intmax_t);
				break;

			case FORMAT_LONGU:
				iptr->val.numu = (mp_uintmax_t)va_arg(arglist, unsigned long);
				break;

			case FORMAT_LONG:
				iptr->val.nums = (mp_intmax_t)va_arg(arglist, long);
				break;

			case FORMAT_INTU:
				iptr->val.numu = (mp_uintmax_t)va_arg(arglist, unsigned int);
				break;

			case FORMAT_INT:
			case FORMAT_WIDTH:
			case FORMAT_PRECISION:
				iptr->val.nums = (mp_intmax_t)va_arg(arglist, int);
				break;

			case FORMAT_DOUBLE:
				iptr->val.dnum = va_arg(arglist, double);
				break;

			default:
				DEBUGASSERT(NULL); /* unexpected */
				break;
		}
	}
	*ipieces = max_param + 1;
	*opieces = ocount;

	return PFMT_OK;
}

/*
 * formatf() - the general printf function.
 *
 * It calls parsefmt() to parse the format string. It populates two arrays;
 * one that describes the input arguments and one that describes a number of
 * output segments.
 *
 * On success, the input array describes the type of all arguments and their
 * values.
 *
 * The function then iterates over the output segments and outputs them one
 * by one until done. Using the appropriate input arguments (if any).
 *
 * All output is sent to the 'stream()' callback, one byte at a time.
 */

static int formatf(
	void* userp, /* untouched by format(), just sent to the stream() function in
					the second argument */
					/* function pointer called for each output character */
	int (*stream)(unsigned char, void*),
	const char* format,    /* %-formatted string */
	va_list ap_save) /* list of parameters */
{
	static const char nilstr[] = "(nil)";
	const char* digits = lower_digits;   /* Base-36 digits for numbers.  */
	int done = 0;   /* number of characters written  */
	int i;
	int ocount = 0; /* number of output segments */
	int icount = 0; /* number of input arguments */

	struct outsegment output[MAX_SEGMENTS];
	struct va_input input[MAX_PARAMETERS];
	char work[BUFFSIZE];

	/* 'workend' points to the final buffer byte position, but with an extra
	   byte as margin to avoid the (FALSE?) warning Coverity gives us
	   otherwise */
	char* workend = &work[sizeof(work) - 2];

	/* Parse the format string */
	if (parsefmt(format, output, input, &ocount, &icount, ap_save))
		return 0;

	for (i = 0; i < ocount; i++)
	{
		struct outsegment* optr = &output[i];
		struct va_input* iptr;
		bool is_alt;            /* Format spec modifiers.  */
		int width;              /* Width of a field.  */
		int prec;               /* Precision of a field.  */
		bool is_neg;            /* Decimal integer is negative.  */
		unsigned long base;     /* Base of a number to be written.  */
		mp_uintmax_t num;       /* Integral values to be written.  */
		mp_intmax_t signed_num; /* Used to convert negative in positive.  */
		char* w;
		size_t outlen = optr->outlen;
		unsigned int flags = optr->flags;

		if (outlen)
		{
			char* str = optr->start;
			for (; outlen && *str; outlen--)
				OUTCHAR(*str++);
			if (optr->flags & FLAGS_SUBSTR)
				/* this is just a substring */
				continue;
		}

		/* pick up the specified width */
		if (flags & FLAGS_WIDTHPARAM)
		{
			width = (int)input[optr->width].val.nums;
			if (width < 0)
			{
				/* "A negative field width is taken as a '-' flag followed by a
				   positive field width." */
				if (width == INT_MIN)
					width = INT_MAX;
				else
					width = -width;
				flags |= FLAGS_LEFT;
				flags &= ~(unsigned int)FLAGS_PAD_NIL;
			}
		}
		else
			width = optr->width;

		/* pick up the specified precision */
		if (flags & FLAGS_PRECPARAM)
		{
			prec = (int)input[optr->precision].val.nums;
			if (prec < 0)
				/* "A negative precision is taken as if the precision were
				   omitted." */
				prec = -1;
		}
		else if (flags & FLAGS_PREC)
			prec = optr->precision;
		else
			prec = -1;

		is_alt = (flags & FLAGS_ALT) ? 1 : 0;
		iptr = &input[optr->input];

		switch (iptr->type)
		{
			case FORMAT_INTU:
			case FORMAT_LONGU:
			case FORMAT_LONGLONGU:
				flags |= FLAGS_UNSIGNED;
				FALLTHROUGH();
			case FORMAT_INT:
			case FORMAT_LONG:
			case FORMAT_LONGLONG:
				num = iptr->val.numu;
				if (flags & FLAGS_CHAR)
				{
					/* Character.  */
					if (!(flags & FLAGS_LEFT))
						while (--width > 0)
							OUTCHAR(' ');
					OUTCHAR((char)num);
					if (flags & FLAGS_LEFT)
						while (--width > 0)
							OUTCHAR(' ');
					break;
				}
				if (flags & FLAGS_OCTAL)
				{
					/* Octal unsigned integer */
					base = 8;
					is_neg = FALSE;
				}
				else if (flags & FLAGS_HEX)
				{
					/* Hexadecimal unsigned integer */
					digits = (flags & FLAGS_UPPER) ? upper_digits : lower_digits;
					base = 16;
					is_neg = FALSE;
				}
				else if (flags & FLAGS_UNSIGNED)
				{
					/* Decimal unsigned integer */
					base = 10;
					is_neg = FALSE;
				}
				else
				{
					/* Decimal integer.  */
					base = 10;

					is_neg = (iptr->val.nums < (mp_intmax_t)0);
					if (is_neg)
					{
						/* signed_num might fail to hold absolute negative minimum by 1 */
						signed_num = iptr->val.nums + (mp_intmax_t)1;
						signed_num = -signed_num;
						num = (mp_uintmax_t)signed_num;
						num += (mp_uintmax_t)1;
					}
				}
number:
				/* Supply a default precision if none was given.  */
				if (prec == -1)
					prec = 1;

				/* Put the number in WORK.  */
				w = workend;
				switch (base)
				{
					case 10:
						while (num > 0)
						{
							*w-- = (char)('0' + (num % 10));
							num /= 10;
						}
						break;
					default:
						while (num > 0)
						{
							*w-- = digits[num % base];
							num /= base;
						}
						break;
				}
				width -= (int)(workend - w);
				prec -= (int)(workend - w);

				if (is_alt && base == 8 && prec <= 0)
				{
					*w-- = '0';
					--width;
				}

				if (prec > 0)
				{
					width -= prec;
					while (prec-- > 0 && w >= work)
						*w-- = '0';
				}

				if (is_alt && base == 16)
					width -= 2;

				if (is_neg || (flags & FLAGS_SHOWSIGN) || (flags & FLAGS_SPACE))
					--width;

				if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_PAD_NIL))
					while (width-- > 0)
						OUTCHAR(' ');

				if (is_neg)
					OUTCHAR('-');
				else if (flags & FLAGS_SHOWSIGN)
					OUTCHAR('+');
				else if (flags & FLAGS_SPACE)
					OUTCHAR(' ');

				if (is_alt && base == 16)
				{
					OUTCHAR('0');
					if (flags & FLAGS_UPPER)
						OUTCHAR('X');
					else
						OUTCHAR('x');
				}

				if (!(flags & FLAGS_LEFT) && (flags & FLAGS_PAD_NIL))
					while (width-- > 0)
						OUTCHAR('0');

				/* Write the number.  */
				while (++w <= workend)
				{
					OUTCHAR(*w);
				}

				if (flags & FLAGS_LEFT)
					while (width-- > 0)
						OUTCHAR(' ');
				break;

			case FORMAT_STRING:
			{
				const char* str;
				size_t len;

				str = (char*)iptr->val.str;
				if (!str)
				{
					/* Write null string if there is space.  */
					if (prec == -1 || prec >= (int)sizeof(nilstr) - 1)
					{
						str = nilstr;
						len = sizeof(nilstr) - 1;
						/* Disable quotes around (nil) */
						flags &= ~(unsigned int)FLAGS_ALT;
					}
					else
					{
						str = "";
						len = 0;
					}
				}
				else if (prec != -1)
					len = (size_t)prec;
				else if (*str == '\0')
					len = 0;
				else
					len = strlen(str);

				width -= (len > INT_MAX) ? INT_MAX : (int)len;

				if (flags & FLAGS_ALT)
					OUTCHAR('"');

				if (!(flags & FLAGS_LEFT))
					while (width-- > 0)
						OUTCHAR(' ');

				for (; len && *str; len--)
					OUTCHAR(*str++);
				if (flags & FLAGS_LEFT)
					while (width-- > 0)
						OUTCHAR(' ');

				if (flags & FLAGS_ALT)
					OUTCHAR('"');
				break;
			}

			case FORMAT_PTR:
				/* Generic pointer.  */
				if (iptr->val.ptr)
				{
					/* If the pointer is not NULL, write it as a %#x spec.  */
					base = 16;
					digits = (flags & FLAGS_UPPER) ? upper_digits : lower_digits;
					is_alt = TRUE;
					num = (size_t)iptr->val.ptr;
					is_neg = FALSE;
					goto number;
				}
				else
				{
					/* Write "(nil)" for a nil pointer.  */
					const char* point;

					width -= (int)(sizeof(nilstr) - 1);
					if (flags & FLAGS_LEFT)
						while (width-- > 0)
							OUTCHAR(' ');
					for (point = nilstr; *point != '\0'; ++point)
						OUTCHAR(*point);
					if (!(flags & FLAGS_LEFT))
						while (width-- > 0)
							OUTCHAR(' ');
				}
				break;

			case FORMAT_DOUBLE:
			{
				char formatbuf[32] = "%";
				char* fptr = &formatbuf[1];
				size_t left = sizeof(formatbuf) - strlen(formatbuf);
				int len;

				if (flags & FLAGS_WIDTH)
					width = optr->width;

				if (flags & FLAGS_PREC)
					prec = optr->precision;

				if (flags & FLAGS_LEFT)
					*fptr++ = '-';
				if (flags & FLAGS_SHOWSIGN)
					*fptr++ = '+';
				if (flags & FLAGS_SPACE)
					*fptr++ = ' ';
				if (flags & FLAGS_ALT)
					*fptr++ = '#';

				*fptr = 0;

				if (width >= 0)
				{
					size_t dlen;
					if (width >= (int)sizeof(work))
						width = sizeof(work) - 1;
					/* RECURSIVE USAGE */
					dlen = (size_t)curl_msnprintf(fptr, left, "%d", width);
					fptr += dlen;
					left -= dlen;
				}
				if (prec >= 0)
				{
					/* for each digit in the integer part, we can have one less
					   precision */
					size_t maxprec = sizeof(work) - 2;
					double val = iptr->val.dnum;
					if (width > 0 && prec <= width)
						maxprec -= (size_t)width;
					while (val >= 10.0)
					{
						val /= 10;
						maxprec--;
					}

					if (prec > (int)maxprec)
						prec = (int)maxprec - 1;
					if (prec < 0)
						prec = 0;
					/* RECURSIVE USAGE */
					len = curl_msnprintf(fptr, left, ".%d", prec);
					fptr += len;
				}
				if (flags & FLAGS_LONG)
					*fptr++ = 'l';

				if (flags & FLAGS_FLOATE)
					*fptr++ = (char)((flags & FLAGS_UPPER) ? 'E' : 'e');
				else if (flags & FLAGS_FLOATG)
					*fptr++ = (char)((flags & FLAGS_UPPER) ? 'G' : 'g');
				else
					*fptr++ = 'f';

				*fptr = 0; /* and a final null-termination */

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
				/* NOTE NOTE NOTE!! Not all sprintf implementations return number of
				   output characters */
#ifdef HAVE_SNPRINTF
				(snprintf)(work, sizeof(work), formatbuf, iptr->val.dnum);
#else
				(sprintf)(work, formatbuf, iptr->val.dnum);
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif
				DEBUGASSERT(strlen(work) <= sizeof(work));
				for (fptr = work; *fptr; fptr++)
					OUTCHAR(*fptr);
				break;
			}

			case FORMAT_INTPTR:
				/* Answer the count of characters written.  */
#ifdef HAVE_LONG_LONG_TYPE
				if (flags & FLAGS_LONGLONG)
					*(LONG_LONG_TYPE*)iptr->val.ptr = (LONG_LONG_TYPE)done;
				else
#endif
					if (flags & FLAGS_LONG)
						*(long*)iptr->val.ptr = (long)done;
					else if (!(flags & FLAGS_SHORT))
						*(int*)iptr->val.ptr = (int)done;
					else
						*(short*)iptr->val.ptr = (short)done;
				break;

			default:
				break;
			}
		}
	return done;
	}

/* fputc() look-alike */
static int addbyter(unsigned char outc, void* f)
{
	struct nsprintf* infop = (nsprintf*)f;
	if (infop->length < infop->max)
	{
		/* only do this if we have not reached max length yet */
		*infop->buffer++ = (char)outc; /* store */
		infop->length++; /* we are now one byte larger */
		return 0;     /* fputc() returns like this on success */
	}
	return 1;
}

int curl_mvsnprintf(char* buffer, size_t maxlength, const char* format,
	va_list ap_save)
{
	int retcode;
	struct nsprintf info;

	info.buffer = buffer;
	info.length = 0;
	info.max = maxlength;

	retcode = formatf(&info, addbyter, format, ap_save);
	if (info.max)
	{
		/* we terminate this with a zero byte */
		if (info.max == info.length)
		{
			/* we are at maximum, scrap the last letter */
			info.buffer[-1] = 0;
			DEBUGASSERT(retcode);
			retcode--; /* do not count the nul byte */
		}
		else
			info.buffer[0] = 0;
	}
	return retcode;
}

int curl_msnprintf(char* buffer, size_t maxlength, const char* format, ...)
{
	int retcode;
	va_list ap_save; /* argument pointer */
	va_start(ap_save, format);
	retcode = curl_mvsnprintf(buffer, maxlength, format, ap_save);
	va_end(ap_save);
	return retcode;
}

/* fputc() look-alike */
static int alloc_addbyter(unsigned char outc, void* f)
{
	struct asprintf* infop = (asprintf*)f;
	CURLcode result = Curl_dyn_addn(infop->b, &outc, 1);
	if (result)
	{
		infop->merr = result == CURLE_TOO_LARGE ? MERR_TOO_LARGE : MERR_MEM;
		return 1; /* fail */
	}
	return 0;
}

/* appends the formatted string, returns MERR error code */
int Curl_dyn_vprintf(struct dynbuf* dyn, const char* format, va_list ap_save)
{
	struct asprintf info;
	info.b = dyn;
	info.merr = MERR_OK;

	(void)formatf(&info, alloc_addbyter, format, ap_save);
	if (info.merr)
	{
		Curl_dyn_free(info.b);
		return info.merr;
	}
	return 0;
}

char* curl_mvaprintf(const char* format, va_list ap_save)
{
	struct asprintf info;
	struct dynbuf dyn;
	info.b = &dyn;
	Curl_dyn_init(info.b, DYN_APRINTF);
	info.merr = MERR_OK;

	(void)formatf(&info, alloc_addbyter, format, ap_save);
	if (info.merr)
	{
		Curl_dyn_free(info.b);
		return NULL;
	}
	if (Curl_dyn_len(info.b))
		return Curl_dyn_ptr(info.b);
	return _strdup("");
}

char* curl_maprintf(const char* format, ...)
{
	va_list ap_save;
	char* s;
	va_start(ap_save, format);
	s = curl_mvaprintf(format, ap_save);
	va_end(ap_save);
	return s;
}

static int storebuffer(unsigned char outc, void* f)
{
	char** buffer = (char**)f;
	**buffer = (char)outc;
	(*buffer)++;
	return 0;
}

int curl_msprintf(char* buffer, const char* format, ...)
{
	va_list ap_save; /* argument pointer */
	int retcode;
	va_start(ap_save, format);
	retcode = formatf(&buffer, storebuffer, format, ap_save);
	va_end(ap_save);
	*buffer = 0; /* we terminate this with a zero byte */
	return retcode;
}

static int fputc_wrapper(unsigned char outc, void* f)
{
	int out = outc;
	FILE* s = (FILE*)f;
	int rc = fputc(out, s);
	return rc == EOF;
}

int curl_mprintf(const char* format, ...)
{
	int retcode;
	va_list ap_save; /* argument pointer */
	va_start(ap_save, format);

	retcode = formatf(stdout, fputc_wrapper, format, ap_save);
	va_end(ap_save);
	return retcode;
}

int curl_mfprintf(FILE* whereto, const char* format, ...)
{
	int retcode;
	va_list ap_save; /* argument pointer */
	va_start(ap_save, format);
	retcode = formatf(whereto, fputc_wrapper, format, ap_save);
	va_end(ap_save);
	return retcode;
}

int curl_mvsprintf(char* buffer, const char* format, va_list ap_save)
{
	int retcode = formatf(&buffer, storebuffer, format, ap_save);
	*buffer = 0; /* we terminate this with a zero byte */
	return retcode;
}

int curl_mvprintf(const char* format, va_list ap_save)
{
	return formatf(stdout, fputc_wrapper, format, ap_save);
}

int curl_mvfprintf(FILE* whereto, const char* format, va_list ap_save)
{
	return formatf(whereto, fputc_wrapper, format, ap_save);
}
#pragma endregion

#pragma region curl_multibyte.h

#if defined(_WIN32)

wchar_t* curlx_convert_UTF8_to_wchar(const char* str_utf8)
{
	wchar_t* str_w = NULL;
	if (str_utf8)
	{
		int str_w_len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str_utf8, -1, NULL, 0);
		if (str_w_len > 0)
		{
			str_w = (wchar_t*)malloc(str_w_len * sizeof(wchar_t));
			if (str_w)
			{
				if (MultiByteToWideChar(CP_UTF8, 0, str_utf8, -1, str_w, str_w_len) == 0)
				{
					free(str_w);
					return NULL;
				}
			}
		}
	}
	return str_w;
}

char* curlx_convert_wchar_to_UTF8(const wchar_t* str_w)
{
	char* str_utf8 = NULL;
	if (str_w)
	{
		int bytes = WideCharToMultiByte(CP_UTF8, 0, str_w, -1, NULL, 0, NULL, NULL);
		if (bytes > 0)
		{
			str_utf8 = (char*)malloc(bytes);
			if (str_utf8)
			{
				if (WideCharToMultiByte(CP_UTF8, 0, str_w, -1, str_utf8, bytes, NULL, NULL) == 0)
				{
					free(str_utf8);
					return NULL;
				}
			}
		}
	}

	return str_utf8;
}

#endif /* _WIN32 */

#if defined(USE_WIN32_LARGE_FILES) || defined(USE_WIN32_SMALL_FILES)
#include <io.h>
#include <fcntl.h>

int curlx_win32_open(const char* filename, int oflag, ...)
{
	int pmode = 0;

#ifdef _UNICODE
	int result = -1;
	wchar_t* filename_w = curlx_convert_UTF8_to_wchar(filename);
#endif

	va_list param;
	va_start(param, oflag);
	if (oflag & O_CREAT)
		pmode = va_arg(param, int);
	va_end(param);

#ifdef _UNICODE
	if (filename_w)
	{
		result = _wopen(filename_w, oflag, pmode);
		curlx_unicodefree(filename_w);
	}
	else
		errno = EINVAL;
	return result;
#else
	return (_open)(filename, oflag, pmode);
#endif
}

FILE* curlx_win32_fopen(const char* filename, const char* mode)
{
#ifdef _UNICODE
	FILE* result = NULL;
	wchar_t* filename_w = curlx_convert_UTF8_to_wchar(filename);
	wchar_t* mode_w = curlx_convert_UTF8_to_wchar(mode);
	if (filename_w && mode_w)
		result = _wfopen(filename_w, mode_w);
	else
		errno = EINVAL;
	curlx_unicodefree(filename_w);
	curlx_unicodefree(mode_w);
	return result;
#else
	return (fopen)(filename, mode);
#endif
}

int curlx_win32_stat(const char* path, struct_stat* buffer)
{
#ifdef _UNICODE
	int result = -1;
	wchar_t* path_w = curlx_convert_UTF8_to_wchar(path);
	if (path_w)
	{
#if defined(USE_WIN32_SMALL_FILES)
		result = _wstat(path_w, buffer);
#else
		result = _wstati64(path_w, buffer);
#endif
		curlx_unicodefree(path_w);
	}
	else
		errno = EINVAL;
	return result;
#else
#if defined(USE_WIN32_SMALL_FILES)
	return _stat(path, buffer);
#else
	return _stati64(path, buffer);
#endif
#endif
}
#endif /* USE_WIN32_LARGE_FILES || USE_WIN32_SMALL_FILES */

#pragma endregion

#pragma region idn.h

#ifdef USE_WIN32_IDN

/* using Windows kernel32 and normaliz libraries. */
#if (!defined(_WIN32_WINNT) || _WIN32_WINNT < 0x600) && (!defined(WINVER) || WINVER < 0x600)

WINBASEAPI int WINAPI IdnToAscii(DWORD dwFlags,
	const WCHAR* lpUnicodeCharStr,
	int cchUnicodeChar,
	WCHAR* lpASCIICharStr,
	int cchASCIIChar);
WINBASEAPI int WINAPI IdnToUnicode(DWORD dwFlags,
	const WCHAR* lpASCIICharStr,
	int cchASCIIChar,
	WCHAR* lpUnicodeCharStr,
	int cchUnicodeChar);
#endif

static CURLcode win32_idn_to_ascii(const char* in, char** out)
{
	wchar_t* in_w = curlx_convert_UTF8_to_wchar(in);
	*out = NULL;
	if (in_w)
	{
		wchar_t punycode[IDN_MAX_LENGTH];
		int chars = IdnToAscii(0,
			in_w, (int)(wcslen(in_w) + 1),
			punycode,
			IDN_MAX_LENGTH);
		curlx_unicodefree(in_w);
		if (chars)
		{
			char* mstr = curlx_convert_wchar_to_UTF8(punycode);
			if (mstr)
			{
				*out = _strdup(mstr);
				curlx_unicodefree(mstr);
				if (!*out)
					return CURLE_OUT_OF_MEMORY;
			}
			else
				return CURLE_OUT_OF_MEMORY;
		}
		else
			return CURLE_URL_MALFORMAT;
	}
	else
		return CURLE_URL_MALFORMAT;

	return CURLE_OK;
	}

static CURLcode win32_ascii_to_idn(const char* in, char** output)
{
	char* out = NULL;
	wchar_t* in_w = curlx_convert_UTF8_to_wchar(in);
	if (in_w)
	{
		WCHAR idn[IDN_MAX_LENGTH]; /* stores a UTF-16 string */
		int chars = IdnToUnicode(0,
			in_w, (int)(wcslen(in_w) + 1),
			idn,
			IDN_MAX_LENGTH);
		if (chars)
		{
			/* 'chars' is "the number of characters retrieved" */
			char* mstr = curlx_convert_wchar_to_UTF8(idn);
			if (mstr)
			{
				out = _strdup(mstr);
				curlx_unicodefree(mstr);
				if (!out)
				{
					return CURLE_OUT_OF_MEMORY;
				}
			}
		}
		else
			return CURLE_URL_MALFORMAT;
	}
	else
		return CURLE_URL_MALFORMAT;

	*output = out;
	return CURLE_OK;
}

#endif /* USE_WIN32_IDN */

bool Curl_is_ASCII_name(const char* hostname)
{
	/* get an UNSIGNED local version of the pointer */
	const unsigned char* ch = (const unsigned char*)hostname;
	/* bad input, consider it ASCII! */
	if (!hostname)
	{
		return TRUE;
	}
	while (*ch)
	{
		if (*ch++ & 0x80)
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * Curl_idn_decode() returns an allocated IDN decoded string if it was
 * possible. NULL on error.
 *
 * CURLE_URL_MALFORMAT - the hostname could not be converted
 * CURLE_OUT_OF_MEMORY - memory problem
 *
 */
static CURLcode idn_decode(const char* input, char** output)
{
	char* decoded = NULL;
	CURLcode result = CURLE_OK;
	result = win32_idn_to_ascii(input, &decoded);
	if (!result)
	{
		*output = decoded;
	}
	return result;
}

static CURLcode idn_encode(const char* puny, char** output)
{
	char* enc = NULL;
	CURLcode result = win32_ascii_to_idn(puny, &enc);
	if (result)
	{
		return result;
	}
	*output = enc;
	return CURLE_OK;
}

CURLcode Curl_idn_decode(const char* input, char** output)
{
	char* d = NULL;
	CURLcode result = idn_decode(input, &d);
	if (!result)
	{
		*output = d;
	}
	return result;
}

CURLcode Curl_idn_encode(const char* puny, char** output)
{
	char* d = NULL;
	CURLcode result = idn_encode(puny, &d);
	if (!result)
	{
		*output = d;
	}
	return result;
}

/*
 * Frees data allocated by idnconvert_hostname()
 */
void Curl_free_idnconverted_hostname(struct hostname* host)
{
	Curl_safefree(host->encalloc);
}

/*
 * Perform any necessary IDN conversion of hostname
 */
CURLcode Curl_idnconvert_hostname(struct hostname* host)
{
	/* set the name we use to display the hostname */
	host->dispname = host->name;

#ifdef USE_IDN
	/* Check name for non-ASCII and convert hostname if we can */
	if (!Curl_is_ASCII_name(host->name))
	{
		char* decoded;
		CURLcode result = Curl_idn_decode(host->name, &decoded);
		if (result)
		{
			return result;
		}
		/* successful */
		host->name = host->encalloc = decoded;
	}
#endif
	return CURLE_OK;
}

#pragma endregion

#pragma region inet.h


#define IN6ADDRSZ       16
#define INADDRSZ         4
#define INT16SZ          2

/*
 * If USE_IPV6 is disabled, we still want to parse IPv6 addresses, so make
 * sure we have _some_ value for AF_INET6 without polluting our fake value
 * everywhere.
 */
#if !defined(USE_IPV6) && !defined(AF_INET6)
#define AF_INET6 (AF_INET + 1)
#endif

 /*
   * Format an IPv4 address, more or less like inet_ntop().
   *
   * Returns `dst' (as a const)
   * Note:
   *  - uses no statics
   *  - takes a unsigned char* not an in_addr as input
   */
static char* inet_ntop4(const unsigned char* src, char* dst, size_t size)
{
	char tmp[sizeof("255.255.255.255")];
	size_t len;

	DEBUGASSERT(size >= 16);

	tmp[0] = '\0';
	(void)msnprintf(tmp, sizeof(tmp), "%d.%d.%d.%d",
		((int)((unsigned char)src[0])) & 0xff,
		((int)((unsigned char)src[1])) & 0xff,
		((int)((unsigned char)src[2])) & 0xff,
		((int)((unsigned char)src[3])) & 0xff);

	len = strlen(tmp);
	if (len == 0 || len >= size)
	{
		errno = ENOSPC;
		return (NULL);
	}
	strcpy(dst, tmp);
	return dst;
}

/*
 * Convert IPv6 binary address into presentation (printable) format.
 */
static char* inet_ntop6(const unsigned char* src, char* dst, size_t size)
{
	/*
	 * Note that int32_t and int16_t need only be "at least" large enough
	 * to contain a value of the specified size. On some systems, like
	 * Crays, there is no such thing as an integer variable with 16 bits.
	 * Keep this in mind if you think this function should have been coded
	 * to use pointer overlays. All the world's not a VAX.
	 */
	char tmp[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
	char* tp;
	struct {
		int base;
		int len;
	} best, cur;
	unsigned int words[IN6ADDRSZ / INT16SZ];
	int i;

	/* Preprocess:
	 *  Copy the input (bytewise) array into a wordwise array.
	 *  Find the longest run of 0x00's in src[] for :: shorthanding.
	 */
	memset(words, '\0', sizeof(words));
	for (i = 0; i < IN6ADDRSZ; i++)
		words[i / 2] |= ((unsigned int)src[i] << ((1 - (i % 2)) << 3));

	best.base = -1;
	cur.base = -1;
	best.len = 0;
	cur.len = 0;

	for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
	{
		if (words[i] == 0)
		{
			if (cur.base == -1)
			{
				cur.base = i; cur.len = 1;
			}
			else
				cur.len++;
		}
		else if (cur.base != -1)
		{
			if (best.base == -1 || cur.len > best.len)
				best = cur;
			cur.base = -1;
		}
	}
	if ((cur.base != -1) && (best.base == -1 || cur.len > best.len))
		best = cur;
	if (best.base != -1 && best.len < 2)
		best.base = -1;
	/* Format the result. */
	tp = tmp;
	for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
	{
		/* Are we inside the best run of 0x00's? */
		if (best.base != -1 && i >= best.base && i < (best.base + best.len))
		{
			if (i == best.base)
				*tp++ = ':';
			continue;
		}

		/* Are we following an initial run of 0x00s or any real hex?
		 */
		if (i)
			*tp++ = ':';

		/* Is this address an encapsulated IPv4?
		 */
		if (i == 6 && best.base == 0 &&
			(best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
		{
			if (!inet_ntop4(src + 12, tp, sizeof(tmp) - (tp - tmp)))
			{
				errno = ENOSPC;
				return (NULL);
			}
			tp += strlen(tp);
			break;
		}
		tp += msnprintf(tp, 5, "%x", words[i]);
	}

	/* Was it a trailing run of 0x00's?
	 */
	if (best.base != -1 && (best.base + best.len) == (IN6ADDRSZ / INT16SZ))
		*tp++ = ':';
	*tp++ = '\0';

	/* Check for overflow, copy, and we are done.
	 */
	if ((size_t)(tp - tmp) > size)
	{
		errno = ENOSPC;
		return (NULL);
	}
	strcpy(dst, tmp);
	return dst;
}

/*
 * Convert a network format address to presentation format.
 *
 * Returns pointer to presentation format address (`buf').
 * Returns NULL on error and errno set with the specific
 * error, EAFNOSUPPORT or ENOSPC.
 *
 * On Windows we store the error in the thread errno, not in the Winsock error
 * code. This is to avoid losing the actual last Winsock error. When this
 * function returns NULL, check errno not SOCKERRNO.
 */
char* Curl_inet_ntop(int af, const void* src, char* buf, size_t size)
{
	switch (af)
	{
		case AF_INET:
			return inet_ntop4((const unsigned char*)src, buf, size);
		case AF_INET6:
			return inet_ntop6((const unsigned char*)src, buf, size);
		default:
			errno = EAFNOSUPPORT;
			return NULL;
	}
}

/* int
 * inet_pton4(src, dst)
 *      like inet_aton() but without all the hexadecimal and shorthand.
 * return:
 *      1 if `src' is a valid dotted quad, else 0.
 * notice:
 *      does not touch `dst' unless it is returning 1.
 * author:
 *      Paul Vixie, 1996.
 */
static int inet_pton4(const char* src, unsigned char* dst)
{
	static const char digits[] = "0123456789";
	int saw_digit, octets, ch;
	unsigned char tmp[INADDRSZ], * tp;

	saw_digit = 0;
	octets = 0;
	tp = tmp;
	*tp = 0;
	while ((ch = *src++) != '\0')
	{
		const char* pch;

		pch = strchr(digits, ch);
		if (pch)
		{
			unsigned int val = (unsigned int)(*tp * 10) +
				(unsigned int)(pch - digits);

			if (saw_digit && *tp == 0)
				return (0);
			if (val > 255)
				return (0);
			*tp = (unsigned char)val;
			if (!saw_digit)
			{
				if (++octets > 4)
					return (0);
				saw_digit = 1;
			}
		}
		else if (ch == '.' && saw_digit)
		{
			if (octets == 4)
				return (0);
			*++tp = 0;
			saw_digit = 0;
		}
		else
			return (0);
	}
	if (octets < 4)
		return (0);
	memcpy(dst, tmp, INADDRSZ);
	return (1);
}

/* int
 * inet_pton6(src, dst)
 *      convert presentation level address to network order binary form.
 * return:
 *      1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *      (1) does not touch `dst' unless it is returning 1.
 *      (2) :: in a full address is silently ignored.
 * credit:
 *      inspired by Mark Andrews.
 * author:
 *      Paul Vixie, 1996.
 */
static int inet_pton6(const char* src, unsigned char* dst)
{
	static const char xdigits_l[] = "0123456789abcdef",
		xdigits_u[] = "0123456789ABCDEF";
	unsigned char tmp[IN6ADDRSZ], * tp, * endp, * colonp;
	const char* curtok;
	int ch, saw_xdigit;
	size_t val;

	memset((tp = tmp), 0, IN6ADDRSZ);
	endp = tp + IN6ADDRSZ;
	colonp = NULL;
	/* Leading :: requires some special handling. */
	if (*src == ':')
		if (*++src != ':')
			return (0);
	curtok = src;
	saw_xdigit = 0;
	val = 0;
	while ((ch = *src++) != '\0')
	{
		const char* xdigits;
		const char* pch;

		pch = strchr((xdigits = xdigits_l), ch);
		if (!pch)
			pch = strchr((xdigits = xdigits_u), ch);
		if (pch)
		{
			val <<= 4;
			val |= (pch - xdigits);
			if (++saw_xdigit > 4)
				return (0);
			continue;
		}
		if (ch == ':')
		{
			curtok = src;
			if (!saw_xdigit)
			{
				if (colonp)
					return (0);
				colonp = tp;
				continue;
			}
			if (tp + INT16SZ > endp)
				return (0);
			*tp++ = (unsigned char)((val >> 8) & 0xff);
			*tp++ = (unsigned char)(val & 0xff);
			saw_xdigit = 0;
			val = 0;
			continue;
		}
		if (ch == '.' && ((tp + INADDRSZ) <= endp) &&
			inet_pton4(curtok, tp) > 0)
		{
			tp += INADDRSZ;
			saw_xdigit = 0;
			break;    /* '\0' was seen by inet_pton4(). */
		}
		return (0);
	}
	if (saw_xdigit)
	{
		if (tp + INT16SZ > endp)
			return (0);
		*tp++ = (unsigned char)((val >> 8) & 0xff);
		*tp++ = (unsigned char)(val & 0xff);
	}
	if (colonp)
	{
		/*
		 * Since some memmove()'s erroneously fail to handle
		 * overlapping regions, we will do the shift by hand.
		 */
		const ssize_t n = tp - colonp;
		ssize_t i;

		if (tp == endp)
			return (0);
		for (i = 1; i <= n; i++)
		{
			*(endp - i) = *(colonp + n - i);
			*(colonp + n - i) = 0;
		}
		tp = endp;
	}
	if (tp != endp)
		return (0);
	memcpy(dst, tmp, IN6ADDRSZ);
	return (1);
}

/* int
 * inet_pton(af, src, dst)
 *      convert from presentation format (which usually means ASCII printable)
 *      to network format (which is usually some kind of binary format).
 * return:
 *      1 if the address was valid for the specified address family
 *      0 if the address was not valid (`dst' is untouched in this case)
 *      -1 if some other error occurred (`dst' is untouched in this case, too)
 * notice:
 *      On Windows we store the error in the thread errno, not
 *      in the Winsock error code. This is to avoid losing the
 *      actual last Winsock error. When this function returns
 *      -1, check errno not SOCKERRNO.
 * author:
 *      Paul Vixie, 1996.
 */
int Curl_inet_pton(int af, const char* src, void* dst)
{
	switch (af)
	{
		case AF_INET:
			return (inet_pton4(src, (unsigned char*)dst));
		case AF_INET6:
			return (inet_pton6(src, (unsigned char*)dst));
		default:
			errno = EAFNOSUPPORT;
			return (-1);
	}
	/* NOTREACHED */
}

/*=================================[timeval.h & timediff.h ]====================================*/

struct timeval* curlx_mstotv(struct timeval* tv, timediff_t ms)
{
	if (!tv)
		return NULL;

	if (ms < 0)
		return NULL;

	if (ms > 0)
	{
		timediff_t tv_sec = ms / 1000;
		timediff_t tv_usec = (ms % 1000) * 1000; /* max=999999 */
#ifdef HAVE_SUSECONDS_T
#if TIMEDIFF_T_MAX > TIME_T_MAX
		/* tv_sec overflow check in case time_t is signed */
		if (tv_sec > TIME_T_MAX)
			tv_sec = TIME_T_MAX;
#endif
		tv->tv_sec = (time_t)tv_sec;
		tv->tv_usec = (suseconds_t)tv_usec;
#elif defined(_WIN32) /* maybe also others in the future */
#if TIMEDIFF_T_MAX > LONG_MAX
		/* tv_sec overflow check on Windows there we know it is long */
		if (tv_sec > LONG_MAX)
			tv_sec = LONG_MAX;
#endif
		tv->tv_sec = (long)tv_sec;
		tv->tv_usec = (long)tv_usec;
#else
#if TIMEDIFF_T_MAX > INT_MAX
		/* tv_sec overflow check in case time_t is signed */
		if (tv_sec > INT_MAX)
			tv_sec = INT_MAX;
#endif
		tv->tv_sec = (int)tv_sec;
		tv->tv_usec = (int)tv_usec;
#endif
	}
	else
	{
		tv->tv_sec = 0;
		tv->tv_usec = 0;
	}

	return tv;
}

timediff_t curlx_tvtoms(struct timeval* tv)
{
	return (tv->tv_sec * 1000) + (timediff_t)(((double)tv->tv_usec) / 1000.0);
}

LARGE_INTEGER Curl_freq;

struct curltime Curl_now(void)
{
	struct curltime now;
	if (TRUE)
	{ /* QPC timer might have issues pre-Vista */
		LARGE_INTEGER count;
		QueryPerformanceCounter(&count);
		now.tv_sec = (time_t)(count.QuadPart / Curl_freq.QuadPart);
		now.tv_usec = (int)((count.QuadPart % Curl_freq.QuadPart) * 1000000 / Curl_freq.QuadPart);
	}
	else
	{
		/* Disable /analyze warning that GetTickCount64 is preferred  */
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:28159)
#endif
		DWORD milliseconds = GetTickCount();
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

		now.tv_sec = (time_t)(milliseconds / 1000);
		now.tv_usec = (int)((milliseconds % 1000) * 1000);
	}
	return now;
}

/*
 * Returns: time difference in number of milliseconds. For too large diffs it
 * returns max value.
 *
 * @unittest: 1323
 */
timediff_t Curl_timediff(struct curltime newer, struct curltime older)
{
	timediff_t diff = (timediff_t)newer.tv_sec - older.tv_sec;
	if (diff >= (TIMEDIFF_T_MAX / 1000))
		return TIMEDIFF_T_MAX;
	else if (diff <= (TIMEDIFF_T_MIN / 1000))
		return TIMEDIFF_T_MIN;
	return diff * 1000 + (newer.tv_usec - older.tv_usec) / 1000;
}

/*
 * Returns: time difference in number of milliseconds, rounded up.
 * For too large diffs it returns max value.
 */
timediff_t Curl_timediff_ceil(struct curltime newer, struct curltime older)
{
	timediff_t diff = (timediff_t)newer.tv_sec - older.tv_sec;
	if (diff >= (TIMEDIFF_T_MAX / 1000))
		return TIMEDIFF_T_MAX;
	else if (diff <= (TIMEDIFF_T_MIN / 1000))
		return TIMEDIFF_T_MIN;
	return diff * 1000 + (newer.tv_usec - older.tv_usec + 999) / 1000;
}

/*
 * Returns: time difference in number of microseconds. For too large diffs it
 * returns max value.
 */
timediff_t Curl_timediff_us(struct curltime newer, struct curltime older)
{
	timediff_t diff = (timediff_t)newer.tv_sec - older.tv_sec;
	if (diff >= (TIMEDIFF_T_MAX / 1000000))
		return TIMEDIFF_T_MAX;
	else if (diff <= (TIMEDIFF_T_MIN / 1000000))
		return TIMEDIFF_T_MIN;
	return diff * 1000000 + newer.tv_usec - older.tv_usec;
}


#pragma endregion