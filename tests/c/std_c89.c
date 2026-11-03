// std: gnu89
#include <stdio.h>

static int kr_add(a, b)
    int a;
    int b;
{
    return a + b;
}

static double kr_mix(x, n, c)
    double x;
    char c;
{
    return x * n + c;
}

static implicit_ret(v) { return v * 2; }

static long kr_ptr(p, n) int *p; long n; { long s = 0; while (n--) s += *p++; return s; }

static float kr_float(f) float f; { return f * 2; }

main()
{
    int arr[3];
    register i;
    arr[0] = 1; arr[1] = 2; arr[2] = 3;
    printf("%d\n", kr_add(2, 3));
    printf("%g\n", kr_mix(1.5, 4, 'A'));
    printf("%d\n", implicit_ret(21));
    printf("%ld\n", kr_ptr(arr, 3L));
    printf("%g\n", kr_float(1.25f));
    for (i = 0; i < 3; i++)
        printf("%d ", arr[i]);
    printf("\n");
    {
        static char msg[] = "c89 block";
        puts(msg);
    }
    return 0;
}
