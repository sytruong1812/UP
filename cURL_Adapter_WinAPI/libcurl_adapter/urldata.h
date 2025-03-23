#pragma once
#include "curl.h"
#include "hash.h"
#include "trace.h"
#include "urlapi.h"
#include "request.h"

#include "ws.h"
#include "ftp.h"
#include "http.h"

#define PROTO_TYPE_SMALL
#ifndef PROTO_TYPE_SMALL
typedef curl_off_t curl_prot_t;
#else
typedef unsigned int curl_prot_t;
#endif

#define READBUFFER_SIZE CURL_MAX_WRITE_SIZE
#define READBUFFER_MAX  CURL_MAX_READ_SIZE
#define READBUFFER_MIN  1024

#define UPLOADBUFFER_DEFAULT 65536
#define UPLOADBUFFER_MAX (2*1024*1024)
#define UPLOADBUFFER_MIN CURL_MAX_WRITE_SIZE

#define PORT_FTP		21
#define PORT_FTPS		990
#define PORT_TFTP		69
#define PORT_HTTP		80
#define PORT_HTTPS		443

#ifndef CURL_DISABLE_WEBSOCKETS
#define CURLPROTO_WS     (1<<30)
#define CURLPROTO_WSS    ((curl_prot_t)1<<31)
#else
#define CURLPROTO_WS 0
#define CURLPROTO_WSS 0
#endif

#define PROTO_FAMILY_HTTP (CURLPROTO_HTTP|CURLPROTO_HTTPS|CURLPROTO_WS| CURLPROTO_WSS)
#define PROTO_FAMILY_FTP  (CURLPROTO_FTP|CURLPROTO_FTPS)

/* Default FTP etc response timeout in milliseconds */
#define RESP_TIMEOUT (120*1000)
#define CURL_MAX_INPUT_LENGTH 8000000

#define DEFAULT_CONNCACHE_SIZE 5

#define CURLEASY_MAGIC_NUMBER	0xc0dedbadU
#define GOOD_EASY_HANDLE(x)		((x) && ((x)->magic == CURLEASY_MAGIC_NUMBER))

#define CURL_MULTI_HANDLE		0x000bab1e
#define GOOD_MULTI_HANDLE(x)	((x) && (x)->magic == CURL_MULTI_HANDLE)

#include "setup.h"

struct Curl_handler 
{
	const char* scheme;       
	CURLcode(*setup_connection)(struct Curl_easy* data, struct connectdata* conn);
	CURLcode(*do_it)(struct Curl_easy* data, bool* done);
	CURLcode(*done)(struct Curl_easy* data, CURLcode, bool);
	CURLcode(*do_more)(struct Curl_easy* data, int*);
	CURLcode(*connect_it)(struct Curl_easy* data, bool* done);
	CURLcode(*connecting)(struct Curl_easy* data, bool* done);
	CURLcode(*doing)(struct Curl_easy* data, bool* done);
	CURLcode(*disconnect)(struct Curl_easy*, struct connectdata*, bool dead_connection);
	CURLcode(*write_resp)(struct Curl_easy* data, const char* buf, size_t blen, bool is_eos);
	CURLcode(*write_resp_hd)(struct Curl_easy* data, const char* hd, size_t hdlen, bool is_eos);
	unsigned int (*connection_check)(struct Curl_easy* data, struct connectdata* conn, unsigned int checks_to_perform);
	void (*attach)(struct Curl_easy* data, struct connectdata* conn);
	int defport;
	curl_prot_t protocol;
	curl_prot_t family;
	unsigned int flags;
};

struct ConnectBits 
{
	BIT(close);				/* if set, we close the connection after this request */
	BIT(reuse);				/* if set, this is a reused connection */
	BIT(altused);			/* this is an alt-svc "redirect" */
	BIT(conn_to_host);		/* if set, this connection has a "connect to host" that overrides the host in the URL */
	BIT(conn_to_port);		/* if set, this connection has a "connect to port" that overrides the port in the URL (remote port) */
	BIT(ipv6_ip);			/* we communicate with a remote site specified with pure IPv6 IP address */
	BIT(ipv6);				/* we communicate with a site using an IPv6 address */
	BIT(do_more);			/* this is set TRUE if the ->curl_do_more() function is supposed to be called, after ->curl_do() */
	BIT(protoconnstart);	/* the protocol layer has STARTED its operation after the TCP layer connect */
	BIT(retry);				/* this connection is about to get closed and then */
	BIT(aborted);			/* connection was aborted, e.g. in unclean state */
	BIT(shutdown_handler);	/* connection shutdown: handler shut down */
	BIT(shutdown_filters);	/* connection shutdown: filters shut down */
	BIT(in_cpool);			/* connection is kept in a connection pool */
};

