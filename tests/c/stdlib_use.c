#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>

static int cmp_int(const void *a, const void *b) { int x = *(const int *)a, y = *(const int *)b; return (x > y) - (x < y); }
static int cmp_str(const void *a, const void *b) { return strcmp(*(const char *const *)a, *(const char *const *)b); }
struct rec { char name[8]; int score; };
static int cmp_rec(const void *a, const void *b) {
    const struct rec *x = a, *y = b; if (x->score != y->score) return y->score - x->score; return strcmp(x->name, y->name);
}

int main(void) {
    int arr[] = {5, -3, 9, 0, 12, -7, 3, 3, 100, -100};
    size_t n = sizeof arr / sizeof *arr;
    qsort(arr, n, sizeof *arr, cmp_int);
    for (size_t i = 0; i < n; i++) printf("%d ", arr[i]); printf("\n");
    int key = 9; int *found = bsearch(&key, arr, n, sizeof *arr, cmp_int); printf("%td\n", found - arr);
    key = 4; printf("%p\n", bsearch(&key, arr, n, sizeof *arr, cmp_int));
    const char *words[] = {"pear", "apple", "fig", "banana", "cherry"};
    qsort(words, 5, sizeof *words, cmp_str); for (int i = 0; i < 5; i++) printf("%s ", words[i]); printf("\n");
    struct rec recs[] = {{"bob", 5}, {"amy", 7}, {"cat", 5}, {"dan", 9}};
    qsort(recs, 4, sizeof *recs, cmp_rec); for (int i = 0; i < 4; i++) printf("%s:%d ", recs[i].name, recs[i].score); printf("\n");
    char *end;
    printf("%ld %ld %ld\n", strtol("  -123abc", &end, 10), strtol("0x1F", NULL, 16), strtol("777", NULL, 8)); printf("[%s]\n", end);
    printf("%ld\n", strtol("0x10", NULL, 0));
    printf("%lu %llu\n", strtoul("4294967295", NULL, 10), strtoull("18446744073709551615", NULL, 10));
    errno = 0; long ov = strtol("99999999999999999999", NULL, 10); printf("%ld %d\n", ov, errno == ERANGE);
    printf("%g %g %g\n", strtod("3.5e2xyz", &end), strtod("-0x1p4", NULL), strtod("inf", NULL)); printf("[%s]\n", end);
    printf("%f %Lf\n", strtof("1.25", NULL), strtold("2.5", NULL));
    printf("%d %ld %lld %g\n", atoi("42"), atol("-7"), atoll("123456789012"), atof("1.5"));
    printf("%d %d %ld\n", abs(-5), abs(INT_MIN + 1), labs(-7L));
    div_t dv = div(17, 5); ldiv_t ldv = ldiv(-17L, 5L); lldiv_t lldv = lldiv(1000000000000LL, 7LL);
    printf("%d %d %ld %ld %lld %lld\n", dv.quot, dv.rem, ldv.quot, ldv.rem, lldv.quot, lldv.rem);
    int *dyn = malloc(10 * sizeof *dyn); for (int i = 0; i < 10; i++) dyn[i] = i;
    dyn = realloc(dyn, 1000 * sizeof *dyn); for (int i = 10; i < 1000; i++) dyn[i] = i;
    long s = 0; for (int i = 0; i < 1000; i++) s += dyn[i]; printf("%ld\n", s); free(dyn);
    int *cz = calloc(50, sizeof(int)); int z = 0; for (int i = 0; i < 50; i++) z |= cz[i]; printf("%d\n", z); free(cz);
    char buf[64] = "Hello, World";
    printf("%zu %d %d %d\n", strlen(buf), strcmp("abc", "abd") < 0, strncmp("abcx", "abcy", 3), memcmp("a\0b", "a\0c", 3) < 0);
    printf("%s %s\n", strchr(buf, 'o'), strrchr(buf, 'o'));
    printf("%s %p\n", strstr(buf, "Wor"), (void *)strstr(buf, "xyz"));
    strcat(buf, "!!"); printf("%s\n", buf);
    strncat(buf, "abcdef", 3); printf("%s\n", buf);
    char cpy[64]; strncpy(cpy, "short", sizeof cpy); printf("%s\n", cpy);
    memmove(buf + 2, buf, 5); printf("%s\n", buf);
    memset(cpy, 'x', 3); printf("%s\n", cpy);
    printf("%zu %zu\n", strspn("aabbcd", "ab"), strcspn("hello world", " "));
    char tok[] = "a,b;;c,d"; for (char *t = strtok(tok, ",;"); t; t = strtok(NULL, ",;")) printf("[%s]", t); printf("\n");
    char *dup = strdup("duplicate"); printf("%s\n", dup); free(dup);
    printf("%s\n", strerror(EDOM) ? "has strerror" : "none");
    for (const char *p = "aZ3 \t!x"; *p; p++) printf("%d%d%d%d%d%d ", !!isalpha(*p), !!isdigit(*p), !!isspace(*p), !!isupper(*p), !!ispunct(*p), !!isalnum(*p));
    printf("\n%c%c%c%c\n", toupper('a'), tolower('Q'), toupper('5'), tolower('z'));
    printf("%d %d\n", isxdigit('f'), !!isprint('\n'));
    printf("%" PRId64 " %" PRIu64 " %" PRIx64 " %" PRIi32 " %" PRIu8 "\n", INT64_MIN, UINT64_MAX, (uint64_t)0xdeadbeefcafeULL, INT32_MAX, (uint8_t)200);
    printf("%d %d %u %d %lld %llu\n", INT8_MIN, INT16_MAX, UINT32_MAX, INT_MIN, LLONG_MAX, ULLONG_MAX);
    printf("%ld %lu %d %d\n", (long)INTPTR_MAX, (unsigned long)SIZE_MAX, SCHAR_MIN, UCHAR_MAX);
    printf("%d %d %d\n", (int)sizeof(intmax_t), (int)sizeof(int_least8_t), (int)sizeof(int_fast16_t));
    printf("%" SCNd32 "\n", 0);
    intmax_t im = INTMAX_MAX; printf("%jd %zu %td\n", im, sizeof(size_t), (ptrdiff_t)-5);
    bool b = true; printf("%d %d %d\n", b, false, (int)sizeof(bool));
    struct off { char a; long b; char c[3]; int d; }; printf("%zu %zu %zu\n", offsetof(struct off, b), offsetof(struct off, c), offsetof(struct off, d));
    assert(1 + 1 == 2);
    time_t t0 = 86400 * 365 + 3600 * 5 + 61; struct tm *tm = gmtime(&t0);
    printf("%d-%02d-%02d %02d:%02d:%02d wday=%d yday=%d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_wday, tm->tm_yday);
    char tbuf[64]; strftime(tbuf, sizeof tbuf, "%Y/%m/%d %H-%M-%S %a %b", tm); printf("%s\n", tbuf);
    struct tm mk = {.tm_year = 100, .tm_mon = 1, .tm_mday = 29, .tm_hour = 12}; time_t back = timegm(&mk); printf("%lld\n", (long long)back);
    srand(1); int r1 = rand(); srand(1); printf("%d\n", r1 == rand());
    char *env = getenv("ALTC_SURELY_UNSET_VAR"); printf("%p\n", (void *)env);
    return 0;
}
