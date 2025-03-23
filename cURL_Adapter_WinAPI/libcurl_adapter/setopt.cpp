#include "url.h"
#include "http.h"
#include "utils.h"
#include "setopt.h"
#include "urldata.h"

#define CONTENT_ENCODING_DEFAULT  "identity"

CURLcode Curl_setstropt(char** charp, const char* s)
{
	/* Release the previous storage at `charp' and replace by a dynamic storage
	   copy of `s'. Return CURLE_OK or CURLE_OUT_OF_MEMORY. */
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

CURLcode Curl_setblobopt(struct curl_blob** blobp, const struct curl_blob* blob)
{
	/* free the previous storage at `blobp' and replace by a dynamic storage
	   copy of blob. If CURL_BLOB_COPY is set, the data is copied. */
	if (blob)
	{
		struct curl_blob* nblob;
		if (blob->len > CURL_MAX_INPUT_LENGTH)
		{
			return CURLE_BAD_FUNCTION_ARGUMENT;
		}
		nblob = (struct curl_blob*)malloc(sizeof(struct curl_blob) + ((blob->flags & CURL_BLOB_COPY) ? blob->len : 0));
		if (!nblob)
		{
			return CURLE_OUT_OF_MEMORY;
		}
		*nblob = *blob;
		if (blob->flags & CURL_BLOB_COPY)
		{
			/* put the data after the blob struct in memory */
			nblob->data = (char*)nblob + sizeof(struct curl_blob);
			memcpy(nblob->data, blob->data, blob->len);
		}
		*blobp = nblob;
		return CURLE_OK;
	}

	return CURLE_OK;
}

static CURLcode protocol2num(const char* str, curl_prot_t* val)
{
	/*
	 * We are asked to cherry-pick protocols, so play it safe and disallow all
	 * protocols to start with, and re-add the wanted ones back in.
	 */
	*val = 0;
	if (!str)
	{
		return CURLE_BAD_FUNCTION_ARGUMENT;
	}

	if (curl_strequal(str, "all"))
	{
		*val = ~(curl_prot_t)0;
		return CURLE_OK;
	}

	do
	{
		const char* token = str;
		size_t tlen;

		str = strchr(str, ',');
		tlen = str ? (size_t)(str - token) : strlen(token);
		if (tlen)
		{
			const struct Curl_handler* h = Curl_getn_scheme_handler(token, tlen);

			if (!h)
				return CURLE_UNSUPPORTED_PROTOCOL;

			*val |= h->protocol;
		}
	} while (str && str++);

	if (!*val)
	{
		/* no protocol listed */
		return CURLE_BAD_FUNCTION_ARGUMENT;
	}
	return CURLE_OK;
}

static CURLcode setstropt_userpwd(char* option, char** userp, char** passwdp)
{
	char* user = NULL;
	char* passwd = NULL;

	DEBUGASSERT(userp);
	DEBUGASSERT(passwdp);

	/* Parse the login details if specified. 
	It not then we treat NULL as a hint to clear the existing data */
	if (option)
	{
		size_t len = strlen(option);
		CURLcode result;
		if (len > CURL_MAX_INPUT_LENGTH)
			return CURLE_BAD_FUNCTION_ARGUMENT;

		result = Curl_parse_login_details(option, len, &user, &passwd, NULL);
		if (result)
			return result;
	}

	free(*userp);
	*userp = user;

	free(*passwdp);
	*passwdp = passwd;

	return CURLE_OK;
}

static CURLcode setstropt_interface(char* option, char** devp, char** ifacep, char** hostp)
{

}

static CURLcode setopt_long(struct Curl_easy* data, CURLoption option, long arg) 
{
	bool enabled = (0 != arg);
	unsigned long uarg = (unsigned long)arg;
	switch (option)
	{
		//case CURLOPT_DNS_CACHE_TIMEOUT:
		//	return CURLE_NOT_BUILT_IN;
		//case CURLOPT_CA_CACHE_TIMEOUT:
		//	return CURLE_NOT_BUILT_IN;
		case CURLOPT_MAXCONNECTS:
		{
			if (uarg > UINT_MAX)
			{
				return CURLE_BAD_FUNCTION_ARGUMENT;
			}
			data->set.maxconnects = (unsigned int)uarg;
			break;
		}
		case CURLOPT_FRESH_CONNECT:
		case CURLOPT_FORBID_REUSE:
			return CURLE_NOT_BUILT_IN;

		case CURLOPT_VERBOSE:
			return CURLE_NOT_BUILT_IN;
		case CURLOPT_HEADER:
		{
			/*
			* Set to include the header in the general data output stream.
			*/
			data->set.include_header = enabled;
			break;
		}
		case CURLOPT_NOPROGRESS:
			return CURLE_NOT_BUILT_IN;
		case CURLOPT_NOBODY:
		{
			/*
			* Do not include the body part in the output data stream.
			*/
			data->set.opt_no_body = enabled;
#ifndef CURL_DISABLE_HTTP
			if (data->set.opt_no_body)
			{
				/* in HTTP lingo, no body means using the HEAD request... */
				data->set.method = HTTPREQ_HEAD;
			}
			else if (data->set.method == HTTPREQ_HEAD)
			{
				data->set.method = HTTPREQ_GET;
			}
#endif
			break;
		}
		case CURLOPT_FAILONERROR:
			return CURLE_NOT_BUILT_IN;
		//case CURLOPT_KEEP_SENDING_ON_ERROR:
		//	return CURLE_NOT_BUILT_IN;
		case CURLOPT_UPLOAD:
		case CURLOPT_PUT:
		{
			/*
			 * We want to sent data to the remote host. If this is HTTP, that equals
			 * using the PUT request.
			 */
			if (arg)
			{
				/* If this is HTTP, PUT is what's needed to "upload" */
				data->set.method = HTTPREQ_PUT;
				data->set.opt_no_body = FALSE; /* this is implied */
			}
			else
			{
				/* In HTTP, the opposite of upload is GET
				 * (unless NOBODY is true as then this can be changed to HEAD later on)
				 */
				data->set.method = HTTPREQ_GET;
			}
			break;
		}
		case CURLOPT_FILETIME:
			return CURLE_NOT_BUILT_IN;
		//case CURLOPT_SERVER_RESPONSE_TIMEOUT:
		//	return CURLE_NOT_BUILT_IN;
		//case CURLOPT_SERVER_RESPONSE_TIMEOUT_MS:
		//	return CURLE_NOT_BUILT_IN;
#ifndef CURL_DISABLE_TFTP
//		case CURLOPT_TFTP_NO_OPTIONS:
//		case CURLOPT_TFTP_BLKSIZE:
//			return CURLE_NOT_BUILT_IN;
#endif
#ifndef CURL_DISABLE_NETRC
		//case CURLOPT_NETRC:
		//	return CURLE_NOT_BUILT_IN;
#endif
		case CURLOPT_TRANSFERTEXT:
			return CURLE_NOT_BUILT_IN;
		case CURLOPT_TIMECONDITION:
			return CURLE_NOT_BUILT_IN;
		case CURLOPT_TIMEVALUE:
			return CURLE_NOT_BUILT_IN;
#ifndef CURL_DISABLE_PROXY
		//case CURLOPT_SSLVERSION:
		//	return CURLE_NOT_BUILT_IN;
#endif
		//case CURLOPT_PROXY_SSLVERSION:
		//	return CURLE_NOT_BUILT_IN;
		case CURLOPT_POSTFIELDSIZE:
		{
			/*
			* The size of the POSTFIELD data to prevent libcurl to do strlen() to
			* figure it out. Enables binary posts.
			*/
			if (arg < -1)
			{
				return CURLE_BAD_FUNCTION_ARGUMENT;
			}
			if (data->set.postfieldsize < arg &&
				data->set.postfields == data->set.str[STRING_COPYPOSTFIELDS])
			{
				/* Previous CURLOPT_COPYPOSTFIELDS is no longer valid. */
				Curl_safefree(data->set.str[STRING_COPYPOSTFIELDS]);
				data->set.postfields = NULL;
			}
			data->set.postfieldsize = arg;
			break;
		}
#ifndef CURL_DISABLE_HTTP
#if !defined(CURL_DISABLE_COOKIES)
		//case CURLOPT_COOKIESESSION:
#endif
		case CURLOPT_AUTOREFERER:
			return CURLE_NOT_BUILT_IN;
		case CURLOPT_TRANSFER_ENCODING:
		{
			data->set.http_transfer_encoding = enabled;
			break;
		}
		case CURLOPT_FOLLOWLOCATION:
			return CURLE_NOT_BUILT_IN;
		//case CURLOPT_UNRESTRICTED_AUTH:
		//	return CURLE_NOT_BUILT_IN;
		case CURLOPT_MAXREDIRS:
			return CURLE_NOT_BUILT_IN;
		//case CURLOPT_POSTREDIR:
		//	return CURLE_NOT_BUILT_IN;
		case CURLOPT_POST:
		{
			/* Does this option serve a purpose anymore? Yes it does, when
			CURLOPT_POSTFIELDS is not used and the POST data is read off the
			callback! */
			if (arg)
			{
				data->set.method = HTTPREQ_POST;
				data->set.opt_no_body = FALSE; /* this is implied */
			}
			else
			{
				data->set.method = HTTPREQ_GET;
			}
			break;
		}
		case CURLOPT_HEADEROPT:
			return CURLE_NOT_BUILT_IN;
		case CURLOPT_HTTPAUTH:
			return CURLE_NOT_BUILT_IN;
		case CURLOPT_HTTPGET:
		{
			/* Set to force us do HTTP GET */
			if (enabled)
			{
				data->set.method = HTTPREQ_GET;
				data->set.opt_no_body = FALSE; /* this is implied */
			}
			break;
		}
		case CURLOPT_HTTP_VERSION:
		{
			/*
			* This sets a requested HTTP version to be used. The value is one of
			* the listed enums in curl/curl.h.
			*/
			switch (arg)
			{
				case CURL_HTTP_VERSION_NONE:
#ifdef USE_HTTP2
					/* TODO: this seems an undesirable quirk to force a behaviour on
					 * lower implementations that they should recognize independently? */
					arg = CURL_HTTP_VERSION_2TLS;
#endif
					/* accepted */
					break;
				case CURL_HTTP_VERSION_1_0:
				case CURL_HTTP_VERSION_1_1:
					/* accepted */
					break;
#ifdef USE_HTTP2
				case CURL_HTTP_VERSION_2_0:
				case CURL_HTTP_VERSION_2TLS:
				case CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE:
					/* accepted */
					break;
#endif
#ifdef USE_HTTP3
				case CURL_HTTP_VERSION_3:
				case CURL_HTTP_VERSION_3ONLY:
					/* accepted */
					break;
#endif
				default:
					/* not accepted */
					if (arg < CURL_HTTP_VERSION_NONE)
					{
						return CURLE_BAD_FUNCTION_ARGUMENT;
					}
					return CURLE_UNSUPPORTED_PROTOCOL;
			}
			data->set.httpwant = (unsigned char)arg;
			break;
		}
		//case CURLOPT_EXPECT_100_TIMEOUT_MS:
		//	return CURLE_NOT_BUILT_IN;
		//case CURLOPT_HTTP09_ALLOWED:
		//	return CURLE_NOT_BUILT_IN;
#endif /* ! CURL_DISABLE_HTTP */
#ifndef CURL_DISABLE_MIME
		//case CURLOPT_MIME_OPTIONS:
		//	return CURLE_NOT_BUILT_IN;
#endif
#ifndef CURL_DISABLE_PROXY
		case CURLOPT_HTTPPROXYTUNNEL:
		case CURLOPT_PROXYPORT:
		//case CURLOPT_PROXYAUTH:
		//case CURLOPT_PROXYTYPE:
		//case CURLOPT_PROXY_TRANSFER_MODE:
		//case CURLOPT_SOCKS5_AUTH:
		//case CURLOPT_HAPROXYPROTOCOL:
		//case CURLOPT_PROXY_SSL_VERIFYPEER:
		//case CURLOPT_PROXY_SSL_VERIFYHOST:
			return CURLE_NOT_BUILT_IN;
#endif /* ! CURL_DISABLE_PROXY */
#if defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)
		case CURLOPT_SOCKS5_GSSAPI_NEC:
			return CURLE_NOT_BUILT_IN;
#endif
#ifdef CURL_LIST_ONLY_PROTOCOL
		case CURLOPT_DIRLISTONLY:
			return CURLE_NOT_BUILT_IN;
#endif
		//case CURLOPT_APPEND:
		//	return CURLE_NOT_BUILT_IN;
#ifndef CURL_DISABLE_FTP
		case CURLOPT_FTP_FILEMETHOD:
			/*
			 * How do access files over FTP.
			 */
			if ((arg < CURLFTPMETHOD_DEFAULT) || (arg >= CURLFTPMETHOD_LAST)) {
				return CURLE_BAD_FUNCTION_ARGUMENT;
			}
			data->set.ftp_filemethod = (unsigned char)arg;
			break;
		case CURLOPT_FTP_USE_EPRT:
			data->set.ftp_use_eprt = enabled;
			break;
		case CURLOPT_FTP_USE_EPSV:
			data->set.ftp_use_epsv = enabled;
			break;
		case CURLOPT_FTP_USE_PRET:
			data->set.ftp_use_pret = enabled;
			break;
		case CURLOPT_FTP_SSL_CCC:
			return CURLE_NOT_BUILT_IN;
		case CURLOPT_FTP_SKIP_PASV_IP:
			data->set.ftp_skip_ip = enabled;
			break;
		case CURLOPT_FTPSSLAUTH:
			if ((arg < CURLFTPAUTH_DEFAULT) || (arg >= CURLFTPAUTH_LAST)) 
			{
				return CURLE_BAD_FUNCTION_ARGUMENT;
			}
			data->set.ftpsslauth = (unsigned char)(curl_ftpauth)arg;
			break;
		case CURLOPT_ACCEPTTIMEOUT_MS:
			if (uarg > UINT_MAX)
				uarg = UINT_MAX;
			data->set.accepttimeout = (unsigned int)uarg;
			break;
		case CURLOPT_WILDCARDMATCH:
			data->set.wildcard_enabled = enabled;
			break;
#endif /* ! CURL_DISABLE_FTP */
#if !defined(CURL_DISABLE_FTP) || defined(USE_SSH)
		case CURLOPT_FTP_CREATE_MISSING_DIRS:
			return CURLE_NOT_BUILT_IN;
#endif /* ! CURL_DISABLE_FTP || USE_SSH */
		case CURLOPT_INFILESIZE:
		{
			/*
			* If known, this should inform curl about the file size of the
			* to-be-uploaded file.
			*/
			if (arg < -1)
			{
				return CURLE_BAD_FUNCTION_ARGUMENT;
			}
			data->set.filesize = arg;
			break;
		}
		case CURLOPT_LOW_SPEED_LIMIT:
			return CURLE_NOT_BUILT_IN;
		case CURLOPT_LOW_SPEED_TIME:
			return CURLE_NOT_BUILT_IN;
		case CURLOPT_PORT:
		{
			/*
			 * The port number to use when getting the URL. 0 disables it.
			 */
			if ((arg < 0) || (arg > 65535))
			{
				return CURLE_BAD_FUNCTION_ARGUMENT;
			}
			data->set.use_port = (unsigned short)arg;
			break;
		}
		case CURLOPT_TIMEOUT:
		{
			//The maximum time you allow curl to use for a single transfer operation.
			if ((arg >= 0) && (arg <= (INT_MAX / 1000)))
			{
				data->set.timeout = (unsigned int)arg * 1000;
			}
			else
			{
				return CURLE_BAD_FUNCTION_ARGUMENT;
			}
			break;
		}
		case CURLOPT_TIMEOUT_MS:
		{
			if (uarg > UINT_MAX)
			{
				uarg = UINT_MAX;
			}
			data->set.timeout = (unsigned int)uarg;
			break;
		}
		case CURLOPT_CONNECTTIMEOUT:
		{
			/*
			 * The maximum time you allow curl to use to connect.
			 */
			if ((arg >= 0) && (arg <= (INT_MAX / 1000)))
			{
				data->set.connecttimeout = (unsigned int)arg * 1000;
			}
			else
			{
				return CURLE_BAD_FUNCTION_ARGUMENT;
			}
			break;
		}
		case CURLOPT_CONNECTTIMEOUT_MS:
		{
			if (uarg > UINT_MAX)
			{
				uarg = UINT_MAX;
			}
			data->set.connecttimeout = (unsigned int)uarg;
			break;
		}
		case CURLOPT_RESUME_FROM:
			return CURLE_NOT_BUILT_IN;
		//case CURLOPT_CRLF:
		//	return CURLE_NOT_BUILT_IN;
#ifndef CURL_DISABLE_BINDLOCAL
		//case CURLOPT_LOCALPORT:
		//	return CURLE_NOT_BUILT_IN;
		//case CURLOPT_LOCALPORTRANGE:
		//	return CURLE_NOT_BUILT_IN;
#endif
		//case CURLOPT_GSSAPI_DELEGATION:
		//	return CURLE_NOT_BUILT_IN;
#ifndef CURL_DISABLE_DOH
		case CURLOPT_SSL_VERIFYPEER:
		{
			/* Enable peer SSL verifying */
			data->set.using_ssl = enabled;
			break;
		}
		//case CURLOPT_DOH_SSL_VERIFYPEER:
		//case CURLOPT_DOH_SSL_VERIFYHOST:
		//case CURLOPT_DOH_SSL_VERIFYSTATUS:
		//	return CURLE_NOT_BUILT_IN;
#endif /* ! CURL_DISABLE_DOH */
		case CURLOPT_SSL_VERIFYHOST:
		{
			/* Enable verification of the hostname in the peer certificate */
			data->set.using_ssl = enabled;
			break;
		}
		//case CURLOPT_SSL_VERIFYSTATUS:
		//	return CURLE_NOT_BUILT_IN;
		//case CURLOPT_SSL_FALSESTART:
		//	return CURLE_NOT_BUILT_IN;
		//case CURLOPT_CERTINFO:
		//	return CURLE_NOT_BUILT_IN;
		case CURLOPT_BUFFERSIZE:
		{
			/*
			* The application kindly asks for a differently sized receive buffer.
			* If it seems reasonable, we will use it.
			*/
			if (arg > READBUFFER_MAX)
			{
				arg = READBUFFER_MAX;	//10MB
			}
			else if (arg < 1)
			{
				arg = READBUFFER_SIZE;	//10MB
			}
			else if (arg < READBUFFER_MIN)
			{
				arg = READBUFFER_MIN;
			}
			data->set.buffer_size = (unsigned int)arg;
			break;
		}
		case CURLOPT_UPLOAD_BUFFERSIZE:
		{
			/*
				* The application kindly asks for a differently sized upload buffer.
				* Cap it to sensible.
				*/
			if (arg > UPLOADBUFFER_MAX)
			{
				arg = UPLOADBUFFER_MAX;
			}
			else if (arg < UPLOADBUFFER_MIN)
			{
				arg = UPLOADBUFFER_MIN;
			}
			data->set.upload_buffer_size = (unsigned int)arg;
			break;
		}
		//case CURLOPT_NOSIGNAL:
		//	return CURLE_NOT_BUILT_IN;
		case CURLOPT_MAXFILESIZE:
			return CURLE_NOT_BUILT_IN;
#ifdef USE_SSL
		case CURLOPT_USE_SSL:
			return CURLE_NOT_BUILT_IN;
		case CURLOPT_SSL_OPTIONS:
			return CURLE_NOT_BUILT_IN;
#ifndef CURL_DISABLE_PROXY
		case CURLOPT_PROXY_SSL_OPTIONS:
			return CURLE_NOT_BUILT_IN;
#endif
#endif /* USE_SSL */
		//case CURLOPT_IPRESOLVE:
		//	return CURLE_NOT_BUILT_IN;
		//case CURLOPT_TCP_NODELAY:
		//	return CURLE_NOT_BUILT_IN;
		case CURLOPT_IGNORE_CONTENT_LENGTH:
		{
			data->set.ignorecl = enabled;
			break;
		}
		case CURLOPT_CONNECT_ONLY:
			return CURLE_NOT_BUILT_IN;
		//case CURLOPT_SSL_SESSIONID_CACHE:
		//	return CURLE_NOT_BUILT_IN;
#ifdef USE_SSH
		case CURLOPT_SSH_AUTH_TYPES:
		case CURLOPT_SSH_COMPRESSION:
			return CURLE_NOT_BUILT_IN;
#endif
		case CURLOPT_HTTP_TRANSFER_DECODING:
		{
			data->set.http_te_skip = !enabled;
			break;
		}
		case CURLOPT_HTTP_CONTENT_DECODING:
		{
			/*
			 * raw data passed to the application when content encoding is used
			 */
			data->set.http_ce_skip = enabled;
			break;
		}
#if !defined(CURL_DISABLE_FTP) || defined(USE_SSH)
		case CURLOPT_NEW_FILE_PERMS:
			return CURLE_NOT_BUILT_IN;
#endif
#ifdef USE_SSH
		case CURLOPT_NEW_DIRECTORY_PERMS:
			return CURLE_NOT_BUILT_IN;
#endif
#ifdef USE_IPV6
		case CURLOPT_ADDRESS_SCOPE:
			return CURLE_NOT_BUILT_IN;
#endif
		//case CURLOPT_PROTOCOLS:
		//case CURLOPT_REDIR_PROTOCOLS:
		//	return CURLE_NOT_BUILT_IN;

#ifndef CURL_DISABLE_SMTP
		/*case CURLOPT_MAIL_RCPT_ALLOWFAILS:
			return CURLE_NOT_BUILT_IN;*/
#endif
		/*case CURLOPT_SASL_IR:
			return CURLE_NOT_BUILT_IN;*/
#ifndef CURL_DISABLE_RTSP
		/*case CURLOPT_RTSP_REQUEST:
		case CURLOPT_RTSP_CLIENT_CSEQ:
		case CURLOPT_RTSP_SERVER_CSEQ:
			return CURLE_NOT_BUILT_IN;*/
#endif
		/*case CURLOPT_TCP_KEEPALIVE:
		case CURLOPT_TCP_KEEPIDLE:
		case CURLOPT_TCP_KEEPINTVL:
		case CURLOPT_TCP_KEEPCNT:
		case CURLOPT_TCP_FASTOPEN:
		case CURLOPT_SSL_ENABLE_NPN:
		case CURLOPT_SSL_ENABLE_ALPN:*/
		case CURLOPT_PATH_AS_IS:
		/*case CURLOPT_PIPEWAIT:
		case CURLOPT_STREAM_WEIGHT:
		case CURLOPT_SUPPRESS_CONNECT_HEADERS:
		case CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS:*/
#ifndef CURL_DISABLE_SHUFFLE_DNS
		//case CURLOPT_DNS_SHUFFLE_ADDRESSES:
#endif
		//case CURLOPT_UPKEEP_INTERVAL_MS:
		//case CURLOPT_MAXAGE_CONN:
		//case CURLOPT_MAXLIFETIME_CONN:
#ifndef CURL_DISABLE_HSTS
		//case CURLOPT_HSTS_CTRL:
#endif
#ifndef CURL_DISABLE_ALTSVC
		//case CURLOPT_ALTSVC_CTRL:
#endif
#ifndef CURL_DISABLE_WEBSOCKETS
		case CURLOPT_WS_OPTIONS:
#endif
		//case CURLOPT_QUICK_EXIT:
		//case CURLOPT_DNS_USE_GLOBAL_CACHE:
		//case CURLOPT_SSLENGINE_DEFAULT:
		//	return CURLE_NOT_BUILT_IN;
		default:
			return CURLE_UNKNOWN_OPTION;
	}
	return CURLE_OK;
}

static CURLcode setopt_slist(struct Curl_easy* data, CURLoption option, struct curl_slist* slist)
{
	CURLcode result = CURLE_OK;
	switch (option)
	{
#ifndef CURL_DISABLE_PROXY
		//case CURLOPT_PROXYHEADER:
		//	return CURLE_NOT_BUILT_IN;
#endif
#ifndef CURL_DISABLE_HTTP
		case CURLOPT_HTTP200ALIASES:
			return CURLE_NOT_BUILT_IN;
#endif
#if !defined(CURL_DISABLE_FTP) || defined(USE_SSH)
		case CURLOPT_POSTQUOTE:
			data->set.postquote = slist;
			break;
		case CURLOPT_PREQUOTE:
			data->set.prequote = slist;
			break;
		case CURLOPT_QUOTE:
			data->set.quote = slist;
			break;
#endif
		//case CURLOPT_RESOLVE:
		//	/*
		//	 * List of HOST:PORT:[addresses] strings to populate the DNS cache with
		//	 * Entries added this way will remain in the cache until explicitly
		//	 * removed or the handle is cleaned up.
		//	 *
		//	 * Prefix the HOST with plus sign (+) to have the entry expire just like
		//	 * automatically added entries.
		//	 *
		//	 * Prefix the HOST with dash (-) to _remove_ the entry from the cache.
		//	 *
		//	 * This API can remove any entry from the DNS cache, but only entries
		//	 * that are not actually in use right now will be pruned immediately.
		//	 */
		//	data->set.resolve = slist;
		//	data->state.resolve = data->set.resolve;
		//	break;
#if !defined(CURL_DISABLE_HTTP) || !defined(CURL_DISABLE_MIME)
		//case CURLOPT_HTTPHEADER:
		//	/*
		//	* Set a list with HTTP headers to use (or replace internals with)
		//	*/
		//	data->set.headers = slist;
		//	break;
#endif
#ifndef CURL_DISABLE_TELNET
		//case CURLOPT_TELNETOPTIONS:
		//	return CURLE_NOT_BUILT_IN;
#endif
#ifndef CURL_DISABLE_SMTP
		//case CURLOPT_MAIL_RCPT:
		//	return CURLE_NOT_BUILT_IN;
#endif
		//case CURLOPT_CONNECT_TO:
		//	data->set.connect_to = slist;
		//	break;
		default:
			return CURLE_UNKNOWN_OPTION;
	}
	return result;
}

static CURLcode setopt_pointers(struct Curl_easy* data, CURLoption option, va_list param)
{
	CURLcode result = CURLE_OK;
	switch (option)
	{
#ifndef CURL_DISABLE_HTTP
#ifndef CURL_DISABLE_FORM_API
		case CURLOPT_HTTPPOST:
			/*
			 * Set to make us do HTTP POST. Legacy API-style.
			 */
			//data->set.httppost = va_arg(param, struct curl_httppost*);
			//data->set.method = HTTPREQ_POST_FORM;
			//data->set.opt_no_body = FALSE; /* this is implied */
			//break;

#endif /* ! CURL_DISABLE_FORM_API */
#endif /* ! CURL_DISABLE_HTTP */
#if !defined(CURL_DISABLE_HTTP) || !defined(CURL_DISABLE_SMTP) || !defined(CURL_DISABLE_IMAP)
# ifndef CURL_DISABLE_MIME
		//case CURLOPT_MIMEPOST:
		//	return CURLE_NOT_BUILT_IN;
#endif /* ! CURL_DISABLE_MIME */
#endif /* ! disabled HTTP, SMTP or IMAP */
		case CURLOPT_STDERR:
			/*
			 * Set to a FILE * that should receive all error writes. This
			 * defaults to stderr for normal operations.
			 */
			data->set.err = va_arg(param, FILE*);
			if (!data->set.err)
			{
				data->set.err = stderr;
			}
			break;
		//case CURLOPT_SHARE:
		//{
		//	return CURLE_NOT_BUILT_IN;
		//}
#ifdef USE_HTTP2
		case CURLOPT_STREAM_DEPENDS:
		case CURLOPT_STREAM_DEPENDS_E:
		{
			struct Curl_easy* dep = va_arg(param, struct Curl_easy*);
			if (!dep || GOOD_EASY_HANDLE(dep))
			{
				return Curl_data_priority_add_child(dep, data, option == CURLOPT_STREAM_DEPENDS_E);
			}
			break;
		}
#endif
		default:
			return CURLE_UNKNOWN_OPTION;
	}
	return result;
}

static CURLcode setopt_cptr(struct Curl_easy* data, CURLoption option, char* ptr)
{
	CURLcode result = CURLE_OK;
	switch (option)
	{
		//case CURLOPT_SSL_CIPHER_LIST:
		//	return CURLE_NOT_BUILT_IN;
#ifndef CURL_DISABLE_PROXY
		//case CURLOPT_PROXY_SSL_CIPHER_LIST:
		//	return CURLE_NOT_BUILT_IN;
#endif
		//case CURLOPT_TLS13_CIPHERS:
		//	return CURLE_NOT_BUILT_IN;
#ifndef CURL_DISABLE_PROXY
		//case CURLOPT_PROXY_TLS13_CIPHERS:
		//	return CURLE_NOT_BUILT_IN;
#endif
		//case CURLOPT_RANDOM_FILE:
		//case CURLOPT_EGDSOCKET:
		//case CURLOPT_REQUEST_TARGET:
		//	return CURLE_NOT_BUILT_IN;
#ifndef CURL_DISABLE_NETRC
		//case CURLOPT_NETRC_FILE:
		//	return CURLE_NOT_BUILT_IN;
#endif

#if !defined(CURL_DISABLE_HTTP) || !defined(CURL_DISABLE_MQTT)
		//case CURLOPT_COPYPOSTFIELDS:
		//	return CURLE_NOT_BUILT_IN;

		case CURLOPT_POSTFIELDS:
		{
			/*
			 * Like above, but use static data instead of copying it.
			 */
			data->set.postfields = ptr;
			/* Release old copied data. */
			Curl_safefree(data->set.str[STRING_COPYPOSTFIELDS]);
			data->set.method = HTTPREQ_POST;
			break;
		}
#ifndef CURL_DISABLE_HTTP
		case CURLOPT_ACCEPT_ENCODING:
		{
			if (ptr && !*ptr)
			{
				return Curl_setstropt(&data->set.str[STRING_ENCODING], CONTENT_ENCODING_DEFAULT);
			}
			return Curl_setstropt(&data->set.str[STRING_ENCODING], ptr);
		}

#if !defined(CURL_DISABLE_AWS)
		//case CURLOPT_AWS_SIGV4:
		//	return CURLE_NOT_BUILT_IN;
#endif
		case CURLOPT_REFERER:
			return CURLE_NOT_BUILT_IN;

		case CURLOPT_USERAGENT:
		{
			/*
			 * String to use in the HTTP User-Agent field
			 */
			return Curl_setstropt(&data->set.str[STRING_USERAGENT], ptr);
		}
#if !defined(CURL_DISABLE_COOKIES)
		/*case CURLOPT_COOKIE:
			return CURLE_NOT_BUILT_IN;

		case CURLOPT_COOKIEFILE:
			return CURLE_NOT_BUILT_IN;

		case CURLOPT_COOKIEJAR:
			return CURLE_NOT_BUILT_IN;

		case CURLOPT_COOKIELIST:
			return CURLE_NOT_BUILT_IN;*/

#endif /* !CURL_DISABLE_COOKIES */

#endif /* ! CURL_DISABLE_HTTP */

		case CURLOPT_CUSTOMREQUEST:
			return CURLE_NOT_BUILT_IN;

#ifndef CURL_DISABLE_PROXY
		case CURLOPT_PROXY:
			return CURLE_NOT_BUILT_IN;

		//case CURLOPT_PRE_PROXY:
		//	return CURLE_NOT_BUILT_IN;
#endif   /* CURL_DISABLE_PROXY */

#ifndef CURL_DISABLE_PROXY
		//case CURLOPT_SOCKS5_GSSAPI_SERVICE:
		//case CURLOPT_PROXY_SERVICE_NAME:
		//	return CURLE_NOT_BUILT_IN;
#endif
		//case CURLOPT_SERVICE_NAME:
		//	return CURLE_NOT_BUILT_IN;

		case CURLOPT_HEADERDATA:
			/*
			* Custom pointer to pass the header write callback function
			*/
			data->set.header_get = (void*)ptr;
			break;

		case CURLOPT_READDATA:
			/*
			 * FILE pointer to read the file to be uploaded from. Or possibly used as
			 * argument to the read callback.
			 */
			data->set.in_set = (void*)ptr;
			break;
		case CURLOPT_WRITEDATA:
			/*
			 * FILE pointer to write to. Or possibly used as argument to the write
			 * callback.
			 */
			data->set.out_get = (void*)ptr;
			break;
//		case CURLOPT_DEBUGDATA:
//			return CURLE_NOT_BUILT_IN;
//
//		case CURLOPT_PROGRESSDATA:
//			return CURLE_NOT_BUILT_IN;
//
//		case CURLOPT_SEEKDATA:
//			return CURLE_NOT_BUILT_IN;
//
//		case CURLOPT_IOCTLDATA:
//			return CURLE_NOT_BUILT_IN;
//
//		case CURLOPT_SSL_CTX_DATA:
//			return CURLE_NOT_BUILT_IN;
//
//		case CURLOPT_SOCKOPTDATA:
//			return CURLE_NOT_BUILT_IN;
//
//		case CURLOPT_OPENSOCKETDATA:
//			return CURLE_NOT_BUILT_IN;
//
//		case CURLOPT_RESOLVER_START_DATA:
//			return CURLE_NOT_BUILT_IN;
//
//		case CURLOPT_CLOSESOCKETDATA:
//			return CURLE_NOT_BUILT_IN;
//
//		case CURLOPT_TRAILERDATA:
#ifndef CURL_DISABLE_HTTP
//			data->set.trailer_data = (void*)ptr;
#endif
//			return CURLE_NOT_BUILT_IN;
//		case CURLOPT_PREREQDATA:
//			return CURLE_NOT_BUILT_IN;

		case CURLOPT_ERRORBUFFER:
			return CURLE_NOT_BUILT_IN;

#ifndef CURL_DISABLE_FTP
		case CURLOPT_FTPPORT:
			/*
			 * Use FTP PORT, this also specifies which IP address to use
			 */
			result = Curl_setstropt(&data->set.str[STRING_FTPPORT], ptr);
			data->set.ftp_use_port = !!(data->set.str[STRING_FTPPORT]);
			break;

		case CURLOPT_FTP_ACCOUNT:
			return Curl_setstropt(&data->set.str[STRING_FTP_ACCOUNT], ptr);

		case CURLOPT_FTP_ALTERNATIVE_TO_USER:
			return Curl_setstropt(&data->set.str[STRING_FTP_ALTERNATIVE_TO_USER], ptr);

#ifdef HAVE_GSSAPI
		case CURLOPT_KRBLEVEL:
			return CURLE_NOT_BUILT_IN;
#endif
#endif
		case CURLOPT_URL:
		{
			if (data->state.url_alloc)
			{
				/* the already set URL is allocated, free it first! */
				Curl_safefree(data->state.url);
				data->state.url_alloc = FALSE;
			}
			result = Curl_setstropt(&data->set.str[STRING_SET_URL], ptr);
			data->state.url = data->set.str[STRING_SET_URL];
			break;
		}
		case CURLOPT_USERPWD:
			/*
			 * user:password to use in the operation
			 */
			return setstropt_userpwd(ptr, &data->set.str[STRING_USERNAME], &data->set.str[STRING_PASSWORD]);

		//case CURLOPT_USERNAME:
		//	/*
		//	 * authentication username to use in the operation
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_USERNAME], ptr);

		//case CURLOPT_PASSWORD:
		//	/*
		//	 * authentication password to use in the operation
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_PASSWORD], ptr);

		//case CURLOPT_LOGIN_OPTIONS:
		//	/*
		//	 * authentication options to use in the operation
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_OPTIONS], ptr);

		//case CURLOPT_XOAUTH2_BEARER:
		//	return CURLE_NOT_BUILT_IN;

