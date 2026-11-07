/* HPACK (RFC 7541), after Go's golang.org/x/net/http2/hpack (BSD-3-Clause,
   see LICENSE-Go): the same table search, indexing choices and errors. */
#include <alt/errors.h>
#include <alt/net/hpack.h>
#include <alt/sync.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const uint32_t alt__hpack_huff_code[256];
extern const uint8_t alt__hpack_huff_len[256];

/* ---- the static table (Appendix A) ---- */

static const struct {
	const char *name, *value;
} static_table[61] = {
	{":authority", ""}, {":method", "GET"}, {":method", "POST"}, {":path", "/"}, {":path", "/index.html"}, {":scheme", "http"},
	{":scheme", "https"}, {":status", "200"}, {":status", "204"}, {":status", "206"}, {":status", "304"}, {":status", "400"},
	{":status", "404"}, {":status", "500"}, {"accept-charset", ""}, {"accept-encoding", "gzip, deflate"},
	{"accept-language", ""}, {"accept-ranges", ""}, {"accept", ""}, {"access-control-allow-origin", ""}, {"age", ""},
	{"allow", ""}, {"authorization", ""}, {"cache-control", ""}, {"content-disposition", ""}, {"content-encoding", ""},
	{"content-language", ""}, {"content-length", ""}, {"content-location", ""}, {"content-range", ""}, {"content-type", ""},
	{"cookie", ""}, {"date", ""}, {"etag", ""}, {"expect", ""}, {"expires", ""}, {"from", ""}, {"host", ""}, {"if-match", ""},
	{"if-modified-since", ""}, {"if-none-match", ""}, {"if-range", ""}, {"if-unmodified-since", ""}, {"last-modified", ""},
	{"link", ""}, {"location", ""}, {"max-forwards", ""}, {"proxy-authenticate", ""}, {"proxy-authorization", ""},
	{"range", ""}, {"referer", ""}, {"refresh", ""}, {"retry-after", ""}, {"server", ""}, {"set-cookie", ""},
	{"strict-transport-security", ""}, {"transfer-encoding", ""}, {"user-agent", ""}, {"vary", ""}, {"via", ""},
	{"www-authenticate", ""},
};
enum { STATIC_LEN = 61 };

static bool eq(alt_str s, const char *c, size_t n) { return s.n == n && memcmp(s.p, c, n) == 0; }

/* Go's staticTable.search: an exact match, else the last entry of the name */
static uint64_t static_search(const alt_hpack_field *f, bool *exact) {
	uint64_t by_name = 0;
	*exact = false;
	for (int i = 0; i < STATIC_LEN; i++) {
		if (!eq(f->name, static_table[i].name, strlen(static_table[i].name)))
			continue;
		by_name = (uint64_t)i + 1;
		if (!f->sensitive && eq(f->value, static_table[i].value, strlen(static_table[i].value))) {
			*exact = true;
			return (uint64_t)i + 1;
		}
	}
	return by_name;
}

/* ---- the dynamic table ---- */

struct alt__hpack_entry {
	char *name, *value;
	size_t nname, nvalue;
};

static uint32_t entry_size(const struct alt__hpack_entry *e) { return (uint32_t)(e->nname + e->nvalue + 32); }

/* the i-th newest entry (1-based) */
static struct alt__hpack_entry *entry(const alt_hpack_table *t, size_t i) { return &t->e[(t->head + t->n - i) % t->cap]; }

static void evict(alt_hpack_table *t) {
	while (t->size > t->max_size && t->n) {
		struct alt__hpack_entry *e = &t->e[t->head];
		t->size -= entry_size(e);
		free(e->name);
		t->head = (t->head + 1) % t->cap;
		t->n--;
	}
}

static void set_max(alt_hpack_table *t, uint32_t v) {
	t->max_size = v;
	evict(t);
}

