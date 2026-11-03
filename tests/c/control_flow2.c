#include <stdio.h>
static int ret_in_stmt_expr(int x) { int y = ({ if (x > 5) return -1; x * 2; }); return y; }
static int break_in_stmt_expr(void) { int i, s = 0; for (i = 0; i < 10; i++) { s += ({ if (i == 4) break; i; }); } return s * 100 + i; }
static int goto_out(int x) { int r = ({ if (x) goto bail; 1; }); return r; bail: return 99; }
static int do_continue(void) { int i = 0, n = 0; do { i++; if (i % 2) continue; n++; } while (i < 10); return n; }
static int nested_loops_labels(void) { int c = 0; for (int i = 0; i < 5; i++) { int j = 0; while (1) { if (++j > i) break; if (j == 2) continue; c++; } } return c; }
static int switch_in_expr(int k) { return ({ int r; switch (k) { case 1: r = 10; break; default: r = 20; } r; }); }
static int comma_loop(void) { int a, b, s = 0; for (a = 0, b = 10; a < b; a += 2, b -= 1) s += a * b; return s; }
static int while_assign(const char *s) { int n = 0; char c; while ((c = *s++)) if (c == 'a') n++; return n; }
static int early_exit_nested(int target) { for (int i = 0; i < 10; i++) for (int j = 0; j < 10; j++) for (int k = 0; k < 10; k++) if (i * 100 + j * 10 + k == target) return i + j + k; return -1; }
static int countdown_unsigned(void) { int n = 0; for (unsigned u = 5; u-- > 0;) n++; return n; }
static int cond_side_effects(void) { int a = 0, b = 0; for (int i = 0; i < 10; i++) *(i & 1 ? &a : &b) += i; return a * 100 + b; }
int main(void) {
    printf("%d %d %d\n", ret_in_stmt_expr(3), ret_in_stmt_expr(7), break_in_stmt_expr());
    printf("%d %d %d %d\n", goto_out(0), goto_out(1), do_continue(), nested_loops_labels());
    printf("%d %d %d %d\n", switch_in_expr(1), switch_in_expr(2), comma_loop(), while_assign("banana"));
    printf("%d %d %d %d\n", early_exit_nested(345), early_exit_nested(1000), countdown_unsigned(), cond_side_effects());
    return 0;
}
