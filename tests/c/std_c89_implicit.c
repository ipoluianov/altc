// std: gnu89
/* no #include: printf and others are implicitly declared */
int sum();
double half();
struct pt { int x, y; };
struct pt mkpt();

main()
{
    double d = 2.5;
    float f = 1.5f;
    char c = 'A';
    short s = -3;
    printf("%g %g %c %d %s\n", d, f, c, s, "implicit printf");
    printf("%d\n", sum(1, 2, 3));
    printf("%g\n", half(5.0));
    printf("%g\n", half(f));
    printf("%d\n", abs(-9));
    printf("%d\n", mkpt(3, 4).y);
    printf("%lu\n", (unsigned long)strlen("hello"));
    return 0;
}

int sum(a, b, c) int a, b, c; { return a + b + c; }
double half(x) float x; { return x / 2; }
struct pt mkpt(x, y) { struct pt p; p.x = x; p.y = y; return p; }