#ifndef CURL_DISABLE_PROXY
		case CURLOPT_PROXYUSERPWD:
			return CURLE_NOT_BUILT_IN;
		//case CURLOPT_PROXYUSERNAME:
		//	/*
		//	 * authentication username to use in the operation
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_PROXYUSERNAME], ptr);

		//case CURLOPT_PROXYPASSWORD:
		//	/*
		//	 * authentication password to use in the operation
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_PROXYPASSWORD], ptr);

		//case CURLOPT_NOPROXY:
		//	/*
		//	 * proxy exception list
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_NOPROXY], ptr);
#endif

		case CURLOPT_RANGE:
			/*
			 * What range of the file you want to transfer
			 */
			return Curl_setstropt(&data->set.str[STRING_SET_RANGE], ptr);

#endif /* ! CURL_DISABLE_PROXY */
		//case CURLOPT_CURLU:
		//{
		//	/*
		//	 * pass CURLU to set URL
		//	 */
		//	data->set.uh = (CURLU*)ptr;
		//	break;
		//}
		//case CURLOPT_SSLCERT:
		//	/*
		//	 * String that holds filename of the SSL certificate to use
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_CERT], ptr);

#ifndef CURL_DISABLE_PROXY
		//case CURLOPT_PROXY_SSLCERT:
		//	/*
		//	 * String that holds filename of the SSL certificate to use for proxy
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_CERT_PROXY], ptr);