struct hostname;
/*
 * The connectdata struct contains all fields and variables that should be
 * unique for an entire connection.
 */
struct connectdata {

	char* user;    /* username string, allocated */
	char* passwd;  /* password string, allocated */
	char* options; /* options string, allocated */

	struct hostname host;
	char* hostname_resolve;			/* hostname to resolve to address, allocated */
	char* secondaryhostname;		/* secondary socket hostname (ftp) */
	struct hostname conn_to_host;	/* the host to connect to. valid only if bits.conn_to_host is set */

	int remote_port;	/* the remote port, not the proxy port! */
	int conn_to_port;	/* the remote port to connect to. valid only if bits.conn_to_port is set */

	struct ConnectBits bits;	/* various state-flags for this connection */

	const struct Curl_handler* handler; /* Connection's protocol handler */
	const struct Curl_handler* given;   /* The protocol first given */

	union {
#ifndef CURL_DISABLE_FTP
		struct ftp_conn* ftpc;
#endif
#ifndef CURL_DISABLE_WEBSOCKETS
		struct websocket* ws;
#endif
		unsigned int unused : 1; /* avoids empty union */
	} proto;

	unsigned char httpversion; /* the HTTP version*10 reported by the server */
	unsigned char connect_only;
};

/* Individual pieces of the URL */
struct urlpieces {
	char* scheme;
	char* hostname;
	char* port;
	char* user;
	char* password;
	char* options;
	char* path;
	char* query;
};

struct UrlState 
{
	char* url;				/* Work URL, copied from UserDefined */
	long followlocation;	/* Redirect counter */
	unsigned char httpwant;
	unsigned char httpversion;
	unsigned char httpreq; 

	void* in;						/* CURLOPT_READDATA */
	curl_read_callback fread_func;	/* read callback/function */

	CURLU* uh;						/* URL handle for the current parsed URL */
	struct urlpieces up;

#ifndef CURL_DISABLE_VERBOSE_STRINGS
	struct curl_trc_feat* feat;		/* Opt. trace feature transfer is part of */
#endif

	/* Dynamically allocated strings, MUST be freed before this struct is killed. */
	struct dynamically_allocated_data 
	{
		char* uagent;
		char* accept_encoding;
		char* userpwd;
		char* rangeline;
		char* ref;
		char* host;
		char* te;		/* TE: request header */
		char* user;		/* Transfer credentials */
		char* passwd;
	} aptr;

	BIT(done);
	BIT(error_buf);				/* Set to TRUE if the error buffer is already filled in */
	BIT(url_alloc);				/* URL string is malloc()'ed */
	BIT(upload);				/* upload request */
	BIT(this_is_a_follow);		/* this is a followed Location: request */
	BIT(wildcardmatch);			/* enable wildcard matching */
	BIT(wildcard_resolve);		/* Set to true if any resolve change is a wildcard */
};

