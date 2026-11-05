// GNU extended inline assembly
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static inline uint64_t add3(uint64_t a, uint64_t b, uint64_t c) {
    uint64_t r;
    __asm__("lea (%1,%2), %0\n\tadd %3, %0" : "=&r"(r) : "r"(a), "r"(b), "g"(c));
    return r;
}

static int bsr32(unsigned x) {
    int r;
    asm("bsrl %1, %0" : "=r"(r) : "rm"(x) : "cc");
    return r;
}

static uint32_t bswap(uint32_t x) {
    asm("bswap %0" : "+r"(x));
    return x;
}

static void cpuid(unsigned leaf, unsigned *a, unsigned *b, unsigned *c, unsigned *d) {
    __asm__ __volatile__("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(leaf), "c"(0));
}

static long sys_write(int fd, const void *buf, unsigned long n) {
    long ret;
    register long r10 __asm__("r10") = 0;
    __asm__ volatile("syscall" : "=a"(ret) : "a"(1L), "D"((long)fd), "S"(buf), "d"(n), "r"(r10) : "rcx", "r11", "memory");
    return ret;
}

static int cas(int *p, int old, int new) {
    int prev;
    __asm__ __volatile__("lock; cmpxchgl %2, %1" : "=a"(prev), "+m"(*p) : "r"(new), "0"(old) : "memory");
    return prev;
}

static int is_zero(long x) {
    int z;
    asm("test %1, %1" : "=@ccz"(z) : "r"(x));
    return z;
}

static _Bool below(unsigned a, unsigned b) {
    _Bool r;
    asm("cmp %2, %1" : "=@ccb"(r) : "r"(a), "r"(b));
    return r;
}

static double fsqrt(double x) {
    double r;
    asm("fsqrt" : "=t"(r) : "0"(x));
    return r;
}

static long double fpatan(long double y, long double x) {
    long double r;
    asm("fpatan" : "=t"(r) : "0"(x), "u"(y) : "st(1)");
    return r;
}

static double sse_add(double a, double b) {
    asm("addsd %1, %0" : "+x"(a) : "x"(b));
    return a;
}

static float sse_max(float a, float b) {
    float r;
    asm("maxss %2, %0" : "=x"(r) : "0"(a), "xm"(b));
    return r;
}

static int named(int a, int b) {
    int r;
    asm("movl %[x], %[out]\n\tsubl %[y], %[out]" : [out] "=&r"(r) : [x] "r"(a), [y] "r"(b));
    return r;
}

static int loop_sum(int n) {
    int s = 0;
    asm("1:\n\t"
        "addl %1, %0\n\t"
        "decl %1\n\t"
        "jnz 1b"
        : "+r"(s), "+r"(n));
    return s;
}

static int sizes(void) {
    unsigned short w = 0x1234;
    unsigned char b;
    asm("movb %h1, %0" : "=Q"(b) : "Q"((unsigned)w));
    asm("rolw $8, %0" : "+r"(w));
    return b * 0x10000 + w;
}

static int goto_test(int x) {
    asm goto("cmpl $5, %0\n\tje %l1" : : "r"(x) : "cc" : five);
    return 0;
five:
    return 1;
}

static int goto_named(int x) {
    asm goto("testl %0, %0; jz %l[zero]" : : "r"(x) : : zero);
    return x;
zero:
    return -1;
}

struct S { int a; long b; };
static long struct_mem(struct S *s) {
    long r;
    asm("movq %1, %0" : "=r"(r) : "m"(s->b));
    asm("addl $10, %0" : "+m"(s->a));
    return r;
}

int global_counter = 5;
static int glob(void) {
    asm("incl %0" : "+m"(global_counter));
    int v;
    asm("movl %1, %0" : "=r"(v) : "m"(global_counter));
    return v;
}

static long imm_ops(void) {
    long r;
    asm("movq %1, %0\n\taddq %2, %0\n\tsubq $%c3, %0" : "=r"(r) : "i"(100), "n"(23), "i"(3));
    return r;
}

static uint64_t mul128(uint64_t a, uint64_t b, uint64_t *hi) {
    uint64_t lo;
    asm("mulq %3" : "=a"(lo), "=d"(*hi) : "a"(a), "rm"(b));
    return lo;
}

static unsigned __int128 pair(unsigned __int128 x) {
    asm("addq $1, %%rax\n\tadcq $0, %%rdx" : "+A"(x));
    return x;
}

static int uniq(int x) {
    int r = 0;
    for (int i = 0; i < 3; i++)
        asm("cmpl $1, %1\n\tjne .Lskip%=\n\taddl $100, %0\n.Lskip%=:\n\taddl $1, %0" : "+r"(r) : "r"(i));
    return r + x;
}

static int clobber_callee(int x) {
    asm volatile("movq $123, %%rbx\n\tmovq $7, %%r12\n\tmovq $9, %%r15" ::: "rbx", "r12", "r15");
    return x * 2;
}

static int array_mem(void) {
    int arr[4] = {1, 2, 3, 4};
    int r;
    asm("movl 8+%1, %0" : "=r"(r) : "m"(arr));
    return r;
}

static long rdtsc_ok(void) {
    unsigned lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long)hi << 32 | lo) != 0;
}

