// GNU vector extensions: element-wise operators, comparisons, subscripts
#include <stdio.h>
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef unsigned v4su __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef unsigned char v16qu __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

static v4sf axpy(float a, v4sf x, v4sf y) { return a * x + y; }
static v4si mix(v4si a, v4si b) { return (a + b) * 3 - (a ^ b) + (a << 2) - (b >> 1); }
static v4si cmp(v4si a, v4si b) { return (a < b) + (a == b) * 2 + (a >= b) * 4; }
static v4si ucmp(v4su a, v4su b) { return a > b; }
static v8hi sh(v8hi a, int n) { return (a << n) | (a >> 1) ; }

int main(void) {
    v4sf x = {1, 2, 3, 4}, y = {0.5f, -1, 8, 0};
    v4sf r = axpy(2, x, y);
    printf("%g %g %g %g\n", r[0], r[1], r[2], r[3]);
    v4si a = {1, -2, 30, 40}, b = {5, -2, -7, 100};
    v4si m = mix(a, b), c = cmp(a, b);
    for (int i = 0; i < 4; i++) printf("%d %d ", m[i], c[i]);
    printf("\n");
    v4su ua = {1, 0x80000000u, 5, 0xffffffffu}, ub = {2, 1, 5, 0};
    v4si u = ucmp(ua, ub);
    printf("%d %d %d %d\n", u[0], u[1], u[2], u[3]);
    v8hi h = {1, 2, 3, -4, 5, 6, 7, -32768};
    v8hi hs = sh(h, 3);
    for (int i = 0; i < 8; i++) printf("%d ", hs[i]);
    printf("\n");
    v2df d = {1.5, -2.25};
    d = d * d - d / 2;
    printf("%g %g\n", d[0], d[1]);
    v4sf neg = -x;
    v4si inv = ~a;
    printf("%g %d\n", neg[2], inv[1]);
    v4sf s = x;
    s[2] = 42;
    v4sf t = s + (v4sf){1, 1, 1, 1};
    printf("%g %g sizeof=%zu align=%zu\n", t[2], t[3], sizeof(v4sf), _Alignof(v8sf));
    v2di q = {1, -1};
    q = q + q;
    printf("%lld %lld\n", q[0], q[1]);
    float acc[4] = {0};
    for (int i = 0; i < 1000; i++) { v4sf z = {i, i * 2, i * 3, 1}; v4sf w = *(v4sf *)acc + z * 0.5f; *(v4sf *)acc = w; }
    printf("%g %g %g %g\n", acc[0], acc[1], acc[2], acc[3]);
    return 0;
}
