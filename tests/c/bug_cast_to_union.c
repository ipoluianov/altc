// GNU cast to union type "(union U)x" is rejected: "error: conversion to non-scalar type requested"
#include <stdio.h>
union U { int i; double d; };
int main(void) {
    union U a = (union U)3.5;
    union U b = (union U)7;
    printf("%g %d\n", a.d, b.i); /* expect: 3.5 7 */
    return 0;
}
