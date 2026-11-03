#ifndef SELF_INCLUDED
#define SELF_INCLUDED 1
#include <stdio.h>
#include <string.h>
#define HDR <stdlib.h>
#include HDR
#define QHDR "preprocessor2.c"

#define STR(x) #x
#define XSTR(...) VSTR(__VA_ARGS__)
#define VSTR(...) #__VA_ARGS__
#define f(x) (x + 1)
#define foo foo
#define a_ b_
#define b_ a_
#define EMPTY
#define CALL_EMPTY() 42
#define TWO_ARGS(a, b) [a|b]
#define VA_OPT_TEST(a, ...) a __VA_OPT__(- __VA_ARGS__ -) end
#define VA_OPT_CAT(a, ...) a ## __VA_OPT__(_tail)
#define F2(x, ...) f2(x __VA_OPT__(,) __VA_ARGS__)
#define GNU_COMMA(fmt, args...) printf(fmt, ## args)
#define NAMED_VA(x, rest...) VSTR(rest)
#define DEFER(m) m EMPTY
#define OBSTRUCT(m) m DEFER(EMPTY)()
#define EXPAND(...) __VA_ARGS__
#define A_() 123
#define CAT3(a, b, c) a ## b ## c
#define IS_DEF defined(SELF_INCLUDED)
#define PLUS +
#define MINUS -
#define NEG(x) -x
#define PARENS ()
#define MACRO_NAME f
#define LONG_MACRO(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12
#define HASH_HASH # ## #
#define mkstr(a) # a
#define in_between(a) mkstr(a)
#define join(c, d) in_between(c HASH_HASH d)
#define t(x, y, z) x ## y ## z
static int f2(int a, ...) { return a; }
#define SELF_PASS 1
#include QHDR

int main(void) {
    printf("%d\n", f(1));
    printf("%d\n", f (2));
    printf("%d\n", f
           (3));
    printf("%d\n", f(/* comment */ 4 /* , */));
    int (*fp)(int, ...) = f2; printf("%d\n", fp != 0);
    int foo = 9; printf("%d\n", foo);
    int a_ = 1; printf("%d\n", a_);
    puts(XSTR(TWO_ARGS(,)));
    puts(XSTR(TWO_ARGS((a, b), "x,y")));
    puts(STR("a,b" 'c'));
    printf("%d\n", CALL_EMPTY());
    puts(XSTR(VA_OPT_TEST(1)));
    puts(XSTR(VA_OPT_TEST(1, 2, 3)));
    puts(XSTR(VA_OPT_TEST(1, )));
    puts(XSTR(VA_OPT_CAT(x)));
    puts(XSTR(VA_OPT_CAT(x, y)));
    printf("%d %d\n", F2(5), F2(6, 7, 8));
    GNU_COMMA("gnu comma\n");
    GNU_COMMA("gnu %d\n", 5);
    puts(NAMED_VA(1, 2, 3));
    puts(XSTR(DEFER(A_)()));
    puts(XSTR(EXPAND(DEFER(A_)())));
    printf("%d\n", CAT3(1, 2, 3));
    puts(XSTR(CAT3(a, , c)));
#if defined(SELF_INCLUDED)
    puts("defined via macro");
#endif
    printf("%d\n", 1 PLUS MINUS 1);
    printf("%d\n", -NEG(1));
    puts(XSTR(-NEG(1)));
    puts(XSTR(MACRO_NAME(5)));
    puts(XSTR(f PARENS));
    printf("%d\n", LONG_MACRO(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12));
    puts(join(x, y));
    puts(XSTR(t(1,2,3) t(,4,5) t(6,,7) t(8,9,) t(10,,) t(,11,) t(,,12) t(,,)));
    pri\
ntf("splice %d\n", 1\
2);
    puts(XSTR(  leading   and   trailing   ));
    puts(XSTR(a
              b));
    puts(STR(@ $ `));
    puts(STR('"' "\\" '\''));
#if 0
#error should be skipped
#garbage directive in skipped group
#endif
#if 0
#elif 1
    puts("elif taken");
#endif
#if (0x7fffffff + 1) > 0 && (-1 / 2 == 0) && (-7 % 3 == -1) && (1 ? -1 : 0u) > 0
    puts("pp arith ok");
#endif
#if 18446744073709551615u == -1
    puts("uintmax wrap");
#endif
#if !defined(nonexistent) && !nonexistent && (nonexistent + 1 == 1)
    puts("undefined is 0");
#endif
#if __has_builtin(__builtin_expect) && __has_attribute(packed)
    puts("has_builtin/attribute");
#endif
    printf("%d\n", included_value());
    return 0;
}
#else
static int included_value(void) { return SELF_PASS * 100; }
#endif
