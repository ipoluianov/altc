#include <stdio.h>
struct point { int x, y; const char *name; unsigned flag : 3; };
typedef struct point point_t;
static int counter = 7;
double scale = 1.5;
int arr[4] = {1, 2, 3, 4};

static int area(point_t *p, int k) {
    int w = p->x * k;
    int h = p->y * k;
    counter++;
    return w * h;
}

int main(void) {
    point_t pt = {3, 4, "corner", 5};
    long total = 0;
    for (int i = 0; i < 3; i++) {
        total += area(&pt, i + 1);
    }
    printf("%ld %d\n", total, counter);
    return 0;
}
