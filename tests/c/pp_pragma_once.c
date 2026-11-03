#pragma once
#include <stdio.h>
static int counter_once = __COUNTER__;
#include "pp_pragma_once.c"
#include "./pp_pragma_once.c"
int main(void) {
    printf("%d %d\n", counter_once, __COUNTER__);
    return 0;
}