#endif
		//case CURLOPT_SSLCERTTYPE:
		//	/*
		//	 * String that holds file type of the SSL certificate to use
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_CERT_TYPE], ptr);

#ifndef CURL_DISABLE_PROXY
		//case CURLOPT_PROXY_SSLCERTTYPE:
		//	/*
		//	 * String that holds file type of the SSL certificate to use for proxy
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_CERT_TYPE_PROXY], ptr);
#endif
		//case CURLOPT_SSLKEY:
		//	/*
		//	 * String that holds filename of the SSL key to use
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_KEY], ptr);

#ifndef CURL_DISABLE_PROXY
		//case CURLOPT_PROXY_SSLKEY:
		//	/*
		//	 * String that holds filename of the SSL key to use for proxy
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_KEY_PROXY], ptr);

#endif
		//case CURLOPT_SSLKEYTYPE:
		//	/*
		//	 * String that holds file type of the SSL key to use
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_KEY_TYPE], ptr);
#ifndef CURL_DISABLE_PROXY
		//case CURLOPT_PROXY_SSLKEYTYPE:
		//	/*
		//	 * String that holds file type of the SSL key to use for proxy
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_KEY_TYPE_PROXY], ptr);

#endif
		//case CURLOPT_KEYPASSWD:
		//	/*
		//	 * String that holds the SSL or SSH private key password.
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_KEY_PASSWD], ptr);

