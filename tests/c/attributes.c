#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

struct __attribute__((packed)) P1 { char c; int i; short s; };
struct P2 { char c; int i __attribute__((packed)); char d; };
#pragma pack(push, 1)
struct P3 { char c; double d; int i; };
#pragma pack(pop)
#pragma pack(2)
struct P4 { char c; int i; };
#pragma pack()
struct A1 { char c; } __attribute__((aligned(16)));
struct A2 { char c; int x __attribute__((aligned(8))); };
typedef int aligned_int __attribute__((aligned(16)));
struct A3 { char c; aligned_int ai; };
struct __attribute__((packed)) PB { unsigned a : 3; unsigned b : 7; unsigned c : 9; };
struct Empty {};
struct WithEmpty { int a; struct Empty e; int b; };

static int order;
__attribute__((constructor)) static void ctor1(void) { printf("ctor1 %d\n", order++); }
__attribute__((constructor(200))) static void ctor_prio(void) { printf("ctor_prio %d\n", order++); }
__attribute__((destructor)) static void dtor1(void) { printf("dtor1\n"); }
__attribute__((noinline)) static int noinl(int x) { return x * 3; }
__attribute__((always_inline)) static inline int alwinl(int x) { return x + 3; }
__attribute__((unused)) static int unused_var;
__attribute__((weak)) int weak_fn(void) { return 77; }
int aliased_target(void) { return 55; }
int alias_fn(void) __attribute__((alias("aliased_target")));
__attribute__((section(".data.custom"))) int in_section = 99;
__attribute__((used)) static int used_var = 5;
static __attribute__((format(printf, 1, 2))) void logf_(const char *fmt, ...) { (void)fmt; }
__attribute__((noreturn)) static void stop(void) { exit(0); }
__attribute__((pure)) static int pure_fn(int x) { return x; }
__attribute__((const)) static int const_fn(int x) { return x; }
__attribute__((deprecated("msg"))) static int depr(void) { return 1; }
__attribute__((warn_unused_result)) static int wur(void) { return 2; }
static void cleanup_fn(int *p) { printf("cleanup %d\n", *p); }
static void cleanup_free(char **p) { printf("free %s\n", *p); free(*p); }
__attribute__((visibility("hidden"))) int hidden_var = 3;
static int __attribute__((cold)) cold_fn(void) { return 4; }
int global_aligned __attribute__((aligned(64))) = 1;

int main(void) {
    printf("%zu %zu %zu\n", sizeof(struct P1), offsetof(struct P1, i), offsetof(struct P1, s));
    printf("%zu %zu %zu\n", sizeof(struct P2), offsetof(struct P2, i), offsetof(struct P2, d));
    printf("%zu %zu %zu\n", sizeof(struct P3), offsetof(struct P3, d), offsetof(struct P3, i));
    printf("%zu %zu\n", sizeof(struct P4), offsetof(struct P4, i));
    printf("%zu %zu\n", sizeof(struct A1), _Alignof(struct A1));
    printf("%zu %zu\n", sizeof(struct A2), offsetof(struct A2, x));
    printf("%zu %zu %zu\n", sizeof(struct A3), offsetof(struct A3, ai), _Alignof(aligned_int));
    printf("%zu\n", sizeof(struct PB));
    printf("%zu %zu\n", sizeof(struct Empty), sizeof(struct WithEmpty));
    struct P1 p1 = {'a', 0x12345678, -2}; p1.i += 1; p1.s *= 3; printf("%c %x %d\n", p1.c, p1.i, p1.s);
    struct P1 arr[3]; memset(arr, 0, sizeof arr); arr[1].i = 5; arr[2].s = 7; printf("%d %d %zu\n", arr[1].i, arr[2].s, sizeof arr);
    int *ip = &arr[1].i; (void)ip;
    struct P3 p3 = {'z', 2.5, 9}; printf("%c %g %d\n", p3.c, p3.d, p3.i);
    struct PB pb = {5, 100, 300}; printf("%u %u %u\n", pb.a, pb.b, pb.c);
    printf("%d %d %d %d\n", noinl(2), alwinl(2), weak_fn(), alias_fn());
    printf("%d %d %d\n", in_section, used_var, hidden_var);
    logf_("x %d", 1);
    printf("%d %d %d %d %d\n", pure_fn(1), const_fn(2), depr(), wur(), cold_fn());
    printf("%d\n", (int)((size_t)&global_aligned % 64));
    {
        __attribute__((cleanup(cleanup_fn))) int guard = 42;
        __attribute__((cleanup(cleanup_free))) char *mem = strdup("heap");
        printf("in scope %d %s\n", guard, mem);
    }
    for (int i = 0; i < 2; i++) { int v __attribute__((cleanup(cleanup_fn))) = i * 10; if (i == 1) break; }
    printf("after\n");
    fflush(stdout);
    stop();
}
