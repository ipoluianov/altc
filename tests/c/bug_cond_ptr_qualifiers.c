// the type of "c ? (void *)p : (const void *)q" must be const void * (qualifiers of both pointed-to types merge)
#include <stdio.h>
int main(void) {
    int x = 0; void *vp = &x; const void *cvp = &x;
    puts(_Generic(1 ? vp : cvp, void *: "void*", const void *: "const void*", default: "other")); /* expect: const void* */
    return 0;
}