static int add(alt_hpack_table *t, alt_str name, alt_str value) {
	if (t->n == t->cap) {
		size_t cap = t->cap ? t->cap * 2 : 16;
		struct alt__hpack_entry *ne = malloc(cap * sizeof *ne);
		if (!ne)
			return -ENOMEM;
		for (size_t i = 0; i < t->n; i++)
			ne[i] = t->e[(t->head + i) % t->cap];
		free(t->e);
		t->e = ne;
		t->cap = cap;
		t->head = 0;
	}
	char *p = malloc(name.n + value.n + 1);
	if (!p)
		return -ENOMEM;
	memcpy(p, name.p, name.n);
	memcpy(p + name.n, value.p, value.n);
	struct alt__hpack_entry *e = &t->e[(t->head + t->n) % t->cap];
	e->name = p, e->nname = name.n, e->value = p + name.n, e->nvalue = value.n;
	t->n++;
	t->size += entry_size(e);
	evict(t);
	return 0;
}

static void table_free(alt_hpack_table *t) {
	for (size_t i = 0; i < t->n; i++)
		free(t->e[(t->head + i) % t->cap].name);
	free(t->e);
	memset(t, 0, sizeof *t);
}

/* the newest exact match, else the newest entry of the name (Go's search) */
static uint64_t dyn_search(const alt_hpack_table *t, const alt_hpack_field *f, bool *exact) {
	uint64_t by_name = 0;
	*exact = false;
	for (size_t i = 1; i <= t->n; i++) {
		const struct alt__hpack_entry *e = entry(t, i);
		if (!eq(f->name, e->name, e->nname))
			continue;
		if (!by_name)
			by_name = i;
		if (!f->sensitive && eq(f->value, e->value, e->nvalue)) {
			*exact = true;
			return i;
		}
	}
	return by_name;
}

/* ---- integers and strings ---- */

static void put_varint(alt_buf *b, int n, uint64_t i, uint8_t first) {
	uint64_t k = ((uint64_t)1 << n) - 1;
	if (i < k) {
		alt_buf_append_byte(b, (int)(first | (uint8_t)i));
		return;
	}
	alt_buf_append_byte(b, (int)(first | (uint8_t)k));
	i -= k;
	for (; i >= 128; i >>= 7)
		alt_buf_append_byte(b, (int)(0x80 | (i & 0x7f)));
	alt_buf_append_byte(b, (int)i);
}

size_t alt_hpack_huffman_encode_len(alt_str s) {
	uint64_t n = 0;
	for (size_t i = 0; i < s.n; i++)
		n += alt__hpack_huff_len[(uint8_t)s.p[i]];
	return (size_t)((n + 7) / 8);
}

int alt_hpack_huffman_append(alt_buf *b, alt_str s) {
	uint64_t x = 0;
	unsigned n = 0;
	for (size_t i = 0; i < s.n; i++) {
		uint8_t c = (uint8_t)s.p[i];
		n += alt__hpack_huff_len[c];
		x = x << (alt__hpack_huff_len[c] % 64) | alt__hpack_huff_code[c];
		if (n >= 32) {
			n %= 32;
			uint32_t y = (uint32_t)(x >> n);
			uint8_t o[4] = {(uint8_t)(y >> 24), (uint8_t)(y >> 16), (uint8_t)(y >> 8), (uint8_t)y};
			alt_buf_append(b, o, 4);
		}
	}
	if (n % 8) {
		/* pad with the start of EOS (all ones) */
		unsigned pad = 8 - n % 8;
		x = (x << pad) | (0xffu >> (n % 8));
		n += pad;
	}
	for (unsigned k = n / 8; k > 0; k--)
		alt_buf_append_byte(b, (int)(uint8_t)(x >> (8 * (k - 1))));
	return b->err;
}

static void put_string(alt_buf *b, alt_str s) {
	size_t h = alt_hpack_huffman_encode_len(s);
	if (h < s.n) {
		put_varint(b, 7, h, 0x80);
		alt_hpack_huffman_append(b, s);
	} else {
		put_varint(b, 7, s.n, 0);
		alt_buf_append(b, s.p, s.n);
	}
}

/* ---- Huffman decoding: Go's tree of 256-way nodes ---- */

enum { MAX_NODES = 64, INTERNAL = 0x8000 };
static uint16_t kids[MAX_NODES][256]; /* 0: none; INTERNAL|node; leaf: 1 + sym + 256 * (bits - 1) */
static int nnodes;
static alt_once tree_once = ALT_ONCE_INIT;

