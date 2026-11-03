#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum { J_NULL, J_BOOL, J_NUM, J_STR, J_ARR, J_OBJ } JType;
typedef struct JVal JVal;
struct JVal { JType t; double num; int b; char *str; JVal **items; char **keys; int n; };
typedef struct { const char *p; int err; } P;

static void ws(P *p) { while (isspace((unsigned char)*p->p)) p->p++; }
static JVal *mk(JType t) { JVal *v = calloc(1, sizeof *v); v->t = t; return v; }
static JVal *value(P *p);
static char *str(P *p) {
    if (*p->p != '"') { p->err = 1; return NULL; }
    p->p++; char buf[256]; int n = 0;
    while (*p->p && *p->p != '"') {
        char c = *p->p++;
        if (c == '\\') { c = *p->p++; switch (c) { case 'n': c = '\n'; break; case 't': c = '\t'; break; case 'u': { unsigned u = 0; for (int i = 0; i < 4; i++) { u = u * 16 + (isdigit((unsigned char)*p->p) ? *p->p - '0' : (tolower(*p->p) - 'a' + 10)); p->p++; } c = u < 128 ? (char)u : '?'; break; } default: break; } }
        if (n < 255) buf[n++] = c;
    }
    if (*p->p == '"') p->p++; else p->err = 1;
    buf[n] = 0; return strdup(buf);
}
static JVal *value(P *p) {
    ws(p);
    if (*p->p == '{') {
        p->p++; JVal *o = mk(J_OBJ); ws(p);
        if (*p->p == '}') { p->p++; return o; }
        for (;;) { ws(p); char *k = str(p); ws(p); if (*p->p != ':') { p->err = 1; return o; } p->p++; JVal *v = value(p);
            o->keys = realloc(o->keys, (o->n + 1) * sizeof *o->keys); o->items = realloc(o->items, (o->n + 1) * sizeof *o->items);
            o->keys[o->n] = k; o->items[o->n++] = v; ws(p); if (*p->p == ',') { p->p++; continue; } if (*p->p == '}') { p->p++; return o; } p->err = 1; return o; }
    }
    if (*p->p == '[') {
        p->p++; JVal *a = mk(J_ARR); ws(p);
        if (*p->p == ']') { p->p++; return a; }
        for (;;) { JVal *v = value(p); a->items = realloc(a->items, (a->n + 1) * sizeof *a->items); a->items[a->n++] = v; ws(p);
            if (*p->p == ',') { p->p++; continue; } if (*p->p == ']') { p->p++; return a; } p->err = 1; return a; }
    }
    if (*p->p == '"') { JVal *s = mk(J_STR); s->str = str(p); return s; }
    if (!strncmp(p->p, "true", 4)) { p->p += 4; JVal *b = mk(J_BOOL); b->b = 1; return b; }
    if (!strncmp(p->p, "false", 5)) { p->p += 5; return mk(J_BOOL); }
    if (!strncmp(p->p, "null", 4)) { p->p += 4; return mk(J_NULL); }
    char *e; double d = strtod(p->p, &e); if (e == p->p) { p->err = 1; return mk(J_NULL); }
    p->p = e; JVal *n = mk(J_NUM); n->num = d; return n;
}
static void dump(const JVal *v, int ind) {
    switch (v->t) {
    case J_NULL: printf("null"); break;
    case J_BOOL: printf(v->b ? "true" : "false"); break;
    case J_NUM: printf("%g", v->num); break;
    case J_STR: putchar('"'); for (const char *s = v->str; *s; s++) { if (*s == '\n') printf("\\n"); else if (*s == '"') printf("\\\""); else putchar(*s); } putchar('"'); break;
    case J_ARR: printf("["); for (int i = 0; i < v->n; i++) { if (i) printf(", "); dump(v->items[i], ind); } printf("]"); break;
    case J_OBJ: printf("{\n"); for (int i = 0; i < v->n; i++) { printf("%*s\"%s\": ", ind + 2, "", v->keys[i]); dump(v->items[i], ind + 2); printf(i + 1 < v->n ? ",\n" : "\n"); } printf("%*s}", ind, ""); break;
    }
}
static double sum_nums(const JVal *v) { double s = v->t == J_NUM ? v->num : 0; if (v->t >= J_ARR) for (int i = 0; i < v->n; i++) s += sum_nums(v->items[i]); return s; }
static void jfree(JVal *v) { for (int i = 0; i < v->n; i++) { jfree(v->items[i]); if (v->keys) free(v->keys[i]); } free(v->items); free(v->keys); free(v->str); free(v); }

int main(void) {
    const char *docs[] = {
        "{\"name\": \"altc\", \"version\": 1.5, \"tags\": [\"c\", \"go\", \"compiler\"], \"ok\": true, \"none\": null,"
        " \"nested\": {\"a\": [1, 2, {\"b\": -3.25e2}], \"esc\": \"line\\nnext \\\"q\\\" \\u0041\"}}",
        "[1, [2, [3, [4, [5]]]], {}, [], \"\"]",
        "{\"bad\": [1, 2,}",
        "  42  ",
    };
    for (int i = 0; i < 4; i++) {
        P p = {docs[i], 0}; JVal *v = value(&p);
        printf("doc %d err=%d sum=%g\n", i, p.err, sum_nums(v)); dump(v, 0); printf("\n"); jfree(v);
    }
    return 0;
}
