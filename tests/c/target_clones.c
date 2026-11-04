// target_clones and target attributes, __builtin_cpu_supports
#include <stdio.h>
#include <immintrin.h>

__attribute__((target_clones("avx2", "sse4.2", "default")))
float sum(const float *a, int n) {
    float s = 0;
    for (int i = 0; i < n; i++) s += a[i] * 0.5f;
    return s;
}

__attribute__((target_clones("arch=haswell", "default")))
const char *which(void) {
#ifdef __AVX2__
    return "file-avx2";
#else
    return "baseline-macros";
#endif
}

__attribute__((target("avx2,fma")))
static float dot8(const float *a, const float *b) {
    __m256 p = _mm256_fmadd_ps(_mm256_loadu_ps(a), _mm256_loadu_ps(b), _mm256_setzero_ps());
    __m128 s = _mm_add_ps(_mm256_castps256_ps128(p), _mm256_extractf128_ps(p, 1));
    s = _mm_hadd_ps(s, s);
    s = _mm_hadd_ps(s, s);
    return _mm_cvtss_f32(s);
}

static float dot_scalar(const float *a, const float *b) { float s = 0; for (int i = 0; i < 8; i++) s += a[i] * b[i]; return s; }

int main(void) {
    float a[8] = {1, 2, 3, 4, 5, 6, 7, 8}, b[8] = {8, 7, 6, 5, 4, 3, 2, 1};
    float d = __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma") ? dot8(a, b) : dot_scalar(a, b);
    printf("sum %g which %s dot %g\n", sum(a, 8), which(), d);
    return 0;
}
