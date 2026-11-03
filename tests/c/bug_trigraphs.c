// std: c11
// trigraphs are not replaced in strict ISO modes (-std=c89..c17 enable them in gcc); expect "what|" and "#[]{}~^\"
#include <stdio.h>
int main(void) {
    puts("what??!");
    printf("%s\n", "??=??(??)??<??>??-??'??/??/");
    return 0;
}
