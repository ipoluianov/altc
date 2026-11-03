#include <stdio.h>
#include <string.h>
#include <stddef.h>

struct point { int x, y; };
struct rect { struct point tl, br; const char *name; };
struct node { int v; struct node *next; };
union u { int i; float f; unsigned char b[4]; };
struct with_union { int tag; union { int i; double d; } val; };
struct anon { int a; struct { int b, c; }; union { int d; float e; }; };
struct flex { int n; int data[]; };
struct pad1 { char c; int i; char d; };
struct pad2 { char c; double d; };
struct pad3 { char a; short b; char c; long long d; };
struct empty_arr { int n; char s[0]; };
typedef struct { char name[16]; int age; double score; } person;

int area(struct rect r) { return (r.br.x - r.tl.x) * (r.br.y - r.tl.y); }
struct point mid(struct rect *r) { struct point p = {(r->tl.x + r->br.x) / 2, (r->tl.y + r->br.y) / 2}; return p; }

int main(void) {
    struct rect r = {{1, 2}, {11, 22}, "box"};
    printf("%d %s\n", area(r), r.name);
    struct point m = mid(&r); printf("%d %d\n", m.x, m.y);
    struct rect r2 = r; r2.tl.x = 5; printf("%d %d\n", r.tl.x, r2.tl.x);
    struct node n3 = {3, NULL}, n2 = {2, &n3}, n1 = {1, &n2};
    for (struct node *p = &n1; p; p = p->next) printf("%d ", p->v); printf("\n");
    union u uu; uu.f = 1.0f; printf("%08x %d %d\n", uu.i, uu.b[3], uu.b[0]);
    uu.i = 0x01020304; printf("%d %d\n", uu.b[0], uu.b[3]);
    struct with_union wu = {1, {.d = 2.5}}; printf("%d %f\n", wu.tag, wu.val.d);
    struct anon an = {1, {2, 3}, {4}}; printf("%d %d %d %d\n", an.a, an.b, an.c, an.d);
    an.e = 1.5f; printf("%f %zu\n", an.e, sizeof an);
    printf("%zu %zu %zu %zu\n", sizeof(struct pad1), sizeof(struct pad2), sizeof(struct pad3), sizeof(struct empty_arr));
    printf("%zu %zu %zu\n", offsetof(struct pad3, b), offsetof(struct pad3, c), offsetof(struct pad3, d));
    printf("%zu %zu\n", _Alignof(struct pad2), _Alignof(struct pad1));
    static _Alignas(8) char storage[64]; struct flex *fl = (struct flex *)storage; fl->n = 3; for (int i = 0; i < 3; i++) fl->data[i] = i * 7;
    printf("%zu %d %d\n", sizeof(struct flex), fl->data[2], fl->n);
    person ps[2] = {{"alice", 30, 9.5}, {"bob", 25, 7.25}};
    person t = ps[0]; ps[0] = ps[1]; ps[1] = t;
    printf("%s %d %.2f / %s %d %.2f\n", ps[0].name, ps[0].age, ps[0].score, ps[1].name, ps[1].age, ps[1].score);
    printf("%zu %zu\n", sizeof(person), offsetof(person, score));
    struct point pts[3] = {[1] = {7, 8}}; printf("%d %d %d\n", pts[0].x, pts[1].y, pts[2].x);
    struct point *pp = pts; pp[2] = (struct point){9, 10}; printf("%d\n", pts[2].y);
    printf("%d\n", ((struct point){3, 4}).y);
    struct rect *rp = &r; rp->br = rp->tl; printf("%d %d\n", r.br.x, r.br.y);
    struct point a = {1, 2}, b = {1, 2}; printf("%d\n", memcmp(&a, &b, sizeof a) == 0);
    struct { int a[3]; } arrs = {{1, 2, 3}}, arrs2; arrs2 = arrs; arrs2.a[0] = 9; printf("%d %d\n", arrs.a[0], arrs2.a[0]);
    struct node *self = &n1; printf("%d\n", self->next->next->v);
    const struct point cp = {5, 6}; printf("%d\n", cp.x + cp.y);
    struct point q = cp; q.x++; printf("%d\n", q.x);
    struct point cond = r.tl.x > 0 ? a : b; printf("%d\n", cond.x);
    return 0;
}
