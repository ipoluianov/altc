// C23 keywords (bool, true, false, nullptr, constexpr, alignas, alignof, static_assert, thread_local,
// typeof_unqual) are reserved in every mode; before C23 they are ordinary identifiers.
// altc -std=gnu17: "typedef name omitted", "expected identifier before 'false'", ...
#include <stdio.h>
typedef int bool;
enum { false, true };
int main(void) {
    bool b = true;
    int nullptr = 1, constexpr = 2, alignof = 3, static_assert = 4, thread_local = 5;
    printf("%d %d %d\n", b, false, nullptr + constexpr + alignof + static_assert + thread_local); /* expect: 1 0 15 */
    return 0;
}
