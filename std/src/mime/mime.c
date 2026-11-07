/* File extensions to MIME types: Go's built-in table (mime/type.go,
   BSD-3-Clause, see LICENSE-Go), then the system's /etc/mime.types */
#include <alt/mime.h>
#include <alt/sync.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const struct {
	const char *ext, *type;
} builtin[] = {
	{".avif", "image/avif"}, {".css", "text/css; charset=utf-8"}, {".gif", "image/gif"}, {".htm", "text/html; charset=utf-8"},
	{".html", "text/html; charset=utf-8"}, {".jpeg", "image/jpeg"}, {".jpg", "image/jpeg"},
	{".js", "text/javascript; charset=utf-8"}, {".json", "application/json"}, {".mjs", "text/javascript; charset=utf-8"},
	{".pdf", "application/pdf"}, {".png", "image/png"}, {".svg", "image/svg+xml"}, {".wasm", "application/wasm"},
	{".webp", "image/webp"}, {".xml", "text/xml; charset=utf-8"},
};

typedef struct {
	char *ext, *type;
} entry;

static entry *sys;
static size_t nsys;
static alt_once loaded = ALT_ONCE_INIT;

static void load(void *arg) {
	(void)arg;
	FILE *f = fopen("/etc/mime.types", "re");
	if (!f)
		return;
	char line[1024];
	size_t cap = 0;
	while (fgets(line, sizeof line, f)) {
		char *hash = strchr(line, '#');
		if (hash)
			*hash = 0;
		alt_str rest = alt_str_cstr(line), type, ext;
		if (!alt_str_fields_next(&rest, &type))
			continue;
		while (alt_str_fields_next(&rest, &ext)) {
			if (nsys == cap) {
				cap = cap ? cap * 2 : 256;
				entry *n = realloc(sys, cap * sizeof *n);
				if (!n)
					goto out;
				sys = n;
			}
			char *e = malloc(ext.n + 2);
			/* text types get a charset, as Go's setExtensionType does */
			bool text = alt_str_has_prefix(type, ALT_STR("text/")) && alt_str_index(type, ALT_STR("charset")) < 0;
			char *t = malloc(type.n + (text ? 16 : 1));
			if (t) {
				memcpy(t, type.p, type.n);
				strcpy(t + type.n, text ? "; charset=utf-8" : "");
			}
			if (!e || !t) {
				free(e);
				free(t);
				goto out;
			}
			e[0] = '.';
			memcpy(e + 1, ext.p, ext.n);
			e[ext.n + 1] = 0;
			alt_ascii_lower(e, ext.n + 1);
			sys[nsys++] = (entry){e, t};
		}
	}
out:
	fclose(f);
}

const char *alt_mime_type_by_extension(alt_str ext) {
	for (size_t i = 0; i < sizeof builtin / sizeof *builtin; i++)
		if (alt_str_eq_fold(ext, alt_str_cstr(builtin[i].ext)))
			return builtin[i].type;
	alt_once_do(&loaded, load, 0);
	for (size_t i = 0; i < nsys; i++)
		if (alt_str_eq_fold(ext, alt_str_cstr(sys[i].ext)))
			return sys[i].type;
	return 0;
}