enum dupstring {
	STRING_CERT,				/* client certificate filename */
	STRING_CERT_TYPE,			/* format for certificate (default: PEM)*/
	// HTTP/HTTPS related
	STRING_CUSTOMREQUEST,       // HTTP request method
	STRING_ENCODING,            // Accept-Encoding string
	STRING_SET_RANGE,			/* range, if used */
	STRING_SET_REFERER,         // HTTP referer field
	STRING_SET_URL,             // Original URL to work on
	STRING_USERAGENT,           // User-Agent string
	STRING_USERNAME,			/* <username>, if used */
	STRING_PASSWORD,			/* <password>, if used */
	STRING_OPTIONS,				/* <options>, if used */
	// Cookie related (HTTP)
#ifndef CURL_DISABLE_COOKIES
	STRING_COOKIE,              // HTTP cookie string to send
	STRING_COOKIEJAR,           // dump all cookies to this file
#endif
	// FTP/FTPS related 
#ifndef CURL_DISABLE_FTP
	STRING_FTP_ACCOUNT,				// ftp account data
	STRING_FTP_ALTERNATIVE_TO_USER, // command if USER/PASS fails
	STRING_FTPPORT,					// port for FTP PORT command
#endif
	// Protocol related
	STRING_DEFAULT_PROTOCOL,    // Default protocol when URL doesn't specify
	// Proxy related (affects HTTP/HTTPS/FTP)
#ifndef CURL_DISABLE_PROXY
	STRING_PROXY,              // proxy to use
	STRING_PRE_PROXY,          // pre socks proxy to use
	STRING_PROXYUSERNAME,      // Proxy username
	STRING_PROXYPASSWORD,      // Proxy password 
	STRING_NOPROXY,            // Hosts to bypass proxy
#endif
	STRING_COPYPOSTFIELDS,		// if POST, set the fields' values here
	STRING_LAST					// not used, just an end-of-list marker
};

enum dupblob {
	BLOB_CERT,
	BLOB_KEY,
	BLOB_SSL_ISSUERCERT,
	BLOB_CAINFO,
#ifndef CURL_DISABLE_PROXY
	BLOB_CERT_PROXY,
	BLOB_KEY_PROXY,
	BLOB_SSL_ISSUERCERT_PROXY,
	BLOB_CAINFO_PROXY,
#endif
	BLOB_LAST
};

struct UserDefined 
{
	FILE* err;			 /* The stderr user data goes here */
	void* debug_data;    /* The data that will be passed to fdebug */
	char* error_buffer;	 /* (Static) store failure messages in here */

	void* in_set;		 /* CURLOPT_READDATA */
	void* out_get;		 /* CURLOPT_WRITEDATA */
	void* header_get;	 /* CURLOPT_HEADERDATA */

	curl_write_callback fwrite_func;	/* function that stores the output */
	curl_write_callback fwrite_header;	/* function that stores headers */
	curl_read_callback  fread_func_set;	/* function that reads the input */
	curl_seek_callback  seek_func;		/* function that seeks the input */
	curl_debug_callback fdebug;			/* function that write informational data */

	void* postfields;			/* if POST, set the fields' values here */
	curl_off_t postfieldsize;	/* if POST, this might have a size to use instead of strlen(), and then the data *may* be binary (contain zero bytes) */

	curl_prot_t allowed_protocols;
	curl_prot_t redir_protocols;

	void* private_data;			/* application-private data */
	char* str[STRING_LAST];		/* array of strings, pointing to allocated memory */
	struct curl_blob* blobs[BLOB_LAST];
	
	CURLU* uh;						/* URL handle for the current parsed URL */

	curl_off_t filesize;			/* size of file to upload, -1 means unknown */
	curl_off_t max_filesize;		/* Maximum file size to download */


