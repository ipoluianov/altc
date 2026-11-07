/* Unicode classes and case mapping, after Go's unicode package
   (BSD-3-Clause, see LICENSE-Go); the tables are in tables.c */
#include <alt/unicode.h>
#include <string.h>

extern const alt_unicode_case_range alt__unicode_case_ranges[];
extern const int alt__unicode_ncase_ranges;
extern const int32_t alt__unicode_orbit[][2];
extern const int alt__unicode_norbit;

static bool in_range(const uint32_t *x, uint32_t r) {
	return r >= x[0] && r <= x[1] && (x[2] == 1 || (r - x[0]) % x[2] == 0);
}

bool alt_unicode_is(const alt_unicode_table *t, int32_t r) {
	if (r < 0 || r > ALT_UNICODE_MAX_RUNE)
		return false;
	uint32_t u = (uint32_t)r;
	if (u < 0x100) {
		for (int i = 0; i < t->n && t->r[i][0] <= u; i++)
			if (in_range(t->r[i], u))
				return true;
		return false;
	}
	int lo = t->latin_offset, hi = t->n;
	while (lo < hi) {
		int m = lo + (hi - lo) / 2;
		if (u < t->r[m][0])
			hi = m;
		else if (u > t->r[m][1])
			lo = m + 1;
		else
			return in_range(t->r[m], u);
	}
	return false;
}

const alt_unicode_table *alt_unicode_category(const char *name) {
#define ALT__UNICODE_NAME(t) \
	if (strcmp(name, #t) == 0) \
		return &alt_unicode_##t;
	ALT__UNICODE_TABLES(ALT__UNICODE_NAME)
#undef ALT__UNICODE_NAME
	return 0;
}

/* Latin-1 properties, as Go's properties table */
enum { pC = 1, pP = 2, pN = 4, pS = 8, pZ = 16, pLu = 32, pLl = 64, pp = 128, pg = pp | pZ, pLo = pLl | pLu, pLmask = pLo };

static unsigned latin1(int32_t r) {
	if (r < 0x20 || (r >= 0x7f && r < 0xa0))
		return pC;
	if (r == ' ')
		return pZ | pp;
	if (r == 0xa0)
		return pZ;
	if (r == 0xad)
		return 0; /* soft hyphen: Cf */
	unsigned p = pp;
	if (alt_unicode_is(&alt_unicode_P, r))
		p |= pP;
	else if (alt_unicode_is(&alt_unicode_N, r))
		p |= pN;
	else if (alt_unicode_is(&alt_unicode_S, r))
		p |= pS;
	else if (alt_unicode_is(&alt_unicode_Lu, r))
		p |= pLu;
	else if (alt_unicode_is(&alt_unicode_Ll, r))
		p |= pLl;
	else if (alt_unicode_is(&alt_unicode_L, r))
		p |= pLo;
	return p;
}

bool alt_unicode_is_letter(int32_t r) {
	if ((uint32_t)r < 0x100)
		return latin1(r) & pLmask;
	return alt_unicode_is(&alt_unicode_L, r);
}
bool alt_unicode_is_digit(int32_t r) {
	if ((uint32_t)r < 0x100)
		return r >= '0' && r <= '9';
	return alt_unicode_is(&alt_unicode_Nd, r);
}
bool alt_unicode_is_number(int32_t r) { return alt_unicode_is(&alt_unicode_N, r); }
bool alt_unicode_is_upper(int32_t r) { return alt_unicode_is(&alt_unicode_Lu, r); }
bool alt_unicode_is_lower(int32_t r) { return alt_unicode_is(&alt_unicode_Ll, r); }
bool alt_unicode_is_title(int32_t r) { return alt_unicode_is(&alt_unicode_Lt, r); }
bool alt_unicode_is_mark(int32_t r) { return alt_unicode_is(&alt_unicode_M, r); }
bool alt_unicode_is_punct(int32_t r) { return alt_unicode_is(&alt_unicode_P, r); }
bool alt_unicode_is_symbol(int32_t r) { return alt_unicode_is(&alt_unicode_S, r); }
bool alt_unicode_is_control(int32_t r) { return (uint32_t)r < 0x100 && (latin1(r) & pC); }

bool alt_unicode_is_space(int32_t r) {
	if ((uint32_t)r < 0x100)
		return r == ' ' || (r >= '\t' && r <= '\r') || r == 0x85 || r == 0xa0;
	return alt_unicode_is(&alt_unicode_White_Space, r);
}

static bool graphic_base(int32_t r) {
	return alt_unicode_is(&alt_unicode_L, r) || alt_unicode_is(&alt_unicode_M, r) || alt_unicode_is(&alt_unicode_N, r) ||
	       alt_unicode_is(&alt_unicode_P, r) || alt_unicode_is(&alt_unicode_S, r);
}

bool alt_unicode_is_print(int32_t r) {
	if ((uint32_t)r < 0x100)
		return latin1(r) & pp;
	return graphic_base(r);
}

bool alt_unicode_is_graphic(int32_t r) {
	if ((uint32_t)r < 0x100)
		return latin1(r) & pg;
	return graphic_base(r) || alt_unicode_is(&alt_unicode_Zs, r);
}

enum { UPPER, LOWER, TITLE };
#define UPPER_LOWER (ALT_UNICODE_MAX_RUNE + 1)

static int32_t to(int c, int32_t r) {
	if (r < 0 || r > ALT_UNICODE_MAX_RUNE)
		return r;
	int lo = 0, hi = alt__unicode_ncase_ranges;
	while (lo < hi) {
		int m = lo + (hi - lo) / 2;
		const alt_unicode_case_range *cr = &alt__unicode_case_ranges[m];
		if ((uint32_t)r < cr->lo)
			hi = m;
		else if ((uint32_t)r > cr->hi)
			lo = m + 1;
		else {
			int32_t d = cr->delta[c];
			if (d > ALT_UNICODE_MAX_RUNE)
				/* alternating upper and lower: an even offset from lo is
				   upper, the next one lower (title is upper) */
				return (int32_t)cr->lo + (((r - (int32_t)cr->lo) & ~1) | (c == LOWER));
			return r + d;
		}
	}
	return r;
}

int32_t alt_unicode_to_upper(int32_t r) {
	if (r < 0x80)
		return r >= 'a' && r <= 'z' ? r - 32 : r;
	return to(UPPER, r);
}
int32_t alt_unicode_to_lower(int32_t r) {
	if (r < 0x80)
		return r >= 'A' && r <= 'Z' ? r + 32 : r;
	return to(LOWER, r);
}
int32_t alt_unicode_to_title(int32_t r) {
	if (r < 0x80)
		return r >= 'a' && r <= 'z' ? r - 32 : r;
	return to(TITLE, r);
}

int32_t alt_unicode_simple_fold(int32_t r) {
	if (r < 0 || r > ALT_UNICODE_MAX_RUNE)
		return r;
	int lo = 0, hi = alt__unicode_norbit;
	while (lo < hi) {
		int m = lo + (hi - lo) / 2;
		if (r < alt__unicode_orbit[m][0])
			hi = m;
		else if (r > alt__unicode_orbit[m][0])
			lo = m + 1;
		else
			return alt__unicode_orbit[m][1];
	}
	int32_t l = alt_unicode_to_lower(r);
	return l != r ? l : alt_unicode_to_upper(r);
}
