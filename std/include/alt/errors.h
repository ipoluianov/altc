/* AltC standard library: error codes.

   Functions return int (ssize_t for counts): >= 0 is success or a count,
   < 0 is a negated error code. System errors are negated errno values
   (-ETIMEDOUT, -ECONNRESET, -ENOMEM); the library's own codes are above
   the errno range, one block per package. alt_strerror describes any
   code, negated or not. */
#ifndef ALT_ERRORS_H
#define ALT_ERRORS_H

#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	ALT_EBASE = 10000,

	/* core */
	ALT_ECLOSED = ALT_EBASE + 1, /* the object (channel, connection, server) is closed */
	ALT_EUNEXPECTED_EOF,         /* the data ended in the middle of a unit */
	ALT_ETOOLARGE,               /* a limit was exceeded */
	ALT_ESYNTAX,                 /* malformed input */
	ALT_ERANGE,                  /* a value out of range */
	ALT_ENOTASK,                 /* the call needs to run in a task */
	ALT_EEOF,                    /* no more data (for calls that return items, not counts) */
	ALT_EBUFFULL,                /* the buffer cannot hold the unit (bufio.ErrBufferFull) */

	/* net */
	ALT_ENET_BASE = ALT_EBASE + 100,
	ALT_ENET_ADDR,               /* malformed address */
	ALT_ENET_NOHOST,             /* no such host */
	ALT_ENET_DNS,                /* the name servers failed */
	ALT_ENET_DNSFORMAT,          /* malformed DNS message */
	ALT_ENET_NETWORK,            /* unknown network ("tcp", "udp", "unix"...) */

	/* net/http */
	ALT_EHTTP_BASE = ALT_EBASE + 200,
	ALT_EHTTP_BADREQUEST,        /* malformed request */
	ALT_EHTTP_BADRESPONSE,       /* malformed response */
	ALT_EHTTP_HEADERTOOLARGE,    /* the header exceeds the limit */
	ALT_EHTTP_BODYTOOLARGE,      /* the body exceeds the limit */
	ALT_EHTTP_BADCHUNK,          /* malformed chunked encoding */
	ALT_EHTTP_REDIRECTS,         /* too many redirects */
	ALT_EHTTP_PATTERN,           /* malformed multiplexer pattern */
	ALT_EHTTP_CONFLICT,          /* conflicting multiplexer patterns */
	ALT_EHTTP_HIJACKED,          /* the connection was taken over by the handler */
	ALT_EHTTP_SCHEME,            /* unsupported URL scheme */
	ALT_EHTTP_WRITTEN,           /* the header was already written */
	ALT_EHTTP_UNSUPPORTED,       /* an unsupported transfer encoding or protocol feature */
	ALT_EHTTP_CONTENTLENGTH,     /* more body written than the Content-Length declared */

	/* net/url */
	ALT_EURL_BASE = ALT_EBASE + 300,
	ALT_EURL_PARSE,              /* malformed URL */
	ALT_EURL_ESCAPE,             /* malformed percent escape */
	ALT_EURL_HOST,               /* malformed host */
	ALT_EURL_PORT,               /* malformed port */

	/* encoding */
	ALT_EENC_BASE = ALT_EBASE + 400,
	ALT_EJSON_SYNTAX,            /* malformed JSON */
	ALT_EJSON_DEPTH,             /* JSON nested too deeply */
	ALT_EJSON_TYPE,              /* a JSON value of another type */
	ALT_EENC_BASE64,             /* malformed base64 */
	ALT_EENC_HEX,                /* malformed hexadecimal */

	/* flag */
	ALT_EFLAG_BASE = ALT_EBASE + 500,
	ALT_EFLAG_HELP,              /* -help or -h without such a flag (flag.ErrHelp) */
	ALT_EFLAG_PARSE,             /* bad flag syntax, unknown flag, missing or bad value */

	/* compress */
	ALT_ECOMPRESS_BASE = ALT_EBASE + 600,
	ALT_EFLATE_CORRUPT,          /* corrupt DEFLATE data */
	ALT_EGZIP_HEADER,            /* invalid gzip or zlib header */
	ALT_ECHECKSUM,               /* the checksum of the data does not match */
	ALT_EZLIB_DICT,              /* the zlib stream needs a dictionary it was not given */

	/* crypto */
	ALT_ECRYPTO_BASE = ALT_EBASE + 700,
	ALT_EAUTH,                   /* message authentication failed (a wrong key or tampered data) */
	ALT_EVERIFY,                 /* a signature does not verify */
	ALT_EKEY,                    /* an invalid key or point */
	ALT_EX509,                   /* a malformed certificate */
	ALT_EX509_VERIFY,            /* a certificate that does not verify */

	/* tls */
	ALT_ETLS_BASE = ALT_EBASE + 800,
	ALT_ETLS_HANDSHAKE,          /* the handshake failed here (alt_tls_error tells why) */
	ALT_ETLS_ALERT,              /* the peer sent a fatal alert */
	ALT_ETLS_CERT,               /* the peer's certificate is not trusted */
	ALT_ETLS_RECORD,             /* a record that does not decrypt or parse */

	ALT_ELAST
};

/* alt_strerror describes an error code (negated or not) */
const char *alt_strerror(int code);

#ifdef __cplusplus
}
#endif

#endif
