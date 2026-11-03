#include <stdio.h>
struct P { int x, y; };
static int *gcl = (int[]){1, 2, 3};
static struct P *gp = &(struct P){5, 6};
static int sum(const int *a, int n) { int s = 0; while (n--) s += a[n]; return s; }
static struct P add(struct P a, struct P b) { return (struct P){a.x + b.x, a.y + b.y}; }
int main(void) {
    for (int i = 0; i < 3; i++) { int *p = (int[]){i, i * 10}; p[0] += 100; printf("%d %d ", p[0], p[1]); } printf("\n");
    for (int i = 0; i < 3; i++) { struct P *q = &(struct P){.x = i}; q->y += 5; printf("%d,%d ", q->x, q->y); } printf("\n");
    gcl[1] += 40; gp->y *= 2; printf("%d %d\n", gcl[1], gp->y);
    printf("%d\n", sum((int[]){1, 2, 3, 4, 5}, 5));
    struct P r = add((struct P){1, 2}, (struct P){.y = 10}); printf("%d %d\n", r.x, r.y);
    const char *names[] = {(const char[]){"one"}, (char[]){"two"}}; printf("%s %s\n", names[0], names[1]);
    int (*pa)[2] = &(int[2]){7, 8}; printf("%d\n", (*pa)[1]);
    char *s = (char[16]){"mutable"}; s[0] = 'M'; printf("%s\n", s);
    int n = 4; int *dyn = (int[]){n, n * n, n * n * n}; printf("%d\n", dyn[2]);
    printf("%zu\n", sizeof((struct P[]){{1, 2}, {3, 4}, {5, 6}}) / sizeof(struct P));
    struct P *arrp = (struct P[]){[2] = {9, 9}}; printf("%d %d\n", arrp[0].x, arrp[2].y);
    return 0;
}
