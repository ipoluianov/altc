// flags: -mavx2 -mfma -mf16c -mbmi -mbmi2
// AVX2, FMA, F16C and BMI2 intrinsics; asm operands in ymm registers
#include <stdio.h>
#include <immintrin.h>
static void p8f(const char *n, __m256 v) { float f[8]; _mm256_storeu_ps(f, v); printf("%s", n); for (int i = 0; i < 8; i++) printf(" %g", f[i]); printf("\n"); }
static void p8i(const char *n, __m256i v) { int f[8]; _mm256_storeu_si256((__m256i *)f, v); printf("%s", n); for (int i = 0; i < 8; i++) printf(" %d", f[i]); printf("\n"); }
static float dot(const float *a, const float *b, int n) {
    __m256 acc = _mm256_setzero_ps();
    for (int i = 0; i < n; i += 8) acc = _mm256_fmadd_ps(_mm256_loadu_ps(a + i), _mm256_loadu_ps(b + i), acc);
    __m128 lo = _mm256_castps256_ps128(acc), hi = _mm256_extractf128_ps(acc, 1);
    __m128 s = _mm_add_ps(lo, hi);
    s = _mm_hadd_ps(s, s);
    s = _mm_hadd_ps(s, s);
    return _mm_cvtss_f32(s);
}
static int count_byte(const unsigned char *p, int n, unsigned char c) {
    __m256i needle = _mm256_set1_epi8((char)c);
    int cnt = 0;
    for (int i = 0; i < n; i += 32) {
        __m256i v = _mm256_loadu_si256((const __m256i *)(p + i));
        cnt += __builtin_popcount((unsigned)_mm256_movemask_epi8(_mm256_cmpeq_epi8(v, needle)));
    }
    return cnt;
}
int main(void) {
    __m256 a = _mm256_setr_ps(1, 2, 3, 4, 5, 6, 7, 8), b = _mm256_set1_ps(0.5f);
    p8f("add", _mm256_add_ps(a, b));
    p8f("fmadd", _mm256_fmadd_ps(a, a, b));
    p8f("fmsub", _mm256_fmsub_ps(a, b, a));
    p8f("perm", _mm256_permute2f128_ps(a, b, 0x21));
    p8f("permil", _mm256_permute_ps(a, _MM_SHUFFLE(0, 1, 2, 3)));
    p8f("permvar", _mm256_permutevar8x32_ps(a, _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0)));
    p8f("blendv", _mm256_blendv_ps(a, b, _mm256_cmp_ps(a, _mm256_set1_ps(4.5f), _CMP_GT_OQ)));
    p8f("round", _mm256_round_ps(_mm256_mul_ps(a, _mm256_set1_ps(0.7f)), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
    p8f("insert", _mm256_insertf128_ps(a, _mm_set1_ps(9), 0));
    printf("movemask %x\n", _mm256_movemask_ps(_mm256_cmp_ps(a, b, _CMP_GE_OQ)));
    __m256i x = _mm256_setr_epi32(1, -2, 3, -4, 5, -6, 7, -8), y = _mm256_set1_epi32(3);
    p8i("addi", _mm256_add_epi32(x, y));
    p8i("mullo", _mm256_mullo_epi32(x, y));
    p8i("sllv", _mm256_sllv_epi32(y, _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7)));
    p8i("srai", _mm256_srai_epi32(x, 1));
    p8i("perm4x64", _mm256_permute4x64_epi64(x, _MM_SHUFFLE(0, 1, 2, 3)));
    p8i("shuffle8", _mm256_shuffle_epi8(x, _mm256_set1_epi32(0x00010203)));
    p8i("cvtepu8", _mm256_cvtepu8_epi32(_mm_setr_epi8(1, 2, 3, (char)200, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0)));
    p8i("abs", _mm256_abs_epi32(x));
    p8i("max", _mm256_max_epi32(x, _mm256_setzero_si256()));
    p8i("unpack", _mm256_unpacklo_epi32(x, y));
    p8i("blend32", _mm256_blend_epi32(x, y, 0xA5));
    p8i("bcast", _mm256_broadcastd_epi32(_mm_cvtsi32_si128(77)));
    printf("extract %d %lld\n", _mm256_extract_epi32(x, 5), (long long)_mm256_extract_epi64(x, 3));
    printf("testz %d %d\n", _mm256_testz_si256(x, _mm256_setzero_si256()), _mm256_testz_si256(x, x));
    float fa[64], fb[64];
    for (int i = 0; i < 64; i++) { fa[i] = i * 0.25f; fb[i] = 1.0f - i * 0.01f; }
    printf("dot %.4f\n", dot(fa, fb, 64));
    unsigned char text[256];
    for (int i = 0; i < 256; i++) text[i] = (unsigned char)(i * 7 % 13);
    printf("count %d\n", count_byte(text, 256, 5));
    __m128i h = _mm_cvtps_ph(_mm_setr_ps(1.5f, -2, 65504, 0.1f), 0);
    __m128 back = _mm_cvtph_ps(h);
    float bf[4]; _mm_storeu_ps(bf, back);
    printf("f16c %g %g %g %g\n", bf[0], bf[1], bf[2], bf[3]);
    printf("bmi2 %x %x %u %u\n", _pdep_u32(0x15, 0xF0F0), _pext_u32(0xABCD, 0xFF00), _bzhi_u32(0xFFFFFFFF, 7), _tzcnt_u32(0));
    int fm[8]; _mm256_storeu_si256((__m256i *)fm, _mm256_maskload_epi32((const int *)fa, _mm256_setr_epi32(-1, 0, -1, 0, -1, 0, -1, 0)));
    printf("maskload %d %d\n", fm[1], fm[2] == *(int *)&fa[2]);
    return 0;
}
