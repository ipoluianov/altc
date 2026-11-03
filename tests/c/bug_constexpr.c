// std: c2x
// constexpr objects are not usable in constant expressions:
// "expression is not a compile-time constant" / "variable length array declaration not allowed at file scope"
#include <stdio.h>
constexpr int CN = 10;
static_assert(CN == 10);
int arr[CN];
int main(void) {
    printf("%zu\n", sizeof arr); /* expect: 40 */
    return 0;
}