#ifndef CURL_DISABLE_PROXY
		//case CURLOPT_PROXY_KEYPASSWD:
		//	/*
		//	 * String that holds the SSL private key password for proxy.
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_KEY_PASSWD_PROXY], ptr);
#endif
		/*case CURLOPT_SSLENGINE:
			return CURLE_NOT_BUILT_IN;*/
#ifndef CURL_DISABLE_PROXY
		/*case CURLOPT_HAPROXY_CLIENT_IP:
			return CURLE_NOT_BUILT_IN;*/
#endif
		/*case CURLOPT_INTERFACE:
			return CURLE_NOT_BUILT_IN;

		case CURLOPT_PINNEDPUBLICKEY:
			return CURLE_NOT_BUILT_IN;*/

#ifndef CURL_DISABLE_PROXY
		/*case CURLOPT_PROXY_PINNEDPUBLICKEY:
			return CURLE_NOT_BUILT_IN;*/
#endif
		//case CURLOPT_CAINFO:
		//	/*
		//	 * Set CA info for SSL connection. Specify filename of the CA certificate
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_SSL_CAFILE], ptr);

#ifndef CURL_DISABLE_PROXY
		/*case CURLOPT_PROXY_CAINFO:
			return CURLE_NOT_BUILT_IN;*/
