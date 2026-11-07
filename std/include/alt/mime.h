/* AltC standard library: MIME types (Go's mime) */
#ifndef ALT_MIME_H
#define ALT_MIME_H

#include <alt/str.h>

#ifdef __cplusplus
extern "C" {
#endif

/* the type of a file extension (".html", case-insensitive): Go's
   built-in table, then /etc/mime.types; NULL when unknown */
const char *alt_mime_type_by_extension(alt_str ext);

#ifdef __cplusplus
}
#endif

#endif
