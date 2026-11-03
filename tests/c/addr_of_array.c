#include <stdio.h>
int main(void) {
    int arr[5];
    printf("%td %td\n", (char *)(&arr + 1) - (char *)arr, *(&arr + 1) - arr);
    int (*p)[5] = &arr; p++; printf("%td\n", (char *)p - (char *)arr);
    return 0;
}
