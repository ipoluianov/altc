/* Structured logging, formats after Go's log/slog (TextHandler,
   JSONHandler; BSD-3-Clause, see LICENSE-Go) */
#include <alt/encoding/json.h>
#include <alt/log.h>
#include <alt/strconv.h>
#include <alt/sync.h>
#include <alt/unicode.h>
#include <alt/unicode/utf8.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

struct alt_logger {
	alt_logger_config cfg;
	alt_mutex *mu;          /* shared with the loggers made by with */
	alt_mutex own_mu;
	alt_log_attr *attrs;    /* added to every record (keys and strings copied) */
	size_t nattrs;
	char *strings;          /* the copies */
	_Atomic int level;
};

static int stderr_fd = 2;

int alt_logger_new(alt_logger **out, const alt_logger_config *cfg) {
	alt_logger *l = calloc(1, sizeof *l);
	if (!l)
		return -ENOMEM;
	if (cfg)
		l->cfg = *cfg;
	if (!l->cfg.out.write)
		l->cfg.out = alt_fd_writer(&stderr_fd);
	l->own_mu = (alt_mutex)ALT_MUTEX_INIT;
	l->mu = &l->own_mu;
	atomic_store(&l->level, l->cfg.level);
	*out = l;
	return 0;
}

void alt_logger_free(alt_logger *l) {
	if (!l)
		return;
	free(l->attrs);
	free(l->strings);
	free(l);
}

int alt_logger_with(alt_logger **out, alt_logger *parent, const alt_log_attr *attrs, size_t n) {
	alt_logger *l = calloc(1, sizeof *l);
	if (!l)
		return -ENOMEM;
	l->cfg = parent->cfg;
	l->mu = parent->mu;
	atomic_store(&l->level, atomic_load(&parent->level));
	size_t total = parent->nattrs + n;
	l->attrs = calloc(total ? total : 1, sizeof *l->attrs);
	/* the strings of the new attributes are copied */
	size_t bytes = 0;
	for (size_t i = 0; i < n; i++)
		bytes += strlen(attrs[i].key) + 1 + (attrs[i].kind == ALT_LOGV_STRING ? attrs[i].s.n + 1 : 0);
	for (size_t i = 0; i < parent->nattrs; i++)
		bytes += strlen(parent->attrs[i].key) + 1 + (parent->attrs[i].kind == ALT_LOGV_STRING ? parent->attrs[i].s.n + 1 : 0);
	l->strings = malloc(bytes ? bytes : 1);
	if (!l->attrs || !l->strings) {
		alt_logger_free(l);
		return -ENOMEM;
	}
	char *p = l->strings;
	for (size_t i = 0; i < total; i++) {
		alt_log_attr a = i < parent->nattrs ? parent->attrs[i] : attrs[i - parent->nattrs];
		size_t kl = strlen(a.key);
		memcpy(p, a.key, kl + 1);
		a.key = p;
		p += kl + 1;
		if (a.kind == ALT_LOGV_STRING) {
			memcpy(p, a.s.p, a.s.n);
			p[a.s.n] = 0;
			a.s.p = p;
			p += a.s.n + 1;
		}
		l->attrs[i] = a;
	}
	l->nattrs = total;
	*out = l;
	return 0;
}

void alt_logger_set_level(alt_logger *l, int level) { atomic_store(&l->level, level); }
bool alt_logger_enabled(const alt_logger *l, int level) { return level >= atomic_load(&((alt_logger *)l)->level); }

static alt_once default_once = ALT_ONCE_INIT;
static alt_logger *default_logger;

static void make_default(void *arg) {
	(void)arg;
	alt_logger_new(&default_logger, 0);
}

alt_logger *alt_log_default(void) {
	alt_once_do(&default_once, make_default, 0);
	return default_logger;
}

static void level_name(alt_buf *b, int level) {
	const char *base;
	int delta;
	if (level < ALT_LOG_INFO)
		base = "DEBUG", delta = level - ALT_LOG_DEBUG;
	else if (level < ALT_LOG_WARN)
		base = "INFO", delta = level - ALT_LOG_INFO;
	else if (level < ALT_LOG_ERROR)
		base = "WARN", delta = level - ALT_LOG_WARN;
	else
		base = "ERROR", delta = level - ALT_LOG_ERROR;
	alt_buf_append_cstr(b, base);
	if (delta)
		alt_buf_printf(b, "%+d", delta);
}

/* slog's needsQuoting: empty, space, '=', '"', controls, and runes
   that are invalid, spaces or not printable */
static bool needs_quoting(alt_str s) {
	if (s.n == 0)
		return true;
	for (size_t i = 0; i < s.n;) {
		unsigned char c = (unsigned char)s.p[i];
		if (c < 0x80) {
			if (c <= ' ' || c == '=' || c == '"')
				return true;
			i++;
			continue;
		}
		int32_t r;
		size_t k = alt_utf8_decode(s.p + i, s.n - i, &r);
		if (r == ALT_UTF8_RUNE_ERROR || alt_unicode_is_space(r) || !alt_unicode_is_print(r))
			return true;
		i += k;
	}
	return false;
}

static void text_str(alt_buf *b, alt_str s) {
	if (needs_quoting(s))
		alt_quote(b, s);
	else
		alt_buf_append_str(b, s);
}

/* slog's appendEscapedJSONString: no HTML escaping, U+2028 and U+2029
   escaped, invalid UTF-8 as U+FFFD (as Go 1.27 writes it) */
