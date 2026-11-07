/* AltC standard library: mail messages (Go's net/mail).

   RFC 5322 with RFC 6532 (UTF-8), with Go's divergences: no obsolete
   address forms (routes), no folding inside addresses, a leading mbox
   "From " line is not special, display names may hold RFC 2047
   encoded-words.

   Errors are -ALT_ESYNTAX with Go's text in err ("mail: missing @ in
   addr-spec", "mail: no angle-addr"...), -ENOENT for a header that is
   not there ("mail: header not in message"), the reader's errors
   (-ALT_EEOF: "EOF") or a charset reader's code. err may be NULL. */
#ifndef ALT_NET_MAIL_H
#define ALT_NET_MAIL_H

#include <alt/arena.h>
#include <alt/errors.h>
#include <alt/io.h>
#include <alt/mime.h>
#include <alt/net/textproto.h>
#include <alt/str.h>
#include <alt/time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* "Barry Gibbs <bg@example.com>": {"Barry Gibbs", "bg@example.com"} */
typedef struct {
	alt_str name;    /* may be empty */
	alt_str address; /* local@domain */
} alt_mail_address;

/* AddressParser */
typedef struct {
	const alt_mime_word_decoder *word_decoder; /* NULL: utf-8, iso-8859-1 and us-ascii only */
} alt_mail_address_parser;

/* ParseAddress: one address (a group must hold exactly one) */
int alt_mail_parse_address(alt_arena *a, alt_str s, alt_mail_address *out, alt_buf *err);
/* ParseAddressList: comma-separated addresses and groups ("g: a@b, c@d;"),
   empty entries skipped; *out (in a) holds *n addresses */
int alt_mail_parse_address_list(alt_arena *a, alt_str s, alt_mail_address **out, size_t *n, alt_buf *err);
int alt_mail_parser_parse(const alt_mail_address_parser *p, alt_arena *a, alt_str s, alt_mail_address *out, alt_buf *err);
int alt_mail_parser_parse_list(const alt_mail_address_parser *p, alt_arena *a, alt_str s, alt_mail_address **out, size_t *n,
	alt_buf *err);

/* Address.String: "\"Barry Gibbs\" <bg@example.com>", the local part
   quoted when needed, non-ASCII names as RFC 2047 encoded-words */
int alt_mail_address_format(alt_buf *b, const alt_mail_address *addr);

/* ParseDate: an RFC 5322 date ("Mon, 02 Jan 2006 15:04:05 -0700" and the
   forms Go accepts) into *t and its zone *z (NULL: not wanted) */
int alt_mail_parse_date(alt_str date, alt_wall *t, alt_zone *z, alt_buf *err);

/* a parsed message: the header (keys canonical when valid, values in
   order) and the body, read from the rest of the stream (the body reads
   through m: the message must not move) */
typedef struct {
	alt_textproto_header header;
	alt_reader body;
	alt_bufreader br;  /* internal */
} alt_mail_message;

/* ReadMessage: reads the header from r (keys, values and the header in
   a); the body reads on from r. A header that ends with the data (no
   empty line) is accepted when it has fields. Free with alt_mail_message_free. */
int alt_mail_read_message(alt_mail_message *m, alt_arena *a, alt_reader r, alt_buf *err);
void alt_mail_message_free(alt_mail_message *m);

/* Header.Date and Header.AddressList */
int alt_mail_header_date(const alt_textproto_header *h, alt_wall *t, alt_zone *z, alt_buf *err);
int alt_mail_header_address_list(const alt_textproto_header *h, alt_arena *a, alt_str key, alt_mail_address **out, size_t *n,
	alt_buf *err);

#ifdef __cplusplus
}
#endif

#endif
