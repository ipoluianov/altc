// "defined" produced by macro expansion inside #if (GCC/Clang accept it):
// altc: "operator "defined" requires an identifier" / "missing binary operator before token "(""
#include <stdio.h>
#define FEATURE_X 1
#define HAS_X defined(FEATURE_X)
#define HAS_Y (defined FEATURE_Y)
int main(void) {
#if HAS_X && !HAS_Y
    puts("ok");
#endif
    return 0; /* expect: ok */
}