static void build_tree(void *arg) {
	(void)arg;
	nnodes = 1;
	for (int sym = 0; sym < 256; sym++) {
		uint32_t code = alt__hpack_huff_code[sym];
		unsigned len = alt__hpack_huff_len[sym];
		int cur = 0;
		while (len > 8) {
			len -= 8;
			uint8_t i = (uint8_t)(code >> len);
			if (!kids[cur][i])
				kids[cur][i] = (uint16_t)(INTERNAL | nnodes++);
			cur = kids[cur][i] & ~INTERNAL;
		}
		unsigned shift = 8 - len;
		unsigned start = (uint8_t)(code << shift), end = 1u << shift;
		for (unsigned i = start; i < start + end; i++)
			kids[cur][i] = (uint16_t)(1 + sym + 256 * (len - 1));
	}
}

int alt_hpack_huffman_decode(alt_buf *b, const void *data, size_t n, size_t max) {
	alt_once_do(&tree_once, build_tree, 0);
	const uint8_t *v = data;
	int node = 0;
	unsigned long cur = 0;
	unsigned cbits = 0, sbits = 0;
	size_t start = b->n;
	for (size_t k = 0; k < n; k++) {
		cur = cur << 8 | v[k];
		cbits += 8;
		sbits += 8;
		while (cbits >= 8) {
			uint8_t idx = (uint8_t)(cur >> (cbits - 8));
			uint16_t c = kids[node][idx];
			if (!c)
				return -ALT_EHPACK;
			if (!(c & INTERNAL)) {
				if (max && b->n - start == max)
					return -ALT_ETOOLARGE;
				alt_buf_append_byte(b, (c - 1) & 0xff);
				cbits -= ((c - 1) >> 8) + 1;
				node = 0;
				sbits = cbits;
			} else {
				node = c & ~INTERNAL;
				cbits -= 8;
			}
		}
	}
	while (cbits > 0) {
		uint16_t c = kids[node][(uint8_t)(cur << (8 - cbits))];
		if (!c)
			return -ALT_EHPACK;
		if ((c & INTERNAL) || ((unsigned)((c - 1) >> 8) + 1) > cbits)
			break;
		if (max && b->n - start == max)
			return -ALT_ETOOLARGE;
		alt_buf_append_byte(b, (c - 1) & 0xff);
		cbits -= ((c - 1) >> 8) + 1;
		node = 0;
		sbits = cbits;
	}
	if (sbits > 7)
		return -ALT_EHPACK; /* an incomplete symbol or overlong padding */
	unsigned long mask = (1ul << cbits) - 1;
	if ((cur & mask) != mask)
		return -ALT_EHPACK; /* the padding must be a prefix of EOS */
	return b->err;
}

/* ---- the encoder ---- */

void alt_hpack_encoder_init(alt_hpack_encoder *e) {
	memset(e, 0, sizeof *e);
	e->min_size = UINT32_MAX;
	e->max_size_limit = 4096;
	set_max(&e->t, 4096);
}

void alt_hpack_encoder_free(alt_hpack_encoder *e) { table_free(&e->t); }

void alt_hpack_encoder_set_max_table_size(alt_hpack_encoder *e, uint32_t v) {
	if (v > e->max_size_limit)
		v = e->max_size_limit;
	if (v < e->min_size)
		e->min_size = v;
	e->table_size_update = true;
	set_max(&e->t, v);
}

void alt_hpack_encoder_set_max_table_size_limit(alt_hpack_encoder *e, uint32_t v) {
	e->max_size_limit = v;
	if (e->t.max_size > v) {
		e->table_size_update = true;
		set_max(&e->t, v);
	}
}

uint32_t alt_hpack_encoder_max_table_size(const alt_hpack_encoder *e) { return e->t.max_size; }

int alt_hpack_encode(alt_hpack_encoder *e, alt_buf *out, const alt_hpack_field *f) {
	if (e->table_size_update) {
		e->table_size_update = false;
		if (e->min_size < e->t.max_size)
			put_varint(out, 5, e->min_size, 0x20);
		e->min_size = UINT32_MAX;
		put_varint(out, 5, e->t.max_size, 0x20);
	}
	bool exact;
	uint64_t idx = static_search(f, &exact);
	if (!exact) {
		bool dexact;
		uint64_t j = dyn_search(&e->t, f, &dexact);
		if (dexact || (idx == 0 && j != 0)) {
			idx = j + STATIC_LEN;
			exact = dexact;
		}
	}
	if (exact) {
		put_varint(out, 7, idx, 0x80);
		return out->err;
	}
	bool indexing = !f->sensitive && alt_hpack_field_size(f) <= e->t.max_size;
	if (indexing) {
		int rc = add(&e->t, f->name, f->value);
		if (rc < 0)
			return rc;
	}
	uint8_t type = f->sensitive ? 0x10 : indexing ? 0x40 : 0;
	if (idx == 0) {
		alt_buf_append_byte(out, type);
		put_string(out, f->name);
	} else
		put_varint(out, indexing ? 6 : 4, idx, type);
	put_string(out, f->value);
	return out->err;
}