#endif

		/*case CURLOPT_CAPATH:
			return CURLE_NOT_BUILT_IN;*/
#ifndef CURL_DISABLE_PROXY
		/*case CURLOPT_PROXY_CAPATH:
			return CURLE_NOT_BUILT_IN;*/
#endif
		//case CURLOPT_CRLFILE:
		//	/*
		//	 * Set CRL file info for SSL connection. Specify filename of the CRL
		//	 * to check certificates revocation
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_SSL_CRLFILE], ptr);

#ifndef CURL_DISABLE_PROXY
		//case CURLOPT_PROXY_CRLFILE:
		//	/*
		//	 * Set CRL file info for SSL connection for proxy. Specify filename of the
		//	 * CRL to check certificates revocation
		//	 */
		//	return Curl_setstropt(&data->set.str[STRING_SSL_CRLFILE_PROXY], ptr);
#endif
		/*case CURLOPT_ISSUERCERT:
			return CURLE_NOT_BUILT_IN;*/
#ifndef CURL_DISABLE_PROXY
		/*case CURLOPT_PROXY_ISSUERCERT:
			return CURLE_NOT_BUILT_IN;*/
#endif
		case CURLOPT_PRIVATE:
			/*
			 * Set private data pointer.
			 */
			data->set.private_data = (void*)ptr;
			break;

