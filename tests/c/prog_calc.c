#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum { T_NUM, T_IDENT, T_OP, T_LP, T_RP, T_END } Kind;
typedef struct { Kind k; double v; char op; char name[16]; } Tok;
typedef struct { const char *s; Tok cur; } Lexer;
typedef struct { char name[16]; double val; } Var;
static Var vars[32]; static int nvars;

static void next(Lexer *L) {
    while (isspace((unsigned char)*L->s)) L->s++;
    char c = *L->s;
    if (!c) { L->cur.k = T_END; return; }
    if (isdigit((unsigned char)c) || c == '.') { char *e; L->cur.v = strtod(L->s, &e); L->s = e; L->cur.k = T_NUM; return; }
    if (isalpha((unsigned char)c)) { int n = 0; while (isalnum((unsigned char)*L->s) && n < 15) L->cur.name[n++] = *L->s++; L->cur.name[n] = 0; L->cur.k = T_IDENT; return; }
    L->s++;
    if (c == '(') L->cur.k = T_LP; else if (c == ')') L->cur.k = T_RP; else { L->cur.k = T_OP; L->cur.op = c; }
}
static double *lookup(const char *n, int create) {
    for (int i = 0; i < nvars; i++) if (!strcmp(vars[i].name, n)) return &vars[i].val;
    if (!create) return NULL;
    strcpy(vars[nvars].name, n); vars[nvars].val = 0; return &vars[nvars++].val;
}
static double expr(Lexer *L);
static double primary(Lexer *L) {
    if (L->cur.k == T_NUM) { double v = L->cur.v; next(L); return v; }
    if (L->cur.k == T_IDENT) {
        char n[16]; strcpy(n, L->cur.name); next(L);
        if (L->cur.k == T_OP && L->cur.op == '=') { next(L); double v = expr(L); *lookup(n, 1) = v; return v; }
        double *p = lookup(n, 0); return p ? *p : 0;
    }
    if (L->cur.k == T_LP) { next(L); double v = expr(L); if (L->cur.k == T_RP) next(L); return v; }
    if (L->cur.k == T_OP && L->cur.op == '-') { next(L); return -primary(L); }
    return 0;
}
static double power(Lexer *L) {
    double b = primary(L);
    if (L->cur.k == T_OP && L->cur.op == '^') { next(L); double e = power(L); double r = 1; for (int i = 0; i < (int)e; i++) r *= b; return r; }
    return b;
}
static double term(Lexer *L) {
    double v = power(L);
    while (L->cur.k == T_OP && (L->cur.op == '*' || L->cur.op == '/' || L->cur.op == '%')) {
        char op = L->cur.op; next(L); double r = power(L);
        v = op == '*' ? v * r : op == '/' ? v / r : (double)((long)v % (long)r);
    }
    return v;
}
static double expr(Lexer *L) {
    double v = term(L);
    while (L->cur.k == T_OP && (L->cur.op == '+' || L->cur.op == '-')) { char op = L->cur.op; next(L); double r = term(L); v = op == '+' ? v + r : v - r; }
    return v;
}
static double eval(const char *s) { Lexer L = {s}; next(&L); return expr(&L); }

int main(void) {
    const char *progs[] = {"1 + 2 * 3", "(1 + 2) * 3", "2 ^ 10", "-3 * -(4 - 6)", "x = 5", "y = x * 2 + 1", "x * y",
        "10 / 4", "17 % 5", "1.5e2 + .25", "z = (x + y) ^ 2 / 4", "z - x - y", "((((1))))", "2 ^ 3 ^ 2", "a = b = 7", "a + b"};
    for (unsigned i = 0; i < sizeof progs / sizeof *progs; i++) printf("%-22s => %.6g\n", progs[i], eval(progs[i]));
    for (int i = 0; i < nvars; i++) printf("%s=%g ", vars[i].name, vars[i].val);
    printf("\n");
    return 0;
}