/* ---- the decoder ---- */

void alt_hpack_decoder_init(alt_hpack_decoder *d, uint32_t max_table_size) {
	memset(d, 0, sizeof *d);
	d->t.allowed_max = max_table_size;
	set_max(&d->t, max_table_size);
}

void alt_hpack_decoder_free(alt_hpack_decoder *d) { table_free(&d->t); }
void alt_hpack_decoder_set_max_table_size(alt_hpack_decoder *d, uint32_t v) { set_max(&d->t, v); }
void alt_hpack_decoder_set_allowed_max_table_size(alt_hpack_decoder *d, uint32_t v) { d->t.allowed_max = v; }
const char *alt_hpack_error(const alt_hpack_decoder *d) { return d->err; }

enum { NEED_MORE = 1 };

static int derr(alt_hpack_decoder *d, const char *msg) {
	snprintf(d->err, sizeof d->err, "%s", msg);
	return -ALT_EHPACK;
}

/* Go's readVarInt: NEED_MORE when p ends first */
static int read_varint(alt_hpack_decoder *d, int n, const uint8_t **pp, const uint8_t *end, uint64_t *out) {
	const uint8_t *p = *pp;
	if (p == end)
		return NEED_MORE;
	uint64_t i = *p & ((1u << n) - 1);
	p++;
	if (i < (1u << n) - 1) {
		*out = i;
		*pp = p;
		return 0;
	}
	unsigned m = 0;
	while (p < end) {
		uint8_t b = *p++;
		i += (uint64_t)(b & 127) << m;
		if (!(b & 128)) {
			*out = i;
			*pp = p;
			return 0;
		}
		m += 7;
		if (m >= 63)
			return derr(d, "decoding error: varint integer overflow");
	}
	return NEED_MORE;
}

typedef struct {
	bool huff;
	const uint8_t *p;
	size_t n;
} raw_str;

static int read_string(alt_hpack_decoder *d, const uint8_t **pp, const uint8_t *end, raw_str *s) {
	if (*pp == end)
		return NEED_MORE;
	s->huff = (**pp & 128) != 0;
	uint64_t len;
	const uint8_t *p = *pp;
	int rc = read_varint(d, 7, &p, end, &len);
	if (rc)
		return rc;
	if (d->max_string && len > d->max_string)
		return derr(d, "hpack: string too long");
	if ((uint64_t)(end - p) < len)
		return NEED_MORE;
	s->p = p;
	s->n = (size_t)len;
	*pp = p + len;
	return 0;
}

static int decode_string(alt_hpack_decoder *d, const raw_str *s, alt_arena *a, alt_str *out) {
	if (!s->huff) {
		*out = alt_arena_str(a, (alt_str){(const char *)s->p, s->n});
		return out->p || !s->n ? 0 : -ENOMEM;
	}
	alt_buf b = ALT_BUF_INIT;
	int rc = alt_hpack_huffman_decode(&b, s->p, s->n, d->max_string);
	if (rc == 0) {
		*out = alt_arena_str(a, alt_buf_str(&b));
		if (!out->p && b.n)
			rc = -ENOMEM;
	} else if (rc == -ALT_ETOOLARGE)
		rc = derr(d, "hpack: string too long");
	else if (rc == -ALT_EHPACK)
		rc = derr(d, "hpack: invalid Huffman-encoded data");
	alt_buf_free(&b);
	return rc;
}

