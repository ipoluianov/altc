#include <stdio.h>
#include <stddef.h>

typedef int T;
typedef struct T2 { int x; } T2;
struct T2 make(int v) { struct T2 t = {v}; return t; }
static int f(T T) { return T * 2; }        /* parameter named like a typedef */
static int g(int (x)) { return x + 1; }    /* parenthesized declarator name */
static int labels(int n) { int x = 0; x: if (n-- > 0) { x++; goto x; } return x; }
static int ternary_chain(int v) { return v < 0 ? -1 : v == 0 ? 0 : v < 10 ? 1 : v < 100 ? 2 : 3; }
static int comma_in_args(int a, int b) { return a * 10 + b; }
struct S { int a; struct S *next; } s_global = {1, &s_global};
static int sizeof_tricks(void) { int a[5]; return sizeof a / sizeof a[0] + sizeof(int[3]) / sizeof(int) + sizeof (T){0}; }
static int cast_ambig(void) { T x = 5; int y = (T)+x; int z = (T)-x; int w = (T)(x) * 2; return y + z + w; }
static int nested_struct_decl(void) { struct { struct { int deep; } mid; } outer = {{7}}; return outer.mid.deep; }
static int anon_enum(void) { enum { A1 = 3, B1 } e = B1; return e; }
static int for_decl(void) { int s = 0; for (struct { int i, j; } p = {0, 10}; p.i < p.j; p.i++, p.j--) s += p.i; return s; }
static int block_typedef(void) { typedef double T; T v = 2.5; return (int)(v * 2); }
static int shadow_typedef(void) { int T = 4; return T * T; }
static int k_and_r_style_call(void);
static int bitwise_prec(int a) { return a & 1 << 2 | a ^ 3 && a; }
static int unary_chain(int a) { return - - a + ~ ~a + ! ! a + -(-a); }
static int postfix_chain(void) { struct S s2 = {5, NULL}; struct S *arr[1] = {&s2}; int first = arr[0]->a++; return first + (*arr)->a; }
static int *ptr_ret(int *p) { return p; }
static int addr_deref(void) { int v = 3; return *&*&v + *ptr_ret(&v); }
static int str_index(void) { return "hello"[1] + 1["ab"]; }
static int gnu_cond(int a) { return a ?: 99; }
static int empty_stmts(void) { ;;; int x = 1; ;; { ; } return x; }
static int long_long_long(void) { long long int a = 1; unsigned long long int b = 2; long double c = 3; short int d = 4; unsigned short int e = 5; signed f = 6; return (int)(a + b + c + d + e + f); }
static int const_ptr_mix(void) { const int x = 1; int const y = 2; const int *const p = &x; int const *q = &y; return *p + *q; }
static int array_of_fp(void) { int (*fps[2])(int) = {f, g}; return fps[0](3) + fps[1](3); }
static int init_with_comma(void) { int a = (1, 2), b[2] = {(3, 4), 5}; return a + b[0] + b[1]; }
static int sizeof_fn_ptr(void) { return (int)(sizeof(int (*)(void)) + sizeof(&f)); }
#define CALL(fn, ...) fn(__VA_ARGS__)

int main(void) {
    printf("%d %d %d\n", f(4), g(4), labels(5));
    for (int v = -5; v < 500; v *= -3) printf("%d ", ternary_chain(v)); printf("\n");
    printf("%d\n", comma_in_args((1, 2), (3, 4)));
    printf("%d %d\n", s_global.next->next->a, make(9).x);
    printf("%d %d %d %d\n", sizeof_tricks(), cast_ambig(), nested_struct_decl(), anon_enum());
    printf("%d %d %d %d\n", for_decl(), block_typedef(), shadow_typedef(), k_and_r_style_call());
    printf("%d %d %d %d\n", bitwise_prec(5), unary_chain(3), postfix_chain(), addr_deref());
    printf("%d %d %d %d\n", str_index(), gnu_cond(0), gnu_cond(5), empty_stmts());
    printf("%d %d %d %d\n", long_long_long(), const_ptr_mix(), array_of_fp(), init_with_comma());
    printf("%d %d\n", sizeof_fn_ptr(), CALL(comma_in_args, 1, 2));
    T2 t2 = make(3); T2 *tp = &t2; printf("%d\n", (*tp).x + tp[0].x);
    int (*arrp)[2] = (int (*)[2])(int[]){1, 2, 3, 4}; printf("%d\n", arrp[1][1]);
    printf("%d\n", (int){7} + (int){1});
    return 0;
}
static int k_and_r_style_call(void) { return 11; }
