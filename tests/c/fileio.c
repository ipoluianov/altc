#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct rec { int id; double val; char tag[8]; };

int main(void) {
    const char *fn = "altc_fileio_test.tmp";
    FILE *f = fopen(fn, "w");
    if (!f) { printf("open fail\n"); return 1; }
    fprintf(f, "line one %d\n", 1);
    fputs("line two\n", f);
    fputc('X', f); fputc('\n', f);
    fprintf(f, "%5.2f %s %x\n", 3.14159, "word", 255);
    fclose(f);
    f = fopen(fn, "r");
    char line[128]; int n = 0;
    while (fgets(line, sizeof line, f)) printf("%d: %s", ++n, line);
    rewind(f);
    int num; char w1[16], w2[16];
    printf("scan %d\n", fscanf(f, "%15s %15s %d", w1, w2, &num)); printf("%s %s %d\n", w1, w2, num);
    fseek(f, 0, SEEK_END); printf("size %ld\n", ftell(f));
    printf("eof %d\n", fgetc(f) == EOF && feof(f));
    fclose(f);
    struct rec out[3] = {{1, 1.5, "a"}, {2, -2.25, "bb"}, {3, 1e10, "ccc"}}, in[3];
    f = fopen(fn, "wb"); printf("wrote %zu\n", fwrite(out, sizeof out[0], 3, f)); fclose(f);
    f = fopen(fn, "rb"); printf("read %zu\n", fread(in, sizeof in[0], 3, f));
    for (int i = 0; i < 3; i++) printf("%d %g %s\n", in[i].id, in[i].val, in[i].tag);
    fseek(f, sizeof(struct rec), SEEK_SET); struct rec one; fread(&one, sizeof one, 1, f); printf("seek %d\n", one.id);
    fclose(f);
    f = fopen(fn, "a+"); fputs("appended", f); fseek(f, 0, SEEK_SET); int c = fgetc(f); printf("first byte %d\n", c); fclose(f);
    printf("remove %d\n", remove(fn));
    int a, b; char s[10]; float fl; double d;
    printf("%d\n", sscanf("12 34 abc 1.5 2.25", "%d %d %9s %f %lf", &a, &b, s, &fl, &d));
    printf("%d %d %s %g %g\n", a, b, s, fl, d);
    unsigned x; char ch; int nsc = sscanf("ff:Z", "%x:%c", &x, &ch); printf("%d %x %c\n", nsc, x, ch);
    int consumed; sscanf("hello world", "%*s%n", &consumed); printf("%d\n", consumed);
    char sb[100]; sprintf(sb, "%s-%05d-%-4s|%+.3e", "pre", 42, "ab", 12345.678); puts(sb);
    printf("%%|%c|%5c|%-5c|\n", 'a', 'b', 'c');
    printf("%5s|%-5s|%.2s|%*d|%-*d|%.*f\n", "ab", "ab", "abcdef", 6, 42, 6, 42, 2, 3.14159);
    printf("%#x %#o %#X %o %X\n", 255, 8, 255, 64, 0xabc);
    printf("%hhd %hd %ld %lld %hhu %hu\n", (signed char)-1, (short)-2, -3L, -4LL, (unsigned char)255, (unsigned short)65535);
    printf("%i %u %5.1u\n", -10, 10u, 7u);
    char big[20]; int ret = snprintf(big, 5, "%d", 1234567); printf("%d %s\n", ret, big);
    setvbuf(stdout, NULL, _IOFBF, 0);
    puts("puts line");
    putchar('!'); putchar('\n');
    return 0;
}