static bool at(const alt_hpack_decoder *d, uint64_t i, alt_str *name, alt_str *value) {
	if (i == 0)
		return false;
	if (i <= STATIC_LEN) {
		*name = alt_str_cstr(static_table[i - 1].name);
		*value = alt_str_cstr(static_table[i - 1].value);
		return true;
	}
	if (i > STATIC_LEN + d->t.n)
		return false;
	const struct alt__hpack_entry *e = entry(&d->t, (size_t)(i - STATIC_LEN));
	*name = (alt_str){e->name, e->nname};
	*value = (alt_str){e->value, e->nvalue};
	return true;
}

static int bad_index(alt_hpack_decoder *d, uint64_t i) {
	snprintf(d->err, sizeof d->err, "decoding error: invalid indexed representation index %llu", (unsigned long long)i);
	return -ALT_EHPACK;
}

static int emit(alt_hpack_decoder *d, alt_hpack_emit_fn fn, void *arg, const alt_hpack_field *f) {
	if (d->max_string && (f->name.n > d->max_string || f->value.n > d->max_string))
		return derr(d, "hpack: string too long");
	return fn ? fn(arg, f) : 0;
}

/* one representation: 0, NEED_MORE, or an error */
static int field(alt_hpack_decoder *d, const uint8_t **pp, const uint8_t *end, bool first, alt_arena *a, alt_hpack_emit_fn fn,
	void *arg) {
	const uint8_t *p = *pp;
	uint8_t b = *p;
	uint64_t idx;
	int rc;
	alt_hpack_field f = {0};
	if (b & 128) {
		if ((rc = read_varint(d, 7, &p, end, &idx)))
			return rc;
		alt_str n, v;
		if (!at(d, idx, &n, &v))
			return bad_index(d, idx);
		*pp = p;
		f.name = n, f.value = v;
		return emit(d, fn, arg, &f);
	}
	if ((b & 224) == 32) {
		/* a table size update: only at the start of a block (RFC 7541 4.2) */
		if (!first && d->t.size > 0)
			return derr(d, "decoding error: dynamic table size update MUST occur at the beginning of a header block");
		uint64_t size;
		if ((rc = read_varint(d, 5, &p, end, &size)))
			return rc;
		if (size > d->t.allowed_max)
			return derr(d, "decoding error: dynamic table size update too large");
		set_max(&d->t, (uint32_t)size);
		*pp = p;
		return 0;
	}
	int n;
	bool indexed = false, sensitive = false;
	if ((b & 192) == 64)
		n = 6, indexed = true;
	else if ((b & 240) == 0)
		n = 4;
	else if ((b & 240) == 16)
		n = 4, sensitive = true;
	else
		return derr(d, "decoding error: invalid encoding");
	if ((rc = read_varint(d, n, &p, end, &idx)))
		return rc;
	raw_str rn = {0}, rv;
	alt_str iname = ALT_STR_NULL, ivalue;
	if (idx > 0) {
		if (!at(d, idx, &iname, &ivalue))
			return bad_index(d, idx);
	} else if ((rc = read_string(d, &p, end, &rn)))
		return rc;
	if ((rc = read_string(d, &p, end, &rv)))
		return rc;
	if (idx > 0)
		f.name = alt_arena_str(a, iname); /* the entry may be evicted by the addition */
	else if ((rc = decode_string(d, &rn, a, &f.name)) < 0)
		return rc;
	if ((rc = decode_string(d, &rv, a, &f.value)) < 0)
		return rc;
	*pp = p;
	if (indexed && (rc = add(&d->t, f.name, f.value)) < 0)
		return rc;
	f.sensitive = sensitive;
	return emit(d, fn, arg, &f);
}

int alt_hpack_decode(alt_hpack_decoder *d, const void *block, size_t n, alt_arena *a, alt_hpack_emit_fn fn, void *arg) {
	d->err[0] = 0;
	const uint8_t *p = block, *end = p + n;
	bool first = true;
	while (p < end) {
		int rc = field(d, &p, end, first, a, fn, arg);
		if (rc == NEED_MORE) {
			/* Go's Write keeps the rest for more; a whole block ends here */
			if (d->max_string && (size_t)(end - p) > 2 * (d->max_string + 8))
				return derr(d, "hpack: string too long");
			return derr(d, "decoding error: truncated headers");
		}
		if (rc < 0) {
			if (!d->err[0])
				snprintf(d->err, sizeof d->err, "%s", alt_strerror(rc));
			return rc;
		}
		first = false;
	}
	return 0;
}
