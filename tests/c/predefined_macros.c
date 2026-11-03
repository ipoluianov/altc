#include <stdio.h>
#include <limits.h>
int main(void) {
    printf("%d %d\n", __STDC__, __STDC_HOSTED__);
    printf("%d\n", __STDC_VERSION__ >= 201112L);
    printf("%d %d %d %d %d\n", __x86_64__, __LP64__, __CHAR_BIT__, __linux__, __ELF__);
    printf("%d %d %d %d\n", __SIZEOF_INT__, __SIZEOF_LONG__, __SIZEOF_POINTER__, __SIZEOF_LONG_DOUBLE__);
    printf("%d\n", __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__);
    printf("%d %ld %lu\n", __INT_MAX__, __LONG_MAX__, __SIZE_MAX__);
    __SIZE_TYPE__ s = sizeof(int); __PTRDIFF_TYPE__ d = -1; printf("%zu %td\n", s, d);
#ifdef unix
    printf("unix %d\n", unix);
#endif
    printf("%d %d\n", CHAR_BIT, CHAR_MIN);
    return 0;
}
