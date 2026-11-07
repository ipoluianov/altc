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
	case ALT_EENC_BASE32: return "illegal base32 data";
	case ALT_EVARINT: return "binary: varint overflows a 64-bit integer";
	case ALT_ECSV: return "malformed CSV";
	case ALT_EMIME: return "malformed MIME data";
	case ALT_EEXIT: return "exit status not zero";
	case ALT_ENOTFOUND: return "executable file not found in $PATH";
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
	case ALT_EHPACK: return "hpack: malformed header block";
	case ALT_ETAR_HEADER: return "archive/tar: invalid tar header";
	case ALT_ETAR_FIELD: return "archive/tar: header field too long";
	case ALT_ETAR_WRITE_TOO_LONG: return "archive/tar: write too long";
	case ALT_ETAR_INSECURE_PATH: return "archive/tar: insecure file path";
	case ALT_EZIP_FORMAT: return "zip: not a valid zip file";
	case ALT_EZIP_ALGORITHM: return "zip: unsupported compression algorithm";
	case ALT_EZIP_INSECURE_PATH: return "zip: insecure file path";
	case ALT_EHTTP2_PROTOCOL: return "http2: protocol error";
	case ALT_EHTTP2_STREAM: return "http2: stream reset";
	case ALT_EHTTP2_GOAWAY: return "http2: connection going away";
	case ALT_EHTTP2_FLOW: return "http2: flow control error";
	}
	return "unknown error";
}
