#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <wchar.h>
#include <locale.h>
#include <stdlib.h>

struct S { int a; long double ld; };
static struct S vs(int n, ...) { va_list ap; va_start(ap, n); struct S s = va_arg(ap, struct S); va_end(ap); return s; }
static long double vld(int n, ...) { va_list ap; va_start(ap, n); long double r = 0; while (n--) { r += va_arg(ap, long double); (void)va_arg(ap, int); } va_end(ap); return r; }
typedef struct { char c[3]; } C3;
static C3 vc3(int n, ...) { va_list ap; va_start(ap, n); C3 r = {{0}}; for (int i = 0; i < n; i++) { C3 t = va_arg(ap, C3); r.c[i % 3] += t.c[0]; } va_end(ap); return r; }
static void va_ptr_pass(va_list *app) { printf("ptr %d\n", va_arg(*app, int)); }
static void vp(int n, ...) { va_list ap; va_start(ap, n); va_ptr_pass(&ap); va_ptr_pass(&ap); va_end(ap); }
struct FS { float a, b, c; };
static float vfs(int n, ...) { va_list ap; va_start(ap, n); float s = 0; for (int i = 0; i < n; i++) { struct FS f = va_arg(ap, struct FS); s += f.a + f.b + f.c; } va_end(ap); return s; }

int main(void) {
    struct S s = vs(1, (struct S){7, 2.5L}); printf("%d %Lg\n", s.a, s.ld);
    printf("%Lg\n", vld(4, 1.0L, 1, 2.0L, 2, 3.0L, 3, 4.5L, 4));
    C3 c = vc3(7, (C3){{1}}, (C3){{2}}, (C3){{3}}, (C3){{4}}, (C3){{5}}, (C3){{6}}, (C3){{7}}); printf("%d %d %d\n", c.c[0], c.c[1], c.c[2]);
    vp(0, 11, 22);
    printf("%g\n", vfs(5, (struct FS){1, 2, 3}, (struct FS){4, 5, 6}, (struct FS){7, 8, 9}, (struct FS){1, 1, 1}, (struct FS){2, 2, 2}));
    setlocale(LC_ALL, "C");
    wchar_t wbuf[32]; swprintf(wbuf, 32, L"%d-%ls", 42, L"wide"); printf("%ls %zu\n", wbuf, wcslen(wbuf));
    char mb[8]; int len = wctomb(mb, L'A'); printf("%d %c\n", len, mb[0]);
    printf("%d\n", wcscmp(L"abc", L"abd") < 0);
    int64_t big = INT64_MIN; printf("%lld %llx\n", (long long)(big / 7), (unsigned long long)big);
    return 0;
}
