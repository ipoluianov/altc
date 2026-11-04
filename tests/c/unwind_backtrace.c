// backtrace() walks the frames of AltC code through .eh_frame
#include <stdio.h>
#include <execinfo.h>
__attribute__((noinline)) int leaf(int x) { void *buf[32]; int n = backtrace(buf, 32); printf("frames=%d\n", n); return n + x; }
__attribute__((noinline)) int mid(int x) { long a[100]; for (int i = 0; i < 100; i++) a[i] = i * x; return leaf((int)a[x % 100]) + 1; }
__attribute__((noinline)) int top(int x) { return mid(x + 1) * 2; }
int main(void) { printf("%d\n", top(3)); return 0; }