#ifdef USE_SSL
		case CURLOPT_SSL_EC_CURVES:
			return CURLE_NOT_BUILT_IN;
#endif
#ifdef USE_SSH
		case CURLOPT_SSH_PUBLIC_KEYFILE:
		case CURLOPT_SSH_PRIVATE_KEYFILE:
		case CURLOPT_SSH_HOST_PUBLIC_KEY_MD5:
		case CURLOPT_SSH_KNOWNHOSTS:
		case CURLOPT_SSH_KEYDATA:
			return CURLE_NOT_BUILT_IN;
#ifdef USE_LIBSSH2
		case CURLOPT_SSH_HOST_PUBLIC_KEY_SHA256:
		case CURLOPT_SSH_HOSTKEYDATA:
			return CURLE_NOT_BUILT_IN;
#endif /* USE_LIBSSH2 */
#endif /* USE_SSH */
		case CURLOPT_PROTOCOLS_STR:
		{
			if (ptr) {
				return protocol2num(ptr, &data->set.allowed_protocols);
			}
			/* make a NULL argument reset to default */
			data->set.allowed_protocols = (curl_prot_t)CURLPROTO_ALL;
			break;
		}
		case CURLOPT_REDIR_PROTOCOLS_STR:
			return CURLE_NOT_BUILT_IN;

		case CURLOPT_DEFAULT_PROTOCOL:
			/* Set the protocol to use when the URL does not include any protocol */
			return Curl_setstropt(&data->set.str[STRING_DEFAULT_PROTOCOL], ptr);

