// sizeof applied to an object whose flexible array member was statically initialized
// must be sizeof(struct type) (gcc: 4 4 4), altc includes the initialized tail (8 16)
#include <stdio.h>
struct F { int n; char tail[]; };
struct G { int n; int tail[]; };
static struct F sf = {3, "abc"};
static struct G sg = {1, {1, 2, 3}};
int main(void) { struct F lf = {1}; printf("%zu %zu %zu %zu %zu\n", sizeof(struct F), sizeof sf, sizeof sg, sizeof lf, sizeof(struct G)); return 0; } /* expect: 4 4 4 4 4 */
