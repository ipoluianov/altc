#include <cstdio>
#include <stdexcept>
extern "C" int mid2(int);
extern "C" void thrower(int x) { if (x > 2) throw std::runtime_error("boom"); }
int main() {
    long sentinel = 12345;
    try { mid2(5); std::puts("no throw"); }
    catch (const std::exception &e) { std::printf("caught %s sentinel=%ld\n", e.what(), sentinel); }
    return 0;
}
