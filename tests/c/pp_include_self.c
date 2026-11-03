#ifndef PASS
#define PASS 1
#include <stdio.h>
#include <string.h>
static const char *base(const char *p) { const char *s = strrchr(p, '/'); return s ? s + 1 : p; }
static int level_outer = __INCLUDE_LEVEL__;
#include "pp_include_self.c"
#undef PASS
#define PASS 2
#include "pp_include_self.c"
int main(void) {
    printf("%d %d %d\n", level_outer, level_inner1, level_inner2);
    printf("%s %s\n", base(__BASE_FILE__), base(__FILE_NAME__));
    printf("%s %s\n", base(file_inner1), base(basefile_inner1));
    printf("%d %d\n", line_inner1, line_inner2);
    return 0;
}
#elif PASS == 1
static int level_inner1 = __INCLUDE_LEVEL__;
static const char *file_inner1 = __FILE__;
static const char *basefile_inner1 = __BASE_FILE__;
static int line_inner1 = __LINE__;
#else
static int level_inner2 = __INCLUDE_LEVEL__;
static int line_inner2 = __LINE__;
#endif
