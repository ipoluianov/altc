#include <alt/errors.h>
#include <string.h>

const char *alt_strerror(int code) {
	if (code < 0)
		code = -code;
	if (code == 0)
		return "success";
	if (code < ALT_EBASE)
		return strerror(code);
	switch (code) {
	case ALT_ECLOSED: return "use of a closed object";
	case ALT_EUNEXPECTED_EOF: return "unexpected end of data";
	case ALT_ETOOLARGE: return "limit exceeded";
	case ALT_ESYNTAX: return "syntax error";
	case ALT_ERANGE: return "value out of range";
	case ALT_ENOTASK: return "not called from a task";
	case ALT_EEOF: return "end of data";
	case ALT_EBUFFULL: return "buffer full";
	case ALT_ENET_ADDR: return "malformed address";
	case ALT_ENET_NOHOST: return "no such host";
	case ALT_ENET_DNS: return "name server failure";
	case ALT_ENET_DNSFORMAT: return "malformed DNS message";
	case ALT_ENET_NETWORK: return "unknown network";
	case ALT_EHTTP_BADREQUEST: return "malformed HTTP request";
	case ALT_EHTTP_BADRESPONSE: return "malformed HTTP response";
	case ALT_EHTTP_HEADERTOOLARGE: return "HTTP header too large";
	case ALT_EHTTP_BODYTOOLARGE: return "HTTP body too large";
	case ALT_EHTTP_BADCHUNK: return "malformed chunked encoding";
	case ALT_EHTTP_REDIRECTS: return "too many redirects";
	case ALT_EHTTP_PATTERN: return "malformed pattern";
	case ALT_EHTTP_CONFLICT: return "conflicting patterns";
	case ALT_EHTTP_HIJACKED: return "connection hijacked";
	case ALT_EHTTP_SCHEME: return "unsupported URL scheme";
	case ALT_EHTTP_WRITTEN: return "HTTP header already written";
	case ALT_EHTTP_UNSUPPORTED: return "unsupported transfer encoding";
	case ALT_EHTTP_CONTENTLENGTH: return "wrote more than the declared Content-Length";
	case ALT_EURL_PARSE: return "malformed URL";
	case ALT_EURL_ESCAPE: return "malformed percent escape";
	case ALT_EURL_HOST: return "malformed host";
	case ALT_EURL_PORT: return "malformed port";
	case ALT_EJSON_SYNTAX: return "malformed JSON";
	case ALT_EJSON_DEPTH: return "JSON nested too deeply";
	case ALT_EJSON_TYPE: return "JSON value of another type";
	case ALT_EENC_BASE64: return "malformed base64";
	case ALT_EENC_HEX: return "malformed hexadecimal";
	case ALT_EFLAG_HELP: return "flag: help requested";
	case ALT_EFLAG_PARSE: return "bad command line flags";
	case ALT_EFLATE_CORRUPT: return "flate: corrupt input";
	case ALT_EGZIP_HEADER: return "invalid header";
	case ALT_ECHECKSUM: return "invalid checksum";
	case ALT_EZLIB_DICT: return "zlib: invalid dictionary";
	case ALT_EAUTH: return "message authentication failed";
	case ALT_EVERIFY: return "verification error";
	case ALT_EKEY: return "invalid key";
	case ALT_EX509: return "x509: malformed certificate";
	case ALT_EX509_VERIFY: return "x509: certificate verification failed";
	case ALT_ETLS_HANDSHAKE: return "tls: handshake failure";
	case ALT_ETLS_ALERT: return "tls: alert from the peer";
	case ALT_ETLS_CERT: return "tls: certificate not trusted";
	case ALT_ETLS_RECORD: return "tls: bad record";
	}
	return "unknown error";
}
