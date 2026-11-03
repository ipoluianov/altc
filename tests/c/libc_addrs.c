#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>

static const struct { const char *name; double (*fn)(double); } mathfns[] = {{"sin", sin}, {"cos", cos}, {"sqrt", sqrt}, {"exp", exp}, {"fabs", fabs}};
static void *(*alloc_fn)(size_t) = malloc;
static void (*free_fn)(void *) = free;
static int (*cmp_fn)(const char *, const char *) = strcmp;
static size_t (*len_fn)(const char *) = strlen;
static FILE **out_stream = &stdout;
static int (*classify[])(int) = {isalpha, isdigit, isspace, isupper};
static char *(*dupper)(const char *) = strdup;
int (*global_puts)(const char *) = puts;
extern char **environ;
static char ***env_ptr = &environ;

int main(void) {
    for (int i = 0; i < 5; i++) printf("%s(0.5) = %.6f\n", mathfns[i].name, mathfns[i].fn(0.5));
    char *p = alloc_fn(16); strcpy(p, "allocated"); printf("%s %zu\n", p, len_fn(p)); free_fn(p);
    printf("%d %d\n", cmp_fn("a", "b") < 0, cmp_fn == strcmp);
    fprintf(*out_stream, "via stream pointer\n");
    for (int i = 0; i < 4; i++) printf("%d", !!classify[i]('A')); printf("\n");
    char *d = dupper("dup"); global_puts(d); free(d);
    printf("%d\n", global_puts == puts);
    printf("%d\n", *env_ptr == environ);
    int *ep = &errno; errno = 33; printf("%d\n", *ep);
    FILE *streams[] = {stdout, stderr}; fprintf(streams[0], "stdout ok\n"); fflush(stdout);
    void (*qs)(void *, size_t, size_t, int (*)(const void *, const void *)) = qsort; printf("%d\n", qs != NULL);
    double (*pw)(double, double) = &pow; printf("%g\n", pw(2, 10));
    return 0;
}