#ifndef CURL_DISABLE_SMTP
		//case CURLOPT_MAIL_FROM:
		//case CURLOPT_MAIL_AUTH:
#endif
		/*case CURLOPT_SASL_AUTHZID:
			return CURLE_NOT_BUILT_IN;*/

#ifndef CURL_DISABLE_RTSP
		//case CURLOPT_RTSP_SESSION_ID:
		//case CURLOPT_RTSP_STREAM_URI:
		//case CURLOPT_RTSP_TRANSPORT:
		//case CURLOPT_INTERLEAVEDATA:
		//	return CURLE_NOT_BUILT_IN;
#endif /* ! CURL_DISABLE_RTSP */
#ifndef CURL_DISABLE_FTP
		case CURLOPT_CHUNK_DATA:
			data->set.wildcardptr = (void*)ptr;
			break;
		case CURLOPT_FNMATCH_DATA:
			data->set.fnmatch_data = (void*)ptr;
			break;
#endif
#ifdef USE_TLS_SRP
		case CURLOPT_TLSAUTH_USERNAME:
			return CURLE_NOT_BUILT_IN;
#ifndef CURL_DISABLE_PROXY
		case CURLOPT_PROXY_TLSAUTH_USERNAME:
			return CURLE_NOT_BUILT_IN;
#endif
		case CURLOPT_TLSAUTH_PASSWORD:
			return CURLE_NOT_BUILT_IN;
#ifndef CURL_DISABLE_PROXY
		case CURLOPT_PROXY_TLSAUTH_PASSWORD:
			return CURLE_NOT_BUILT_IN;
#endif
		case CURLOPT_TLSAUTH_TYPE:
			return CURLE_NOT_BUILT_IN;
#ifndef CURL_DISABLE_PROXY
		case CURLOPT_PROXY_TLSAUTH_TYPE:
			return CURLE_NOT_BUILT_IN;

#endif
#endif
#ifdef USE_ARES
		case CURLOPT_DNS_SERVERS:
		case CURLOPT_DNS_INTERFACE:
		case CURLOPT_DNS_LOCAL_IP4:
		case CURLOPT_DNS_LOCAL_IP6:
			return CURLE_NOT_BUILT_IN;
#endif
#ifdef USE_UNIX_SOCKETS
		case CURLOPT_UNIX_SOCKET_PATH:
		case CURLOPT_ABSTRACT_UNIX_SOCKET:
			return CURLE_NOT_BUILT_IN;
#endif

#ifndef CURL_DISABLE_DOH
		//case CURLOPT_DOH_URL:
		//	return CURLE_NOT_BUILT_IN;
#endif
#ifndef CURL_DISABLE_HSTS
		//case CURLOPT_HSTSREADDATA:
		//case CURLOPT_HSTSWRITEDATA:
		//case CURLOPT_HSTS:
		//	return CURLE_NOT_BUILT_IN;
#endif /* ! CURL_DISABLE_HSTS */
#ifndef CURL_DISABLE_ALTSVC
		//case CURLOPT_ALTSVC:
		//	return CURLE_NOT_BUILT_IN;
#endif /* ! CURL_DISABLE_ALTSVC */
#ifdef USE_ECH
		case CURLOPT_ECH:
			return CURLE_NOT_BUILT_IN;
#endif
		default:
			return CURLE_UNKNOWN_OPTION;
	}
	return result;
}