static unsigned long copy_string(char *dst, const char *src, unsigned long n) {
    unsigned long d0, d1, d2;
    asm volatile("rep movsb" : "=&c"(d0), "=&D"(d1), "=&S"(d2) : "0"(n), "1"(dst), "2"(src) : "memory");
    return d0;
}

__asm__(".text\n"
        ".globl asm_triple\n"
        ".type asm_triple, @function\n"
        "asm_triple:\n"
        "\tleal (%rdi,%rdi,2), %eax\n"
        "\tret\n");
int asm_triple(int);

__asm__(".section .rodata\n"
        "asm_table: .long 10, 20, 30\n"
        ".globl asm_table\n"
        ".text");
extern int asm_table[3];

static int dialect(int x) {
    int r;
    asm("{movl %1, %0|mov %0, %1}" : "=r"(r) : "r"(x));
    return r;
}

static int sum_regs(int a, int b, int c, int d, int e, int f, int g) {
    int r;
    asm("movl %1, %0\n\taddl %2, %0\n\taddl %3, %0\n\taddl %4, %0\n\taddl %5, %0\n\taddl %6, %0\n\taddl %7, %0"
        : "=&r"(r) : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g));
    return r;
}

int main(void) {
    printf("add3 %llu\n", (unsigned long long)add3(1, 2, 39));
    printf("bsr %d %d\n", bsr32(1), bsr32(0x80000000u));
    printf("bswap %08x\n", bswap(0x11223344));
    unsigned a, b, c, d;
    cpuid(0, &a, &b, &c, &d);
    char vendor[13];
    memcpy(vendor, &b, 4);
    memcpy(vendor + 4, &d, 4);
    memcpy(vendor + 8, &c, 4);
    vendor[12] = 0;
    printf("cpuid ok %d\n", a > 0 && strlen(vendor) == 12);
    fflush(stdout);
    sys_write(1, "syscall\n", 8);
    int v = 10;
    int prev = cas(&v, 10, 20);
    printf("cas %d %d\n", prev, v);
    prev = cas(&v, 10, 30);
    printf("cas %d %d\n", prev, v);
    printf("flags %d %d %d %d\n", is_zero(0), is_zero(5), below(1, 2), below(3, 2));
    printf("fsqrt %.6f\n", fsqrt(2.0));
    printf("fpatan %.6Lf %.6Lf\n", fpatan(1.0L, 1.0L), fpatan(3.0L, -1.0L));
    printf("sse %.2f %.2f\n", sse_add(1.5, 2.25), sse_max(3.0f, -1.0f));
    printf("named %d\n", named(50, 8));
    printf("loop %d\n", loop_sum(10));
    printf("sizes %x\n", sizes());
    printf("goto %d %d %d %d\n", goto_test(5), goto_test(6), goto_named(0), goto_named(7));
    struct S s = {1, 99};
    long sm = struct_mem(&s);
    printf("struct %ld %d\n", sm, s.a);
    int gv = glob();
    printf("glob %d %d\n", gv, global_counter);
    printf("imm %ld\n", imm_ops());
    uint64_t hi;
    uint64_t lo = mul128(0xFFFFFFFFFFFFFFFFull, 3, &hi);
    printf("mul %llx %llx\n", (unsigned long long)hi, (unsigned long long)lo);
    unsigned __int128 p = pair(((unsigned __int128)1 << 64) - 1);
    printf("pair %llx %llx\n", (unsigned long long)(p >> 64), (unsigned long long)p);
    printf("uniq %d\n", uniq(1000));
    printf("callee %d\n", clobber_callee(21));
    printf("array %d\n", array_mem());
    printf("rdtsc %ld\n", rdtsc_ok());
    char buf[16];
    unsigned long left = copy_string(buf, "hello world", 12);
    printf("copy %s %lu\n", buf, left);
    printf("toplevel %d %d %d\n", asm_triple(14), asm_table[0] + asm_table[2], asm_table[1]);
    printf("dialect %d\n", dialect(77));
    printf("regs %d\n", sum_regs(1, 2, 3, 4, 5, 6, 7));
    return 0;
}
