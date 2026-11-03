// constructor priorities are ignored: constructor(200) must run before a plain constructor
#include <stdio.h>
__attribute__((constructor)) static void plain(void) { puts("plain"); }
__attribute__((constructor(200))) static void prio(void) { puts("prio 200"); }
int main(void) { return 0; } /* expect: prio 200 / plain */