	unsigned char method;		/* what kind of HTTP request: Curl_HttpReq */
	unsigned char httpwant;		/* when non-zero, a specific HTTP version requested to be used in the library's request(s) */
	unsigned short use_port;	/* which port to use (when not using default) */
	unsigned char connect_only;	/* make connection/request, then let application use the socket */
#ifndef CURL_DISABLE_PROXY
	unsigned short proxyport;	/* If non-zero, use this port number by default. If the proxy string features a ":[port]" that one will override this. */
#endif
	unsigned int timeout;					/* ms, 0 means no timeout */
	unsigned int buffer_size;				/* size of receive buffer to use */
	unsigned int upload_buffer_size;		/* size of upload buffer to use, keep it >= CURL_MAX_WRITE_SIZE */
	unsigned int maxconnects;				/* max idle connections in the connection cache */
	unsigned int connecttimeout;			/* ms, 0 means default timeout */
	unsigned int server_response_timeout;	/* ms, 0 means no timeout */
	unsigned int shutdowntimeout;			/* ms, 0 means default timeout */

#ifndef CURL_DISABLE_FTP
	unsigned char ftp_filemethod;		/* how to get to a file: curl_ftpfile  */
	unsigned char ftpsslauth;			/* what AUTH XXX to try: curl_ftpauth */
	unsigned char ftp_ccc;				/* FTP CCC options: curl_ftpccc */
	unsigned int accepttimeout;			/* in milliseconds, 0 means no timeout */
	void* fnmatch_data;
	void* wildcardptr;
#endif
#if !defined(CURL_DISABLE_FTP) || defined(USE_SSH)
	struct curl_slist* quote;			/* after connection is established */
	struct curl_slist* postquote;		/* after the transfer */
	struct curl_slist* prequote;		/* before the transfer, after type */
	unsigned char ftp_create_missing_dirs;
#endif
	BIT(verbose);					/* output verbosity */
	BIT(ignorecl);					/* ignore content length */
	BIT(quick_exit);
	BIT(using_ssl);					/* TODO: Teddy set using ssl for HTTPS/WSS/FPTS */
	BIT(opt_no_body);				/* as set with CURLOPT_NOBODY */
	BIT(is_fread_set);				/* has read callback been set to non-NULL? */
	BIT(include_header);			/* include received protocol headers in data output */
	BIT(http_follow_location);		/* follow HTTP redirects */
	BIT(http_transfer_encoding);	/* request compressed HTTP transfer-encoding */
	BIT(ssl_enable_alpn);			/* TLS ALPN extension? */
	BIT(path_as_is);				/* allow dotdots? */
	BIT(disallow_username_in_url);	/* disallow username in URL */
	BIT(http_te_skip);				/* pass the raw body data to the user, even when transfer-encoded (chunked, compressed) */
	BIT(http_ce_skip);				/* pass the raw body data to the user, even when content-encoded (chunked, compressed) */
#ifndef CURL_DISABLE_PROXY
	BIT(tunnel_thru_httpproxy);		/* use CONNECT through an HTTP proxy */
#endif
#ifndef CURL_DISABLE_FTP
	BIT(ftp_use_port);     /* use the FTP PORT command */
	BIT(ftp_use_epsv);     /* if EPSV is to be attempted or not */
	BIT(ftp_use_eprt);     /* if EPRT is to be attempted or not */
	BIT(ftp_use_pret);     /* if PRET is to be used before PASV or not */
	BIT(ftp_skip_ip);      /* skip the IP address the FTP server passes on to us */
	BIT(wildcard_enabled); /* enable wildcard matching */
#endif

};

struct Curl_multi 
{
	unsigned int magic;
	unsigned int num_easy;			/* amount of entries in the linked list above. */
	unsigned int num_alive;			/* amount of easy handles that are added but have not yet reached COMPLETE state */
	unsigned int maxconnects;

	/* Each added easy handle is added to ONE of these three lists */
	struct Curl_llist process;		/* not in PENDING or MSGSENT */
	struct Curl_llist pending;		/* in PENDING */
	struct Curl_llist msgsent;		/* in MSGSENT */
	curl_off_t next_easy_mid;		/* next multi-id for easy handle added */

	long max_host_connections;		/* if >0, a fixed limit of the maximum number of connections per host */
	long max_total_connections;		/* if >0, a fixed limit of the maximum number of connections in total */
	long max_shutdown_connections;	/* if >0, a fixed limit of the maximum number of connections in shutdown handling */

	BIT(dead);
	BIT(in_callback);   /* true while executing a callback */
	BIT(recheckstate);  /* see Curl_multi_connchanged */
};

struct Curl_easy 
{
	unsigned int magic;	/* First a simple identifier to easier detect if a user mix up this easy handle with a multi handle. Set this to CURLEASY_MAGIC_NUMBER */

	curl_off_t id;
	CURLcode result;						/* Previous result */
	CURLMstate mstate;						/* The handle's state */

	struct Curl_multi* multi;
	struct Curl_multi* multi_easy;

	struct connectdata* conn;
	struct SingleRequest req;				/* Request-specific data */
	struct UserDefined set;					/* Values set by the libcurl user */
	struct UrlState state;					/* struct for fields used for state info and other dynamic purposes */

	struct Curl_llist_node multi_queue;		/* For multihandle list management */
	struct Curl_llist_node conn_queue;		/* List per connectdata */

};