static void json_str(alt_buf *b, alt_str s) {
	static const char hex[] = "0123456789abcdef";
	alt_buf_append_byte(b, '"');
	for (size_t i = 0; i < s.n;) {
		unsigned char c = (unsigned char)s.p[i];
		if (c < 0x80) {
			i++;
			if (c >= ' ' && c != '"' && c != '\\') {
				alt_buf_append_byte(b, c);
				continue;
			}
			alt_buf_append_byte(b, '\\');
			switch (c) {
			case '\\': case '"': alt_buf_append_byte(b, c); break;
			case '\n': alt_buf_append_byte(b, 'n'); break;
			case '\r': alt_buf_append_byte(b, 'r'); break;
			case '\t': alt_buf_append_byte(b, 't'); break;
			default:
				alt_buf_append_cstr(b, "u00");
				alt_buf_append_byte(b, hex[c >> 4]);
				alt_buf_append_byte(b, hex[c & 15]);
			}
			continue;
		}
		int32_t r;
		size_t k = alt_utf8_decode(s.p + i, s.n - i, &r);
		if (r == ALT_UTF8_RUNE_ERROR && k == 1)
			alt_buf_append_cstr(b, "\xef\xbf\xbd");
		else if (r == 0x2028 || r == 0x2029)
			alt_buf_printf(b, "\\u%04x", (unsigned)r);
		else
			alt_buf_append(b, s.p + i, k);
		i += k;
	}
	alt_buf_append_byte(b, '"');
}

static void value_text(alt_buf *b, const alt_log_attr *a, bool json) {
	switch (a->kind) {
	case ALT_LOGV_STRING:
		if (json)
			json_str(b, a->s);
		else
			text_str(b, a->s);
		return;
	case ALT_LOGV_INT: alt_buf_append_int(b, a->i); return;
	case ALT_LOGV_UINT: alt_buf_append_uint(b, a->u); return;
	case ALT_LOGV_FLOAT:
		if (json) { /* as json.Marshal */
			alt_json_enc e;
			alt_json_enc_init(&e, b);
			alt_json_number(&e, a->f);
			if (e.err)
				alt_buf_append_cstr(b, "null");
		} else
			alt_format_float(b, a->f, 'g', -1);
		return;
	case ALT_LOGV_BOOL: alt_buf_append_cstr(b, a->b ? "true" : "false"); return;
	case ALT_LOGV_DUR:
		if (json)
			alt_buf_append_int(b, a->d); /* nanoseconds, as slog's JSON */
		else
			alt_dur_format(b, a->d);
		return;
	case ALT_LOGV_ERR: {
		alt_str s = alt_str_cstr(alt_strerror(a->err));
		if (json)
			json_str(b, s);
		else
			text_str(b, s);
		return;
	}
	}
}

static void attr(alt_buf *b, const alt_log_attr *a, bool json, bool *first) {
	if (json) {
		alt_buf_append_byte(b, ',');
		json_str(b, alt_str_cstr(a->key));
		alt_buf_append_byte(b, ':');
	} else {
		if (!*first)
			alt_buf_append_byte(b, ' ');
		text_str(b, alt_str_cstr(a->key));
		alt_buf_append_byte(b, '=');
	}
	*first = false;
	value_text(b, a, json);
}

void alt_log_attrs(alt_logger *l, int level, const char *msg, const alt_log_attr *attrs, size_t n) {
	if (!l)
		l = alt_log_default();
	if (!l || level < atomic_load(&l->level))
		return;
	bool json = l->cfg.format == ALT_LOG_JSON, first = true;
	alt_buf b = ALT_BUF_INIT;
	if (json)
		alt_buf_append_byte(&b, '{');
	if (!l->cfg.no_time) {
		alt_wall now = alt_wall_now();
		alt_zone z;
		if (l->cfg.utc)
			z = alt_zone_utc;
		else
			alt_zone_local(now, &z);
		alt_buf_append_cstr(&b, json ? "\"time\":\"" : "time=");
		alt_wall_format(&b, now, &z, "2006-01-02T15:04:05.000Z07:00");
		if (json)
			alt_buf_append_cstr(&b, "\",");
		else
			alt_buf_append_byte(&b, ' ');
	}
	alt_buf_append_cstr(&b, json ? "\"level\":\"" : "level=");
	level_name(&b, level);
	if (json) {
		alt_buf_append_cstr(&b, "\",\"msg\":");
		json_str(&b, alt_str_cstr(msg ? msg : ""));
	} else {
		alt_buf_append_cstr(&b, " msg=");
		text_str(&b, alt_str_cstr(msg ? msg : ""));
	}
	first = false;
	for (size_t i = 0; i < l->nattrs; i++)
		attr(&b, &l->attrs[i], json, &first);
	for (size_t i = 0; i < n; i++)
		attr(&b, &attrs[i], json, &first);
	alt_buf_append_cstr(&b, json ? "}\n" : "\n");
	if (!b.err) {
		alt_mutex_lock(l->mu);
		alt_write_all(l->cfg.out, b.p, b.n);
		alt_mutex_unlock(l->mu);
	}
	alt_buf_free(&b);
}

void alt_logf(alt_logger *l, int level, const char *fmt, ...) {
	if (!l)
		l = alt_log_default();
	if (!l || level < atomic_load(&l->level))
		return;
	alt_buf b = ALT_BUF_INIT;
	va_list ap;
	va_start(ap, fmt);
	alt_buf_vprintf(&b, fmt, ap);
	va_end(ap);
	alt_log_attrs(l, level, alt_buf_cstr(&b), 0, 0);
	alt_buf_free(&b);
}