static CURLcode setopt_func(struct Curl_easy* data, CURLoption option, va_list param)
{
	switch (option)
	{
		/*case CURLOPT_PROGRESSFUNCTION:
			return CURLE_NOT_BUILT_IN;
		case CURLOPT_XFERINFOFUNCTION:
			return CURLE_NOT_BUILT_IN;
		case CURLOPT_DEBUGFUNCTION:
			return CURLE_NOT_BUILT_IN;*/
		case CURLOPT_HEADERFUNCTION:
			/*
			 * Set header write callback
			 */
			data->set.fwrite_header = va_arg(param, curl_write_callback);
			break;
		case CURLOPT_WRITEFUNCTION:
			/*
			 * Set data write callback
			 */
			data->set.fwrite_func = va_arg(param, curl_write_callback);
			if (!data->set.fwrite_func)
			{
				/* When set to NULL, reset to our internal default function */
				data->set.fwrite_func = (curl_write_callback)fwrite;
			}
			break;
		case CURLOPT_READFUNCTION:
			/*
			 * Read data callback
			 */
			data->set.fread_func_set = va_arg(param, curl_read_callback);
			if (!data->set.fread_func_set)
			{
				data->set.is_fread_set = 0;
				/* When set to NULL, reset to our internal default function */
				data->set.fread_func_set = (curl_read_callback)fread;
			}
			else
				data->set.is_fread_set = 1;
			break;
		case CURLOPT_SEEKFUNCTION:
			/*
			 * Seek callback. Might be NULL.
			 */
			data->set.seek_func = va_arg(param, curl_seek_callback);
			break;
		//case CURLOPT_IOCTLFUNCTION:
		//case CURLOPT_SSL_CTX_FUNCTION:
		//case CURLOPT_SOCKOPTFUNCTION:
		//case CURLOPT_OPENSOCKETFUNCTION:
		//case CURLOPT_CLOSESOCKETFUNCTION:
		//case CURLOPT_RESOLVER_START_FUNCTION:
			return CURLE_NOT_BUILT_IN;
#ifdef USE_SSH
#ifdef USE_LIBSSH2
		case CURLOPT_SSH_HOSTKEYFUNCTION:
			return CURLE_NOT_BUILT_IN;
#endif
		case CURLOPT_SSH_KEYFUNCTION:
			return CURLE_NOT_BUILT_IN;

#endif /* USE_SSH */

#ifndef CURL_DISABLE_RTSP
		//case CURLOPT_INTERLEAVEFUNCTION:
		//	return CURLE_NOT_BUILT_IN;
#endif
#ifndef CURL_DISABLE_FTP
		case CURLOPT_CHUNK_BGN_FUNCTION:
		case CURLOPT_CHUNK_END_FUNCTION:
		case CURLOPT_FNMATCH_FUNCTION:
			return CURLE_NOT_BUILT_IN;
#endif
#ifndef CURL_DISABLE_HTTP
		//case CURLOPT_TRAILERFUNCTION:
		//	return CURLE_NOT_BUILT_IN;
#endif
#ifndef CURL_DISABLE_HSTS
		//case CURLOPT_HSTSREADFUNCTION:
		//case CURLOPT_HSTSWRITEFUNCTION:
		//	return CURLE_NOT_BUILT_IN;
#endif
		//case CURLOPT_PREREQFUNCTION:
		//	return CURLE_NOT_BUILT_IN;
		default:
			return CURLE_UNKNOWN_OPTION;
	}
	return CURLE_OK;
}

static CURLcode setopt_offt(struct Curl_easy* data, CURLoption option, curl_off_t offt)
{
	switch (option)
	{
		//case CURLOPT_TIMEVALUE_LARGE:
		//	return CURLE_NOT_BUILT_IN;
		case CURLOPT_POSTFIELDSIZE_LARGE:
		{
			/*
			 * The size of the POSTFIELD data to prevent libcurl to do strlen() to
			 * figure it out. Enables binary posts.
			 */
			if (offt < -1)
			{
				return CURLE_BAD_FUNCTION_ARGUMENT;
			}
			if (data->set.postfieldsize < offt && data->set.postfields == data->set.str[STRING_COPYPOSTFIELDS])
			{
				/* Previous CURLOPT_COPYPOSTFIELDS is no longer valid. */
				Curl_safefree(data->set.str[STRING_COPYPOSTFIELDS]);
				data->set.postfields = NULL;
			}
			data->set.postfieldsize = offt;
			break;
		}
		case CURLOPT_INFILESIZE_LARGE:
		{
			/*
			 * If known, this should inform curl about the file size of the
			 * to-be-uploaded file.
			 */
			if (offt < -1)
			{
				return CURLE_BAD_FUNCTION_ARGUMENT;
			}
			data->set.filesize = offt;
			break;
		}
		//case CURLOPT_MAX_SEND_SPEED_LARGE:
		//case CURLOPT_MAX_RECV_SPEED_LARGE:
		//case CURLOPT_RESUME_FROM_LARGE:
		//	return CURLE_NOT_BUILT_IN;
		case CURLOPT_MAXFILESIZE_LARGE:
		{
			/*
			 * Set the maximum size of a file to download.
			 */
			if (offt < 0) 
			{
				return CURLE_BAD_FUNCTION_ARGUMENT;
			}
			data->set.max_filesize = offt;
			break;
		}
		default:
			return CURLE_UNKNOWN_OPTION;
	}
	return CURLE_OK;
}

static CURLcode setopt_blob(struct Curl_easy* data, CURLoption option, struct curl_blob* blob)
{
	switch (option)
	{
		//case CURLOPT_SSLCERT_BLOB:
		//	return CURLE_NOT_BUILT_IN;
#ifndef CURL_DISABLE_PROXY
		//case CURLOPT_PROXY_SSLCERT_BLOB:
		//case CURLOPT_PROXY_SSLKEY_BLOB:
		//case CURLOPT_PROXY_CAINFO_BLOB:
		//case CURLOPT_PROXY_ISSUERCERT_BLOB:
		//	return CURLE_NOT_BUILT_IN;
#endif
		//case CURLOPT_SSLKEY_BLOB:
		//case CURLOPT_CAINFO_BLOB:
		//	return CURLE_NOT_BUILT_IN;
		//case CURLOPT_ISSUERCERT_BLOB:
		//	return CURLE_NOT_BUILT_IN;
		default:
			return CURLE_UNKNOWN_OPTION;
	}
}

CURLcode Curl_vsetopt(struct Curl_easy* data, CURLoption option, va_list param)
{
	if (option < CURLOPTTYPE_OBJECTPOINT)
	{
		return setopt_long(data, option, va_arg(param, long));
	}
	else if (option < CURLOPTTYPE_FUNCTIONPOINT)
	{
		/* unfortunately, different pointer types cannot be identified any other
		   way than being listed explicitly */
		switch (option)
		{
			case CURLOPT_HTTPHEADER:
			//case CURLOPT_QUOTE:
			//case CURLOPT_POSTQUOTE:
			//case CURLOPT_TELNETOPTIONS:
			//case CURLOPT_PREQUOTE:
			case CURLOPT_HTTP200ALIASES:
			//case CURLOPT_MAIL_RCPT:
			//case CURLOPT_RESOLVE:
			//case CURLOPT_PROXYHEADER:
			//case CURLOPT_CONNECT_TO:
				return setopt_slist(data, option, va_arg(param, struct curl_slist*));
			case CURLOPT_HTTPPOST:         /* curl_httppost * */
			//case CURLOPT_MIMEPOST:         /* curl_mime * */
			case CURLOPT_STDERR:           /* FILE * */
			//case CURLOPT_SHARE:            /* CURLSH * */
			//case CURLOPT_STREAM_DEPENDS:   /* CURL * */
			//case CURLOPT_STREAM_DEPENDS_E: /* CURL * */
				return setopt_pointers(data, option, param);
			default:
				break;
		}
		/* the char pointer options */
		return setopt_cptr(data, option, va_arg(param, char*));
	}
	else if (option < CURLOPTTYPE_OFF_T)
	{
		return setopt_func(data, option, param);
	}
	else if (option < CURLOPTTYPE_BLOB)
	{
		return setopt_offt(data, option, va_arg(param, curl_off_t));
	}
	return setopt_blob(data, option, va_arg(param, struct curl_blob*));
}
