#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

typedef struct { char *buf; size_t len, cap; } SB;
static void sb_grow(SB *s, size_t need) { if (s->len + need + 1 > s->cap) { while (s->len + need + 1 > s->cap) s->cap = s->cap ? s->cap * 2 : 16; s->buf = realloc(s->buf, s->cap); } }
static void sb_puts(SB *s, const char *t) { size_t n = strlen(t); sb_grow(s, n); memcpy(s->buf + s->len, t, n + 1); s->len += n; }
static void sb_printf(SB *s, const char *fmt, ...) { va_list ap, ap2; va_start(ap, fmt); va_copy(ap2, ap); int n = vsnprintf(NULL, 0, fmt, ap); va_end(ap); sb_grow(s, n); vsnprintf(s->buf + s->len, n + 1, fmt, ap2); va_end(ap2); s->len += n; }
static void sb_putc(SB *s, char c) { sb_grow(s, 1); s->buf[s->len++] = c; s->buf[s->len] = 0; }
static char *reverse(char *s) { for (size_t i = 0, j = strlen(s); i + 1 < j; i++, j--) { char t = s[i]; s[i] = s[j - 1]; s[j - 1] = t; } return s; }
static int is_palin(const char *s) { const char *e = s + strlen(s) - 1; while (s < e) { while (s < e && !isalnum((unsigned char)*s)) s++; while (s < e && !isalnum((unsigned char)*e)) e--; if (tolower((unsigned char)*s++) != tolower((unsigned char)*e--)) return 0; } return 1; }
static char *replace_all(const char *s, const char *from, const char *to) { SB o = {0}; size_t fl = strlen(from); const char *p; sb_puts(&o, ""); while ((p = strstr(s, from))) { sb_grow(&o, p - s); memcpy(o.buf + o.len, s, p - s); o.len += p - s; o.buf[o.len] = 0; sb_puts(&o, to); s = p + fl; } sb_puts(&o, s); return o.buf; }
static void caesar(char *s, int k) { for (; *s; s++) { if (islower((unsigned char)*s)) *s = 'a' + (*s - 'a' + k) % 26; else if (isupper((unsigned char)*s)) *s = 'A' + (*s - 'A' + k) % 26; } }
static int levenshtein(const char *a, const char *b) { int la = strlen(a), lb = strlen(b); int d[32][32]; for (int i = 0; i <= la; i++) d[i][0] = i; for (int j = 0; j <= lb; j++) d[0][j] = j;
    for (int i = 1; i <= la; i++) for (int j = 1; j <= lb; j++) { int c = d[i - 1][j - 1] + (a[i - 1] != b[j - 1]); if (d[i - 1][j] + 1 < c) c = d[i - 1][j] + 1; if (d[i][j - 1] + 1 < c) c = d[i][j - 1] + 1; d[i][j] = c; } return d[la][lb]; }

int main(void) {
    SB s = {0};
    for (int i = 0; i < 20; i++) sb_printf(&s, "%d,", i * i);
    sb_puts(&s, " end"); sb_putc(&s, '!'); printf("%s (%zu)\n", s.buf, s.len);
    char r[] = "Hello, World"; printf("%s\n", reverse(r));
    printf("%d %d %d\n", is_palin("A man, a plan, a canal: Panama"), is_palin("hello"), is_palin("No 'x' in Nixon"));
    char *rep = replace_all("the cat sat on the mat with the hat", "the", "a"); printf("%s\n", rep); free(rep);
    rep = replace_all("aaaa", "a", "bb"); printf("%s\n", rep); free(rep);
    char c[] = "Attack at Dawn, xyz"; caesar(c, 3); printf("%s\n", c); caesar(c, 23); printf("%s\n", c);
    printf("%d %d %d\n", levenshtein("kitten", "sitting"), levenshtein("", "abc"), levenshtein("flaw", "lawn"));
    char words[] = "  split   these words\tplease  "; int count = 0; for (char *t = strtok(words, " \t"); t; t = strtok(NULL, " \t")) printf("<%s>", t), count++; printf(" %d\n", count);
    char num[32]; for (long v = -1234567; v != 0; v /= 10) { snprintf(num, sizeof num, "%ld", v); printf("%s ", num); } printf("\n");
    const char *hay = "abcabcabd"; const char *needle = "abd"; printf("%td\n", strstr(hay, needle) - hay);
    char upper[64]; strcpy(upper, "MiXeD CaSe 123"); for (char *p = upper; *p; p++) *p = isupper((unsigned char)*p) ? tolower(*p) : toupper(*p); printf("%s\n", upper);
    free(s.buf);
    return 0;
}
