/* PEM, translated from Go's encoding/pem (BSD-3-Clause, see LICENSE-Go) */
#include <alt/encoding/base64.h>
#include <alt/encoding/pem.h>
#include <string.h>

static const char pem_start[] = "\n-----BEGIN ", pem_end[] = "\n-----END ", pem_eol[] = "-----";

static ssize_t index_of(alt_str s, const char *sub) {
	size_t n = strlen(sub);
	if (n > s.n)
		return -1;
	for (size_t i = 0; i + n <= s.n; i++)
		if (memcmp(s.p + i, sub, n) == 0)
			return (ssize_t)i;
	return -1;
}

static ssize_t last_index_of(alt_str s, const char *sub) {
	size_t n = strlen(sub);
	if (n > s.n)
		return -1;
	for (size_t i = s.n - n + 1; i-- > 0;)
		if (memcmp(s.p + i, sub, n) == 0)
			return (ssize_t)i;
	return -1;
}

/* Go's getLine: the line without \r\n and trailing spaces and tabs */
static alt_str get_line(alt_str data, alt_str *rest, size_t *consumed) {
	ssize_t i = alt_str_index_byte(data, '\n');
	size_t j;
	if (i < 0) {
		i = (ssize_t)data.n;
		j = data.n;
	} else {
		j = (size_t)i + 1;
		if (i > 0 && data.p[i - 1] == '\r')
			i--;
	}
	alt_str line = alt_str_slice(data, 0, (size_t)i);
	while (line.n && (line.p[line.n - 1] == ' ' || line.p[line.n - 1] == '\t'))
		line.n--;
	if (rest)
		*rest = alt_str_slice(data, j, data.n);
	if (consumed)
		*consumed = j;
	return line;
}

static bool has_suffix(alt_str s, const char *x) { return alt_str_has_suffix(s, alt_str_cstr(x)); }

bool alt_pem_decode(alt_arena *a, alt_str *data, alt_pem_block *p) {
	alt_str rest = *data;
	ssize_t end_trailer = 0;
	for (;;) {
		if (end_trailer < 0 || (size_t)end_trailer > rest.n)
			return false;
		rest = alt_str_slice(rest, (size_t)end_trailer, rest.n);
		ssize_t end = index_of(rest, pem_end);
		if (end < 0)
			return false;
		end_trailer = end + (ssize_t)strlen(pem_end);
		ssize_t begin = last_index_of(alt_str_slice(rest, 0, (size_t)end), pem_start + 1);
		if (begin < 0 || (begin > 0 && rest.p[begin - 1] != '\n'))
			continue;
		size_t skip = (size_t)begin + strlen(pem_start) - 1;
		rest = alt_str_slice(rest, skip, rest.n);
		end -= (ssize_t)skip;
		end_trailer -= (ssize_t)skip;
		size_t consumed;
		alt_str type = get_line(rest, &rest, &consumed);
		end -= (ssize_t)consumed;
		end_trailer -= (ssize_t)consumed;
		if (!has_suffix(type, pem_eol))
			continue;
		type.n -= strlen(pem_eol);
		p->type = type;
		p->headers = 0;
		p->nheaders = 0;
		for (;;) {
			if (rest.n == 0)
				return false;
			alt_str next;
			alt_str line = get_line(rest, &next, &consumed);
			alt_str key, val;
			if (!alt_str_cut_byte(line, ':', &key, &val))
				break;
			alt_pem_header h = {alt_str_trim_space(key), alt_str_trim_space(val)};
			/* a map in Go: a repeated key replaces the value */
			size_t k;
			for (k = 0; k < p->nheaders; k++)
				if (alt_str_eq(p->headers[k].key, h.key))
					break;
			if (k == p->nheaders) {
				alt_pem_header *nh = alt_arena_realloc(a, p->headers, k * sizeof *nh, (k + 1) * sizeof *nh);
				if (!nh)
					return false;
				p->headers = nh;
				p->nheaders++;
			}
			p->headers[k] = h;
			rest = next;
			end -= (ssize_t)consumed;
			end_trailer -= (ssize_t)consumed;
		}
		if (p->nheaders > 0 && end < 0)
			continue;
		if (end_trailer < 0 || (size_t)end_trailer > rest.n)
			continue;
		alt_str trailer = alt_str_slice(rest, (size_t)end_trailer, rest.n);
		size_t tl = type.n + strlen(pem_eol);
		if (trailer.n < tl)
			continue;
		alt_str rest_of_end = alt_str_slice(trailer, tl, trailer.n);
		trailer.n = tl;
		if (!alt_str_has_prefix(trailer, type) || !has_suffix(trailer, pem_eol))
			continue;
		if (get_line(rest_of_end, 0, 0).n != 0)
			continue;
		p->bytes = alt_str_make("", 0);
		if (end > 0) {
			alt_buf clean = ALT_BUF_INIT, dec = ALT_BUF_INIT;
			for (ssize_t i = 0; i < end; i++)
				if (rest.p[i] != ' ' && rest.p[i] != '\t')
					alt_buf_append_byte(&clean, rest.p[i]);
			int rc = alt_base64_decode(&alt_base64_std, &dec, alt_buf_str(&clean));
			alt_buf_free(&clean);
			if (rc < 0) {
				alt_buf_free(&dec);
				continue;
			}
			p->bytes = alt_arena_str(a, alt_buf_str(&dec));
			alt_buf_free(&dec);
		}
		ssize_t at = end + (ssize_t)strlen(pem_end) - 1;
		alt_str after = at < 0 ? rest : alt_str_slice(rest, (size_t)at, rest.n);
		get_line(after, &after, 0);
		*data = after;
		return true;
	}
}

int alt_pem_encode(alt_buf *b, const char *type, const alt_pem_header *h, size_t nh, const void *data, size_t n) {
	alt_buf_printf(b, "-----BEGIN %s-----\n", type);
	if (nh) {
		for (size_t i = 0; i < nh; i++)
			if (alt_str_eq_cstr(h[i].key, "Proc-Type"))
				alt_buf_printf(b, "Proc-Type: %.*s\n", (int)h[i].value.n, h[i].value.p);
		/* the others sorted by key, as Go */
		bool done[64] = {0};
		for (size_t k = 0; k < nh && k < 64; k++) {
			size_t best = nh;
			for (size_t i = 0; i < nh && i < 64; i++) {
				if (done[i] || alt_str_eq_cstr(h[i].key, "Proc-Type"))
					continue;
				if (best == nh || alt_str_cmp(h[i].key, h[best].key) < 0)
					best = i;
			}
			if (best == nh)
				break;
			done[best] = true;
			alt_buf_printf(b, "%.*s: %.*s\n", (int)h[best].key.n, h[best].key.p, (int)h[best].value.n, h[best].value.p);
		}
		alt_buf_append_byte(b, '\n');
	}
	alt_buf enc = ALT_BUF_INIT;
	alt_base64_encode(&alt_base64_std, &enc, data, n);
	for (size_t i = 0; i < enc.n; i += 64) {
		size_t k = enc.n - i < 64 ? enc.n - i : 64;
		alt_buf_append(b, enc.p + i, k);
		alt_buf_append_byte(b, '\n');
	}
	alt_buf_free(&enc);
	alt_buf_printf(b, "-----END %s-----\n", type);
	return b->err;
}
