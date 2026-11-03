#include <stdio.h>
#include <string.h>

#define max(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); _a > _b ? _a : _b; })
#define swap(a, b) do { __auto_type _t = (a); (a) = (b); (b) = _t; } while (0)
struct pt { int x, y; };
static int interp(const unsigned char *code) {
    static const int offs[] = {&&op_inc - &&op_inc, &&op_dbl - &&op_inc, &&op_end - &&op_inc};
    int acc = 0;
    goto *(&&op_inc + offs[*code++]);
op_inc: acc++; goto *(&&op_inc + offs[*code++]);
op_dbl: acc *= 2; goto *(&&op_inc + offs[*code++]);
op_end: return acc;
}
static int dispatch(const char *prog) {
    void *tab[128] = {0}; tab['+'] = &&plus; tab['-'] = &&minus; tab['*'] = &&times; tab[0] = &&done;
    int v = 1;
    goto *tab[(int)*prog++];
plus: v += 3; goto *tab[(int)*prog++];
minus: v -= 1; goto *tab[(int)*prog++];
times: v *= 2; goto *tab[(int)*prog++];
done: return v;
}

int main(void) {
    int i = 3; double d = 2.5; printf("%g %d\n", max(i, d), max(i, 7));
    int a = 1, b = 2; swap(a, b); printf("%d %d\n", a, b);
    struct pt p = ({ struct pt t = {1, 2}; t.x += 10; t; }); printf("%d %d\n", p.x, p.y);
    int v = ({ int s = 0; for (int k = 0; k < 5; k++) s += k; s; }); printf("%d\n", v);
    const char *str = ({ static char buf[8]; strcpy(buf, "stmt"); buf; }); printf("%s\n", str);
    __auto_type au = 1.5f; printf("%zu\n", sizeof au);
    unsigned char code[] = {0, 0, 1, 0, 1, 2}; printf("%d\n", interp(code));
    printf("%d\n", dispatch("++*-*"));
    int arr[5] = {[0 ... 4] = 3}; printf("%d %d\n", arr[0], arr[4]);
    char z[0]; printf("%zu\n", sizeof z);
    printf("%d\n", __alignof__(double));
    __extension__ long long ell = 5; printf("%lld\n", ell);
    int x = 5; __typeof__(x) *xp = &x; printf("%d\n", *xp);
    printf("%s\n", __PRETTY_FUNCTION__);
    printf("%s\n", __FUNCTION__);
    int cr = 3; switch (cr) { case 1 ... 2: puts("low"); break; case 3 ... 10: puts("mid"); break; }
    void *pv = &x; pv = (char *)pv + 0; printf("%d\n", *(int *)pv);
    int vsize = sizeof(void); printf("%d\n", vsize);
    char *cp = "abc"; void *vp2 = cp; vp2++; printf("%c\n", *(char *)vp2);
    return 0;
}
