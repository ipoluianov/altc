#include <stdio.h>
#include <string.h>

#define STR(x) #x
#define XSTR(x) STR(x)
#define VSTR(...) VSTR_(__VA_ARGS__)
#define VSTR_(...) #__VA_ARGS__
#define CAT(a, b) a##b
#define XCAT(a, b) CAT(a, b)
#define NUM 42
#define EMPTY
#define LPAREN (
#define F(x) [x]
#define G F
#define H(x) x(1)
#define SQUARE(x) ((x) * (x))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define LOG(fmt, ...) printf("log: " fmt "\n", ##__VA_ARGS__)
#define LOG2(fmt, ...) printf("log2: " fmt "\n" __VA_OPT__(,) __VA_ARGS__)
#define COUNT(...) COUNT_(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define COUNT_(a, b, c, d, e, n, ...) n
#define FIRST(a, ...) a
#define REST(a, ...) __VA_ARGS__
#define APPLY(m, ...) m(__VA_ARGS__)
#define SELF SELF + 1
#define REC(x) x + REC
#define OBJ_FN FN_LIKE
#define FN_LIKE(x) (x * 2)

/* C standard 6.10.3.5 example 3 */
#define x 3
#define f(a) f(x * (a))
#undef x
#define x 2
#define g f
#define z z[0]
#define h g(~
#define m(a) a(w)
#define w 0,1
#define t(a) a
#define p() int
#define q(x) x
#define r(x,y) x ## y
#define str(x) # x

/* example 4 */
#define xstr(s) str(s)
#define debug(s, t) printf("x" # s "= %d, x" # t "= %s", \
 x ## s, x ## t)
#define INCFILE(n) vers ## n
#define glue(a, b) a ## b
#define xglue(a, b) glue(a, b)
#define HIGHLOW "hello"
#define LOW LOW ", world"

/* example 7 */
#define showlist(...) puts(#__VA_ARGS__)
#define report(test, ...) ((test)?puts(#test): printf(__VA_ARGS__))

#if defined(NUM) && NUM > 40 && !defined(UNDEFINED_THING)
#define CHECK1 1
#else
#define CHECK1 0
#endif
#if (1 << 4) == 16 && (-1 < 0) && (0xffffffffffffffff > 0) && (10 / 3 == 3) && (10 % 3 == 1)
#define CHECK2 1
#endif
#if -1 > 0u
#define CHECK3 "unsigned arith"
#else
#define CHECK3 "signed arith"
#endif
#if UNDEFINED_MACRO == 0
#define CHECK4 1
#endif
#ifdef NUM
#  if NUM == 41
#    error no
#  elif NUM == 42
#    define CHECK5 42
#  else
#    error no
#  endif
#endif
#ifndef CHECK5
#error missing
#endif
#if 'A' == 65 && '\n' == 10
#define CHECK6 1
#endif
#if 1 ? 2 : (1/0)
#define CHECK7 1
#endif
#if __has_include(<stdio.h>) && !__has_include("does_not_exist.h")
#define CHECK8 1
#endif
#define VERSION 3
#if VERSION >= 3 && defined VERSION
#define CHECK9 1
#endif
#define PRAGMA_TEST _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic pop") int pragma_ok = 1;
PRAGMA_TEST
#line 1000 "renamed.c"
static int line_after = __LINE__;
#line 60 "preprocessor.c"

int main(void) {
    int x_ = 0; (void)x_;
    puts(STR(hello world));
    puts(XSTR(NUM));
    puts(STR(NUM));
    puts(STR(  a   +   b  ));
    puts(STR(a/**/b));
    puts(XSTR(EMPTY));
    puts(STR());
    printf("%d\n", CAT(N, UM));
    printf("%d\n", XCAT(1, 2) + 1);
    int ab = 5; printf("%d\n", CAT(a, b));
    printf("%d %d\n", SQUARE(1 + 2), MAX(3, SQUARE(2)));
    LOG("no args");
    LOG("args %d %s", 1, "two");
    LOG2("no args");
    LOG2("args %d", 3);
    printf("%d %d %d\n", COUNT(a), COUNT(a, b, c), COUNT(1, 2, 3, 4, 5));
    printf("%d\n", FIRST(7, 8, 9));
    int rest[] = {REST(0, 1, 2, 3)}; printf("%zu\n", sizeof rest / sizeof *rest);
    printf("%d\n", APPLY(MAX, 4, 9));
    int SELF_v = 1; (void)SELF_v;
    puts(XSTR(SELF));
    puts(XSTR(REC(1)));
    printf("%d\n", OBJ_FN(21));
    puts(XSTR(G(5)));
    puts(XSTR(H(F)));
    /* 6.10.3.5 example 3 */
    puts(VSTR(f(y+1) + f(f(z)) % t(t(g)(0) + t)(1);));
    puts(VSTR(p() i[q()] = { q(1), r(2,3), r(4,), r(,5), r(,) };));
    puts(VSTR(char c[2][6] = { str(hello), str() };));
    /* example 4 */
    puts(VSTR(debug(1, 2);));
#undef h
    puts(XSTR(xstr(INCFILE(2).h)));
    puts(VSTR(glue(HIGH, LOW);));
    puts(xglue(HIGH, LOW));
    /* example 7 */
    puts(VSTR(showlist(The first, second, and third items.);));
    puts(VSTR(report(x>y, "x is %d but y is %d", x, y);));
    showlist(The first, second, and third items.);
    printf("%d %d %s %d %d %d %d %d %d %d\n", CHECK1, CHECK2, CHECK3, CHECK4, CHECK5, CHECK6, CHECK7, CHECK8, CHECK9, pragma_ok);
    printf("%d %d\n", line_after, __LINE__);
    printf("%s\n", __FILE__ + (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') - __FILE__ + 1 : 0));
    printf("%s\n", __func__);
    printf("%d %d %d\n", __COUNTER__, __COUNTER__, __COUNTER__);
    printf("%d\n", (int)sizeof(__DATE__) == 12 && (int)sizeof(__TIME__) == 9);
#define LATE 1
#undef LATE
#ifdef LATE
    puts("bad");
#endif
#define MULTI \
    "multi" \
    "line"
    puts(MULTI);
#define PASTE_OP(a, b) a ## b
    int v = 3; v PASTE_OP(+, =) 2; printf("%d\n", v);
    printf("%d\n", PASTE_OP(0x, 1F));
    printf("%s\n", PASTE_OP(u8, "s"));
#define ARGS_WITH_PARENS(x) x
    printf("%d\n", ARGS_WITH_PARENS((1, 2)));
#define EMPTY_ARG(a, b) a b 1
    printf("%d\n", EMPTY_ARG(, ));
#define NESTED_CALL(x) SQUARE(x) + SQUARE(SQUARE(x))
    printf("%d\n", NESTED_CALL(2));
    return 0;
}
