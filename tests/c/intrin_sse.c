// flags: -msse4.1 -mssse3
// SSE to SSE4.1 intrinsics from <immintrin.h>
#include <stdio.h>
#include <immintrin.h>
static void p4f(const char *n, __m128 v) { float f[4]; _mm_storeu_ps(f, v); printf("%s %g %g %g %g\n", n, f[0], f[1], f[2], f[3]); }
static void p4i(const char *n, __m128i v) { int f[4]; _mm_storeu_si128((__m128i *)f, v); printf("%s %d %d %d %d\n", n, f[0], f[1], f[2], f[3]); }
int main(void) {
    __m128 a = _mm_set_ps(4, 3, 2, 1), b = _mm_set1_ps(0.5f);
    p4f("add", _mm_add_ps(a, b));
    p4f("mul", _mm_mul_ps(a, _mm_sub_ps(a, b)));
    p4f("shuf", _mm_shuffle_ps(a, b, _MM_SHUFFLE(0, 1, 2, 3)));
    p4f("max", _mm_max_ps(a, _mm_set_ps(0, 10, 0, 10)));
    p4f("sqrt", _mm_sqrt_ps(a));
    p4f("unpack", _mm_unpacklo_ps(a, b));
    printf("movemask %d\n", _mm_movemask_ps(_mm_cmplt_ps(a, _mm_set1_ps(2.5f))));
    printf("cvt %d %g\n", _mm_cvtss_si32(_mm_set_ss(3.7f)), _mm_cvtss_f32(_mm_add_ss(a, b)));
    __m128i x = _mm_setr_epi32(1, -2, 3, 0x7fffffff), y = _mm_set1_epi32(5);
    p4i("addi", _mm_add_epi32(x, y));
    p4i("cmpgt", _mm_cmpgt_epi32(x, y));
    p4i("slli", _mm_slli_epi32(x, 3));
    int n = 2;
    p4i("srai", _mm_srai_epi32(x, n));
    p4i("shuffle", _mm_shuffle_epi32(x, _MM_SHUFFLE(0, 1, 2, 3)));
    p4i("mullo", _mm_mullo_epi32(x, y));
    p4i("min", _mm_min_epi32(x, y));
    p4i("abs", _mm_abs_epi32(x));
    __m128i bytes = _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    __m128i rev = _mm_shuffle_epi8(bytes, _mm_setr_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0));
    p4i("pshufb", rev);
    p4i("srli_si128", _mm_srli_si128(bytes, 4));
    printf("extract %d %d %lld\n", _mm_extract_epi16(bytes, 3), _mm_extract_epi32(x, 2), _mm_extract_epi64(x, 1));
    p4i("insert", _mm_insert_epi32(x, 99, 1));
    printf("testz %d %d\n", _mm_testz_si128(x, _mm_setzero_si128()), _mm_testz_si128(x, x));
    printf("movemask8 %x\n", _mm_movemask_epi8(_mm_cmpeq_epi8(bytes, _mm_set1_epi8(5))));
    __m128d d = _mm_set_pd(2.5, -1.25);
    printf("pd %g %g\n", _mm_cvtsd_f64(_mm_mul_pd(d, d)), _mm_cvtsd_f64(_mm_unpackhi_pd(d, d)));
    p4f("blendv", _mm_blendv_ps(a, b, _mm_cmpgt_ps(a, _mm_set1_ps(2))));
    p4f("round", _mm_floor_ps(_mm_set_ps(-1.5f, 2.7f, -0.2f, 9.9f)));
    p4i("sad", _mm_sad_epu8(bytes, _mm_setzero_si128()));
    long long sum = 0;
    int arr[1024];
    for (int i = 0; i < 1024; i++) arr[i] = i * 7 - 300;
    __m128i acc = _mm_setzero_si128();
    for (int i = 0; i < 1024; i += 4) acc = _mm_add_epi32(acc, _mm_loadu_si128((const __m128i *)(arr + i)));
    acc = _mm_hadd_epi32(acc, acc);
    acc = _mm_hadd_epi32(acc, acc);
    sum = _mm_cvtsi128_si32(acc);
    printf("sum %lld\n", sum);
    return 0;
}
