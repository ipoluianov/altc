#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stddef.h>

int main(void) {
    printf("%d %d %d %d %d %d %d\n", '\0', '\a', '\b', '\f', '\v', '\r', '\t');
    printf("%d %d %d %d %d\n", '\x41', '\101', '\'', '\"', '\\');
    printf("%d %d %d\n", '\377', '\xff', (unsigned char)'\xff');
    printf("%d %d\n", 'ab', 'abcd');
    printf("%d %d\n", '?', '\?');
    char s[] = "a\0b\0c"; printf("%zu %zu\n", sizeof s, strlen(s));
    printf("%zu\n", sizeof("con" "cat" "enation"));
    printf("%s\n", "con" "cat");
    char oct[] = "\1\12\123\1234"; for (size_t i = 0; i < sizeof oct; i++) printf("%d ", oct[i]); printf("\n");
    char hex[] = "\x1\x12\x7f"; for (size_t i = 0; i < sizeof hex; i++) printf("%d ", hex[i]); printf("\n");
    const char *u8 = u8"é中\U0001F600"; for (const unsigned char *p = (const unsigned char *)u8; *p; p++) printf("%02x", *p); printf("\n");
    const char *plain = "é"; printf("%zu\n", strlen(plain));
    wchar_t ws[] = L"wideé"; printf("%zu %zu %d\n", sizeof ws / sizeof *ws, wcslen(ws), (int)ws[4]);
    wchar_t wc = L'\x263a'; printf("%d\n", (int)wc);
    printf("%ls\n", L"wide print");
    unsigned short u16[] = {0}; (void)u16;
    printf("%zu %zu %zu\n", sizeof(u"x"), sizeof(U"x"), sizeof(L"x"));
    printf("%d %d\n", (int)u'é', (int)U'\U0001F600');
    printf("%lu %lu %lu\n", sizeof 1u, sizeof 1l, sizeof 1ull);
    printf("%d %d %d\n", 0x10, 010, 0b11);
    printf("%llu %llx\n", 0xffffffffffffffffull, 0XABCDEFull);
    printf("%g %g %g %g\n", 1e3, 1E-3, 0x1p10, 0x1.8P1);
    printf("%g %g\n", 1.0f, 1.5F);
    printf("%Lg %Lg\n", 1.0l, 2.5L);
    printf("%d\n", 1000000 == 1e6);
    printf("%zu %zu\n", sizeof(1.0f), sizeof(1.0));
    printf("%s|%s\n", "tab\there", "nl\\n");
    printf("%s\n", "trigraph-free ??!");
    printf("%c%c%c\n", 72, 'i', 33);
    printf("%d\n", "abc"[3]);
    printf("%zu\n", sizeof(L'a'));
    const char *multi = "line1\n"
                        "line2\n"
                        "line3";
    puts(multi);
    printf("%d\n", 2147483647 + 0);
    printf("%ld\n", 2147483648);
    printf("%ld\n", -2147483648);
    printf("%lld\n", 9223372036854775807);
    printf("%lu\n", 0x8000000000000000);
    return 0;
}
