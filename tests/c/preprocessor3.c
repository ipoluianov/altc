#include <stdio.h>
#include <ctype.h>
static int twice(int x) { return x * 2; }
#define twice(x) ((x) * 1000)
#define VALUE 1
#define VALUE 1
#define FN(a, b) a + b
#define FN(a, b) a + b
#define obj (obj + 1)
#define EMPTY_FN()
#define CONCAT_NUM(a, b) a ## b
#define STRINGIZE_NUM(x) #x
#define INC(x) x + 1
#define TIMES2(x) INC(x) * 2
#define ID(x) x
#define PAREN_ARG(x) ID x
#define COMMA ,
#define APPLY2(m, a, b) m(a, b)
#define PLUS(a, b) ((a) + (b))
int main(void) {
    printf("%d %d\n", twice(3), (twice)(3));
    int (*fp)(int) = twice; printf("%d\n", fp(4));
    printf("%d\n", (isdigit)('5') != 0);
    { int o = 5; int *objp = &o; (void)objp; }
    EMPTY_FN() printf("%d\n", VALUE);
    printf("%lld %s\n", CONCAT_NUM(12, 34LL), STRINGIZE_NUM(0x1.8p3));
    printf("%d\n", TIMES2(3));
    printf("%d\n", PAREN_ARG((7)));
    printf("%d\n", ID(ID(ID(9))));
#undef twice
    printf("%d\n", twice(5));
#ifdef __FILE__
    puts("FILE defined");
#endif
#if defined __LINE__ && __LINE__ > 0
    puts("LINE defined");
#endif
    return 0;
}
