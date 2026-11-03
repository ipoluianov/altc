#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

struct item { const char *name; int qty; double price; };
struct pair { int *p; char *s; };
struct inner { int a[3]; char tag; };
struct outer { struct inner in[2]; const struct item *it; long n; };
struct vt { int (*f)(int); const char *nm; };
static int neg(int x) { return -x; }
static int inc(int x) { return x + 1; }

static int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static char text[] = "abcdefghij";
static const struct item items[] = {{"apple", 3, 0.5}, {"pear", 0, 1.25}, {"kiwi", 12, 0.2}, {0}};
static struct pair pairs[] = {{&arr[2], text + 3}, {arr + 9, &text[9]}, {0, 0}};
static int *end_ptr = arr + 10;
static int *mid_ptr = &arr[5] - 2;
static char *str_off = &"literal"[3];
static struct outer outer = {{{{1, 2, 3}, 'x'}, {{4}, 'y'}}, &items[1], sizeof items / sizeof items[0]};
static struct vt vtable[] = {{neg, "neg"}, {inc, "inc"}, {.nm = "none"}};
static const struct vt *vt_ptr = &vtable[1];
static uintptr_t addr_as_int = (uintptr_t)&arr;   /* relocation as integer */
static long offset_val = offsetof(struct outer, it);
static char *strs2d[2][2] = {{"a", "b"}, {"c", "d"}};
static int matrix[3][4] = {{1, 2, 3, 4}, [2] = {9, 10}};
static int (*row_ptr)[4] = &matrix[1];
static int *elem = &matrix[2][1];
static void *generic[] = {arr, text, (void *)items, neg};
static const char *const *strs_ptr = (const char *const *)&strs2d[1][0];
static struct { char c; int i; } packed_like[] = {{'a', 1}, {'b', 2}};
static double dbl_arr[] = {1.0 / 3, 2.0 * 3.5, (double)(1 << 20), -0.0, 1e-300 * 1e-300};
static float flt_arr[] = {1.0f / 3, (float)1e40 * 0, 16777217};
static long double ld_val = 1.0L / 7;
static int computed[] = {sizeof(int) * 8, (int)sizeof(struct item), 'A' + 2, 10 / 3, -7 % 3, 1 << 4 | 1, (int)3.99, (int)-2.5};
static unsigned char bytes[] = {300 & 0xff, -1, 'z'};
static short sh_arr[4] = {-1, 32767, (short)40000};
static _Bool bools[] = {0, 1, 2, 0.1};
static const char *ptr_cond = 1 ? "yes" : "no";
static struct item *nullp = NULL;
static int self_ref_idx[] = {[3] = 3, [1] = 1, 4, [0] = 0, 2};
static const int *cptr_into_items_qty = &items[2].qty;
static char char_arr_from_str[5][4] = {"one", "two", "six"};
static struct { int n; const char *s[3]; } nested_strs = {2, {"x", "yy"}};
static int zero_len_tail[3];

int main(void) {
    for (const struct item *p = items; p->name; p++) printf("%s %d %g\n", p->name, p->qty, p->price);
    for (struct pair *p = pairs; p->p; p++) printf("%d %c\n", *p->p, *p->s);
    printf("%td %d %s\n", end_ptr - arr, *mid_ptr, str_off);
    printf("%d %d %c %c %s %ld\n", outer.in[0].a[2], outer.in[1].a[0], outer.in[0].tag, outer.in[1].tag, outer.it->name, outer.n);
    for (int i = 0; i < 2; i++) printf("%s:%d ", vtable[i].nm, vtable[i].f(10)); printf("%s %d\n", vtable[2].nm, vtable[2].f == 0);
    printf("%s %d\n", vt_ptr->nm, vt_ptr->f(1));
    printf("%d %ld\n", (int *)addr_as_int == arr, offset_val);
    printf("%s %s\n", strs2d[1][1], strs2d[0][1]);
    printf("%d %d %d %d\n", matrix[0][3], matrix[1][0], matrix[2][1], (*row_ptr)[0]);
    printf("%d\n", *elem);
    printf("%d %c %s %d\n", ((int *)generic[0])[4], ((char *)generic[1])[2], ((struct item *)generic[2])->name, ((int (*)(int))generic[3])(5));
    printf("%s\n", strs_ptr[1]);
    printf("%c %d\n", packed_like[1].c, packed_like[1].i);
    for (int i = 0; i < 5; i++) printf("%g ", dbl_arr[i]); printf("\n");
    for (int i = 0; i < 3; i++) printf("%.9g ", flt_arr[i]); printf("\n");
    printf("%.20Lf\n", ld_val);
    for (unsigned i = 0; i < sizeof computed / sizeof *computed; i++) printf("%d ", computed[i]); printf("\n");
    printf("%d %d %d\n", bytes[0], bytes[1], bytes[2]);
    printf("%d %d %d %d\n", sh_arr[0], sh_arr[1], sh_arr[2], sh_arr[3]);
    printf("%d %d %d %d\n", bools[0], bools[1], bools[2], bools[3]);
    printf("%s %p\n", ptr_cond, (void *)nullp);
    for (int i = 0; i < 4; i++) printf("%d", self_ref_idx[i]); printf(" %zu\n", sizeof self_ref_idx / sizeof(int));
    printf("%d\n", *cptr_into_items_qty);
    for (int i = 0; i < 5; i++) printf("[%s]", char_arr_from_str[i]); printf("\n");
    printf("%d %s %s %p\n", nested_strs.n, nested_strs.s[0], nested_strs.s[1], (void *)nested_strs.s[2]);
    printf("%d\n", zero_len_tail[2]);
    return 0;
}
