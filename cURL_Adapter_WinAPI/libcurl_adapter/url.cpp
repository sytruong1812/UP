#include "url.h"
#include "http.h"
#include "ws.h"
#include "setopt.h"

void Curl_freeset(struct Curl_easy* data)
{
	/* Free all dynamic strings stored in the data->set substructure. */
	for (int i = 0; i < STRING_LAST; i++)
	{
		Curl_safefree(data->set.str[i]);
	}
	for (int j = 0; j < BLOB_LAST; j++)
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

CURLcode Curl_init_do(struct Curl_easy* data, struct connectdata* conn)
{
	if (conn)
	{
		conn->bits.do_more = FALSE; /* by default there is no curl_do_more() to use */
		if (data->state.wildcardmatch && !(conn->handler->flags & PROTOPT_WILDCARD))
		{
			data->state.wildcardmatch = FALSE;	/* if the protocol used does not support wildcards, switch it off */
		}
	}
	data->state.done = FALSE; /* *_done() is not called yet */
	if (data->req.no_body)
	{
		data->state.httpreq = HTTPREQ_HEAD;		/* in HTTP lingo, no body means using the HEAD request... */
	}

	return Curl_req_start(&data->req, data);
}

CURLcode Curl_open(struct Curl_easy** curl)
{
	CURLcode result;
	struct Curl_easy* data = (struct Curl_easy*)calloc(1, sizeof(struct Curl_easy));
	if (!data)
	{
		DEBUGF(fprintf(stderr, "Error: calloc of Curl_easy failed\n"));
		return CURLE_OUT_OF_MEMORY;
	}
	data->magic = CURLEASY_MAGIC_NUMBER;

	Curl_req_init(&data->req);

	result = Curl_init_userdefined(data);
	if (!result)
	{
		//Curl_dyn_init(&data->state.headerb, CURL_MAX_HTTP_HEADER);
		
#ifndef CURL_DISABLE_HTTP
		//Curl_llist_init(&data->state.httphdrs, NULL);
#endif
	}

	if (result)
	{
		//Curl_dyn_free(&data->state.headerb);
		Curl_req_free(&data->req);
		Curl_freeset(data);
		free(data);
		data = NULL;
	}
	else {
		*curl = data;
	}
	return result;
}

CURLcode Curl_setup_conn(struct Curl_easy* data, bool* protocol_done) 
{
	CURLcode result = CURLE_OK;
	struct connectdata* conn = data->conn;
	if (conn->handler->flags & PROTOPT_NONETWORK)
	{
		/* Nothing to setup when not using a network */
		*protocol_done = TRUE;
		return result;
	}

	return result;
}

CURLcode Curl_uc_to_curlcode(CURLUcode uc)
{
	switch (uc)
	{
		default:
			return CURLE_URL_MALFORMAT;
		case CURLUE_UNSUPPORTED_SCHEME:
			return CURLE_UNSUPPORTED_PROTOCOL;
		case CURLUE_OUT_OF_MEMORY:
			return CURLE_OUT_OF_MEMORY;
		case CURLUE_USER_NOT_ALLOWED:
			return CURLE_LOGIN_DENIED;
	}
}


static void up_free(struct Curl_easy* data)
{
	struct urlpieces* up = &data->state.up;
	Curl_safefree(up->scheme);
	Curl_safefree(up->hostname);
	Curl_safefree(up->port);
	Curl_safefree(up->user);
	Curl_safefree(up->password);
	Curl_safefree(up->options);
	Curl_safefree(up->path);
	Curl_safefree(up->query);
	curl_url_cleanup(data->state.uh);
	data->state.uh = NULL;
}

static CURLcode findprotocol(struct Curl_easy* data, struct connectdata* conn, const char* protostr)
{	
	/* Protocol found in table. Check if allowed */
	const struct Curl_handler* p = Curl_get_scheme_handler(protostr);
	if (p && p->protocol)
	{
		/* it is allowed for "normal" request, now do an extra check if this is the result of a redirect */
		if (data->state.this_is_a_follow && !(data->set.redir_protocols & p->protocol))
		{
			;	/* nope, get out */
		}
		else
		{
			conn->handler = conn->given = p;	/* Perform setup complement if some. */
			return CURLE_OK;					/* 'port' and 'remote_port' are set in setup_connection_internals() */
		}
	}
	failf(data, "Protocol \"%s\" %s%s", protostr, p ? "disabled" : "not supported", data->state.this_is_a_follow ? " (in redirect)" : "");

	return CURLE_UNSUPPORTED_PROTOCOL;
}

//TODO: Parse URL and fill in the relevant members of the connection struct.
static CURLcode parseurlandfillconn(struct Curl_easy* data, struct connectdata* conn)
{
	CURLU* uh;
	CURLUcode uc;
	CURLcode result;
	char* hostname;
	bool use_set_uh = (data->set.uh && !data->state.this_is_a_follow);

	up_free(data); /* cleanup previous leftovers first */

	if (use_set_uh)	{
		uh = data->state.uh = curl_url_dup(data->set.uh);
	}
	else {
		uh = data->state.uh = curl_url();
	}

	if (!uh) {
		return CURLE_OUT_OF_MEMORY;
	}
	if (data->set.str[STRING_DEFAULT_PROTOCOL] && !Curl_is_absolute_url(data->state.url, NULL, 0, TRUE))
	{
		char* url = aprintf("%s://%s", data->set.str[STRING_DEFAULT_PROTOCOL], data->state.url);
		if (!url)
		{
			return CURLE_OUT_OF_MEMORY;
		}
		if (data->state.url_alloc)
		{
			free(data->state.url);
		}
		data->state.url = url;
		data->state.url_alloc = TRUE;
	}

	if (!use_set_uh)
	{
		char* newurl = NULL;
		uc = curl_url_set(uh, CURLUPART_URL, data->state.url, 
						  (unsigned int)(CURLU_GUESS_SCHEME | CURLU_NON_SUPPORT_SCHEME | 
						  (data->set.disallow_username_in_url ? CURLU_DISALLOW_USER : 0) | 
					      (data->set.path_as_is ? CURLU_PATH_AS_IS : 0)));
		if (uc)
		{
			failf(data, "URL rejected: %s", curl_url_strerror(uc));
			return Curl_uc_to_curlcode(uc);
		}
		/* After it was parsed, get the generated normalized version */
		uc = curl_url_get(uh, CURLUPART_URL, &newurl, 0);
		if (uc)
		{
			return Curl_uc_to_curlcode(uc);
		}
		if (data->state.url_alloc)
		{
			free(data->state.url);
		}
		data->state.url = newurl;
		data->state.url_alloc = TRUE;
	}

	uc = curl_url_get(uh, CURLUPART_SCHEME, &data->state.up.scheme, 0);
	if (uc)
	{
		return Curl_uc_to_curlcode(uc);
	}

	uc = curl_url_get(uh, CURLUPART_HOST, &data->state.up.hostname, 0);
	if (uc)
	{
		if (!strcasecompare("file", data->state.up.scheme))
		{
			return CURLE_OUT_OF_MEMORY;
		}
	}
	else if (strlen(data->state.up.hostname) > MAX_URL_LEN)
	{
		failf(data, "Too long hostname (maximum is %d)", MAX_URL_LEN);
		return CURLE_URL_MALFORMAT;
	}

	hostname = data->state.up.hostname;
	if (hostname && hostname[0] == '[')
	{
		size_t hlen;
		conn->bits.ipv6_ip = TRUE;	/* This looks like an IPv6 address literal. See if there is an address scope. */
		hostname++;
		hlen = strlen(hostname);	/* cut off the brackets! */
		hostname[hlen - 1] = 0;
	}

	/* Make sure the connect struct gets its own copy of the hostname */
	conn->host.rawalloc = _strdup(hostname ? hostname : "");		
	if (!conn->host.rawalloc)
	{
		return CURLE_OUT_OF_MEMORY;
	}
	conn->host.name = conn->host.rawalloc;

	/* IDN-convert the hostnames */
	result = Curl_idnconvert_hostname(&conn->host);
	if (result)
	{
		return result;
	}
	//TODO: Find protocol with scheme here!
	result = findprotocol(data, conn, data->state.up.scheme);
	if (result)
	{
		return result;
	}

	if (!data->state.aptr.passwd)
	{
		uc = curl_url_get(uh, CURLUPART_PASSWORD, &data->state.up.password, 0);
		if (!uc)
		{
			char* decoded;
			result = Curl_urldecode(data->state.up.password, 0, &decoded, NULL, conn->handler->flags & PROTOPT_USERPWDCTRL ? REJECT_ZERO : REJECT_CTRL);
			if (result)
			{
				return result;
			}

			conn->passwd = decoded;
			result = Curl_setstropt(&data->state.aptr.passwd, decoded);
			if (result)
			{
				return result;
			}
		}
		else if (uc != CURLUE_NO_PASSWORD)
		{
			return Curl_uc_to_curlcode(uc);
		}
	}

	if (!data->state.aptr.user)
	{
		uc = curl_url_get(uh, CURLUPART_USER, &data->state.up.user, 0);
		if (!uc)
		{
			char* decoded;
			result = Curl_urldecode(data->state.up.user, 0, &decoded, NULL, conn->handler->flags & PROTOPT_USERPWDCTRL ? REJECT_ZERO : REJECT_CTRL);
			if (result)
			{
				return result;
			}
			conn->user = decoded;
			result = Curl_setstropt(&data->state.aptr.user, decoded);
		}
		else if (uc != CURLUE_NO_USER)
		{
			return Curl_uc_to_curlcode(uc);
		}

		if (result)
		{
			return result;
		}
	}

	uc = curl_url_get(uh, CURLUPART_OPTIONS, &data->state.up.options, CURLU_URLDECODE);
	if (!uc)
	{
		conn->options = _strdup(data->state.up.options);
		if (!conn->options)
		{
			return CURLE_OUT_OF_MEMORY;
		}
	}
	else if (uc != CURLUE_NO_OPTIONS)
	{
		return Curl_uc_to_curlcode(uc);
	}


	uc = curl_url_get(uh, CURLUPART_PATH, &data->state.up.path, CURLU_URLENCODE);
	if (uc)
	{
		return Curl_uc_to_curlcode(uc);
	}


	uc = curl_url_get(uh, CURLUPART_PORT, &data->state.up.port, CURLU_DEFAULT_PORT);
	if (uc)
	{
		if (!strcasecompare("file", data->state.up.scheme))
		{
			return CURLE_OUT_OF_MEMORY;
		}
	}
	else
	{
		unsigned long port = strtoul(data->state.up.port, NULL, 10);
	}

	(void)curl_url_get(uh, CURLUPART_QUERY, &data->state.up.query, 0);

	return CURLE_OK;
}

//TODO: Set the login details so they are available in the connection
static CURLcode set_login(struct Curl_easy* data, struct connectdata* conn)
{
	CURLcode result = CURLE_OK;
	const char* setuser = CURL_DEFAULT_USER;
	const char* setpasswd = CURL_DEFAULT_PASSWORD;

	/* If our protocol needs a password and we have none, use the defaults */
	if ((conn->handler->flags & PROTOPT_NEEDSPWD) && !data->state.aptr.user)
	{
		;
	}
	else
	{
		setuser = "";
		setpasswd = "";
	}
	/* Store the default user */
	if (!conn->user)
	{
		conn->user = _strdup(setuser);
		if (!conn->user)
		{
			return CURLE_OUT_OF_MEMORY;
		}
	}
	/* Store the default password */
	if (!conn->passwd)
	{
		conn->passwd = _strdup(setpasswd);
		if (!conn->passwd)
		{
			result = CURLE_OUT_OF_MEMORY;
		}
	}
	return result;
}

//TODO: Here parse url -> get protocol -> set info for connectdata.
CURLcode Curl_connect(struct Curl_easy* data, bool* async, bool* protocol_connect)
{
	*async = FALSE; /* assume synchronous resolves by default */
	CURLcode result;
	struct connectdata* conn;

	Curl_req_hard_reset(&data->req, data);

	if (!data->state.url)
	{
		result = CURLE_URL_MALFORMAT;
		goto out;
	}

	conn = (struct connectdata*)calloc(1, sizeof(struct connectdata));
	if (!conn)
	{
		result = CURLE_OUT_OF_MEMORY;
		goto out;
	}
	
	//TODO: Get handler with schema protocol here!
	result = parseurlandfillconn(data, conn);
	if (result)
	{
		goto out;
	}

	result = set_login(data, conn); /* Default credentials */
	if (result)
	{
		goto out;
	}

	if (conn->bits.conn_to_host)
	{
		result = Curl_idnconvert_hostname(&conn->conn_to_host);
		if (result)
		{
			return result;
		}
	}

	/*************************************************************
	 * Check whether the host and the "connect to host" are equal.
	 * Do this after the hostnames have been IDN-converted.
	 *************************************************************/
	if (conn->bits.conn_to_host && strcasecompare(conn->conn_to_host.name, conn->host.name))
	{
		conn->bits.conn_to_host = FALSE;
	}
	/*************************************************************
	 * Check whether the port and the "connect to port" are equal.
	 * Do this after the remote port number has been fixed in the URL.
	 *************************************************************/
	if (conn->bits.conn_to_port && conn->conn_to_port == conn->remote_port)
	{
		conn->bits.conn_to_port = FALSE;
	}

	DEBUGASSERT(conn->user);
	DEBUGASSERT(conn->passwd);

out:
	return result;
}

//TODO: Here disconnect and remove conn for list -> free buf
CURLcode Curl_close(struct Curl_easy** datap) 
{
	CURLcode result = CURLE_OK;


	return result;
}

//TODO: Here handling disconnect for protocol 
bool Curl_on_disconnect(struct Curl_easy* data, struct connectdata* conn, bool aborted) 
{
	bool result = true;
	

	return result;
}

CURLcode Curl_init_userdefined(struct Curl_easy* data)
{
	CURLcode result = CURLE_OK;
	struct UserDefined* set = &data->set;

	set->err = stderr;		/* default stderr to stderr */
	set->in_set = stdin;	/* default input from stdin */
	set->out_get = stdout;	/* default output to stdout */

	set->is_fread_set = 0;	
	set->fwrite_func = (curl_write_callback)fwrite;		/* use fread as default function to read input */
	set->fread_func_set = (curl_read_callback)fread;	/* use fwrite as default function to store output */

	set->maxconnects = DEFAULT_CONNCACHE_SIZE;
	set->postfieldsize = -1;   /* unknown size */
	set->method = HTTPREQ_GET; /* Default HTTP request */
	set->httpwant = CURL_HTTP_VERSION_1_1;
	set->str[STRING_USERAGENT] = (char*)"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/100.0.4896.75 Safari/537.36 Edg/100.0.1185.36";

#ifndef CURL_DISABLE_FTP
	set->ftp_use_epsv = TRUE;   /* FTP defaults to EPSV operations */
	set->ftp_use_eprt = TRUE;   /* FTP defaults to EPRT operations */
	set->ftp_use_pret = FALSE;  /* mainly useful for drftpd servers */
	set->ftp_skip_ip = TRUE;    /* skip PASV IP by default */
	set->ftp_filemethod = FTPFILE_MULTICWD;
	set->wildcard_enabled = FALSE;
#endif

	set->buffer_size = READBUFFER_SIZE;
	set->upload_buffer_size = UPLOADBUFFER_DEFAULT;
	set->quick_exit = 0L;

	return result;
}

/*
 * Curl_parse_login_details()
 *
 * This is used to parse a login string for username, password and options in
 * the following formats:
 *
 *   user
 *   user:password
 *   user:password;options
 *   user;options
 *   user;options:password
 *   :password
 *   :password;options
 *   ;options
 *   ;options:password
 *
 * Parameters:
 *
 * login    [in]     - login string.
 * len      [in]     - length of the login string.
 * userp    [in/out] - address where a pointer to newly allocated memory
 *                     holding the user will be stored upon completion.
 * passwdp  [in/out] - address where a pointer to newly allocated memory
 *                     holding the password will be stored upon completion.
 * optionsp [in/out] - OPTIONAL address where a pointer to newly allocated
 *                     memory holding the options will be stored upon
 *                     completion.
 *
 * Returns CURLE_OK on success.
 */
CURLcode Curl_parse_login_details(const char* login, const size_t len, char** userp, char** passwdp, char** optionsp)
{
	char* ubuf = NULL;
	char* pbuf = NULL;
	const char* psep = NULL;
	const char* osep = NULL;
	size_t ulen;
	size_t plen;
	size_t olen;

	DEBUGASSERT(userp);
	DEBUGASSERT(passwdp);

	psep = (const char*)memchr(login, ':', len);		/* Attempt to find the password separator */

	if (optionsp)
	{
		osep = (const char*)memchr(login, ';', len);	/* Attempt to find the options separator */
	}

	/* Calculate the portion lengths */
	ulen = (psep ? (size_t)(osep && psep > osep ? osep - login : psep - login) : (osep ? (size_t)(osep - login) : len));
	plen = (psep ? (osep && osep > psep ? (size_t)(osep - psep) : (size_t)(login + len - psep)) - 1 : 0);
	olen = (osep ? (psep && psep > osep ? (size_t)(psep - osep) : (size_t)(login + len - osep)) - 1 : 0);

	/* Clone the user portion buffer, which can be zero length */
	ubuf = (char*)Curl_memdup0(login, ulen);
	if (!ubuf)
	{
		goto error;
	}
	/* Clone the password portion buffer */
	if (psep)
	{
		pbuf = (char*)Curl_memdup0(&psep[1], plen);
		if (!pbuf)
		{
			goto error;
		}
	}
	/* Allocate the options portion buffer */
	if (optionsp)
	{
		char* obuf = NULL;
		if (olen)
		{
			obuf = (char*)Curl_memdup0(&osep[1], olen);
			if (!obuf)
			{
				goto error;
			}
		}
		*optionsp = obuf;
	}
	*userp = ubuf;
	*passwdp = pbuf;
	return CURLE_OK;
error:
	free(ubuf);
	free(pbuf);
	return CURLE_OUT_OF_MEMORY;
}

const struct Curl_handler* Curl_get_scheme_handler(const char* scheme)
{
	return Curl_getn_scheme_handler(scheme, strlen(scheme));
}

const struct Curl_handler* Curl_getn_scheme_handler(const char* scheme, size_t len)
{
	static const struct Curl_handler* const protocols[] = 
	{
#ifndef CURL_DISABLE_HTTP
		& Curl_handler_http,    /* "http" */
#if defined(USE_SSL)
		&Curl_handler_https,	/* "https" */
#endif
#if !defined(CURL_DISABLE_WEBSOCKETS)
		&Curl_handler_ws,		/* "ws" */
#if defined(USE_SSL)
		&Curl_handler_wss,		/* "wss" */
#endif
#endif
#endif

#ifndef CURL_DISABLE_FTP
		& Curl_handler_ftp,     /* "ftp" */
#if defined(USE_SSL)
		&Curl_handler_ftps,		/* "ftps" */
#endif
#endif
		NULL					/* End marker */
	};

	if (!scheme || !len || len >= 5) {
		return NULL;
	}

	for (const struct Curl_handler* const* p = protocols; *p; p++)
	{
		const struct Curl_handler* h = *p;
		if (h && strncasecompare(scheme, h->scheme, len) && !h->scheme[len])
		{
			return h;
		}
	}

	return NULL;
}