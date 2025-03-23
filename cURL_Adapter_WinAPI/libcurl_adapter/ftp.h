#pragma once
#include "curl.h"
#include "setup.h"

#ifndef CURL_DISABLE_FTP
extern const struct Curl_handler Curl_handler_ftp;
#ifdef USE_SSL
extern const struct Curl_handler Curl_handler_ftps;
#endif /* USE_SSL */
#endif /* CURL_DISABLE_FTP */

struct ftp_conn
{

};

typedef enum
{
    FTPFILE_MULTICWD = 1,       /* as defined by RFC1738 */
    FTPFILE_NOCWD = 2,          /* use SIZE / RETR / STOR on the full path */
    FTPFILE_SINGLECWD = 3       /* make one CWD, then SIZE / RETR / STOR on the file */
} curl_ftpfile;

enum
{
    FTP_STOP,       /* do nothing state, stops the state machine */
    FTP_WAIT220,    /* waiting for the initial 220 response immediately after a connect */
    FTP_AUTH,
    FTP_USER,
    FTP_PASS,
    FTP_ACCT,
    FTP_PBSZ,
    FTP_PROT,
    FTP_CCC,
    FTP_PWD,
    FTP_SYST,
    FTP_NAMEFMT,
    FTP_QUOTE,      /* waiting for a response to a command sent in a quote list */
    FTP_RETR_PREQUOTE,
    FTP_STOR_PREQUOTE,
    FTP_POSTQUOTE,
    FTP_CWD,        /* change dir */
    FTP_MKD,        /* if the dir did not exist */
    FTP_MDTM,       /* to figure out the datestamp */
    FTP_TYPE,       /* to set type when doing a head-like request */
    FTP_LIST_TYPE,  /* set type when about to do a dir list */
    FTP_RETR_TYPE,  /* set type when about to RETR a file */
    FTP_STOR_TYPE,  /* set type when about to STOR a file */
    FTP_SIZE,       /* get the remote file's size for head-like request */
    FTP_RETR_SIZE,  /* get the remote file's size for RETR */
    FTP_STOR_SIZE,  /* get the size for STOR */
    FTP_REST,       /* when used to check if the server supports it in head-like */
    FTP_RETR_REST,  /* when asking for "resume" in for RETR */
    FTP_PORT,       /* generic state for PORT, LPRT and EPRT, check count1 */
    FTP_PRET,       /* generic state for PRET RETR, PRET STOR and PRET LIST/NLST */
    FTP_PASV,       /* generic state for PASV and EPSV, check count1 */
    FTP_LIST,       /* generic state for LIST, NLST or a custom list command */
    FTP_RETR,
    FTP_STOR,       /* generic state for STOR and APPE */
    FTP_QUIT,
    FTP_LAST        /* never used */
};
typedef unsigned char ftpstate; /* use the enum values */
