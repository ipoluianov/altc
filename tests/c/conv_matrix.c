#include <stdio.h>
int main(void) {
    { volatile signed char v = (signed char)0; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)0; static signed char r3 = (signed char)(signed char)0;
      printf("1: (signed char)(signed char)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)0; static unsigned char r3 = (unsigned char)(signed char)0;
      printf("2: (unsigned char)(signed char)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0; short r1 = (short)v; short r2 = (short)(signed char)0; static short r3 = (short)(signed char)0;
      printf("3: (short)(signed char)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)0; static unsigned short r3 = (unsigned short)(signed char)0;
      printf("4: (unsigned short)(signed char)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0; int r1 = (int)v; int r2 = (int)(signed char)0; static int r3 = (int)(signed char)0;
      printf("5: (int)(signed char)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)0; static unsigned r3 = (unsigned)(signed char)0;
      printf("6: (unsigned)(signed char)0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)0; long r1 = (long)v; long r2 = (long)(signed char)0; static long r3 = (long)(signed char)0;
      printf("7: (long)(signed char)0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)0; static unsigned long r3 = (unsigned long)(signed char)0;
      printf("8: (unsigned long)(signed char)0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)0; long long r1 = (long long)v; long long r2 = (long long)(signed char)0; static long long r3 = (long long)(signed char)0;
      printf("9: (long long)(signed char)0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)0; static unsigned long long r3 = (unsigned long long)(signed char)0;
      printf("10: (unsigned long long)(signed char)0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)0; static _Bool r3 = (_Bool)(signed char)0;
      printf("11: (_Bool)(signed char)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0; float r1 = (float)v; float r2 = (float)(signed char)0; static float r3 = (float)(signed char)0;
      printf("12: (float)(signed char)0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)0; double r1 = (double)v; double r2 = (double)(signed char)0; static double r3 = (double)(signed char)0;
      printf("13: (double)(signed char)0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)0; long double r1 = (long double)v; long double r2 = (long double)(signed char)0; static long double r3 = (long double)(signed char)0;
      printf("14: (long double)(signed char)0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)1; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)1; static signed char r3 = (signed char)(signed char)1;
      printf("15: (signed char)(signed char)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)1; static unsigned char r3 = (unsigned char)(signed char)1;
      printf("16: (unsigned char)(signed char)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)1; short r1 = (short)v; short r2 = (short)(signed char)1; static short r3 = (short)(signed char)1;
      printf("17: (short)(signed char)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)1; static unsigned short r3 = (unsigned short)(signed char)1;
      printf("18: (unsigned short)(signed char)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)1; int r1 = (int)v; int r2 = (int)(signed char)1; static int r3 = (int)(signed char)1;
      printf("19: (int)(signed char)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)1; static unsigned r3 = (unsigned)(signed char)1;
      printf("20: (unsigned)(signed char)1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)1; long r1 = (long)v; long r2 = (long)(signed char)1; static long r3 = (long)(signed char)1;
      printf("21: (long)(signed char)1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)1; static unsigned long r3 = (unsigned long)(signed char)1;
      printf("22: (unsigned long)(signed char)1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)1; long long r1 = (long long)v; long long r2 = (long long)(signed char)1; static long long r3 = (long long)(signed char)1;
      printf("23: (long long)(signed char)1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)1; static unsigned long long r3 = (unsigned long long)(signed char)1;
      printf("24: (unsigned long long)(signed char)1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)1; static _Bool r3 = (_Bool)(signed char)1;
      printf("25: (_Bool)(signed char)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)1; float r1 = (float)v; float r2 = (float)(signed char)1; static float r3 = (float)(signed char)1;
      printf("26: (float)(signed char)1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)1; double r1 = (double)v; double r2 = (double)(signed char)1; static double r3 = (double)(signed char)1;
      printf("27: (double)(signed char)1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)1; long double r1 = (long double)v; long double r2 = (long double)(signed char)1; static long double r3 = (long double)(signed char)1;
      printf("28: (long double)(signed char)1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)-1; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)-1; static signed char r3 = (signed char)(signed char)-1;
      printf("29: (signed char)(signed char)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)-1; static unsigned char r3 = (unsigned char)(signed char)-1;
      printf("30: (unsigned char)(signed char)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-1; short r1 = (short)v; short r2 = (short)(signed char)-1; static short r3 = (short)(signed char)-1;
      printf("31: (short)(signed char)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)-1; static unsigned short r3 = (unsigned short)(signed char)-1;
      printf("32: (unsigned short)(signed char)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-1; int r1 = (int)v; int r2 = (int)(signed char)-1; static int r3 = (int)(signed char)-1;
      printf("33: (int)(signed char)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)-1; static unsigned r3 = (unsigned)(signed char)-1;
      printf("34: (unsigned)(signed char)-1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)-1; long r1 = (long)v; long r2 = (long)(signed char)-1; static long r3 = (long)(signed char)-1;
      printf("35: (long)(signed char)-1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)-1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)-1; static unsigned long r3 = (unsigned long)(signed char)-1;
      printf("36: (unsigned long)(signed char)-1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)-1; long long r1 = (long long)v; long long r2 = (long long)(signed char)-1; static long long r3 = (long long)(signed char)-1;
      printf("37: (long long)(signed char)-1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)-1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)-1; static unsigned long long r3 = (unsigned long long)(signed char)-1;
      printf("38: (unsigned long long)(signed char)-1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)-1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)-1; static _Bool r3 = (_Bool)(signed char)-1;
      printf("39: (_Bool)(signed char)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-1; float r1 = (float)v; float r2 = (float)(signed char)-1; static float r3 = (float)(signed char)-1;
      printf("40: (float)(signed char)-1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)-1; double r1 = (double)v; double r2 = (double)(signed char)-1; static double r3 = (double)(signed char)-1;
      printf("41: (double)(signed char)-1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)-1; long double r1 = (long double)v; long double r2 = (long double)(signed char)-1; static long double r3 = (long double)(signed char)-1;
      printf("42: (long double)(signed char)-1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)127; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)127; static signed char r3 = (signed char)(signed char)127;
      printf("43: (signed char)(signed char)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)127; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)127; static unsigned char r3 = (unsigned char)(signed char)127;
      printf("44: (unsigned char)(signed char)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)127; short r1 = (short)v; short r2 = (short)(signed char)127; static short r3 = (short)(signed char)127;
      printf("45: (short)(signed char)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)127; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)127; static unsigned short r3 = (unsigned short)(signed char)127;
      printf("46: (unsigned short)(signed char)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)127; int r1 = (int)v; int r2 = (int)(signed char)127; static int r3 = (int)(signed char)127;
      printf("47: (int)(signed char)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)127; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)127; static unsigned r3 = (unsigned)(signed char)127;
      printf("48: (unsigned)(signed char)127 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)127; long r1 = (long)v; long r2 = (long)(signed char)127; static long r3 = (long)(signed char)127;
      printf("49: (long)(signed char)127 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)127; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)127; static unsigned long r3 = (unsigned long)(signed char)127;
      printf("50: (unsigned long)(signed char)127 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)127; long long r1 = (long long)v; long long r2 = (long long)(signed char)127; static long long r3 = (long long)(signed char)127;
      printf("51: (long long)(signed char)127 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)127; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)127; static unsigned long long r3 = (unsigned long long)(signed char)127;
      printf("52: (unsigned long long)(signed char)127 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)127; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)127; static _Bool r3 = (_Bool)(signed char)127;
      printf("53: (_Bool)(signed char)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)127; float r1 = (float)v; float r2 = (float)(signed char)127; static float r3 = (float)(signed char)127;
      printf("54: (float)(signed char)127 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)127; double r1 = (double)v; double r2 = (double)(signed char)127; static double r3 = (double)(signed char)127;
      printf("55: (double)(signed char)127 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)127; long double r1 = (long double)v; long double r2 = (long double)(signed char)127; static long double r3 = (long double)(signed char)127;
      printf("56: (long double)(signed char)127 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)-128; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)-128; static signed char r3 = (signed char)(signed char)-128;
      printf("57: (signed char)(signed char)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-128; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)-128; static unsigned char r3 = (unsigned char)(signed char)-128;
      printf("58: (unsigned char)(signed char)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-128; short r1 = (short)v; short r2 = (short)(signed char)-128; static short r3 = (short)(signed char)-128;
      printf("59: (short)(signed char)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-128; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)-128; static unsigned short r3 = (unsigned short)(signed char)-128;
      printf("60: (unsigned short)(signed char)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-128; int r1 = (int)v; int r2 = (int)(signed char)-128; static int r3 = (int)(signed char)-128;
      printf("61: (int)(signed char)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-128; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)-128; static unsigned r3 = (unsigned)(signed char)-128;
      printf("62: (unsigned)(signed char)-128 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)-128; long r1 = (long)v; long r2 = (long)(signed char)-128; static long r3 = (long)(signed char)-128;
      printf("63: (long)(signed char)-128 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)-128; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)-128; static unsigned long r3 = (unsigned long)(signed char)-128;
      printf("64: (unsigned long)(signed char)-128 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)-128; long long r1 = (long long)v; long long r2 = (long long)(signed char)-128; static long long r3 = (long long)(signed char)-128;
      printf("65: (long long)(signed char)-128 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)-128; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)-128; static unsigned long long r3 = (unsigned long long)(signed char)-128;
      printf("66: (unsigned long long)(signed char)-128 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)-128; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)-128; static _Bool r3 = (_Bool)(signed char)-128;
      printf("67: (_Bool)(signed char)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-128; float r1 = (float)v; float r2 = (float)(signed char)-128; static float r3 = (float)(signed char)-128;
      printf("68: (float)(signed char)-128 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)-128; double r1 = (double)v; double r2 = (double)(signed char)-128; static double r3 = (double)(signed char)-128;
      printf("69: (double)(signed char)-128 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)-128; long double r1 = (long double)v; long double r2 = (long double)(signed char)-128; static long double r3 = (long double)(signed char)-128;
      printf("70: (long double)(signed char)-128 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)255; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)255; static signed char r3 = (signed char)(signed char)255;
      printf("71: (signed char)(signed char)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)255; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)255; static unsigned char r3 = (unsigned char)(signed char)255;
      printf("72: (unsigned char)(signed char)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)255; short r1 = (short)v; short r2 = (short)(signed char)255; static short r3 = (short)(signed char)255;
      printf("73: (short)(signed char)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)255; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)255; static unsigned short r3 = (unsigned short)(signed char)255;
      printf("74: (unsigned short)(signed char)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)255; int r1 = (int)v; int r2 = (int)(signed char)255; static int r3 = (int)(signed char)255;
      printf("75: (int)(signed char)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)255; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)255; static unsigned r3 = (unsigned)(signed char)255;
      printf("76: (unsigned)(signed char)255 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)255; long r1 = (long)v; long r2 = (long)(signed char)255; static long r3 = (long)(signed char)255;
      printf("77: (long)(signed char)255 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)255; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)255; static unsigned long r3 = (unsigned long)(signed char)255;
      printf("78: (unsigned long)(signed char)255 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)255; long long r1 = (long long)v; long long r2 = (long long)(signed char)255; static long long r3 = (long long)(signed char)255;
      printf("79: (long long)(signed char)255 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)255; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)255; static unsigned long long r3 = (unsigned long long)(signed char)255;
      printf("80: (unsigned long long)(signed char)255 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)255; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)255; static _Bool r3 = (_Bool)(signed char)255;
      printf("81: (_Bool)(signed char)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)255; float r1 = (float)v; float r2 = (float)(signed char)255; static float r3 = (float)(signed char)255;
      printf("82: (float)(signed char)255 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)255; double r1 = (double)v; double r2 = (double)(signed char)255; static double r3 = (double)(signed char)255;
      printf("83: (double)(signed char)255 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)255; long double r1 = (long double)v; long double r2 = (long double)(signed char)255; static long double r3 = (long double)(signed char)255;
      printf("84: (long double)(signed char)255 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)256; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)256; static signed char r3 = (signed char)(signed char)256;
      printf("85: (signed char)(signed char)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)256; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)256; static unsigned char r3 = (unsigned char)(signed char)256;
      printf("86: (unsigned char)(signed char)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)256; short r1 = (short)v; short r2 = (short)(signed char)256; static short r3 = (short)(signed char)256;
      printf("87: (short)(signed char)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)256; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)256; static unsigned short r3 = (unsigned short)(signed char)256;
      printf("88: (unsigned short)(signed char)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)256; int r1 = (int)v; int r2 = (int)(signed char)256; static int r3 = (int)(signed char)256;
      printf("89: (int)(signed char)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)256; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)256; static unsigned r3 = (unsigned)(signed char)256;
      printf("90: (unsigned)(signed char)256 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)256; long r1 = (long)v; long r2 = (long)(signed char)256; static long r3 = (long)(signed char)256;
      printf("91: (long)(signed char)256 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)256; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)256; static unsigned long r3 = (unsigned long)(signed char)256;
      printf("92: (unsigned long)(signed char)256 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)256; long long r1 = (long long)v; long long r2 = (long long)(signed char)256; static long long r3 = (long long)(signed char)256;
      printf("93: (long long)(signed char)256 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)256; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)256; static unsigned long long r3 = (unsigned long long)(signed char)256;
      printf("94: (unsigned long long)(signed char)256 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)256; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)256; static _Bool r3 = (_Bool)(signed char)256;
      printf("95: (_Bool)(signed char)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)256; float r1 = (float)v; float r2 = (float)(signed char)256; static float r3 = (float)(signed char)256;
      printf("96: (float)(signed char)256 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)256; double r1 = (double)v; double r2 = (double)(signed char)256; static double r3 = (double)(signed char)256;
      printf("97: (double)(signed char)256 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)256; long double r1 = (long double)v; long double r2 = (long double)(signed char)256; static long double r3 = (long double)(signed char)256;
      printf("98: (long double)(signed char)256 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)32767; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)32767; static signed char r3 = (signed char)(signed char)32767;
      printf("99: (signed char)(signed char)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)32767; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)32767; static unsigned char r3 = (unsigned char)(signed char)32767;
      printf("100: (unsigned char)(signed char)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)32767; short r1 = (short)v; short r2 = (short)(signed char)32767; static short r3 = (short)(signed char)32767;
      printf("101: (short)(signed char)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)32767; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)32767; static unsigned short r3 = (unsigned short)(signed char)32767;
      printf("102: (unsigned short)(signed char)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)32767; int r1 = (int)v; int r2 = (int)(signed char)32767; static int r3 = (int)(signed char)32767;
      printf("103: (int)(signed char)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)32767; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)32767; static unsigned r3 = (unsigned)(signed char)32767;
      printf("104: (unsigned)(signed char)32767 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)32767; long r1 = (long)v; long r2 = (long)(signed char)32767; static long r3 = (long)(signed char)32767;
      printf("105: (long)(signed char)32767 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)32767; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)32767; static unsigned long r3 = (unsigned long)(signed char)32767;
      printf("106: (unsigned long)(signed char)32767 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)32767; long long r1 = (long long)v; long long r2 = (long long)(signed char)32767; static long long r3 = (long long)(signed char)32767;
      printf("107: (long long)(signed char)32767 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)32767; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)32767; static unsigned long long r3 = (unsigned long long)(signed char)32767;
      printf("108: (unsigned long long)(signed char)32767 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)32767; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)32767; static _Bool r3 = (_Bool)(signed char)32767;
      printf("109: (_Bool)(signed char)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)32767; float r1 = (float)v; float r2 = (float)(signed char)32767; static float r3 = (float)(signed char)32767;
      printf("110: (float)(signed char)32767 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)32767; double r1 = (double)v; double r2 = (double)(signed char)32767; static double r3 = (double)(signed char)32767;
      printf("111: (double)(signed char)32767 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)32767; long double r1 = (long double)v; long double r2 = (long double)(signed char)32767; static long double r3 = (long double)(signed char)32767;
      printf("112: (long double)(signed char)32767 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)-32768; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)-32768; static signed char r3 = (signed char)(signed char)-32768;
      printf("113: (signed char)(signed char)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-32768; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)-32768; static unsigned char r3 = (unsigned char)(signed char)-32768;
      printf("114: (unsigned char)(signed char)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-32768; short r1 = (short)v; short r2 = (short)(signed char)-32768; static short r3 = (short)(signed char)-32768;
      printf("115: (short)(signed char)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-32768; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)-32768; static unsigned short r3 = (unsigned short)(signed char)-32768;
      printf("116: (unsigned short)(signed char)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-32768; int r1 = (int)v; int r2 = (int)(signed char)-32768; static int r3 = (int)(signed char)-32768;
      printf("117: (int)(signed char)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-32768; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)-32768; static unsigned r3 = (unsigned)(signed char)-32768;
      printf("118: (unsigned)(signed char)-32768 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)-32768; long r1 = (long)v; long r2 = (long)(signed char)-32768; static long r3 = (long)(signed char)-32768;
      printf("119: (long)(signed char)-32768 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)-32768; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)-32768; static unsigned long r3 = (unsigned long)(signed char)-32768;
      printf("120: (unsigned long)(signed char)-32768 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)-32768; long long r1 = (long long)v; long long r2 = (long long)(signed char)-32768; static long long r3 = (long long)(signed char)-32768;
      printf("121: (long long)(signed char)-32768 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)-32768; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)-32768; static unsigned long long r3 = (unsigned long long)(signed char)-32768;
      printf("122: (unsigned long long)(signed char)-32768 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)-32768; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)-32768; static _Bool r3 = (_Bool)(signed char)-32768;
      printf("123: (_Bool)(signed char)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)-32768; float r1 = (float)v; float r2 = (float)(signed char)-32768; static float r3 = (float)(signed char)-32768;
      printf("124: (float)(signed char)-32768 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)-32768; double r1 = (double)v; double r2 = (double)(signed char)-32768; static double r3 = (double)(signed char)-32768;
      printf("125: (double)(signed char)-32768 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)-32768; long double r1 = (long double)v; long double r2 = (long double)(signed char)-32768; static long double r3 = (long double)(signed char)-32768;
      printf("126: (long double)(signed char)-32768 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)65535; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)65535; static signed char r3 = (signed char)(signed char)65535;
      printf("127: (signed char)(signed char)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)65535; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)65535; static unsigned char r3 = (unsigned char)(signed char)65535;
      printf("128: (unsigned char)(signed char)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)65535; short r1 = (short)v; short r2 = (short)(signed char)65535; static short r3 = (short)(signed char)65535;
      printf("129: (short)(signed char)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)65535; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)65535; static unsigned short r3 = (unsigned short)(signed char)65535;
      printf("130: (unsigned short)(signed char)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)65535; int r1 = (int)v; int r2 = (int)(signed char)65535; static int r3 = (int)(signed char)65535;
      printf("131: (int)(signed char)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)65535; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)65535; static unsigned r3 = (unsigned)(signed char)65535;
      printf("132: (unsigned)(signed char)65535 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)65535; long r1 = (long)v; long r2 = (long)(signed char)65535; static long r3 = (long)(signed char)65535;
      printf("133: (long)(signed char)65535 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)65535; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)65535; static unsigned long r3 = (unsigned long)(signed char)65535;
      printf("134: (unsigned long)(signed char)65535 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)65535; long long r1 = (long long)v; long long r2 = (long long)(signed char)65535; static long long r3 = (long long)(signed char)65535;
      printf("135: (long long)(signed char)65535 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)65535; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)65535; static unsigned long long r3 = (unsigned long long)(signed char)65535;
      printf("136: (unsigned long long)(signed char)65535 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)65535; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)65535; static _Bool r3 = (_Bool)(signed char)65535;
      printf("137: (_Bool)(signed char)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)65535; float r1 = (float)v; float r2 = (float)(signed char)65535; static float r3 = (float)(signed char)65535;
      printf("138: (float)(signed char)65535 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)65535; double r1 = (double)v; double r2 = (double)(signed char)65535; static double r3 = (double)(signed char)65535;
      printf("139: (double)(signed char)65535 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)65535; long double r1 = (long double)v; long double r2 = (long double)(signed char)65535; static long double r3 = (long double)(signed char)65535;
      printf("140: (long double)(signed char)65535 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)2147483647; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)2147483647; static signed char r3 = (signed char)(signed char)2147483647;
      printf("141: (signed char)(signed char)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)2147483647; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)2147483647; static unsigned char r3 = (unsigned char)(signed char)2147483647;
      printf("142: (unsigned char)(signed char)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)2147483647; short r1 = (short)v; short r2 = (short)(signed char)2147483647; static short r3 = (short)(signed char)2147483647;
      printf("143: (short)(signed char)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)2147483647; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)2147483647; static unsigned short r3 = (unsigned short)(signed char)2147483647;
      printf("144: (unsigned short)(signed char)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)2147483647; int r1 = (int)v; int r2 = (int)(signed char)2147483647; static int r3 = (int)(signed char)2147483647;
      printf("145: (int)(signed char)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)2147483647; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)2147483647; static unsigned r3 = (unsigned)(signed char)2147483647;
      printf("146: (unsigned)(signed char)2147483647 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)2147483647; long r1 = (long)v; long r2 = (long)(signed char)2147483647; static long r3 = (long)(signed char)2147483647;
      printf("147: (long)(signed char)2147483647 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)2147483647; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)2147483647; static unsigned long r3 = (unsigned long)(signed char)2147483647;
      printf("148: (unsigned long)(signed char)2147483647 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)2147483647; long long r1 = (long long)v; long long r2 = (long long)(signed char)2147483647; static long long r3 = (long long)(signed char)2147483647;
      printf("149: (long long)(signed char)2147483647 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)2147483647; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)2147483647; static unsigned long long r3 = (unsigned long long)(signed char)2147483647;
      printf("150: (unsigned long long)(signed char)2147483647 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)2147483647; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)2147483647; static _Bool r3 = (_Bool)(signed char)2147483647;
      printf("151: (_Bool)(signed char)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)2147483647; float r1 = (float)v; float r2 = (float)(signed char)2147483647; static float r3 = (float)(signed char)2147483647;
      printf("152: (float)(signed char)2147483647 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)2147483647; double r1 = (double)v; double r2 = (double)(signed char)2147483647; static double r3 = (double)(signed char)2147483647;
      printf("153: (double)(signed char)2147483647 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)2147483647; long double r1 = (long double)v; long double r2 = (long double)(signed char)2147483647; static long double r3 = (long double)(signed char)2147483647;
      printf("154: (long double)(signed char)2147483647 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)(-2147483647-1); static signed char r3 = (signed char)(signed char)(-2147483647-1);
      printf("155: (signed char)(signed char)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)(-2147483647-1); static unsigned char r3 = (unsigned char)(signed char)(-2147483647-1);
      printf("156: (unsigned char)(signed char)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); short r1 = (short)v; short r2 = (short)(signed char)(-2147483647-1); static short r3 = (short)(signed char)(-2147483647-1);
      printf("157: (short)(signed char)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)(-2147483647-1); static unsigned short r3 = (unsigned short)(signed char)(-2147483647-1);
      printf("158: (unsigned short)(signed char)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); int r1 = (int)v; int r2 = (int)(signed char)(-2147483647-1); static int r3 = (int)(signed char)(-2147483647-1);
      printf("159: (int)(signed char)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)(-2147483647-1); static unsigned r3 = (unsigned)(signed char)(-2147483647-1);
      printf("160: (unsigned)(signed char)(-2147483647-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); long r1 = (long)v; long r2 = (long)(signed char)(-2147483647-1); static long r3 = (long)(signed char)(-2147483647-1);
      printf("161: (long)(signed char)(-2147483647-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)(-2147483647-1); static unsigned long r3 = (unsigned long)(signed char)(-2147483647-1);
      printf("162: (unsigned long)(signed char)(-2147483647-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); long long r1 = (long long)v; long long r2 = (long long)(signed char)(-2147483647-1); static long long r3 = (long long)(signed char)(-2147483647-1);
      printf("163: (long long)(signed char)(-2147483647-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)(-2147483647-1); static unsigned long long r3 = (unsigned long long)(signed char)(-2147483647-1);
      printf("164: (unsigned long long)(signed char)(-2147483647-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)(-2147483647-1); static _Bool r3 = (_Bool)(signed char)(-2147483647-1);
      printf("165: (_Bool)(signed char)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); float r1 = (float)v; float r2 = (float)(signed char)(-2147483647-1); static float r3 = (float)(signed char)(-2147483647-1);
      printf("166: (float)(signed char)(-2147483647-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); double r1 = (double)v; double r2 = (double)(signed char)(-2147483647-1); static double r3 = (double)(signed char)(-2147483647-1);
      printf("167: (double)(signed char)(-2147483647-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)(-2147483647-1); long double r1 = (long double)v; long double r2 = (long double)(signed char)(-2147483647-1); static long double r3 = (long double)(signed char)(-2147483647-1);
      printf("168: (long double)(signed char)(-2147483647-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)4294967295LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)4294967295LL; static signed char r3 = (signed char)(signed char)4294967295LL;
      printf("169: (signed char)(signed char)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)4294967295LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)4294967295LL; static unsigned char r3 = (unsigned char)(signed char)4294967295LL;
      printf("170: (unsigned char)(signed char)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)4294967295LL; short r1 = (short)v; short r2 = (short)(signed char)4294967295LL; static short r3 = (short)(signed char)4294967295LL;
      printf("171: (short)(signed char)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)4294967295LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)4294967295LL; static unsigned short r3 = (unsigned short)(signed char)4294967295LL;
      printf("172: (unsigned short)(signed char)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)4294967295LL; int r1 = (int)v; int r2 = (int)(signed char)4294967295LL; static int r3 = (int)(signed char)4294967295LL;
      printf("173: (int)(signed char)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)4294967295LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)4294967295LL; static unsigned r3 = (unsigned)(signed char)4294967295LL;
      printf("174: (unsigned)(signed char)4294967295LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)4294967295LL; long r1 = (long)v; long r2 = (long)(signed char)4294967295LL; static long r3 = (long)(signed char)4294967295LL;
      printf("175: (long)(signed char)4294967295LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)4294967295LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)4294967295LL; static unsigned long r3 = (unsigned long)(signed char)4294967295LL;
      printf("176: (unsigned long)(signed char)4294967295LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)4294967295LL; long long r1 = (long long)v; long long r2 = (long long)(signed char)4294967295LL; static long long r3 = (long long)(signed char)4294967295LL;
      printf("177: (long long)(signed char)4294967295LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)4294967295LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)4294967295LL; static unsigned long long r3 = (unsigned long long)(signed char)4294967295LL;
      printf("178: (unsigned long long)(signed char)4294967295LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)4294967295LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)4294967295LL; static _Bool r3 = (_Bool)(signed char)4294967295LL;
      printf("179: (_Bool)(signed char)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)4294967295LL; float r1 = (float)v; float r2 = (float)(signed char)4294967295LL; static float r3 = (float)(signed char)4294967295LL;
      printf("180: (float)(signed char)4294967295LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)4294967295LL; double r1 = (double)v; double r2 = (double)(signed char)4294967295LL; static double r3 = (double)(signed char)4294967295LL;
      printf("181: (double)(signed char)4294967295LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)4294967295LL; long double r1 = (long double)v; long double r2 = (long double)(signed char)4294967295LL; static long double r3 = (long double)(signed char)4294967295LL;
      printf("182: (long double)(signed char)4294967295LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)9223372036854775807LL; static signed char r3 = (signed char)(signed char)9223372036854775807LL;
      printf("183: (signed char)(signed char)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)9223372036854775807LL; static unsigned char r3 = (unsigned char)(signed char)9223372036854775807LL;
      printf("184: (unsigned char)(signed char)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; short r1 = (short)v; short r2 = (short)(signed char)9223372036854775807LL; static short r3 = (short)(signed char)9223372036854775807LL;
      printf("185: (short)(signed char)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)9223372036854775807LL; static unsigned short r3 = (unsigned short)(signed char)9223372036854775807LL;
      printf("186: (unsigned short)(signed char)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; int r1 = (int)v; int r2 = (int)(signed char)9223372036854775807LL; static int r3 = (int)(signed char)9223372036854775807LL;
      printf("187: (int)(signed char)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)9223372036854775807LL; static unsigned r3 = (unsigned)(signed char)9223372036854775807LL;
      printf("188: (unsigned)(signed char)9223372036854775807LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; long r1 = (long)v; long r2 = (long)(signed char)9223372036854775807LL; static long r3 = (long)(signed char)9223372036854775807LL;
      printf("189: (long)(signed char)9223372036854775807LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)9223372036854775807LL; static unsigned long r3 = (unsigned long)(signed char)9223372036854775807LL;
      printf("190: (unsigned long)(signed char)9223372036854775807LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; long long r1 = (long long)v; long long r2 = (long long)(signed char)9223372036854775807LL; static long long r3 = (long long)(signed char)9223372036854775807LL;
      printf("191: (long long)(signed char)9223372036854775807LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)9223372036854775807LL; static unsigned long long r3 = (unsigned long long)(signed char)9223372036854775807LL;
      printf("192: (unsigned long long)(signed char)9223372036854775807LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)9223372036854775807LL; static _Bool r3 = (_Bool)(signed char)9223372036854775807LL;
      printf("193: (_Bool)(signed char)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; float r1 = (float)v; float r2 = (float)(signed char)9223372036854775807LL; static float r3 = (float)(signed char)9223372036854775807LL;
      printf("194: (float)(signed char)9223372036854775807LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; double r1 = (double)v; double r2 = (double)(signed char)9223372036854775807LL; static double r3 = (double)(signed char)9223372036854775807LL;
      printf("195: (double)(signed char)9223372036854775807LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)9223372036854775807LL; long double r1 = (long double)v; long double r2 = (long double)(signed char)9223372036854775807LL; static long double r3 = (long double)(signed char)9223372036854775807LL;
      printf("196: (long double)(signed char)9223372036854775807LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)(-9223372036854775807LL-1); static signed char r3 = (signed char)(signed char)(-9223372036854775807LL-1);
      printf("197: (signed char)(signed char)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)(-9223372036854775807LL-1); static unsigned char r3 = (unsigned char)(signed char)(-9223372036854775807LL-1);
      printf("198: (unsigned char)(signed char)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); short r1 = (short)v; short r2 = (short)(signed char)(-9223372036854775807LL-1); static short r3 = (short)(signed char)(-9223372036854775807LL-1);
      printf("199: (short)(signed char)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)(-9223372036854775807LL-1); static unsigned short r3 = (unsigned short)(signed char)(-9223372036854775807LL-1);
      printf("200: (unsigned short)(signed char)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); int r1 = (int)v; int r2 = (int)(signed char)(-9223372036854775807LL-1); static int r3 = (int)(signed char)(-9223372036854775807LL-1);
      printf("201: (int)(signed char)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)(-9223372036854775807LL-1); static unsigned r3 = (unsigned)(signed char)(-9223372036854775807LL-1);
      printf("202: (unsigned)(signed char)(-9223372036854775807LL-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); long r1 = (long)v; long r2 = (long)(signed char)(-9223372036854775807LL-1); static long r3 = (long)(signed char)(-9223372036854775807LL-1);
      printf("203: (long)(signed char)(-9223372036854775807LL-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)(-9223372036854775807LL-1); static unsigned long r3 = (unsigned long)(signed char)(-9223372036854775807LL-1);
      printf("204: (unsigned long)(signed char)(-9223372036854775807LL-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); long long r1 = (long long)v; long long r2 = (long long)(signed char)(-9223372036854775807LL-1); static long long r3 = (long long)(signed char)(-9223372036854775807LL-1);
      printf("205: (long long)(signed char)(-9223372036854775807LL-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)(-9223372036854775807LL-1); static unsigned long long r3 = (unsigned long long)(signed char)(-9223372036854775807LL-1);
      printf("206: (unsigned long long)(signed char)(-9223372036854775807LL-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)(-9223372036854775807LL-1); static _Bool r3 = (_Bool)(signed char)(-9223372036854775807LL-1);
      printf("207: (_Bool)(signed char)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); float r1 = (float)v; float r2 = (float)(signed char)(-9223372036854775807LL-1); static float r3 = (float)(signed char)(-9223372036854775807LL-1);
      printf("208: (float)(signed char)(-9223372036854775807LL-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); double r1 = (double)v; double r2 = (double)(signed char)(-9223372036854775807LL-1); static double r3 = (double)(signed char)(-9223372036854775807LL-1);
      printf("209: (double)(signed char)(-9223372036854775807LL-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)(-9223372036854775807LL-1); long double r1 = (long double)v; long double r2 = (long double)(signed char)(-9223372036854775807LL-1); static long double r3 = (long double)(signed char)(-9223372036854775807LL-1);
      printf("210: (long double)(signed char)(-9223372036854775807LL-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)0x8000000000000000ULL; static signed char r3 = (signed char)(signed char)0x8000000000000000ULL;
      printf("211: (signed char)(signed char)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)0x8000000000000000ULL; static unsigned char r3 = (unsigned char)(signed char)0x8000000000000000ULL;
      printf("212: (unsigned char)(signed char)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; short r1 = (short)v; short r2 = (short)(signed char)0x8000000000000000ULL; static short r3 = (short)(signed char)0x8000000000000000ULL;
      printf("213: (short)(signed char)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)0x8000000000000000ULL; static unsigned short r3 = (unsigned short)(signed char)0x8000000000000000ULL;
      printf("214: (unsigned short)(signed char)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; int r1 = (int)v; int r2 = (int)(signed char)0x8000000000000000ULL; static int r3 = (int)(signed char)0x8000000000000000ULL;
      printf("215: (int)(signed char)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)0x8000000000000000ULL; static unsigned r3 = (unsigned)(signed char)0x8000000000000000ULL;
      printf("216: (unsigned)(signed char)0x8000000000000000ULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; long r1 = (long)v; long r2 = (long)(signed char)0x8000000000000000ULL; static long r3 = (long)(signed char)0x8000000000000000ULL;
      printf("217: (long)(signed char)0x8000000000000000ULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)0x8000000000000000ULL; static unsigned long r3 = (unsigned long)(signed char)0x8000000000000000ULL;
      printf("218: (unsigned long)(signed char)0x8000000000000000ULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; long long r1 = (long long)v; long long r2 = (long long)(signed char)0x8000000000000000ULL; static long long r3 = (long long)(signed char)0x8000000000000000ULL;
      printf("219: (long long)(signed char)0x8000000000000000ULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)0x8000000000000000ULL; static unsigned long long r3 = (unsigned long long)(signed char)0x8000000000000000ULL;
      printf("220: (unsigned long long)(signed char)0x8000000000000000ULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)0x8000000000000000ULL; static _Bool r3 = (_Bool)(signed char)0x8000000000000000ULL;
      printf("221: (_Bool)(signed char)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; float r1 = (float)v; float r2 = (float)(signed char)0x8000000000000000ULL; static float r3 = (float)(signed char)0x8000000000000000ULL;
      printf("222: (float)(signed char)0x8000000000000000ULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; double r1 = (double)v; double r2 = (double)(signed char)0x8000000000000000ULL; static double r3 = (double)(signed char)0x8000000000000000ULL;
      printf("223: (double)(signed char)0x8000000000000000ULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)0x8000000000000000ULL; long double r1 = (long double)v; long double r2 = (long double)(signed char)0x8000000000000000ULL; static long double r3 = (long double)(signed char)0x8000000000000000ULL;
      printf("224: (long double)(signed char)0x8000000000000000ULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(signed char)0xffffffffffffffffULL; static signed char r3 = (signed char)(signed char)0xffffffffffffffffULL;
      printf("225: (signed char)(signed char)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(signed char)0xffffffffffffffffULL; static unsigned char r3 = (unsigned char)(signed char)0xffffffffffffffffULL;
      printf("226: (unsigned char)(signed char)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; short r1 = (short)v; short r2 = (short)(signed char)0xffffffffffffffffULL; static short r3 = (short)(signed char)0xffffffffffffffffULL;
      printf("227: (short)(signed char)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(signed char)0xffffffffffffffffULL; static unsigned short r3 = (unsigned short)(signed char)0xffffffffffffffffULL;
      printf("228: (unsigned short)(signed char)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; int r1 = (int)v; int r2 = (int)(signed char)0xffffffffffffffffULL; static int r3 = (int)(signed char)0xffffffffffffffffULL;
      printf("229: (int)(signed char)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(signed char)0xffffffffffffffffULL; static unsigned r3 = (unsigned)(signed char)0xffffffffffffffffULL;
      printf("230: (unsigned)(signed char)0xffffffffffffffffULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; long r1 = (long)v; long r2 = (long)(signed char)0xffffffffffffffffULL; static long r3 = (long)(signed char)0xffffffffffffffffULL;
      printf("231: (long)(signed char)0xffffffffffffffffULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(signed char)0xffffffffffffffffULL; static unsigned long r3 = (unsigned long)(signed char)0xffffffffffffffffULL;
      printf("232: (unsigned long)(signed char)0xffffffffffffffffULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; long long r1 = (long long)v; long long r2 = (long long)(signed char)0xffffffffffffffffULL; static long long r3 = (long long)(signed char)0xffffffffffffffffULL;
      printf("233: (long long)(signed char)0xffffffffffffffffULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(signed char)0xffffffffffffffffULL; static unsigned long long r3 = (unsigned long long)(signed char)0xffffffffffffffffULL;
      printf("234: (unsigned long long)(signed char)0xffffffffffffffffULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(signed char)0xffffffffffffffffULL; static _Bool r3 = (_Bool)(signed char)0xffffffffffffffffULL;
      printf("235: (_Bool)(signed char)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; float r1 = (float)v; float r2 = (float)(signed char)0xffffffffffffffffULL; static float r3 = (float)(signed char)0xffffffffffffffffULL;
      printf("236: (float)(signed char)0xffffffffffffffffULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; double r1 = (double)v; double r2 = (double)(signed char)0xffffffffffffffffULL; static double r3 = (double)(signed char)0xffffffffffffffffULL;
      printf("237: (double)(signed char)0xffffffffffffffffULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile signed char v = (signed char)0xffffffffffffffffULL; long double r1 = (long double)v; long double r2 = (long double)(signed char)0xffffffffffffffffULL; static long double r3 = (long double)(signed char)0xffffffffffffffffULL;
      printf("238: (long double)(signed char)0xffffffffffffffffULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)0; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)0; static signed char r3 = (signed char)(unsigned char)0;
      printf("239: (signed char)(unsigned char)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)0; static unsigned char r3 = (unsigned char)(unsigned char)0;
      printf("240: (unsigned char)(unsigned char)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0; short r1 = (short)v; short r2 = (short)(unsigned char)0; static short r3 = (short)(unsigned char)0;
      printf("241: (short)(unsigned char)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)0; static unsigned short r3 = (unsigned short)(unsigned char)0;
      printf("242: (unsigned short)(unsigned char)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0; int r1 = (int)v; int r2 = (int)(unsigned char)0; static int r3 = (int)(unsigned char)0;
      printf("243: (int)(unsigned char)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)0; static unsigned r3 = (unsigned)(unsigned char)0;
      printf("244: (unsigned)(unsigned char)0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)0; long r1 = (long)v; long r2 = (long)(unsigned char)0; static long r3 = (long)(unsigned char)0;
      printf("245: (long)(unsigned char)0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)0; static unsigned long r3 = (unsigned long)(unsigned char)0;
      printf("246: (unsigned long)(unsigned char)0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)0; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)0; static long long r3 = (long long)(unsigned char)0;
      printf("247: (long long)(unsigned char)0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)0; static unsigned long long r3 = (unsigned long long)(unsigned char)0;
      printf("248: (unsigned long long)(unsigned char)0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)0; static _Bool r3 = (_Bool)(unsigned char)0;
      printf("249: (_Bool)(unsigned char)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0; float r1 = (float)v; float r2 = (float)(unsigned char)0; static float r3 = (float)(unsigned char)0;
      printf("250: (float)(unsigned char)0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)0; double r1 = (double)v; double r2 = (double)(unsigned char)0; static double r3 = (double)(unsigned char)0;
      printf("251: (double)(unsigned char)0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)0; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)0; static long double r3 = (long double)(unsigned char)0;
      printf("252: (long double)(unsigned char)0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)1; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)1; static signed char r3 = (signed char)(unsigned char)1;
      printf("253: (signed char)(unsigned char)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)1; static unsigned char r3 = (unsigned char)(unsigned char)1;
      printf("254: (unsigned char)(unsigned char)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)1; short r1 = (short)v; short r2 = (short)(unsigned char)1; static short r3 = (short)(unsigned char)1;
      printf("255: (short)(unsigned char)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)1; static unsigned short r3 = (unsigned short)(unsigned char)1;
      printf("256: (unsigned short)(unsigned char)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)1; int r1 = (int)v; int r2 = (int)(unsigned char)1; static int r3 = (int)(unsigned char)1;
      printf("257: (int)(unsigned char)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)1; static unsigned r3 = (unsigned)(unsigned char)1;
      printf("258: (unsigned)(unsigned char)1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)1; long r1 = (long)v; long r2 = (long)(unsigned char)1; static long r3 = (long)(unsigned char)1;
      printf("259: (long)(unsigned char)1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)1; static unsigned long r3 = (unsigned long)(unsigned char)1;
      printf("260: (unsigned long)(unsigned char)1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)1; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)1; static long long r3 = (long long)(unsigned char)1;
      printf("261: (long long)(unsigned char)1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)1; static unsigned long long r3 = (unsigned long long)(unsigned char)1;
      printf("262: (unsigned long long)(unsigned char)1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)1; static _Bool r3 = (_Bool)(unsigned char)1;
      printf("263: (_Bool)(unsigned char)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)1; float r1 = (float)v; float r2 = (float)(unsigned char)1; static float r3 = (float)(unsigned char)1;
      printf("264: (float)(unsigned char)1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)1; double r1 = (double)v; double r2 = (double)(unsigned char)1; static double r3 = (double)(unsigned char)1;
      printf("265: (double)(unsigned char)1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)1; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)1; static long double r3 = (long double)(unsigned char)1;
      printf("266: (long double)(unsigned char)1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)-1; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)-1; static signed char r3 = (signed char)(unsigned char)-1;
      printf("267: (signed char)(unsigned char)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)-1; static unsigned char r3 = (unsigned char)(unsigned char)-1;
      printf("268: (unsigned char)(unsigned char)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-1; short r1 = (short)v; short r2 = (short)(unsigned char)-1; static short r3 = (short)(unsigned char)-1;
      printf("269: (short)(unsigned char)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)-1; static unsigned short r3 = (unsigned short)(unsigned char)-1;
      printf("270: (unsigned short)(unsigned char)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-1; int r1 = (int)v; int r2 = (int)(unsigned char)-1; static int r3 = (int)(unsigned char)-1;
      printf("271: (int)(unsigned char)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)-1; static unsigned r3 = (unsigned)(unsigned char)-1;
      printf("272: (unsigned)(unsigned char)-1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)-1; long r1 = (long)v; long r2 = (long)(unsigned char)-1; static long r3 = (long)(unsigned char)-1;
      printf("273: (long)(unsigned char)-1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)-1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)-1; static unsigned long r3 = (unsigned long)(unsigned char)-1;
      printf("274: (unsigned long)(unsigned char)-1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)-1; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)-1; static long long r3 = (long long)(unsigned char)-1;
      printf("275: (long long)(unsigned char)-1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)-1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)-1; static unsigned long long r3 = (unsigned long long)(unsigned char)-1;
      printf("276: (unsigned long long)(unsigned char)-1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)-1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)-1; static _Bool r3 = (_Bool)(unsigned char)-1;
      printf("277: (_Bool)(unsigned char)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-1; float r1 = (float)v; float r2 = (float)(unsigned char)-1; static float r3 = (float)(unsigned char)-1;
      printf("278: (float)(unsigned char)-1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)-1; double r1 = (double)v; double r2 = (double)(unsigned char)-1; static double r3 = (double)(unsigned char)-1;
      printf("279: (double)(unsigned char)-1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)-1; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)-1; static long double r3 = (long double)(unsigned char)-1;
      printf("280: (long double)(unsigned char)-1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)127; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)127; static signed char r3 = (signed char)(unsigned char)127;
      printf("281: (signed char)(unsigned char)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)127; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)127; static unsigned char r3 = (unsigned char)(unsigned char)127;
      printf("282: (unsigned char)(unsigned char)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)127; short r1 = (short)v; short r2 = (short)(unsigned char)127; static short r3 = (short)(unsigned char)127;
      printf("283: (short)(unsigned char)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)127; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)127; static unsigned short r3 = (unsigned short)(unsigned char)127;
      printf("284: (unsigned short)(unsigned char)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)127; int r1 = (int)v; int r2 = (int)(unsigned char)127; static int r3 = (int)(unsigned char)127;
      printf("285: (int)(unsigned char)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)127; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)127; static unsigned r3 = (unsigned)(unsigned char)127;
      printf("286: (unsigned)(unsigned char)127 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)127; long r1 = (long)v; long r2 = (long)(unsigned char)127; static long r3 = (long)(unsigned char)127;
      printf("287: (long)(unsigned char)127 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)127; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)127; static unsigned long r3 = (unsigned long)(unsigned char)127;
      printf("288: (unsigned long)(unsigned char)127 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)127; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)127; static long long r3 = (long long)(unsigned char)127;
      printf("289: (long long)(unsigned char)127 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)127; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)127; static unsigned long long r3 = (unsigned long long)(unsigned char)127;
      printf("290: (unsigned long long)(unsigned char)127 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)127; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)127; static _Bool r3 = (_Bool)(unsigned char)127;
      printf("291: (_Bool)(unsigned char)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)127; float r1 = (float)v; float r2 = (float)(unsigned char)127; static float r3 = (float)(unsigned char)127;
      printf("292: (float)(unsigned char)127 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)127; double r1 = (double)v; double r2 = (double)(unsigned char)127; static double r3 = (double)(unsigned char)127;
      printf("293: (double)(unsigned char)127 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)127; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)127; static long double r3 = (long double)(unsigned char)127;
      printf("294: (long double)(unsigned char)127 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)-128; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)-128; static signed char r3 = (signed char)(unsigned char)-128;
      printf("295: (signed char)(unsigned char)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-128; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)-128; static unsigned char r3 = (unsigned char)(unsigned char)-128;
      printf("296: (unsigned char)(unsigned char)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-128; short r1 = (short)v; short r2 = (short)(unsigned char)-128; static short r3 = (short)(unsigned char)-128;
      printf("297: (short)(unsigned char)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-128; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)-128; static unsigned short r3 = (unsigned short)(unsigned char)-128;
      printf("298: (unsigned short)(unsigned char)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-128; int r1 = (int)v; int r2 = (int)(unsigned char)-128; static int r3 = (int)(unsigned char)-128;
      printf("299: (int)(unsigned char)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-128; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)-128; static unsigned r3 = (unsigned)(unsigned char)-128;
      printf("300: (unsigned)(unsigned char)-128 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)-128; long r1 = (long)v; long r2 = (long)(unsigned char)-128; static long r3 = (long)(unsigned char)-128;
      printf("301: (long)(unsigned char)-128 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)-128; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)-128; static unsigned long r3 = (unsigned long)(unsigned char)-128;
      printf("302: (unsigned long)(unsigned char)-128 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)-128; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)-128; static long long r3 = (long long)(unsigned char)-128;
      printf("303: (long long)(unsigned char)-128 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)-128; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)-128; static unsigned long long r3 = (unsigned long long)(unsigned char)-128;
      printf("304: (unsigned long long)(unsigned char)-128 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)-128; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)-128; static _Bool r3 = (_Bool)(unsigned char)-128;
      printf("305: (_Bool)(unsigned char)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-128; float r1 = (float)v; float r2 = (float)(unsigned char)-128; static float r3 = (float)(unsigned char)-128;
      printf("306: (float)(unsigned char)-128 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)-128; double r1 = (double)v; double r2 = (double)(unsigned char)-128; static double r3 = (double)(unsigned char)-128;
      printf("307: (double)(unsigned char)-128 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)-128; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)-128; static long double r3 = (long double)(unsigned char)-128;
      printf("308: (long double)(unsigned char)-128 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)255; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)255; static signed char r3 = (signed char)(unsigned char)255;
      printf("309: (signed char)(unsigned char)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)255; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)255; static unsigned char r3 = (unsigned char)(unsigned char)255;
      printf("310: (unsigned char)(unsigned char)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)255; short r1 = (short)v; short r2 = (short)(unsigned char)255; static short r3 = (short)(unsigned char)255;
      printf("311: (short)(unsigned char)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)255; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)255; static unsigned short r3 = (unsigned short)(unsigned char)255;
      printf("312: (unsigned short)(unsigned char)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)255; int r1 = (int)v; int r2 = (int)(unsigned char)255; static int r3 = (int)(unsigned char)255;
      printf("313: (int)(unsigned char)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)255; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)255; static unsigned r3 = (unsigned)(unsigned char)255;
      printf("314: (unsigned)(unsigned char)255 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)255; long r1 = (long)v; long r2 = (long)(unsigned char)255; static long r3 = (long)(unsigned char)255;
      printf("315: (long)(unsigned char)255 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)255; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)255; static unsigned long r3 = (unsigned long)(unsigned char)255;
      printf("316: (unsigned long)(unsigned char)255 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)255; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)255; static long long r3 = (long long)(unsigned char)255;
      printf("317: (long long)(unsigned char)255 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)255; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)255; static unsigned long long r3 = (unsigned long long)(unsigned char)255;
      printf("318: (unsigned long long)(unsigned char)255 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)255; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)255; static _Bool r3 = (_Bool)(unsigned char)255;
      printf("319: (_Bool)(unsigned char)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)255; float r1 = (float)v; float r2 = (float)(unsigned char)255; static float r3 = (float)(unsigned char)255;
      printf("320: (float)(unsigned char)255 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)255; double r1 = (double)v; double r2 = (double)(unsigned char)255; static double r3 = (double)(unsigned char)255;
      printf("321: (double)(unsigned char)255 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)255; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)255; static long double r3 = (long double)(unsigned char)255;
      printf("322: (long double)(unsigned char)255 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)256; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)256; static signed char r3 = (signed char)(unsigned char)256;
      printf("323: (signed char)(unsigned char)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)256; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)256; static unsigned char r3 = (unsigned char)(unsigned char)256;
      printf("324: (unsigned char)(unsigned char)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)256; short r1 = (short)v; short r2 = (short)(unsigned char)256; static short r3 = (short)(unsigned char)256;
      printf("325: (short)(unsigned char)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)256; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)256; static unsigned short r3 = (unsigned short)(unsigned char)256;
      printf("326: (unsigned short)(unsigned char)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)256; int r1 = (int)v; int r2 = (int)(unsigned char)256; static int r3 = (int)(unsigned char)256;
      printf("327: (int)(unsigned char)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)256; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)256; static unsigned r3 = (unsigned)(unsigned char)256;
      printf("328: (unsigned)(unsigned char)256 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)256; long r1 = (long)v; long r2 = (long)(unsigned char)256; static long r3 = (long)(unsigned char)256;
      printf("329: (long)(unsigned char)256 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)256; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)256; static unsigned long r3 = (unsigned long)(unsigned char)256;
      printf("330: (unsigned long)(unsigned char)256 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)256; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)256; static long long r3 = (long long)(unsigned char)256;
      printf("331: (long long)(unsigned char)256 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)256; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)256; static unsigned long long r3 = (unsigned long long)(unsigned char)256;
      printf("332: (unsigned long long)(unsigned char)256 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)256; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)256; static _Bool r3 = (_Bool)(unsigned char)256;
      printf("333: (_Bool)(unsigned char)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)256; float r1 = (float)v; float r2 = (float)(unsigned char)256; static float r3 = (float)(unsigned char)256;
      printf("334: (float)(unsigned char)256 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)256; double r1 = (double)v; double r2 = (double)(unsigned char)256; static double r3 = (double)(unsigned char)256;
      printf("335: (double)(unsigned char)256 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)256; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)256; static long double r3 = (long double)(unsigned char)256;
      printf("336: (long double)(unsigned char)256 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)32767; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)32767; static signed char r3 = (signed char)(unsigned char)32767;
      printf("337: (signed char)(unsigned char)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)32767; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)32767; static unsigned char r3 = (unsigned char)(unsigned char)32767;
      printf("338: (unsigned char)(unsigned char)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)32767; short r1 = (short)v; short r2 = (short)(unsigned char)32767; static short r3 = (short)(unsigned char)32767;
      printf("339: (short)(unsigned char)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)32767; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)32767; static unsigned short r3 = (unsigned short)(unsigned char)32767;
      printf("340: (unsigned short)(unsigned char)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)32767; int r1 = (int)v; int r2 = (int)(unsigned char)32767; static int r3 = (int)(unsigned char)32767;
      printf("341: (int)(unsigned char)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)32767; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)32767; static unsigned r3 = (unsigned)(unsigned char)32767;
      printf("342: (unsigned)(unsigned char)32767 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)32767; long r1 = (long)v; long r2 = (long)(unsigned char)32767; static long r3 = (long)(unsigned char)32767;
      printf("343: (long)(unsigned char)32767 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)32767; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)32767; static unsigned long r3 = (unsigned long)(unsigned char)32767;
      printf("344: (unsigned long)(unsigned char)32767 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)32767; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)32767; static long long r3 = (long long)(unsigned char)32767;
      printf("345: (long long)(unsigned char)32767 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)32767; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)32767; static unsigned long long r3 = (unsigned long long)(unsigned char)32767;
      printf("346: (unsigned long long)(unsigned char)32767 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)32767; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)32767; static _Bool r3 = (_Bool)(unsigned char)32767;
      printf("347: (_Bool)(unsigned char)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)32767; float r1 = (float)v; float r2 = (float)(unsigned char)32767; static float r3 = (float)(unsigned char)32767;
      printf("348: (float)(unsigned char)32767 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)32767; double r1 = (double)v; double r2 = (double)(unsigned char)32767; static double r3 = (double)(unsigned char)32767;
      printf("349: (double)(unsigned char)32767 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)32767; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)32767; static long double r3 = (long double)(unsigned char)32767;
      printf("350: (long double)(unsigned char)32767 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)-32768; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)-32768; static signed char r3 = (signed char)(unsigned char)-32768;
      printf("351: (signed char)(unsigned char)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-32768; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)-32768; static unsigned char r3 = (unsigned char)(unsigned char)-32768;
      printf("352: (unsigned char)(unsigned char)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-32768; short r1 = (short)v; short r2 = (short)(unsigned char)-32768; static short r3 = (short)(unsigned char)-32768;
      printf("353: (short)(unsigned char)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-32768; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)-32768; static unsigned short r3 = (unsigned short)(unsigned char)-32768;
      printf("354: (unsigned short)(unsigned char)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-32768; int r1 = (int)v; int r2 = (int)(unsigned char)-32768; static int r3 = (int)(unsigned char)-32768;
      printf("355: (int)(unsigned char)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-32768; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)-32768; static unsigned r3 = (unsigned)(unsigned char)-32768;
      printf("356: (unsigned)(unsigned char)-32768 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)-32768; long r1 = (long)v; long r2 = (long)(unsigned char)-32768; static long r3 = (long)(unsigned char)-32768;
      printf("357: (long)(unsigned char)-32768 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)-32768; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)-32768; static unsigned long r3 = (unsigned long)(unsigned char)-32768;
      printf("358: (unsigned long)(unsigned char)-32768 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)-32768; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)-32768; static long long r3 = (long long)(unsigned char)-32768;
      printf("359: (long long)(unsigned char)-32768 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)-32768; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)-32768; static unsigned long long r3 = (unsigned long long)(unsigned char)-32768;
      printf("360: (unsigned long long)(unsigned char)-32768 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)-32768; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)-32768; static _Bool r3 = (_Bool)(unsigned char)-32768;
      printf("361: (_Bool)(unsigned char)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)-32768; float r1 = (float)v; float r2 = (float)(unsigned char)-32768; static float r3 = (float)(unsigned char)-32768;
      printf("362: (float)(unsigned char)-32768 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)-32768; double r1 = (double)v; double r2 = (double)(unsigned char)-32768; static double r3 = (double)(unsigned char)-32768;
      printf("363: (double)(unsigned char)-32768 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)-32768; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)-32768; static long double r3 = (long double)(unsigned char)-32768;
      printf("364: (long double)(unsigned char)-32768 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)65535; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)65535; static signed char r3 = (signed char)(unsigned char)65535;
      printf("365: (signed char)(unsigned char)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)65535; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)65535; static unsigned char r3 = (unsigned char)(unsigned char)65535;
      printf("366: (unsigned char)(unsigned char)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)65535; short r1 = (short)v; short r2 = (short)(unsigned char)65535; static short r3 = (short)(unsigned char)65535;
      printf("367: (short)(unsigned char)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)65535; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)65535; static unsigned short r3 = (unsigned short)(unsigned char)65535;
      printf("368: (unsigned short)(unsigned char)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)65535; int r1 = (int)v; int r2 = (int)(unsigned char)65535; static int r3 = (int)(unsigned char)65535;
      printf("369: (int)(unsigned char)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)65535; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)65535; static unsigned r3 = (unsigned)(unsigned char)65535;
      printf("370: (unsigned)(unsigned char)65535 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)65535; long r1 = (long)v; long r2 = (long)(unsigned char)65535; static long r3 = (long)(unsigned char)65535;
      printf("371: (long)(unsigned char)65535 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)65535; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)65535; static unsigned long r3 = (unsigned long)(unsigned char)65535;
      printf("372: (unsigned long)(unsigned char)65535 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)65535; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)65535; static long long r3 = (long long)(unsigned char)65535;
      printf("373: (long long)(unsigned char)65535 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)65535; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)65535; static unsigned long long r3 = (unsigned long long)(unsigned char)65535;
      printf("374: (unsigned long long)(unsigned char)65535 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)65535; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)65535; static _Bool r3 = (_Bool)(unsigned char)65535;
      printf("375: (_Bool)(unsigned char)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)65535; float r1 = (float)v; float r2 = (float)(unsigned char)65535; static float r3 = (float)(unsigned char)65535;
      printf("376: (float)(unsigned char)65535 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)65535; double r1 = (double)v; double r2 = (double)(unsigned char)65535; static double r3 = (double)(unsigned char)65535;
      printf("377: (double)(unsigned char)65535 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)65535; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)65535; static long double r3 = (long double)(unsigned char)65535;
      printf("378: (long double)(unsigned char)65535 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)2147483647; static signed char r3 = (signed char)(unsigned char)2147483647;
      printf("379: (signed char)(unsigned char)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)2147483647; static unsigned char r3 = (unsigned char)(unsigned char)2147483647;
      printf("380: (unsigned char)(unsigned char)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; short r1 = (short)v; short r2 = (short)(unsigned char)2147483647; static short r3 = (short)(unsigned char)2147483647;
      printf("381: (short)(unsigned char)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)2147483647; static unsigned short r3 = (unsigned short)(unsigned char)2147483647;
      printf("382: (unsigned short)(unsigned char)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; int r1 = (int)v; int r2 = (int)(unsigned char)2147483647; static int r3 = (int)(unsigned char)2147483647;
      printf("383: (int)(unsigned char)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)2147483647; static unsigned r3 = (unsigned)(unsigned char)2147483647;
      printf("384: (unsigned)(unsigned char)2147483647 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; long r1 = (long)v; long r2 = (long)(unsigned char)2147483647; static long r3 = (long)(unsigned char)2147483647;
      printf("385: (long)(unsigned char)2147483647 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)2147483647; static unsigned long r3 = (unsigned long)(unsigned char)2147483647;
      printf("386: (unsigned long)(unsigned char)2147483647 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)2147483647; static long long r3 = (long long)(unsigned char)2147483647;
      printf("387: (long long)(unsigned char)2147483647 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)2147483647; static unsigned long long r3 = (unsigned long long)(unsigned char)2147483647;
      printf("388: (unsigned long long)(unsigned char)2147483647 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)2147483647; static _Bool r3 = (_Bool)(unsigned char)2147483647;
      printf("389: (_Bool)(unsigned char)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; float r1 = (float)v; float r2 = (float)(unsigned char)2147483647; static float r3 = (float)(unsigned char)2147483647;
      printf("390: (float)(unsigned char)2147483647 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; double r1 = (double)v; double r2 = (double)(unsigned char)2147483647; static double r3 = (double)(unsigned char)2147483647;
      printf("391: (double)(unsigned char)2147483647 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)2147483647; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)2147483647; static long double r3 = (long double)(unsigned char)2147483647;
      printf("392: (long double)(unsigned char)2147483647 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)(-2147483647-1); static signed char r3 = (signed char)(unsigned char)(-2147483647-1);
      printf("393: (signed char)(unsigned char)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)(-2147483647-1); static unsigned char r3 = (unsigned char)(unsigned char)(-2147483647-1);
      printf("394: (unsigned char)(unsigned char)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); short r1 = (short)v; short r2 = (short)(unsigned char)(-2147483647-1); static short r3 = (short)(unsigned char)(-2147483647-1);
      printf("395: (short)(unsigned char)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)(-2147483647-1); static unsigned short r3 = (unsigned short)(unsigned char)(-2147483647-1);
      printf("396: (unsigned short)(unsigned char)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); int r1 = (int)v; int r2 = (int)(unsigned char)(-2147483647-1); static int r3 = (int)(unsigned char)(-2147483647-1);
      printf("397: (int)(unsigned char)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)(-2147483647-1); static unsigned r3 = (unsigned)(unsigned char)(-2147483647-1);
      printf("398: (unsigned)(unsigned char)(-2147483647-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); long r1 = (long)v; long r2 = (long)(unsigned char)(-2147483647-1); static long r3 = (long)(unsigned char)(-2147483647-1);
      printf("399: (long)(unsigned char)(-2147483647-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)(-2147483647-1); static unsigned long r3 = (unsigned long)(unsigned char)(-2147483647-1);
      printf("400: (unsigned long)(unsigned char)(-2147483647-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); long long r1 = (long long)v; long long r2 = (long long)(unsigned char)(-2147483647-1); static long long r3 = (long long)(unsigned char)(-2147483647-1);
      printf("401: (long long)(unsigned char)(-2147483647-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)(-2147483647-1); static unsigned long long r3 = (unsigned long long)(unsigned char)(-2147483647-1);
      printf("402: (unsigned long long)(unsigned char)(-2147483647-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)(-2147483647-1); static _Bool r3 = (_Bool)(unsigned char)(-2147483647-1);
      printf("403: (_Bool)(unsigned char)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); float r1 = (float)v; float r2 = (float)(unsigned char)(-2147483647-1); static float r3 = (float)(unsigned char)(-2147483647-1);
      printf("404: (float)(unsigned char)(-2147483647-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); double r1 = (double)v; double r2 = (double)(unsigned char)(-2147483647-1); static double r3 = (double)(unsigned char)(-2147483647-1);
      printf("405: (double)(unsigned char)(-2147483647-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)(-2147483647-1); long double r1 = (long double)v; long double r2 = (long double)(unsigned char)(-2147483647-1); static long double r3 = (long double)(unsigned char)(-2147483647-1);
      printf("406: (long double)(unsigned char)(-2147483647-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)4294967295LL; static signed char r3 = (signed char)(unsigned char)4294967295LL;
      printf("407: (signed char)(unsigned char)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)4294967295LL; static unsigned char r3 = (unsigned char)(unsigned char)4294967295LL;
      printf("408: (unsigned char)(unsigned char)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; short r1 = (short)v; short r2 = (short)(unsigned char)4294967295LL; static short r3 = (short)(unsigned char)4294967295LL;
      printf("409: (short)(unsigned char)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)4294967295LL; static unsigned short r3 = (unsigned short)(unsigned char)4294967295LL;
      printf("410: (unsigned short)(unsigned char)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; int r1 = (int)v; int r2 = (int)(unsigned char)4294967295LL; static int r3 = (int)(unsigned char)4294967295LL;
      printf("411: (int)(unsigned char)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)4294967295LL; static unsigned r3 = (unsigned)(unsigned char)4294967295LL;
      printf("412: (unsigned)(unsigned char)4294967295LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; long r1 = (long)v; long r2 = (long)(unsigned char)4294967295LL; static long r3 = (long)(unsigned char)4294967295LL;
      printf("413: (long)(unsigned char)4294967295LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)4294967295LL; static unsigned long r3 = (unsigned long)(unsigned char)4294967295LL;
      printf("414: (unsigned long)(unsigned char)4294967295LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)4294967295LL; static long long r3 = (long long)(unsigned char)4294967295LL;
      printf("415: (long long)(unsigned char)4294967295LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)4294967295LL; static unsigned long long r3 = (unsigned long long)(unsigned char)4294967295LL;
      printf("416: (unsigned long long)(unsigned char)4294967295LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)4294967295LL; static _Bool r3 = (_Bool)(unsigned char)4294967295LL;
      printf("417: (_Bool)(unsigned char)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; float r1 = (float)v; float r2 = (float)(unsigned char)4294967295LL; static float r3 = (float)(unsigned char)4294967295LL;
      printf("418: (float)(unsigned char)4294967295LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; double r1 = (double)v; double r2 = (double)(unsigned char)4294967295LL; static double r3 = (double)(unsigned char)4294967295LL;
      printf("419: (double)(unsigned char)4294967295LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)4294967295LL; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)4294967295LL; static long double r3 = (long double)(unsigned char)4294967295LL;
      printf("420: (long double)(unsigned char)4294967295LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)9223372036854775807LL; static signed char r3 = (signed char)(unsigned char)9223372036854775807LL;
      printf("421: (signed char)(unsigned char)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)9223372036854775807LL; static unsigned char r3 = (unsigned char)(unsigned char)9223372036854775807LL;
      printf("422: (unsigned char)(unsigned char)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; short r1 = (short)v; short r2 = (short)(unsigned char)9223372036854775807LL; static short r3 = (short)(unsigned char)9223372036854775807LL;
      printf("423: (short)(unsigned char)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)9223372036854775807LL; static unsigned short r3 = (unsigned short)(unsigned char)9223372036854775807LL;
      printf("424: (unsigned short)(unsigned char)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; int r1 = (int)v; int r2 = (int)(unsigned char)9223372036854775807LL; static int r3 = (int)(unsigned char)9223372036854775807LL;
      printf("425: (int)(unsigned char)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)9223372036854775807LL; static unsigned r3 = (unsigned)(unsigned char)9223372036854775807LL;
      printf("426: (unsigned)(unsigned char)9223372036854775807LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; long r1 = (long)v; long r2 = (long)(unsigned char)9223372036854775807LL; static long r3 = (long)(unsigned char)9223372036854775807LL;
      printf("427: (long)(unsigned char)9223372036854775807LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)9223372036854775807LL; static unsigned long r3 = (unsigned long)(unsigned char)9223372036854775807LL;
      printf("428: (unsigned long)(unsigned char)9223372036854775807LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)9223372036854775807LL; static long long r3 = (long long)(unsigned char)9223372036854775807LL;
      printf("429: (long long)(unsigned char)9223372036854775807LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)9223372036854775807LL; static unsigned long long r3 = (unsigned long long)(unsigned char)9223372036854775807LL;
      printf("430: (unsigned long long)(unsigned char)9223372036854775807LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)9223372036854775807LL; static _Bool r3 = (_Bool)(unsigned char)9223372036854775807LL;
      printf("431: (_Bool)(unsigned char)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; float r1 = (float)v; float r2 = (float)(unsigned char)9223372036854775807LL; static float r3 = (float)(unsigned char)9223372036854775807LL;
      printf("432: (float)(unsigned char)9223372036854775807LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; double r1 = (double)v; double r2 = (double)(unsigned char)9223372036854775807LL; static double r3 = (double)(unsigned char)9223372036854775807LL;
      printf("433: (double)(unsigned char)9223372036854775807LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)9223372036854775807LL; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)9223372036854775807LL; static long double r3 = (long double)(unsigned char)9223372036854775807LL;
      printf("434: (long double)(unsigned char)9223372036854775807LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)(-9223372036854775807LL-1); static signed char r3 = (signed char)(unsigned char)(-9223372036854775807LL-1);
      printf("435: (signed char)(unsigned char)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)(-9223372036854775807LL-1); static unsigned char r3 = (unsigned char)(unsigned char)(-9223372036854775807LL-1);
      printf("436: (unsigned char)(unsigned char)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); short r1 = (short)v; short r2 = (short)(unsigned char)(-9223372036854775807LL-1); static short r3 = (short)(unsigned char)(-9223372036854775807LL-1);
      printf("437: (short)(unsigned char)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)(-9223372036854775807LL-1); static unsigned short r3 = (unsigned short)(unsigned char)(-9223372036854775807LL-1);
      printf("438: (unsigned short)(unsigned char)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); int r1 = (int)v; int r2 = (int)(unsigned char)(-9223372036854775807LL-1); static int r3 = (int)(unsigned char)(-9223372036854775807LL-1);
      printf("439: (int)(unsigned char)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)(-9223372036854775807LL-1); static unsigned r3 = (unsigned)(unsigned char)(-9223372036854775807LL-1);
      printf("440: (unsigned)(unsigned char)(-9223372036854775807LL-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); long r1 = (long)v; long r2 = (long)(unsigned char)(-9223372036854775807LL-1); static long r3 = (long)(unsigned char)(-9223372036854775807LL-1);
      printf("441: (long)(unsigned char)(-9223372036854775807LL-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)(-9223372036854775807LL-1); static unsigned long r3 = (unsigned long)(unsigned char)(-9223372036854775807LL-1);
      printf("442: (unsigned long)(unsigned char)(-9223372036854775807LL-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); long long r1 = (long long)v; long long r2 = (long long)(unsigned char)(-9223372036854775807LL-1); static long long r3 = (long long)(unsigned char)(-9223372036854775807LL-1);
      printf("443: (long long)(unsigned char)(-9223372036854775807LL-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)(-9223372036854775807LL-1); static unsigned long long r3 = (unsigned long long)(unsigned char)(-9223372036854775807LL-1);
      printf("444: (unsigned long long)(unsigned char)(-9223372036854775807LL-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)(-9223372036854775807LL-1); static _Bool r3 = (_Bool)(unsigned char)(-9223372036854775807LL-1);
      printf("445: (_Bool)(unsigned char)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); float r1 = (float)v; float r2 = (float)(unsigned char)(-9223372036854775807LL-1); static float r3 = (float)(unsigned char)(-9223372036854775807LL-1);
      printf("446: (float)(unsigned char)(-9223372036854775807LL-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); double r1 = (double)v; double r2 = (double)(unsigned char)(-9223372036854775807LL-1); static double r3 = (double)(unsigned char)(-9223372036854775807LL-1);
      printf("447: (double)(unsigned char)(-9223372036854775807LL-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)(-9223372036854775807LL-1); long double r1 = (long double)v; long double r2 = (long double)(unsigned char)(-9223372036854775807LL-1); static long double r3 = (long double)(unsigned char)(-9223372036854775807LL-1);
      printf("448: (long double)(unsigned char)(-9223372036854775807LL-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)0x8000000000000000ULL; static signed char r3 = (signed char)(unsigned char)0x8000000000000000ULL;
      printf("449: (signed char)(unsigned char)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)0x8000000000000000ULL; static unsigned char r3 = (unsigned char)(unsigned char)0x8000000000000000ULL;
      printf("450: (unsigned char)(unsigned char)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; short r1 = (short)v; short r2 = (short)(unsigned char)0x8000000000000000ULL; static short r3 = (short)(unsigned char)0x8000000000000000ULL;
      printf("451: (short)(unsigned char)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)0x8000000000000000ULL; static unsigned short r3 = (unsigned short)(unsigned char)0x8000000000000000ULL;
      printf("452: (unsigned short)(unsigned char)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; int r1 = (int)v; int r2 = (int)(unsigned char)0x8000000000000000ULL; static int r3 = (int)(unsigned char)0x8000000000000000ULL;
      printf("453: (int)(unsigned char)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)0x8000000000000000ULL; static unsigned r3 = (unsigned)(unsigned char)0x8000000000000000ULL;
      printf("454: (unsigned)(unsigned char)0x8000000000000000ULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; long r1 = (long)v; long r2 = (long)(unsigned char)0x8000000000000000ULL; static long r3 = (long)(unsigned char)0x8000000000000000ULL;
      printf("455: (long)(unsigned char)0x8000000000000000ULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)0x8000000000000000ULL; static unsigned long r3 = (unsigned long)(unsigned char)0x8000000000000000ULL;
      printf("456: (unsigned long)(unsigned char)0x8000000000000000ULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)0x8000000000000000ULL; static long long r3 = (long long)(unsigned char)0x8000000000000000ULL;
      printf("457: (long long)(unsigned char)0x8000000000000000ULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)0x8000000000000000ULL; static unsigned long long r3 = (unsigned long long)(unsigned char)0x8000000000000000ULL;
      printf("458: (unsigned long long)(unsigned char)0x8000000000000000ULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)0x8000000000000000ULL; static _Bool r3 = (_Bool)(unsigned char)0x8000000000000000ULL;
      printf("459: (_Bool)(unsigned char)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; float r1 = (float)v; float r2 = (float)(unsigned char)0x8000000000000000ULL; static float r3 = (float)(unsigned char)0x8000000000000000ULL;
      printf("460: (float)(unsigned char)0x8000000000000000ULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; double r1 = (double)v; double r2 = (double)(unsigned char)0x8000000000000000ULL; static double r3 = (double)(unsigned char)0x8000000000000000ULL;
      printf("461: (double)(unsigned char)0x8000000000000000ULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)0x8000000000000000ULL; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)0x8000000000000000ULL; static long double r3 = (long double)(unsigned char)0x8000000000000000ULL;
      printf("462: (long double)(unsigned char)0x8000000000000000ULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned char)0xffffffffffffffffULL; static signed char r3 = (signed char)(unsigned char)0xffffffffffffffffULL;
      printf("463: (signed char)(unsigned char)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned char)0xffffffffffffffffULL; static unsigned char r3 = (unsigned char)(unsigned char)0xffffffffffffffffULL;
      printf("464: (unsigned char)(unsigned char)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; short r1 = (short)v; short r2 = (short)(unsigned char)0xffffffffffffffffULL; static short r3 = (short)(unsigned char)0xffffffffffffffffULL;
      printf("465: (short)(unsigned char)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned char)0xffffffffffffffffULL; static unsigned short r3 = (unsigned short)(unsigned char)0xffffffffffffffffULL;
      printf("466: (unsigned short)(unsigned char)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; int r1 = (int)v; int r2 = (int)(unsigned char)0xffffffffffffffffULL; static int r3 = (int)(unsigned char)0xffffffffffffffffULL;
      printf("467: (int)(unsigned char)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned char)0xffffffffffffffffULL; static unsigned r3 = (unsigned)(unsigned char)0xffffffffffffffffULL;
      printf("468: (unsigned)(unsigned char)0xffffffffffffffffULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; long r1 = (long)v; long r2 = (long)(unsigned char)0xffffffffffffffffULL; static long r3 = (long)(unsigned char)0xffffffffffffffffULL;
      printf("469: (long)(unsigned char)0xffffffffffffffffULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned char)0xffffffffffffffffULL; static unsigned long r3 = (unsigned long)(unsigned char)0xffffffffffffffffULL;
      printf("470: (unsigned long)(unsigned char)0xffffffffffffffffULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; long long r1 = (long long)v; long long r2 = (long long)(unsigned char)0xffffffffffffffffULL; static long long r3 = (long long)(unsigned char)0xffffffffffffffffULL;
      printf("471: (long long)(unsigned char)0xffffffffffffffffULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned char)0xffffffffffffffffULL; static unsigned long long r3 = (unsigned long long)(unsigned char)0xffffffffffffffffULL;
      printf("472: (unsigned long long)(unsigned char)0xffffffffffffffffULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned char)0xffffffffffffffffULL; static _Bool r3 = (_Bool)(unsigned char)0xffffffffffffffffULL;
      printf("473: (_Bool)(unsigned char)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; float r1 = (float)v; float r2 = (float)(unsigned char)0xffffffffffffffffULL; static float r3 = (float)(unsigned char)0xffffffffffffffffULL;
      printf("474: (float)(unsigned char)0xffffffffffffffffULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; double r1 = (double)v; double r2 = (double)(unsigned char)0xffffffffffffffffULL; static double r3 = (double)(unsigned char)0xffffffffffffffffULL;
      printf("475: (double)(unsigned char)0xffffffffffffffffULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char v = (unsigned char)0xffffffffffffffffULL; long double r1 = (long double)v; long double r2 = (long double)(unsigned char)0xffffffffffffffffULL; static long double r3 = (long double)(unsigned char)0xffffffffffffffffULL;
      printf("476: (long double)(unsigned char)0xffffffffffffffffULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)0; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)0; static signed char r3 = (signed char)(short)0;
      printf("477: (signed char)(short)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)0; static unsigned char r3 = (unsigned char)(short)0;
      printf("478: (unsigned char)(short)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0; short r1 = (short)v; short r2 = (short)(short)0; static short r3 = (short)(short)0;
      printf("479: (short)(short)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)0; static unsigned short r3 = (unsigned short)(short)0;
      printf("480: (unsigned short)(short)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0; int r1 = (int)v; int r2 = (int)(short)0; static int r3 = (int)(short)0;
      printf("481: (int)(short)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)0; static unsigned r3 = (unsigned)(short)0;
      printf("482: (unsigned)(short)0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)0; long r1 = (long)v; long r2 = (long)(short)0; static long r3 = (long)(short)0;
      printf("483: (long)(short)0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)0; static unsigned long r3 = (unsigned long)(short)0;
      printf("484: (unsigned long)(short)0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)0; long long r1 = (long long)v; long long r2 = (long long)(short)0; static long long r3 = (long long)(short)0;
      printf("485: (long long)(short)0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)0; static unsigned long long r3 = (unsigned long long)(short)0;
      printf("486: (unsigned long long)(short)0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)0; static _Bool r3 = (_Bool)(short)0;
      printf("487: (_Bool)(short)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0; float r1 = (float)v; float r2 = (float)(short)0; static float r3 = (float)(short)0;
      printf("488: (float)(short)0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)0; double r1 = (double)v; double r2 = (double)(short)0; static double r3 = (double)(short)0;
      printf("489: (double)(short)0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)0; long double r1 = (long double)v; long double r2 = (long double)(short)0; static long double r3 = (long double)(short)0;
      printf("490: (long double)(short)0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)1; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)1; static signed char r3 = (signed char)(short)1;
      printf("491: (signed char)(short)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)1; static unsigned char r3 = (unsigned char)(short)1;
      printf("492: (unsigned char)(short)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)1; short r1 = (short)v; short r2 = (short)(short)1; static short r3 = (short)(short)1;
      printf("493: (short)(short)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)1; static unsigned short r3 = (unsigned short)(short)1;
      printf("494: (unsigned short)(short)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)1; int r1 = (int)v; int r2 = (int)(short)1; static int r3 = (int)(short)1;
      printf("495: (int)(short)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)1; static unsigned r3 = (unsigned)(short)1;
      printf("496: (unsigned)(short)1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)1; long r1 = (long)v; long r2 = (long)(short)1; static long r3 = (long)(short)1;
      printf("497: (long)(short)1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)1; static unsigned long r3 = (unsigned long)(short)1;
      printf("498: (unsigned long)(short)1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)1; long long r1 = (long long)v; long long r2 = (long long)(short)1; static long long r3 = (long long)(short)1;
      printf("499: (long long)(short)1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)1; static unsigned long long r3 = (unsigned long long)(short)1;
      printf("500: (unsigned long long)(short)1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)1; static _Bool r3 = (_Bool)(short)1;
      printf("501: (_Bool)(short)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)1; float r1 = (float)v; float r2 = (float)(short)1; static float r3 = (float)(short)1;
      printf("502: (float)(short)1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)1; double r1 = (double)v; double r2 = (double)(short)1; static double r3 = (double)(short)1;
      printf("503: (double)(short)1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)1; long double r1 = (long double)v; long double r2 = (long double)(short)1; static long double r3 = (long double)(short)1;
      printf("504: (long double)(short)1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)-1; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)-1; static signed char r3 = (signed char)(short)-1;
      printf("505: (signed char)(short)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)-1; static unsigned char r3 = (unsigned char)(short)-1;
      printf("506: (unsigned char)(short)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-1; short r1 = (short)v; short r2 = (short)(short)-1; static short r3 = (short)(short)-1;
      printf("507: (short)(short)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)-1; static unsigned short r3 = (unsigned short)(short)-1;
      printf("508: (unsigned short)(short)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-1; int r1 = (int)v; int r2 = (int)(short)-1; static int r3 = (int)(short)-1;
      printf("509: (int)(short)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)-1; static unsigned r3 = (unsigned)(short)-1;
      printf("510: (unsigned)(short)-1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)-1; long r1 = (long)v; long r2 = (long)(short)-1; static long r3 = (long)(short)-1;
      printf("511: (long)(short)-1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)-1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)-1; static unsigned long r3 = (unsigned long)(short)-1;
      printf("512: (unsigned long)(short)-1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)-1; long long r1 = (long long)v; long long r2 = (long long)(short)-1; static long long r3 = (long long)(short)-1;
      printf("513: (long long)(short)-1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)-1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)-1; static unsigned long long r3 = (unsigned long long)(short)-1;
      printf("514: (unsigned long long)(short)-1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)-1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)-1; static _Bool r3 = (_Bool)(short)-1;
      printf("515: (_Bool)(short)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-1; float r1 = (float)v; float r2 = (float)(short)-1; static float r3 = (float)(short)-1;
      printf("516: (float)(short)-1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)-1; double r1 = (double)v; double r2 = (double)(short)-1; static double r3 = (double)(short)-1;
      printf("517: (double)(short)-1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)-1; long double r1 = (long double)v; long double r2 = (long double)(short)-1; static long double r3 = (long double)(short)-1;
      printf("518: (long double)(short)-1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)127; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)127; static signed char r3 = (signed char)(short)127;
      printf("519: (signed char)(short)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)127; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)127; static unsigned char r3 = (unsigned char)(short)127;
      printf("520: (unsigned char)(short)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)127; short r1 = (short)v; short r2 = (short)(short)127; static short r3 = (short)(short)127;
      printf("521: (short)(short)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)127; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)127; static unsigned short r3 = (unsigned short)(short)127;
      printf("522: (unsigned short)(short)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)127; int r1 = (int)v; int r2 = (int)(short)127; static int r3 = (int)(short)127;
      printf("523: (int)(short)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)127; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)127; static unsigned r3 = (unsigned)(short)127;
      printf("524: (unsigned)(short)127 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)127; long r1 = (long)v; long r2 = (long)(short)127; static long r3 = (long)(short)127;
      printf("525: (long)(short)127 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)127; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)127; static unsigned long r3 = (unsigned long)(short)127;
      printf("526: (unsigned long)(short)127 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)127; long long r1 = (long long)v; long long r2 = (long long)(short)127; static long long r3 = (long long)(short)127;
      printf("527: (long long)(short)127 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)127; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)127; static unsigned long long r3 = (unsigned long long)(short)127;
      printf("528: (unsigned long long)(short)127 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)127; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)127; static _Bool r3 = (_Bool)(short)127;
      printf("529: (_Bool)(short)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)127; float r1 = (float)v; float r2 = (float)(short)127; static float r3 = (float)(short)127;
      printf("530: (float)(short)127 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)127; double r1 = (double)v; double r2 = (double)(short)127; static double r3 = (double)(short)127;
      printf("531: (double)(short)127 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)127; long double r1 = (long double)v; long double r2 = (long double)(short)127; static long double r3 = (long double)(short)127;
      printf("532: (long double)(short)127 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)-128; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)-128; static signed char r3 = (signed char)(short)-128;
      printf("533: (signed char)(short)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-128; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)-128; static unsigned char r3 = (unsigned char)(short)-128;
      printf("534: (unsigned char)(short)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-128; short r1 = (short)v; short r2 = (short)(short)-128; static short r3 = (short)(short)-128;
      printf("535: (short)(short)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-128; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)-128; static unsigned short r3 = (unsigned short)(short)-128;
      printf("536: (unsigned short)(short)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-128; int r1 = (int)v; int r2 = (int)(short)-128; static int r3 = (int)(short)-128;
      printf("537: (int)(short)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-128; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)-128; static unsigned r3 = (unsigned)(short)-128;
      printf("538: (unsigned)(short)-128 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)-128; long r1 = (long)v; long r2 = (long)(short)-128; static long r3 = (long)(short)-128;
      printf("539: (long)(short)-128 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)-128; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)-128; static unsigned long r3 = (unsigned long)(short)-128;
      printf("540: (unsigned long)(short)-128 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)-128; long long r1 = (long long)v; long long r2 = (long long)(short)-128; static long long r3 = (long long)(short)-128;
      printf("541: (long long)(short)-128 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)-128; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)-128; static unsigned long long r3 = (unsigned long long)(short)-128;
      printf("542: (unsigned long long)(short)-128 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)-128; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)-128; static _Bool r3 = (_Bool)(short)-128;
      printf("543: (_Bool)(short)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-128; float r1 = (float)v; float r2 = (float)(short)-128; static float r3 = (float)(short)-128;
      printf("544: (float)(short)-128 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)-128; double r1 = (double)v; double r2 = (double)(short)-128; static double r3 = (double)(short)-128;
      printf("545: (double)(short)-128 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)-128; long double r1 = (long double)v; long double r2 = (long double)(short)-128; static long double r3 = (long double)(short)-128;
      printf("546: (long double)(short)-128 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)255; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)255; static signed char r3 = (signed char)(short)255;
      printf("547: (signed char)(short)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)255; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)255; static unsigned char r3 = (unsigned char)(short)255;
      printf("548: (unsigned char)(short)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)255; short r1 = (short)v; short r2 = (short)(short)255; static short r3 = (short)(short)255;
      printf("549: (short)(short)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)255; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)255; static unsigned short r3 = (unsigned short)(short)255;
      printf("550: (unsigned short)(short)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)255; int r1 = (int)v; int r2 = (int)(short)255; static int r3 = (int)(short)255;
      printf("551: (int)(short)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)255; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)255; static unsigned r3 = (unsigned)(short)255;
      printf("552: (unsigned)(short)255 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)255; long r1 = (long)v; long r2 = (long)(short)255; static long r3 = (long)(short)255;
      printf("553: (long)(short)255 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)255; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)255; static unsigned long r3 = (unsigned long)(short)255;
      printf("554: (unsigned long)(short)255 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)255; long long r1 = (long long)v; long long r2 = (long long)(short)255; static long long r3 = (long long)(short)255;
      printf("555: (long long)(short)255 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)255; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)255; static unsigned long long r3 = (unsigned long long)(short)255;
      printf("556: (unsigned long long)(short)255 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)255; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)255; static _Bool r3 = (_Bool)(short)255;
      printf("557: (_Bool)(short)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)255; float r1 = (float)v; float r2 = (float)(short)255; static float r3 = (float)(short)255;
      printf("558: (float)(short)255 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)255; double r1 = (double)v; double r2 = (double)(short)255; static double r3 = (double)(short)255;
      printf("559: (double)(short)255 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)255; long double r1 = (long double)v; long double r2 = (long double)(short)255; static long double r3 = (long double)(short)255;
      printf("560: (long double)(short)255 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)256; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)256; static signed char r3 = (signed char)(short)256;
      printf("561: (signed char)(short)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)256; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)256; static unsigned char r3 = (unsigned char)(short)256;
      printf("562: (unsigned char)(short)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)256; short r1 = (short)v; short r2 = (short)(short)256; static short r3 = (short)(short)256;
      printf("563: (short)(short)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)256; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)256; static unsigned short r3 = (unsigned short)(short)256;
      printf("564: (unsigned short)(short)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)256; int r1 = (int)v; int r2 = (int)(short)256; static int r3 = (int)(short)256;
      printf("565: (int)(short)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)256; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)256; static unsigned r3 = (unsigned)(short)256;
      printf("566: (unsigned)(short)256 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)256; long r1 = (long)v; long r2 = (long)(short)256; static long r3 = (long)(short)256;
      printf("567: (long)(short)256 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)256; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)256; static unsigned long r3 = (unsigned long)(short)256;
      printf("568: (unsigned long)(short)256 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)256; long long r1 = (long long)v; long long r2 = (long long)(short)256; static long long r3 = (long long)(short)256;
      printf("569: (long long)(short)256 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)256; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)256; static unsigned long long r3 = (unsigned long long)(short)256;
      printf("570: (unsigned long long)(short)256 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)256; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)256; static _Bool r3 = (_Bool)(short)256;
      printf("571: (_Bool)(short)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)256; float r1 = (float)v; float r2 = (float)(short)256; static float r3 = (float)(short)256;
      printf("572: (float)(short)256 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)256; double r1 = (double)v; double r2 = (double)(short)256; static double r3 = (double)(short)256;
      printf("573: (double)(short)256 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)256; long double r1 = (long double)v; long double r2 = (long double)(short)256; static long double r3 = (long double)(short)256;
      printf("574: (long double)(short)256 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)32767; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)32767; static signed char r3 = (signed char)(short)32767;
      printf("575: (signed char)(short)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)32767; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)32767; static unsigned char r3 = (unsigned char)(short)32767;
      printf("576: (unsigned char)(short)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)32767; short r1 = (short)v; short r2 = (short)(short)32767; static short r3 = (short)(short)32767;
      printf("577: (short)(short)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)32767; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)32767; static unsigned short r3 = (unsigned short)(short)32767;
      printf("578: (unsigned short)(short)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)32767; int r1 = (int)v; int r2 = (int)(short)32767; static int r3 = (int)(short)32767;
      printf("579: (int)(short)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)32767; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)32767; static unsigned r3 = (unsigned)(short)32767;
      printf("580: (unsigned)(short)32767 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)32767; long r1 = (long)v; long r2 = (long)(short)32767; static long r3 = (long)(short)32767;
      printf("581: (long)(short)32767 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)32767; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)32767; static unsigned long r3 = (unsigned long)(short)32767;
      printf("582: (unsigned long)(short)32767 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)32767; long long r1 = (long long)v; long long r2 = (long long)(short)32767; static long long r3 = (long long)(short)32767;
      printf("583: (long long)(short)32767 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)32767; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)32767; static unsigned long long r3 = (unsigned long long)(short)32767;
      printf("584: (unsigned long long)(short)32767 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)32767; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)32767; static _Bool r3 = (_Bool)(short)32767;
      printf("585: (_Bool)(short)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)32767; float r1 = (float)v; float r2 = (float)(short)32767; static float r3 = (float)(short)32767;
      printf("586: (float)(short)32767 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)32767; double r1 = (double)v; double r2 = (double)(short)32767; static double r3 = (double)(short)32767;
      printf("587: (double)(short)32767 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)32767; long double r1 = (long double)v; long double r2 = (long double)(short)32767; static long double r3 = (long double)(short)32767;
      printf("588: (long double)(short)32767 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)-32768; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)-32768; static signed char r3 = (signed char)(short)-32768;
      printf("589: (signed char)(short)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-32768; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)-32768; static unsigned char r3 = (unsigned char)(short)-32768;
      printf("590: (unsigned char)(short)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-32768; short r1 = (short)v; short r2 = (short)(short)-32768; static short r3 = (short)(short)-32768;
      printf("591: (short)(short)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-32768; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)-32768; static unsigned short r3 = (unsigned short)(short)-32768;
      printf("592: (unsigned short)(short)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-32768; int r1 = (int)v; int r2 = (int)(short)-32768; static int r3 = (int)(short)-32768;
      printf("593: (int)(short)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-32768; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)-32768; static unsigned r3 = (unsigned)(short)-32768;
      printf("594: (unsigned)(short)-32768 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)-32768; long r1 = (long)v; long r2 = (long)(short)-32768; static long r3 = (long)(short)-32768;
      printf("595: (long)(short)-32768 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)-32768; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)-32768; static unsigned long r3 = (unsigned long)(short)-32768;
      printf("596: (unsigned long)(short)-32768 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)-32768; long long r1 = (long long)v; long long r2 = (long long)(short)-32768; static long long r3 = (long long)(short)-32768;
      printf("597: (long long)(short)-32768 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)-32768; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)-32768; static unsigned long long r3 = (unsigned long long)(short)-32768;
      printf("598: (unsigned long long)(short)-32768 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)-32768; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)-32768; static _Bool r3 = (_Bool)(short)-32768;
      printf("599: (_Bool)(short)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)-32768; float r1 = (float)v; float r2 = (float)(short)-32768; static float r3 = (float)(short)-32768;
      printf("600: (float)(short)-32768 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)-32768; double r1 = (double)v; double r2 = (double)(short)-32768; static double r3 = (double)(short)-32768;
      printf("601: (double)(short)-32768 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)-32768; long double r1 = (long double)v; long double r2 = (long double)(short)-32768; static long double r3 = (long double)(short)-32768;
      printf("602: (long double)(short)-32768 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)65535; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)65535; static signed char r3 = (signed char)(short)65535;
      printf("603: (signed char)(short)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)65535; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)65535; static unsigned char r3 = (unsigned char)(short)65535;
      printf("604: (unsigned char)(short)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)65535; short r1 = (short)v; short r2 = (short)(short)65535; static short r3 = (short)(short)65535;
      printf("605: (short)(short)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)65535; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)65535; static unsigned short r3 = (unsigned short)(short)65535;
      printf("606: (unsigned short)(short)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)65535; int r1 = (int)v; int r2 = (int)(short)65535; static int r3 = (int)(short)65535;
      printf("607: (int)(short)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)65535; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)65535; static unsigned r3 = (unsigned)(short)65535;
      printf("608: (unsigned)(short)65535 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)65535; long r1 = (long)v; long r2 = (long)(short)65535; static long r3 = (long)(short)65535;
      printf("609: (long)(short)65535 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)65535; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)65535; static unsigned long r3 = (unsigned long)(short)65535;
      printf("610: (unsigned long)(short)65535 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)65535; long long r1 = (long long)v; long long r2 = (long long)(short)65535; static long long r3 = (long long)(short)65535;
      printf("611: (long long)(short)65535 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)65535; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)65535; static unsigned long long r3 = (unsigned long long)(short)65535;
      printf("612: (unsigned long long)(short)65535 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)65535; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)65535; static _Bool r3 = (_Bool)(short)65535;
      printf("613: (_Bool)(short)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)65535; float r1 = (float)v; float r2 = (float)(short)65535; static float r3 = (float)(short)65535;
      printf("614: (float)(short)65535 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)65535; double r1 = (double)v; double r2 = (double)(short)65535; static double r3 = (double)(short)65535;
      printf("615: (double)(short)65535 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)65535; long double r1 = (long double)v; long double r2 = (long double)(short)65535; static long double r3 = (long double)(short)65535;
      printf("616: (long double)(short)65535 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)2147483647; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)2147483647; static signed char r3 = (signed char)(short)2147483647;
      printf("617: (signed char)(short)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)2147483647; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)2147483647; static unsigned char r3 = (unsigned char)(short)2147483647;
      printf("618: (unsigned char)(short)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)2147483647; short r1 = (short)v; short r2 = (short)(short)2147483647; static short r3 = (short)(short)2147483647;
      printf("619: (short)(short)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)2147483647; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)2147483647; static unsigned short r3 = (unsigned short)(short)2147483647;
      printf("620: (unsigned short)(short)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)2147483647; int r1 = (int)v; int r2 = (int)(short)2147483647; static int r3 = (int)(short)2147483647;
      printf("621: (int)(short)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)2147483647; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)2147483647; static unsigned r3 = (unsigned)(short)2147483647;
      printf("622: (unsigned)(short)2147483647 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)2147483647; long r1 = (long)v; long r2 = (long)(short)2147483647; static long r3 = (long)(short)2147483647;
      printf("623: (long)(short)2147483647 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)2147483647; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)2147483647; static unsigned long r3 = (unsigned long)(short)2147483647;
      printf("624: (unsigned long)(short)2147483647 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)2147483647; long long r1 = (long long)v; long long r2 = (long long)(short)2147483647; static long long r3 = (long long)(short)2147483647;
      printf("625: (long long)(short)2147483647 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)2147483647; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)2147483647; static unsigned long long r3 = (unsigned long long)(short)2147483647;
      printf("626: (unsigned long long)(short)2147483647 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)2147483647; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)2147483647; static _Bool r3 = (_Bool)(short)2147483647;
      printf("627: (_Bool)(short)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)2147483647; float r1 = (float)v; float r2 = (float)(short)2147483647; static float r3 = (float)(short)2147483647;
      printf("628: (float)(short)2147483647 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)2147483647; double r1 = (double)v; double r2 = (double)(short)2147483647; static double r3 = (double)(short)2147483647;
      printf("629: (double)(short)2147483647 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)2147483647; long double r1 = (long double)v; long double r2 = (long double)(short)2147483647; static long double r3 = (long double)(short)2147483647;
      printf("630: (long double)(short)2147483647 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)(-2147483647-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(short)(-2147483647-1); static signed char r3 = (signed char)(short)(-2147483647-1);
      printf("631: (signed char)(short)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)(-2147483647-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)(-2147483647-1); static unsigned char r3 = (unsigned char)(short)(-2147483647-1);
      printf("632: (unsigned char)(short)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)(-2147483647-1); short r1 = (short)v; short r2 = (short)(short)(-2147483647-1); static short r3 = (short)(short)(-2147483647-1);
      printf("633: (short)(short)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)(-2147483647-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)(-2147483647-1); static unsigned short r3 = (unsigned short)(short)(-2147483647-1);
      printf("634: (unsigned short)(short)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)(-2147483647-1); int r1 = (int)v; int r2 = (int)(short)(-2147483647-1); static int r3 = (int)(short)(-2147483647-1);
      printf("635: (int)(short)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)(-2147483647-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)(-2147483647-1); static unsigned r3 = (unsigned)(short)(-2147483647-1);
      printf("636: (unsigned)(short)(-2147483647-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)(-2147483647-1); long r1 = (long)v; long r2 = (long)(short)(-2147483647-1); static long r3 = (long)(short)(-2147483647-1);
      printf("637: (long)(short)(-2147483647-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)(-2147483647-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)(-2147483647-1); static unsigned long r3 = (unsigned long)(short)(-2147483647-1);
      printf("638: (unsigned long)(short)(-2147483647-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)(-2147483647-1); long long r1 = (long long)v; long long r2 = (long long)(short)(-2147483647-1); static long long r3 = (long long)(short)(-2147483647-1);
      printf("639: (long long)(short)(-2147483647-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)(-2147483647-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)(-2147483647-1); static unsigned long long r3 = (unsigned long long)(short)(-2147483647-1);
      printf("640: (unsigned long long)(short)(-2147483647-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)(-2147483647-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)(-2147483647-1); static _Bool r3 = (_Bool)(short)(-2147483647-1);
      printf("641: (_Bool)(short)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)(-2147483647-1); float r1 = (float)v; float r2 = (float)(short)(-2147483647-1); static float r3 = (float)(short)(-2147483647-1);
      printf("642: (float)(short)(-2147483647-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)(-2147483647-1); double r1 = (double)v; double r2 = (double)(short)(-2147483647-1); static double r3 = (double)(short)(-2147483647-1);
      printf("643: (double)(short)(-2147483647-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)(-2147483647-1); long double r1 = (long double)v; long double r2 = (long double)(short)(-2147483647-1); static long double r3 = (long double)(short)(-2147483647-1);
      printf("644: (long double)(short)(-2147483647-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)4294967295LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)4294967295LL; static signed char r3 = (signed char)(short)4294967295LL;
      printf("645: (signed char)(short)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)4294967295LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)4294967295LL; static unsigned char r3 = (unsigned char)(short)4294967295LL;
      printf("646: (unsigned char)(short)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)4294967295LL; short r1 = (short)v; short r2 = (short)(short)4294967295LL; static short r3 = (short)(short)4294967295LL;
      printf("647: (short)(short)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)4294967295LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)4294967295LL; static unsigned short r3 = (unsigned short)(short)4294967295LL;
      printf("648: (unsigned short)(short)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)4294967295LL; int r1 = (int)v; int r2 = (int)(short)4294967295LL; static int r3 = (int)(short)4294967295LL;
      printf("649: (int)(short)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)4294967295LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)4294967295LL; static unsigned r3 = (unsigned)(short)4294967295LL;
      printf("650: (unsigned)(short)4294967295LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)4294967295LL; long r1 = (long)v; long r2 = (long)(short)4294967295LL; static long r3 = (long)(short)4294967295LL;
      printf("651: (long)(short)4294967295LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)4294967295LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)4294967295LL; static unsigned long r3 = (unsigned long)(short)4294967295LL;
      printf("652: (unsigned long)(short)4294967295LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)4294967295LL; long long r1 = (long long)v; long long r2 = (long long)(short)4294967295LL; static long long r3 = (long long)(short)4294967295LL;
      printf("653: (long long)(short)4294967295LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)4294967295LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)4294967295LL; static unsigned long long r3 = (unsigned long long)(short)4294967295LL;
      printf("654: (unsigned long long)(short)4294967295LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)4294967295LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)4294967295LL; static _Bool r3 = (_Bool)(short)4294967295LL;
      printf("655: (_Bool)(short)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)4294967295LL; float r1 = (float)v; float r2 = (float)(short)4294967295LL; static float r3 = (float)(short)4294967295LL;
      printf("656: (float)(short)4294967295LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)4294967295LL; double r1 = (double)v; double r2 = (double)(short)4294967295LL; static double r3 = (double)(short)4294967295LL;
      printf("657: (double)(short)4294967295LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)4294967295LL; long double r1 = (long double)v; long double r2 = (long double)(short)4294967295LL; static long double r3 = (long double)(short)4294967295LL;
      printf("658: (long double)(short)4294967295LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)9223372036854775807LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)9223372036854775807LL; static signed char r3 = (signed char)(short)9223372036854775807LL;
      printf("659: (signed char)(short)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)9223372036854775807LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)9223372036854775807LL; static unsigned char r3 = (unsigned char)(short)9223372036854775807LL;
      printf("660: (unsigned char)(short)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)9223372036854775807LL; short r1 = (short)v; short r2 = (short)(short)9223372036854775807LL; static short r3 = (short)(short)9223372036854775807LL;
      printf("661: (short)(short)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)9223372036854775807LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)9223372036854775807LL; static unsigned short r3 = (unsigned short)(short)9223372036854775807LL;
      printf("662: (unsigned short)(short)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)9223372036854775807LL; int r1 = (int)v; int r2 = (int)(short)9223372036854775807LL; static int r3 = (int)(short)9223372036854775807LL;
      printf("663: (int)(short)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)9223372036854775807LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)9223372036854775807LL; static unsigned r3 = (unsigned)(short)9223372036854775807LL;
      printf("664: (unsigned)(short)9223372036854775807LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)9223372036854775807LL; long r1 = (long)v; long r2 = (long)(short)9223372036854775807LL; static long r3 = (long)(short)9223372036854775807LL;
      printf("665: (long)(short)9223372036854775807LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)9223372036854775807LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)9223372036854775807LL; static unsigned long r3 = (unsigned long)(short)9223372036854775807LL;
      printf("666: (unsigned long)(short)9223372036854775807LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)9223372036854775807LL; long long r1 = (long long)v; long long r2 = (long long)(short)9223372036854775807LL; static long long r3 = (long long)(short)9223372036854775807LL;
      printf("667: (long long)(short)9223372036854775807LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)9223372036854775807LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)9223372036854775807LL; static unsigned long long r3 = (unsigned long long)(short)9223372036854775807LL;
      printf("668: (unsigned long long)(short)9223372036854775807LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)9223372036854775807LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)9223372036854775807LL; static _Bool r3 = (_Bool)(short)9223372036854775807LL;
      printf("669: (_Bool)(short)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)9223372036854775807LL; float r1 = (float)v; float r2 = (float)(short)9223372036854775807LL; static float r3 = (float)(short)9223372036854775807LL;
      printf("670: (float)(short)9223372036854775807LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)9223372036854775807LL; double r1 = (double)v; double r2 = (double)(short)9223372036854775807LL; static double r3 = (double)(short)9223372036854775807LL;
      printf("671: (double)(short)9223372036854775807LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)9223372036854775807LL; long double r1 = (long double)v; long double r2 = (long double)(short)9223372036854775807LL; static long double r3 = (long double)(short)9223372036854775807LL;
      printf("672: (long double)(short)9223372036854775807LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(short)(-9223372036854775807LL-1); static signed char r3 = (signed char)(short)(-9223372036854775807LL-1);
      printf("673: (signed char)(short)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)(-9223372036854775807LL-1); static unsigned char r3 = (unsigned char)(short)(-9223372036854775807LL-1);
      printf("674: (unsigned char)(short)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); short r1 = (short)v; short r2 = (short)(short)(-9223372036854775807LL-1); static short r3 = (short)(short)(-9223372036854775807LL-1);
      printf("675: (short)(short)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)(-9223372036854775807LL-1); static unsigned short r3 = (unsigned short)(short)(-9223372036854775807LL-1);
      printf("676: (unsigned short)(short)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); int r1 = (int)v; int r2 = (int)(short)(-9223372036854775807LL-1); static int r3 = (int)(short)(-9223372036854775807LL-1);
      printf("677: (int)(short)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)(-9223372036854775807LL-1); static unsigned r3 = (unsigned)(short)(-9223372036854775807LL-1);
      printf("678: (unsigned)(short)(-9223372036854775807LL-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); long r1 = (long)v; long r2 = (long)(short)(-9223372036854775807LL-1); static long r3 = (long)(short)(-9223372036854775807LL-1);
      printf("679: (long)(short)(-9223372036854775807LL-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)(-9223372036854775807LL-1); static unsigned long r3 = (unsigned long)(short)(-9223372036854775807LL-1);
      printf("680: (unsigned long)(short)(-9223372036854775807LL-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); long long r1 = (long long)v; long long r2 = (long long)(short)(-9223372036854775807LL-1); static long long r3 = (long long)(short)(-9223372036854775807LL-1);
      printf("681: (long long)(short)(-9223372036854775807LL-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)(-9223372036854775807LL-1); static unsigned long long r3 = (unsigned long long)(short)(-9223372036854775807LL-1);
      printf("682: (unsigned long long)(short)(-9223372036854775807LL-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)(-9223372036854775807LL-1); static _Bool r3 = (_Bool)(short)(-9223372036854775807LL-1);
      printf("683: (_Bool)(short)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); float r1 = (float)v; float r2 = (float)(short)(-9223372036854775807LL-1); static float r3 = (float)(short)(-9223372036854775807LL-1);
      printf("684: (float)(short)(-9223372036854775807LL-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); double r1 = (double)v; double r2 = (double)(short)(-9223372036854775807LL-1); static double r3 = (double)(short)(-9223372036854775807LL-1);
      printf("685: (double)(short)(-9223372036854775807LL-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)(-9223372036854775807LL-1); long double r1 = (long double)v; long double r2 = (long double)(short)(-9223372036854775807LL-1); static long double r3 = (long double)(short)(-9223372036854775807LL-1);
      printf("686: (long double)(short)(-9223372036854775807LL-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)0x8000000000000000ULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)0x8000000000000000ULL; static signed char r3 = (signed char)(short)0x8000000000000000ULL;
      printf("687: (signed char)(short)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0x8000000000000000ULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)0x8000000000000000ULL; static unsigned char r3 = (unsigned char)(short)0x8000000000000000ULL;
      printf("688: (unsigned char)(short)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0x8000000000000000ULL; short r1 = (short)v; short r2 = (short)(short)0x8000000000000000ULL; static short r3 = (short)(short)0x8000000000000000ULL;
      printf("689: (short)(short)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0x8000000000000000ULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)0x8000000000000000ULL; static unsigned short r3 = (unsigned short)(short)0x8000000000000000ULL;
      printf("690: (unsigned short)(short)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0x8000000000000000ULL; int r1 = (int)v; int r2 = (int)(short)0x8000000000000000ULL; static int r3 = (int)(short)0x8000000000000000ULL;
      printf("691: (int)(short)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0x8000000000000000ULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)0x8000000000000000ULL; static unsigned r3 = (unsigned)(short)0x8000000000000000ULL;
      printf("692: (unsigned)(short)0x8000000000000000ULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)0x8000000000000000ULL; long r1 = (long)v; long r2 = (long)(short)0x8000000000000000ULL; static long r3 = (long)(short)0x8000000000000000ULL;
      printf("693: (long)(short)0x8000000000000000ULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)0x8000000000000000ULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)0x8000000000000000ULL; static unsigned long r3 = (unsigned long)(short)0x8000000000000000ULL;
      printf("694: (unsigned long)(short)0x8000000000000000ULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)0x8000000000000000ULL; long long r1 = (long long)v; long long r2 = (long long)(short)0x8000000000000000ULL; static long long r3 = (long long)(short)0x8000000000000000ULL;
      printf("695: (long long)(short)0x8000000000000000ULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)0x8000000000000000ULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)0x8000000000000000ULL; static unsigned long long r3 = (unsigned long long)(short)0x8000000000000000ULL;
      printf("696: (unsigned long long)(short)0x8000000000000000ULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)0x8000000000000000ULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)0x8000000000000000ULL; static _Bool r3 = (_Bool)(short)0x8000000000000000ULL;
      printf("697: (_Bool)(short)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0x8000000000000000ULL; float r1 = (float)v; float r2 = (float)(short)0x8000000000000000ULL; static float r3 = (float)(short)0x8000000000000000ULL;
      printf("698: (float)(short)0x8000000000000000ULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)0x8000000000000000ULL; double r1 = (double)v; double r2 = (double)(short)0x8000000000000000ULL; static double r3 = (double)(short)0x8000000000000000ULL;
      printf("699: (double)(short)0x8000000000000000ULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)0x8000000000000000ULL; long double r1 = (long double)v; long double r2 = (long double)(short)0x8000000000000000ULL; static long double r3 = (long double)(short)0x8000000000000000ULL;
      printf("700: (long double)(short)0x8000000000000000ULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(short)0xffffffffffffffffULL; static signed char r3 = (signed char)(short)0xffffffffffffffffULL;
      printf("701: (signed char)(short)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(short)0xffffffffffffffffULL; static unsigned char r3 = (unsigned char)(short)0xffffffffffffffffULL;
      printf("702: (unsigned char)(short)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; short r1 = (short)v; short r2 = (short)(short)0xffffffffffffffffULL; static short r3 = (short)(short)0xffffffffffffffffULL;
      printf("703: (short)(short)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(short)0xffffffffffffffffULL; static unsigned short r3 = (unsigned short)(short)0xffffffffffffffffULL;
      printf("704: (unsigned short)(short)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; int r1 = (int)v; int r2 = (int)(short)0xffffffffffffffffULL; static int r3 = (int)(short)0xffffffffffffffffULL;
      printf("705: (int)(short)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(short)0xffffffffffffffffULL; static unsigned r3 = (unsigned)(short)0xffffffffffffffffULL;
      printf("706: (unsigned)(short)0xffffffffffffffffULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; long r1 = (long)v; long r2 = (long)(short)0xffffffffffffffffULL; static long r3 = (long)(short)0xffffffffffffffffULL;
      printf("707: (long)(short)0xffffffffffffffffULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(short)0xffffffffffffffffULL; static unsigned long r3 = (unsigned long)(short)0xffffffffffffffffULL;
      printf("708: (unsigned long)(short)0xffffffffffffffffULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; long long r1 = (long long)v; long long r2 = (long long)(short)0xffffffffffffffffULL; static long long r3 = (long long)(short)0xffffffffffffffffULL;
      printf("709: (long long)(short)0xffffffffffffffffULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(short)0xffffffffffffffffULL; static unsigned long long r3 = (unsigned long long)(short)0xffffffffffffffffULL;
      printf("710: (unsigned long long)(short)0xffffffffffffffffULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(short)0xffffffffffffffffULL; static _Bool r3 = (_Bool)(short)0xffffffffffffffffULL;
      printf("711: (_Bool)(short)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; float r1 = (float)v; float r2 = (float)(short)0xffffffffffffffffULL; static float r3 = (float)(short)0xffffffffffffffffULL;
      printf("712: (float)(short)0xffffffffffffffffULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; double r1 = (double)v; double r2 = (double)(short)0xffffffffffffffffULL; static double r3 = (double)(short)0xffffffffffffffffULL;
      printf("713: (double)(short)0xffffffffffffffffULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short v = (short)0xffffffffffffffffULL; long double r1 = (long double)v; long double r2 = (long double)(short)0xffffffffffffffffULL; static long double r3 = (long double)(short)0xffffffffffffffffULL;
      printf("714: (long double)(short)0xffffffffffffffffULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)0; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)0; static signed char r3 = (signed char)(unsigned short)0;
      printf("715: (signed char)(unsigned short)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)0; static unsigned char r3 = (unsigned char)(unsigned short)0;
      printf("716: (unsigned char)(unsigned short)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0; short r1 = (short)v; short r2 = (short)(unsigned short)0; static short r3 = (short)(unsigned short)0;
      printf("717: (short)(unsigned short)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)0; static unsigned short r3 = (unsigned short)(unsigned short)0;
      printf("718: (unsigned short)(unsigned short)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0; int r1 = (int)v; int r2 = (int)(unsigned short)0; static int r3 = (int)(unsigned short)0;
      printf("719: (int)(unsigned short)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)0; static unsigned r3 = (unsigned)(unsigned short)0;
      printf("720: (unsigned)(unsigned short)0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)0; long r1 = (long)v; long r2 = (long)(unsigned short)0; static long r3 = (long)(unsigned short)0;
      printf("721: (long)(unsigned short)0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)0; static unsigned long r3 = (unsigned long)(unsigned short)0;
      printf("722: (unsigned long)(unsigned short)0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)0; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)0; static long long r3 = (long long)(unsigned short)0;
      printf("723: (long long)(unsigned short)0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)0; static unsigned long long r3 = (unsigned long long)(unsigned short)0;
      printf("724: (unsigned long long)(unsigned short)0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)0; static _Bool r3 = (_Bool)(unsigned short)0;
      printf("725: (_Bool)(unsigned short)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0; float r1 = (float)v; float r2 = (float)(unsigned short)0; static float r3 = (float)(unsigned short)0;
      printf("726: (float)(unsigned short)0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)0; double r1 = (double)v; double r2 = (double)(unsigned short)0; static double r3 = (double)(unsigned short)0;
      printf("727: (double)(unsigned short)0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)0; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)0; static long double r3 = (long double)(unsigned short)0;
      printf("728: (long double)(unsigned short)0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)1; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)1; static signed char r3 = (signed char)(unsigned short)1;
      printf("729: (signed char)(unsigned short)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)1; static unsigned char r3 = (unsigned char)(unsigned short)1;
      printf("730: (unsigned char)(unsigned short)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)1; short r1 = (short)v; short r2 = (short)(unsigned short)1; static short r3 = (short)(unsigned short)1;
      printf("731: (short)(unsigned short)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)1; static unsigned short r3 = (unsigned short)(unsigned short)1;
      printf("732: (unsigned short)(unsigned short)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)1; int r1 = (int)v; int r2 = (int)(unsigned short)1; static int r3 = (int)(unsigned short)1;
      printf("733: (int)(unsigned short)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)1; static unsigned r3 = (unsigned)(unsigned short)1;
      printf("734: (unsigned)(unsigned short)1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)1; long r1 = (long)v; long r2 = (long)(unsigned short)1; static long r3 = (long)(unsigned short)1;
      printf("735: (long)(unsigned short)1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)1; static unsigned long r3 = (unsigned long)(unsigned short)1;
      printf("736: (unsigned long)(unsigned short)1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)1; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)1; static long long r3 = (long long)(unsigned short)1;
      printf("737: (long long)(unsigned short)1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)1; static unsigned long long r3 = (unsigned long long)(unsigned short)1;
      printf("738: (unsigned long long)(unsigned short)1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)1; static _Bool r3 = (_Bool)(unsigned short)1;
      printf("739: (_Bool)(unsigned short)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)1; float r1 = (float)v; float r2 = (float)(unsigned short)1; static float r3 = (float)(unsigned short)1;
      printf("740: (float)(unsigned short)1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)1; double r1 = (double)v; double r2 = (double)(unsigned short)1; static double r3 = (double)(unsigned short)1;
      printf("741: (double)(unsigned short)1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)1; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)1; static long double r3 = (long double)(unsigned short)1;
      printf("742: (long double)(unsigned short)1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)-1; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)-1; static signed char r3 = (signed char)(unsigned short)-1;
      printf("743: (signed char)(unsigned short)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)-1; static unsigned char r3 = (unsigned char)(unsigned short)-1;
      printf("744: (unsigned char)(unsigned short)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-1; short r1 = (short)v; short r2 = (short)(unsigned short)-1; static short r3 = (short)(unsigned short)-1;
      printf("745: (short)(unsigned short)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)-1; static unsigned short r3 = (unsigned short)(unsigned short)-1;
      printf("746: (unsigned short)(unsigned short)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-1; int r1 = (int)v; int r2 = (int)(unsigned short)-1; static int r3 = (int)(unsigned short)-1;
      printf("747: (int)(unsigned short)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)-1; static unsigned r3 = (unsigned)(unsigned short)-1;
      printf("748: (unsigned)(unsigned short)-1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)-1; long r1 = (long)v; long r2 = (long)(unsigned short)-1; static long r3 = (long)(unsigned short)-1;
      printf("749: (long)(unsigned short)-1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)-1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)-1; static unsigned long r3 = (unsigned long)(unsigned short)-1;
      printf("750: (unsigned long)(unsigned short)-1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)-1; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)-1; static long long r3 = (long long)(unsigned short)-1;
      printf("751: (long long)(unsigned short)-1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)-1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)-1; static unsigned long long r3 = (unsigned long long)(unsigned short)-1;
      printf("752: (unsigned long long)(unsigned short)-1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)-1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)-1; static _Bool r3 = (_Bool)(unsigned short)-1;
      printf("753: (_Bool)(unsigned short)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-1; float r1 = (float)v; float r2 = (float)(unsigned short)-1; static float r3 = (float)(unsigned short)-1;
      printf("754: (float)(unsigned short)-1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)-1; double r1 = (double)v; double r2 = (double)(unsigned short)-1; static double r3 = (double)(unsigned short)-1;
      printf("755: (double)(unsigned short)-1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)-1; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)-1; static long double r3 = (long double)(unsigned short)-1;
      printf("756: (long double)(unsigned short)-1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)127; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)127; static signed char r3 = (signed char)(unsigned short)127;
      printf("757: (signed char)(unsigned short)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)127; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)127; static unsigned char r3 = (unsigned char)(unsigned short)127;
      printf("758: (unsigned char)(unsigned short)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)127; short r1 = (short)v; short r2 = (short)(unsigned short)127; static short r3 = (short)(unsigned short)127;
      printf("759: (short)(unsigned short)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)127; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)127; static unsigned short r3 = (unsigned short)(unsigned short)127;
      printf("760: (unsigned short)(unsigned short)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)127; int r1 = (int)v; int r2 = (int)(unsigned short)127; static int r3 = (int)(unsigned short)127;
      printf("761: (int)(unsigned short)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)127; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)127; static unsigned r3 = (unsigned)(unsigned short)127;
      printf("762: (unsigned)(unsigned short)127 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)127; long r1 = (long)v; long r2 = (long)(unsigned short)127; static long r3 = (long)(unsigned short)127;
      printf("763: (long)(unsigned short)127 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)127; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)127; static unsigned long r3 = (unsigned long)(unsigned short)127;
      printf("764: (unsigned long)(unsigned short)127 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)127; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)127; static long long r3 = (long long)(unsigned short)127;
      printf("765: (long long)(unsigned short)127 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)127; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)127; static unsigned long long r3 = (unsigned long long)(unsigned short)127;
      printf("766: (unsigned long long)(unsigned short)127 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)127; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)127; static _Bool r3 = (_Bool)(unsigned short)127;
      printf("767: (_Bool)(unsigned short)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)127; float r1 = (float)v; float r2 = (float)(unsigned short)127; static float r3 = (float)(unsigned short)127;
      printf("768: (float)(unsigned short)127 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)127; double r1 = (double)v; double r2 = (double)(unsigned short)127; static double r3 = (double)(unsigned short)127;
      printf("769: (double)(unsigned short)127 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)127; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)127; static long double r3 = (long double)(unsigned short)127;
      printf("770: (long double)(unsigned short)127 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)-128; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)-128; static signed char r3 = (signed char)(unsigned short)-128;
      printf("771: (signed char)(unsigned short)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-128; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)-128; static unsigned char r3 = (unsigned char)(unsigned short)-128;
      printf("772: (unsigned char)(unsigned short)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-128; short r1 = (short)v; short r2 = (short)(unsigned short)-128; static short r3 = (short)(unsigned short)-128;
      printf("773: (short)(unsigned short)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-128; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)-128; static unsigned short r3 = (unsigned short)(unsigned short)-128;
      printf("774: (unsigned short)(unsigned short)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-128; int r1 = (int)v; int r2 = (int)(unsigned short)-128; static int r3 = (int)(unsigned short)-128;
      printf("775: (int)(unsigned short)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-128; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)-128; static unsigned r3 = (unsigned)(unsigned short)-128;
      printf("776: (unsigned)(unsigned short)-128 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)-128; long r1 = (long)v; long r2 = (long)(unsigned short)-128; static long r3 = (long)(unsigned short)-128;
      printf("777: (long)(unsigned short)-128 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)-128; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)-128; static unsigned long r3 = (unsigned long)(unsigned short)-128;
      printf("778: (unsigned long)(unsigned short)-128 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)-128; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)-128; static long long r3 = (long long)(unsigned short)-128;
      printf("779: (long long)(unsigned short)-128 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)-128; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)-128; static unsigned long long r3 = (unsigned long long)(unsigned short)-128;
      printf("780: (unsigned long long)(unsigned short)-128 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)-128; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)-128; static _Bool r3 = (_Bool)(unsigned short)-128;
      printf("781: (_Bool)(unsigned short)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-128; float r1 = (float)v; float r2 = (float)(unsigned short)-128; static float r3 = (float)(unsigned short)-128;
      printf("782: (float)(unsigned short)-128 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)-128; double r1 = (double)v; double r2 = (double)(unsigned short)-128; static double r3 = (double)(unsigned short)-128;
      printf("783: (double)(unsigned short)-128 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)-128; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)-128; static long double r3 = (long double)(unsigned short)-128;
      printf("784: (long double)(unsigned short)-128 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)255; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)255; static signed char r3 = (signed char)(unsigned short)255;
      printf("785: (signed char)(unsigned short)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)255; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)255; static unsigned char r3 = (unsigned char)(unsigned short)255;
      printf("786: (unsigned char)(unsigned short)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)255; short r1 = (short)v; short r2 = (short)(unsigned short)255; static short r3 = (short)(unsigned short)255;
      printf("787: (short)(unsigned short)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)255; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)255; static unsigned short r3 = (unsigned short)(unsigned short)255;
      printf("788: (unsigned short)(unsigned short)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)255; int r1 = (int)v; int r2 = (int)(unsigned short)255; static int r3 = (int)(unsigned short)255;
      printf("789: (int)(unsigned short)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)255; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)255; static unsigned r3 = (unsigned)(unsigned short)255;
      printf("790: (unsigned)(unsigned short)255 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)255; long r1 = (long)v; long r2 = (long)(unsigned short)255; static long r3 = (long)(unsigned short)255;
      printf("791: (long)(unsigned short)255 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)255; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)255; static unsigned long r3 = (unsigned long)(unsigned short)255;
      printf("792: (unsigned long)(unsigned short)255 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)255; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)255; static long long r3 = (long long)(unsigned short)255;
      printf("793: (long long)(unsigned short)255 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)255; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)255; static unsigned long long r3 = (unsigned long long)(unsigned short)255;
      printf("794: (unsigned long long)(unsigned short)255 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)255; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)255; static _Bool r3 = (_Bool)(unsigned short)255;
      printf("795: (_Bool)(unsigned short)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)255; float r1 = (float)v; float r2 = (float)(unsigned short)255; static float r3 = (float)(unsigned short)255;
      printf("796: (float)(unsigned short)255 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)255; double r1 = (double)v; double r2 = (double)(unsigned short)255; static double r3 = (double)(unsigned short)255;
      printf("797: (double)(unsigned short)255 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)255; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)255; static long double r3 = (long double)(unsigned short)255;
      printf("798: (long double)(unsigned short)255 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)256; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)256; static signed char r3 = (signed char)(unsigned short)256;
      printf("799: (signed char)(unsigned short)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)256; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)256; static unsigned char r3 = (unsigned char)(unsigned short)256;
      printf("800: (unsigned char)(unsigned short)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)256; short r1 = (short)v; short r2 = (short)(unsigned short)256; static short r3 = (short)(unsigned short)256;
      printf("801: (short)(unsigned short)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)256; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)256; static unsigned short r3 = (unsigned short)(unsigned short)256;
      printf("802: (unsigned short)(unsigned short)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)256; int r1 = (int)v; int r2 = (int)(unsigned short)256; static int r3 = (int)(unsigned short)256;
      printf("803: (int)(unsigned short)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)256; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)256; static unsigned r3 = (unsigned)(unsigned short)256;
      printf("804: (unsigned)(unsigned short)256 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)256; long r1 = (long)v; long r2 = (long)(unsigned short)256; static long r3 = (long)(unsigned short)256;
      printf("805: (long)(unsigned short)256 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)256; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)256; static unsigned long r3 = (unsigned long)(unsigned short)256;
      printf("806: (unsigned long)(unsigned short)256 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)256; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)256; static long long r3 = (long long)(unsigned short)256;
      printf("807: (long long)(unsigned short)256 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)256; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)256; static unsigned long long r3 = (unsigned long long)(unsigned short)256;
      printf("808: (unsigned long long)(unsigned short)256 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)256; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)256; static _Bool r3 = (_Bool)(unsigned short)256;
      printf("809: (_Bool)(unsigned short)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)256; float r1 = (float)v; float r2 = (float)(unsigned short)256; static float r3 = (float)(unsigned short)256;
      printf("810: (float)(unsigned short)256 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)256; double r1 = (double)v; double r2 = (double)(unsigned short)256; static double r3 = (double)(unsigned short)256;
      printf("811: (double)(unsigned short)256 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)256; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)256; static long double r3 = (long double)(unsigned short)256;
      printf("812: (long double)(unsigned short)256 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)32767; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)32767; static signed char r3 = (signed char)(unsigned short)32767;
      printf("813: (signed char)(unsigned short)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)32767; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)32767; static unsigned char r3 = (unsigned char)(unsigned short)32767;
      printf("814: (unsigned char)(unsigned short)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)32767; short r1 = (short)v; short r2 = (short)(unsigned short)32767; static short r3 = (short)(unsigned short)32767;
      printf("815: (short)(unsigned short)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)32767; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)32767; static unsigned short r3 = (unsigned short)(unsigned short)32767;
      printf("816: (unsigned short)(unsigned short)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)32767; int r1 = (int)v; int r2 = (int)(unsigned short)32767; static int r3 = (int)(unsigned short)32767;
      printf("817: (int)(unsigned short)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)32767; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)32767; static unsigned r3 = (unsigned)(unsigned short)32767;
      printf("818: (unsigned)(unsigned short)32767 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)32767; long r1 = (long)v; long r2 = (long)(unsigned short)32767; static long r3 = (long)(unsigned short)32767;
      printf("819: (long)(unsigned short)32767 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)32767; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)32767; static unsigned long r3 = (unsigned long)(unsigned short)32767;
      printf("820: (unsigned long)(unsigned short)32767 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)32767; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)32767; static long long r3 = (long long)(unsigned short)32767;
      printf("821: (long long)(unsigned short)32767 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)32767; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)32767; static unsigned long long r3 = (unsigned long long)(unsigned short)32767;
      printf("822: (unsigned long long)(unsigned short)32767 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)32767; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)32767; static _Bool r3 = (_Bool)(unsigned short)32767;
      printf("823: (_Bool)(unsigned short)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)32767; float r1 = (float)v; float r2 = (float)(unsigned short)32767; static float r3 = (float)(unsigned short)32767;
      printf("824: (float)(unsigned short)32767 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)32767; double r1 = (double)v; double r2 = (double)(unsigned short)32767; static double r3 = (double)(unsigned short)32767;
      printf("825: (double)(unsigned short)32767 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)32767; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)32767; static long double r3 = (long double)(unsigned short)32767;
      printf("826: (long double)(unsigned short)32767 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)-32768; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)-32768; static signed char r3 = (signed char)(unsigned short)-32768;
      printf("827: (signed char)(unsigned short)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-32768; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)-32768; static unsigned char r3 = (unsigned char)(unsigned short)-32768;
      printf("828: (unsigned char)(unsigned short)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-32768; short r1 = (short)v; short r2 = (short)(unsigned short)-32768; static short r3 = (short)(unsigned short)-32768;
      printf("829: (short)(unsigned short)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-32768; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)-32768; static unsigned short r3 = (unsigned short)(unsigned short)-32768;
      printf("830: (unsigned short)(unsigned short)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-32768; int r1 = (int)v; int r2 = (int)(unsigned short)-32768; static int r3 = (int)(unsigned short)-32768;
      printf("831: (int)(unsigned short)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-32768; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)-32768; static unsigned r3 = (unsigned)(unsigned short)-32768;
      printf("832: (unsigned)(unsigned short)-32768 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)-32768; long r1 = (long)v; long r2 = (long)(unsigned short)-32768; static long r3 = (long)(unsigned short)-32768;
      printf("833: (long)(unsigned short)-32768 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)-32768; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)-32768; static unsigned long r3 = (unsigned long)(unsigned short)-32768;
      printf("834: (unsigned long)(unsigned short)-32768 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)-32768; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)-32768; static long long r3 = (long long)(unsigned short)-32768;
      printf("835: (long long)(unsigned short)-32768 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)-32768; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)-32768; static unsigned long long r3 = (unsigned long long)(unsigned short)-32768;
      printf("836: (unsigned long long)(unsigned short)-32768 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)-32768; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)-32768; static _Bool r3 = (_Bool)(unsigned short)-32768;
      printf("837: (_Bool)(unsigned short)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)-32768; float r1 = (float)v; float r2 = (float)(unsigned short)-32768; static float r3 = (float)(unsigned short)-32768;
      printf("838: (float)(unsigned short)-32768 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)-32768; double r1 = (double)v; double r2 = (double)(unsigned short)-32768; static double r3 = (double)(unsigned short)-32768;
      printf("839: (double)(unsigned short)-32768 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)-32768; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)-32768; static long double r3 = (long double)(unsigned short)-32768;
      printf("840: (long double)(unsigned short)-32768 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)65535; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)65535; static signed char r3 = (signed char)(unsigned short)65535;
      printf("841: (signed char)(unsigned short)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)65535; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)65535; static unsigned char r3 = (unsigned char)(unsigned short)65535;
      printf("842: (unsigned char)(unsigned short)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)65535; short r1 = (short)v; short r2 = (short)(unsigned short)65535; static short r3 = (short)(unsigned short)65535;
      printf("843: (short)(unsigned short)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)65535; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)65535; static unsigned short r3 = (unsigned short)(unsigned short)65535;
      printf("844: (unsigned short)(unsigned short)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)65535; int r1 = (int)v; int r2 = (int)(unsigned short)65535; static int r3 = (int)(unsigned short)65535;
      printf("845: (int)(unsigned short)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)65535; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)65535; static unsigned r3 = (unsigned)(unsigned short)65535;
      printf("846: (unsigned)(unsigned short)65535 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)65535; long r1 = (long)v; long r2 = (long)(unsigned short)65535; static long r3 = (long)(unsigned short)65535;
      printf("847: (long)(unsigned short)65535 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)65535; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)65535; static unsigned long r3 = (unsigned long)(unsigned short)65535;
      printf("848: (unsigned long)(unsigned short)65535 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)65535; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)65535; static long long r3 = (long long)(unsigned short)65535;
      printf("849: (long long)(unsigned short)65535 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)65535; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)65535; static unsigned long long r3 = (unsigned long long)(unsigned short)65535;
      printf("850: (unsigned long long)(unsigned short)65535 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)65535; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)65535; static _Bool r3 = (_Bool)(unsigned short)65535;
      printf("851: (_Bool)(unsigned short)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)65535; float r1 = (float)v; float r2 = (float)(unsigned short)65535; static float r3 = (float)(unsigned short)65535;
      printf("852: (float)(unsigned short)65535 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)65535; double r1 = (double)v; double r2 = (double)(unsigned short)65535; static double r3 = (double)(unsigned short)65535;
      printf("853: (double)(unsigned short)65535 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)65535; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)65535; static long double r3 = (long double)(unsigned short)65535;
      printf("854: (long double)(unsigned short)65535 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)2147483647; static signed char r3 = (signed char)(unsigned short)2147483647;
      printf("855: (signed char)(unsigned short)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)2147483647; static unsigned char r3 = (unsigned char)(unsigned short)2147483647;
      printf("856: (unsigned char)(unsigned short)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; short r1 = (short)v; short r2 = (short)(unsigned short)2147483647; static short r3 = (short)(unsigned short)2147483647;
      printf("857: (short)(unsigned short)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)2147483647; static unsigned short r3 = (unsigned short)(unsigned short)2147483647;
      printf("858: (unsigned short)(unsigned short)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; int r1 = (int)v; int r2 = (int)(unsigned short)2147483647; static int r3 = (int)(unsigned short)2147483647;
      printf("859: (int)(unsigned short)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)2147483647; static unsigned r3 = (unsigned)(unsigned short)2147483647;
      printf("860: (unsigned)(unsigned short)2147483647 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; long r1 = (long)v; long r2 = (long)(unsigned short)2147483647; static long r3 = (long)(unsigned short)2147483647;
      printf("861: (long)(unsigned short)2147483647 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)2147483647; static unsigned long r3 = (unsigned long)(unsigned short)2147483647;
      printf("862: (unsigned long)(unsigned short)2147483647 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)2147483647; static long long r3 = (long long)(unsigned short)2147483647;
      printf("863: (long long)(unsigned short)2147483647 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)2147483647; static unsigned long long r3 = (unsigned long long)(unsigned short)2147483647;
      printf("864: (unsigned long long)(unsigned short)2147483647 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)2147483647; static _Bool r3 = (_Bool)(unsigned short)2147483647;
      printf("865: (_Bool)(unsigned short)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; float r1 = (float)v; float r2 = (float)(unsigned short)2147483647; static float r3 = (float)(unsigned short)2147483647;
      printf("866: (float)(unsigned short)2147483647 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; double r1 = (double)v; double r2 = (double)(unsigned short)2147483647; static double r3 = (double)(unsigned short)2147483647;
      printf("867: (double)(unsigned short)2147483647 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)2147483647; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)2147483647; static long double r3 = (long double)(unsigned short)2147483647;
      printf("868: (long double)(unsigned short)2147483647 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)(-2147483647-1); static signed char r3 = (signed char)(unsigned short)(-2147483647-1);
      printf("869: (signed char)(unsigned short)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)(-2147483647-1); static unsigned char r3 = (unsigned char)(unsigned short)(-2147483647-1);
      printf("870: (unsigned char)(unsigned short)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); short r1 = (short)v; short r2 = (short)(unsigned short)(-2147483647-1); static short r3 = (short)(unsigned short)(-2147483647-1);
      printf("871: (short)(unsigned short)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)(-2147483647-1); static unsigned short r3 = (unsigned short)(unsigned short)(-2147483647-1);
      printf("872: (unsigned short)(unsigned short)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); int r1 = (int)v; int r2 = (int)(unsigned short)(-2147483647-1); static int r3 = (int)(unsigned short)(-2147483647-1);
      printf("873: (int)(unsigned short)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)(-2147483647-1); static unsigned r3 = (unsigned)(unsigned short)(-2147483647-1);
      printf("874: (unsigned)(unsigned short)(-2147483647-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); long r1 = (long)v; long r2 = (long)(unsigned short)(-2147483647-1); static long r3 = (long)(unsigned short)(-2147483647-1);
      printf("875: (long)(unsigned short)(-2147483647-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)(-2147483647-1); static unsigned long r3 = (unsigned long)(unsigned short)(-2147483647-1);
      printf("876: (unsigned long)(unsigned short)(-2147483647-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); long long r1 = (long long)v; long long r2 = (long long)(unsigned short)(-2147483647-1); static long long r3 = (long long)(unsigned short)(-2147483647-1);
      printf("877: (long long)(unsigned short)(-2147483647-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)(-2147483647-1); static unsigned long long r3 = (unsigned long long)(unsigned short)(-2147483647-1);
      printf("878: (unsigned long long)(unsigned short)(-2147483647-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)(-2147483647-1); static _Bool r3 = (_Bool)(unsigned short)(-2147483647-1);
      printf("879: (_Bool)(unsigned short)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); float r1 = (float)v; float r2 = (float)(unsigned short)(-2147483647-1); static float r3 = (float)(unsigned short)(-2147483647-1);
      printf("880: (float)(unsigned short)(-2147483647-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); double r1 = (double)v; double r2 = (double)(unsigned short)(-2147483647-1); static double r3 = (double)(unsigned short)(-2147483647-1);
      printf("881: (double)(unsigned short)(-2147483647-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)(-2147483647-1); long double r1 = (long double)v; long double r2 = (long double)(unsigned short)(-2147483647-1); static long double r3 = (long double)(unsigned short)(-2147483647-1);
      printf("882: (long double)(unsigned short)(-2147483647-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)4294967295LL; static signed char r3 = (signed char)(unsigned short)4294967295LL;
      printf("883: (signed char)(unsigned short)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)4294967295LL; static unsigned char r3 = (unsigned char)(unsigned short)4294967295LL;
      printf("884: (unsigned char)(unsigned short)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; short r1 = (short)v; short r2 = (short)(unsigned short)4294967295LL; static short r3 = (short)(unsigned short)4294967295LL;
      printf("885: (short)(unsigned short)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)4294967295LL; static unsigned short r3 = (unsigned short)(unsigned short)4294967295LL;
      printf("886: (unsigned short)(unsigned short)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; int r1 = (int)v; int r2 = (int)(unsigned short)4294967295LL; static int r3 = (int)(unsigned short)4294967295LL;
      printf("887: (int)(unsigned short)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)4294967295LL; static unsigned r3 = (unsigned)(unsigned short)4294967295LL;
      printf("888: (unsigned)(unsigned short)4294967295LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; long r1 = (long)v; long r2 = (long)(unsigned short)4294967295LL; static long r3 = (long)(unsigned short)4294967295LL;
      printf("889: (long)(unsigned short)4294967295LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)4294967295LL; static unsigned long r3 = (unsigned long)(unsigned short)4294967295LL;
      printf("890: (unsigned long)(unsigned short)4294967295LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)4294967295LL; static long long r3 = (long long)(unsigned short)4294967295LL;
      printf("891: (long long)(unsigned short)4294967295LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)4294967295LL; static unsigned long long r3 = (unsigned long long)(unsigned short)4294967295LL;
      printf("892: (unsigned long long)(unsigned short)4294967295LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)4294967295LL; static _Bool r3 = (_Bool)(unsigned short)4294967295LL;
      printf("893: (_Bool)(unsigned short)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; float r1 = (float)v; float r2 = (float)(unsigned short)4294967295LL; static float r3 = (float)(unsigned short)4294967295LL;
      printf("894: (float)(unsigned short)4294967295LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; double r1 = (double)v; double r2 = (double)(unsigned short)4294967295LL; static double r3 = (double)(unsigned short)4294967295LL;
      printf("895: (double)(unsigned short)4294967295LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)4294967295LL; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)4294967295LL; static long double r3 = (long double)(unsigned short)4294967295LL;
      printf("896: (long double)(unsigned short)4294967295LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)9223372036854775807LL; static signed char r3 = (signed char)(unsigned short)9223372036854775807LL;
      printf("897: (signed char)(unsigned short)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)9223372036854775807LL; static unsigned char r3 = (unsigned char)(unsigned short)9223372036854775807LL;
      printf("898: (unsigned char)(unsigned short)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; short r1 = (short)v; short r2 = (short)(unsigned short)9223372036854775807LL; static short r3 = (short)(unsigned short)9223372036854775807LL;
      printf("899: (short)(unsigned short)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)9223372036854775807LL; static unsigned short r3 = (unsigned short)(unsigned short)9223372036854775807LL;
      printf("900: (unsigned short)(unsigned short)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; int r1 = (int)v; int r2 = (int)(unsigned short)9223372036854775807LL; static int r3 = (int)(unsigned short)9223372036854775807LL;
      printf("901: (int)(unsigned short)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)9223372036854775807LL; static unsigned r3 = (unsigned)(unsigned short)9223372036854775807LL;
      printf("902: (unsigned)(unsigned short)9223372036854775807LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; long r1 = (long)v; long r2 = (long)(unsigned short)9223372036854775807LL; static long r3 = (long)(unsigned short)9223372036854775807LL;
      printf("903: (long)(unsigned short)9223372036854775807LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)9223372036854775807LL; static unsigned long r3 = (unsigned long)(unsigned short)9223372036854775807LL;
      printf("904: (unsigned long)(unsigned short)9223372036854775807LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)9223372036854775807LL; static long long r3 = (long long)(unsigned short)9223372036854775807LL;
      printf("905: (long long)(unsigned short)9223372036854775807LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)9223372036854775807LL; static unsigned long long r3 = (unsigned long long)(unsigned short)9223372036854775807LL;
      printf("906: (unsigned long long)(unsigned short)9223372036854775807LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)9223372036854775807LL; static _Bool r3 = (_Bool)(unsigned short)9223372036854775807LL;
      printf("907: (_Bool)(unsigned short)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; float r1 = (float)v; float r2 = (float)(unsigned short)9223372036854775807LL; static float r3 = (float)(unsigned short)9223372036854775807LL;
      printf("908: (float)(unsigned short)9223372036854775807LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; double r1 = (double)v; double r2 = (double)(unsigned short)9223372036854775807LL; static double r3 = (double)(unsigned short)9223372036854775807LL;
      printf("909: (double)(unsigned short)9223372036854775807LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)9223372036854775807LL; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)9223372036854775807LL; static long double r3 = (long double)(unsigned short)9223372036854775807LL;
      printf("910: (long double)(unsigned short)9223372036854775807LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)(-9223372036854775807LL-1); static signed char r3 = (signed char)(unsigned short)(-9223372036854775807LL-1);
      printf("911: (signed char)(unsigned short)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)(-9223372036854775807LL-1); static unsigned char r3 = (unsigned char)(unsigned short)(-9223372036854775807LL-1);
      printf("912: (unsigned char)(unsigned short)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); short r1 = (short)v; short r2 = (short)(unsigned short)(-9223372036854775807LL-1); static short r3 = (short)(unsigned short)(-9223372036854775807LL-1);
      printf("913: (short)(unsigned short)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)(-9223372036854775807LL-1); static unsigned short r3 = (unsigned short)(unsigned short)(-9223372036854775807LL-1);
      printf("914: (unsigned short)(unsigned short)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); int r1 = (int)v; int r2 = (int)(unsigned short)(-9223372036854775807LL-1); static int r3 = (int)(unsigned short)(-9223372036854775807LL-1);
      printf("915: (int)(unsigned short)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)(-9223372036854775807LL-1); static unsigned r3 = (unsigned)(unsigned short)(-9223372036854775807LL-1);
      printf("916: (unsigned)(unsigned short)(-9223372036854775807LL-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); long r1 = (long)v; long r2 = (long)(unsigned short)(-9223372036854775807LL-1); static long r3 = (long)(unsigned short)(-9223372036854775807LL-1);
      printf("917: (long)(unsigned short)(-9223372036854775807LL-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)(-9223372036854775807LL-1); static unsigned long r3 = (unsigned long)(unsigned short)(-9223372036854775807LL-1);
      printf("918: (unsigned long)(unsigned short)(-9223372036854775807LL-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); long long r1 = (long long)v; long long r2 = (long long)(unsigned short)(-9223372036854775807LL-1); static long long r3 = (long long)(unsigned short)(-9223372036854775807LL-1);
      printf("919: (long long)(unsigned short)(-9223372036854775807LL-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)(-9223372036854775807LL-1); static unsigned long long r3 = (unsigned long long)(unsigned short)(-9223372036854775807LL-1);
      printf("920: (unsigned long long)(unsigned short)(-9223372036854775807LL-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)(-9223372036854775807LL-1); static _Bool r3 = (_Bool)(unsigned short)(-9223372036854775807LL-1);
      printf("921: (_Bool)(unsigned short)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); float r1 = (float)v; float r2 = (float)(unsigned short)(-9223372036854775807LL-1); static float r3 = (float)(unsigned short)(-9223372036854775807LL-1);
      printf("922: (float)(unsigned short)(-9223372036854775807LL-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); double r1 = (double)v; double r2 = (double)(unsigned short)(-9223372036854775807LL-1); static double r3 = (double)(unsigned short)(-9223372036854775807LL-1);
      printf("923: (double)(unsigned short)(-9223372036854775807LL-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)(-9223372036854775807LL-1); long double r1 = (long double)v; long double r2 = (long double)(unsigned short)(-9223372036854775807LL-1); static long double r3 = (long double)(unsigned short)(-9223372036854775807LL-1);
      printf("924: (long double)(unsigned short)(-9223372036854775807LL-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)0x8000000000000000ULL; static signed char r3 = (signed char)(unsigned short)0x8000000000000000ULL;
      printf("925: (signed char)(unsigned short)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)0x8000000000000000ULL; static unsigned char r3 = (unsigned char)(unsigned short)0x8000000000000000ULL;
      printf("926: (unsigned char)(unsigned short)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; short r1 = (short)v; short r2 = (short)(unsigned short)0x8000000000000000ULL; static short r3 = (short)(unsigned short)0x8000000000000000ULL;
      printf("927: (short)(unsigned short)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)0x8000000000000000ULL; static unsigned short r3 = (unsigned short)(unsigned short)0x8000000000000000ULL;
      printf("928: (unsigned short)(unsigned short)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; int r1 = (int)v; int r2 = (int)(unsigned short)0x8000000000000000ULL; static int r3 = (int)(unsigned short)0x8000000000000000ULL;
      printf("929: (int)(unsigned short)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)0x8000000000000000ULL; static unsigned r3 = (unsigned)(unsigned short)0x8000000000000000ULL;
      printf("930: (unsigned)(unsigned short)0x8000000000000000ULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; long r1 = (long)v; long r2 = (long)(unsigned short)0x8000000000000000ULL; static long r3 = (long)(unsigned short)0x8000000000000000ULL;
      printf("931: (long)(unsigned short)0x8000000000000000ULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)0x8000000000000000ULL; static unsigned long r3 = (unsigned long)(unsigned short)0x8000000000000000ULL;
      printf("932: (unsigned long)(unsigned short)0x8000000000000000ULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)0x8000000000000000ULL; static long long r3 = (long long)(unsigned short)0x8000000000000000ULL;
      printf("933: (long long)(unsigned short)0x8000000000000000ULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)0x8000000000000000ULL; static unsigned long long r3 = (unsigned long long)(unsigned short)0x8000000000000000ULL;
      printf("934: (unsigned long long)(unsigned short)0x8000000000000000ULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)0x8000000000000000ULL; static _Bool r3 = (_Bool)(unsigned short)0x8000000000000000ULL;
      printf("935: (_Bool)(unsigned short)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; float r1 = (float)v; float r2 = (float)(unsigned short)0x8000000000000000ULL; static float r3 = (float)(unsigned short)0x8000000000000000ULL;
      printf("936: (float)(unsigned short)0x8000000000000000ULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; double r1 = (double)v; double r2 = (double)(unsigned short)0x8000000000000000ULL; static double r3 = (double)(unsigned short)0x8000000000000000ULL;
      printf("937: (double)(unsigned short)0x8000000000000000ULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)0x8000000000000000ULL; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)0x8000000000000000ULL; static long double r3 = (long double)(unsigned short)0x8000000000000000ULL;
      printf("938: (long double)(unsigned short)0x8000000000000000ULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned short)0xffffffffffffffffULL; static signed char r3 = (signed char)(unsigned short)0xffffffffffffffffULL;
      printf("939: (signed char)(unsigned short)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned short)0xffffffffffffffffULL; static unsigned char r3 = (unsigned char)(unsigned short)0xffffffffffffffffULL;
      printf("940: (unsigned char)(unsigned short)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; short r1 = (short)v; short r2 = (short)(unsigned short)0xffffffffffffffffULL; static short r3 = (short)(unsigned short)0xffffffffffffffffULL;
      printf("941: (short)(unsigned short)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned short)0xffffffffffffffffULL; static unsigned short r3 = (unsigned short)(unsigned short)0xffffffffffffffffULL;
      printf("942: (unsigned short)(unsigned short)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; int r1 = (int)v; int r2 = (int)(unsigned short)0xffffffffffffffffULL; static int r3 = (int)(unsigned short)0xffffffffffffffffULL;
      printf("943: (int)(unsigned short)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned short)0xffffffffffffffffULL; static unsigned r3 = (unsigned)(unsigned short)0xffffffffffffffffULL;
      printf("944: (unsigned)(unsigned short)0xffffffffffffffffULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; long r1 = (long)v; long r2 = (long)(unsigned short)0xffffffffffffffffULL; static long r3 = (long)(unsigned short)0xffffffffffffffffULL;
      printf("945: (long)(unsigned short)0xffffffffffffffffULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned short)0xffffffffffffffffULL; static unsigned long r3 = (unsigned long)(unsigned short)0xffffffffffffffffULL;
      printf("946: (unsigned long)(unsigned short)0xffffffffffffffffULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; long long r1 = (long long)v; long long r2 = (long long)(unsigned short)0xffffffffffffffffULL; static long long r3 = (long long)(unsigned short)0xffffffffffffffffULL;
      printf("947: (long long)(unsigned short)0xffffffffffffffffULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned short)0xffffffffffffffffULL; static unsigned long long r3 = (unsigned long long)(unsigned short)0xffffffffffffffffULL;
      printf("948: (unsigned long long)(unsigned short)0xffffffffffffffffULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned short)0xffffffffffffffffULL; static _Bool r3 = (_Bool)(unsigned short)0xffffffffffffffffULL;
      printf("949: (_Bool)(unsigned short)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; float r1 = (float)v; float r2 = (float)(unsigned short)0xffffffffffffffffULL; static float r3 = (float)(unsigned short)0xffffffffffffffffULL;
      printf("950: (float)(unsigned short)0xffffffffffffffffULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; double r1 = (double)v; double r2 = (double)(unsigned short)0xffffffffffffffffULL; static double r3 = (double)(unsigned short)0xffffffffffffffffULL;
      printf("951: (double)(unsigned short)0xffffffffffffffffULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned short v = (unsigned short)0xffffffffffffffffULL; long double r1 = (long double)v; long double r2 = (long double)(unsigned short)0xffffffffffffffffULL; static long double r3 = (long double)(unsigned short)0xffffffffffffffffULL;
      printf("952: (long double)(unsigned short)0xffffffffffffffffULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)0; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)0; static signed char r3 = (signed char)(int)0;
      printf("953: (signed char)(int)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)0; static unsigned char r3 = (unsigned char)(int)0;
      printf("954: (unsigned char)(int)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0; short r1 = (short)v; short r2 = (short)(int)0; static short r3 = (short)(int)0;
      printf("955: (short)(int)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)0; static unsigned short r3 = (unsigned short)(int)0;
      printf("956: (unsigned short)(int)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0; int r1 = (int)v; int r2 = (int)(int)0; static int r3 = (int)(int)0;
      printf("957: (int)(int)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)0; static unsigned r3 = (unsigned)(int)0;
      printf("958: (unsigned)(int)0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)0; long r1 = (long)v; long r2 = (long)(int)0; static long r3 = (long)(int)0;
      printf("959: (long)(int)0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)0; static unsigned long r3 = (unsigned long)(int)0;
      printf("960: (unsigned long)(int)0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)0; long long r1 = (long long)v; long long r2 = (long long)(int)0; static long long r3 = (long long)(int)0;
      printf("961: (long long)(int)0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)0; static unsigned long long r3 = (unsigned long long)(int)0;
      printf("962: (unsigned long long)(int)0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)0; static _Bool r3 = (_Bool)(int)0;
      printf("963: (_Bool)(int)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0; float r1 = (float)v; float r2 = (float)(int)0; static float r3 = (float)(int)0;
      printf("964: (float)(int)0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)0; double r1 = (double)v; double r2 = (double)(int)0; static double r3 = (double)(int)0;
      printf("965: (double)(int)0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)0; long double r1 = (long double)v; long double r2 = (long double)(int)0; static long double r3 = (long double)(int)0;
      printf("966: (long double)(int)0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)1; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)1; static signed char r3 = (signed char)(int)1;
      printf("967: (signed char)(int)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)1; static unsigned char r3 = (unsigned char)(int)1;
      printf("968: (unsigned char)(int)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)1; short r1 = (short)v; short r2 = (short)(int)1; static short r3 = (short)(int)1;
      printf("969: (short)(int)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)1; static unsigned short r3 = (unsigned short)(int)1;
      printf("970: (unsigned short)(int)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)1; int r1 = (int)v; int r2 = (int)(int)1; static int r3 = (int)(int)1;
      printf("971: (int)(int)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)1; static unsigned r3 = (unsigned)(int)1;
      printf("972: (unsigned)(int)1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)1; long r1 = (long)v; long r2 = (long)(int)1; static long r3 = (long)(int)1;
      printf("973: (long)(int)1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)1; static unsigned long r3 = (unsigned long)(int)1;
      printf("974: (unsigned long)(int)1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)1; long long r1 = (long long)v; long long r2 = (long long)(int)1; static long long r3 = (long long)(int)1;
      printf("975: (long long)(int)1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)1; static unsigned long long r3 = (unsigned long long)(int)1;
      printf("976: (unsigned long long)(int)1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)1; static _Bool r3 = (_Bool)(int)1;
      printf("977: (_Bool)(int)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)1; float r1 = (float)v; float r2 = (float)(int)1; static float r3 = (float)(int)1;
      printf("978: (float)(int)1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)1; double r1 = (double)v; double r2 = (double)(int)1; static double r3 = (double)(int)1;
      printf("979: (double)(int)1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)1; long double r1 = (long double)v; long double r2 = (long double)(int)1; static long double r3 = (long double)(int)1;
      printf("980: (long double)(int)1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)-1; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)-1; static signed char r3 = (signed char)(int)-1;
      printf("981: (signed char)(int)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)-1; static unsigned char r3 = (unsigned char)(int)-1;
      printf("982: (unsigned char)(int)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-1; short r1 = (short)v; short r2 = (short)(int)-1; static short r3 = (short)(int)-1;
      printf("983: (short)(int)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)-1; static unsigned short r3 = (unsigned short)(int)-1;
      printf("984: (unsigned short)(int)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-1; int r1 = (int)v; int r2 = (int)(int)-1; static int r3 = (int)(int)-1;
      printf("985: (int)(int)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)-1; static unsigned r3 = (unsigned)(int)-1;
      printf("986: (unsigned)(int)-1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)-1; long r1 = (long)v; long r2 = (long)(int)-1; static long r3 = (long)(int)-1;
      printf("987: (long)(int)-1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)-1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)-1; static unsigned long r3 = (unsigned long)(int)-1;
      printf("988: (unsigned long)(int)-1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)-1; long long r1 = (long long)v; long long r2 = (long long)(int)-1; static long long r3 = (long long)(int)-1;
      printf("989: (long long)(int)-1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)-1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)-1; static unsigned long long r3 = (unsigned long long)(int)-1;
      printf("990: (unsigned long long)(int)-1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)-1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)-1; static _Bool r3 = (_Bool)(int)-1;
      printf("991: (_Bool)(int)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-1; float r1 = (float)v; float r2 = (float)(int)-1; static float r3 = (float)(int)-1;
      printf("992: (float)(int)-1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)-1; double r1 = (double)v; double r2 = (double)(int)-1; static double r3 = (double)(int)-1;
      printf("993: (double)(int)-1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)-1; long double r1 = (long double)v; long double r2 = (long double)(int)-1; static long double r3 = (long double)(int)-1;
      printf("994: (long double)(int)-1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)127; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)127; static signed char r3 = (signed char)(int)127;
      printf("995: (signed char)(int)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)127; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)127; static unsigned char r3 = (unsigned char)(int)127;
      printf("996: (unsigned char)(int)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)127; short r1 = (short)v; short r2 = (short)(int)127; static short r3 = (short)(int)127;
      printf("997: (short)(int)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)127; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)127; static unsigned short r3 = (unsigned short)(int)127;
      printf("998: (unsigned short)(int)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)127; int r1 = (int)v; int r2 = (int)(int)127; static int r3 = (int)(int)127;
      printf("999: (int)(int)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)127; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)127; static unsigned r3 = (unsigned)(int)127;
      printf("1000: (unsigned)(int)127 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)127; long r1 = (long)v; long r2 = (long)(int)127; static long r3 = (long)(int)127;
      printf("1001: (long)(int)127 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)127; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)127; static unsigned long r3 = (unsigned long)(int)127;
      printf("1002: (unsigned long)(int)127 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)127; long long r1 = (long long)v; long long r2 = (long long)(int)127; static long long r3 = (long long)(int)127;
      printf("1003: (long long)(int)127 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)127; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)127; static unsigned long long r3 = (unsigned long long)(int)127;
      printf("1004: (unsigned long long)(int)127 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)127; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)127; static _Bool r3 = (_Bool)(int)127;
      printf("1005: (_Bool)(int)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)127; float r1 = (float)v; float r2 = (float)(int)127; static float r3 = (float)(int)127;
      printf("1006: (float)(int)127 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)127; double r1 = (double)v; double r2 = (double)(int)127; static double r3 = (double)(int)127;
      printf("1007: (double)(int)127 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)127; long double r1 = (long double)v; long double r2 = (long double)(int)127; static long double r3 = (long double)(int)127;
      printf("1008: (long double)(int)127 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)-128; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)-128; static signed char r3 = (signed char)(int)-128;
      printf("1009: (signed char)(int)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-128; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)-128; static unsigned char r3 = (unsigned char)(int)-128;
      printf("1010: (unsigned char)(int)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-128; short r1 = (short)v; short r2 = (short)(int)-128; static short r3 = (short)(int)-128;
      printf("1011: (short)(int)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-128; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)-128; static unsigned short r3 = (unsigned short)(int)-128;
      printf("1012: (unsigned short)(int)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-128; int r1 = (int)v; int r2 = (int)(int)-128; static int r3 = (int)(int)-128;
      printf("1013: (int)(int)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-128; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)-128; static unsigned r3 = (unsigned)(int)-128;
      printf("1014: (unsigned)(int)-128 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)-128; long r1 = (long)v; long r2 = (long)(int)-128; static long r3 = (long)(int)-128;
      printf("1015: (long)(int)-128 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)-128; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)-128; static unsigned long r3 = (unsigned long)(int)-128;
      printf("1016: (unsigned long)(int)-128 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)-128; long long r1 = (long long)v; long long r2 = (long long)(int)-128; static long long r3 = (long long)(int)-128;
      printf("1017: (long long)(int)-128 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)-128; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)-128; static unsigned long long r3 = (unsigned long long)(int)-128;
      printf("1018: (unsigned long long)(int)-128 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)-128; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)-128; static _Bool r3 = (_Bool)(int)-128;
      printf("1019: (_Bool)(int)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-128; float r1 = (float)v; float r2 = (float)(int)-128; static float r3 = (float)(int)-128;
      printf("1020: (float)(int)-128 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)-128; double r1 = (double)v; double r2 = (double)(int)-128; static double r3 = (double)(int)-128;
      printf("1021: (double)(int)-128 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)-128; long double r1 = (long double)v; long double r2 = (long double)(int)-128; static long double r3 = (long double)(int)-128;
      printf("1022: (long double)(int)-128 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)255; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)255; static signed char r3 = (signed char)(int)255;
      printf("1023: (signed char)(int)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)255; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)255; static unsigned char r3 = (unsigned char)(int)255;
      printf("1024: (unsigned char)(int)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)255; short r1 = (short)v; short r2 = (short)(int)255; static short r3 = (short)(int)255;
      printf("1025: (short)(int)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)255; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)255; static unsigned short r3 = (unsigned short)(int)255;
      printf("1026: (unsigned short)(int)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)255; int r1 = (int)v; int r2 = (int)(int)255; static int r3 = (int)(int)255;
      printf("1027: (int)(int)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)255; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)255; static unsigned r3 = (unsigned)(int)255;
      printf("1028: (unsigned)(int)255 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)255; long r1 = (long)v; long r2 = (long)(int)255; static long r3 = (long)(int)255;
      printf("1029: (long)(int)255 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)255; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)255; static unsigned long r3 = (unsigned long)(int)255;
      printf("1030: (unsigned long)(int)255 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)255; long long r1 = (long long)v; long long r2 = (long long)(int)255; static long long r3 = (long long)(int)255;
      printf("1031: (long long)(int)255 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)255; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)255; static unsigned long long r3 = (unsigned long long)(int)255;
      printf("1032: (unsigned long long)(int)255 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)255; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)255; static _Bool r3 = (_Bool)(int)255;
      printf("1033: (_Bool)(int)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)255; float r1 = (float)v; float r2 = (float)(int)255; static float r3 = (float)(int)255;
      printf("1034: (float)(int)255 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)255; double r1 = (double)v; double r2 = (double)(int)255; static double r3 = (double)(int)255;
      printf("1035: (double)(int)255 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)255; long double r1 = (long double)v; long double r2 = (long double)(int)255; static long double r3 = (long double)(int)255;
      printf("1036: (long double)(int)255 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)256; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)256; static signed char r3 = (signed char)(int)256;
      printf("1037: (signed char)(int)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)256; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)256; static unsigned char r3 = (unsigned char)(int)256;
      printf("1038: (unsigned char)(int)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)256; short r1 = (short)v; short r2 = (short)(int)256; static short r3 = (short)(int)256;
      printf("1039: (short)(int)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)256; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)256; static unsigned short r3 = (unsigned short)(int)256;
      printf("1040: (unsigned short)(int)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)256; int r1 = (int)v; int r2 = (int)(int)256; static int r3 = (int)(int)256;
      printf("1041: (int)(int)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)256; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)256; static unsigned r3 = (unsigned)(int)256;
      printf("1042: (unsigned)(int)256 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)256; long r1 = (long)v; long r2 = (long)(int)256; static long r3 = (long)(int)256;
      printf("1043: (long)(int)256 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)256; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)256; static unsigned long r3 = (unsigned long)(int)256;
      printf("1044: (unsigned long)(int)256 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)256; long long r1 = (long long)v; long long r2 = (long long)(int)256; static long long r3 = (long long)(int)256;
      printf("1045: (long long)(int)256 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)256; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)256; static unsigned long long r3 = (unsigned long long)(int)256;
      printf("1046: (unsigned long long)(int)256 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)256; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)256; static _Bool r3 = (_Bool)(int)256;
      printf("1047: (_Bool)(int)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)256; float r1 = (float)v; float r2 = (float)(int)256; static float r3 = (float)(int)256;
      printf("1048: (float)(int)256 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)256; double r1 = (double)v; double r2 = (double)(int)256; static double r3 = (double)(int)256;
      printf("1049: (double)(int)256 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)256; long double r1 = (long double)v; long double r2 = (long double)(int)256; static long double r3 = (long double)(int)256;
      printf("1050: (long double)(int)256 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)32767; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)32767; static signed char r3 = (signed char)(int)32767;
      printf("1051: (signed char)(int)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)32767; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)32767; static unsigned char r3 = (unsigned char)(int)32767;
      printf("1052: (unsigned char)(int)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)32767; short r1 = (short)v; short r2 = (short)(int)32767; static short r3 = (short)(int)32767;
      printf("1053: (short)(int)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)32767; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)32767; static unsigned short r3 = (unsigned short)(int)32767;
      printf("1054: (unsigned short)(int)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)32767; int r1 = (int)v; int r2 = (int)(int)32767; static int r3 = (int)(int)32767;
      printf("1055: (int)(int)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)32767; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)32767; static unsigned r3 = (unsigned)(int)32767;
      printf("1056: (unsigned)(int)32767 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)32767; long r1 = (long)v; long r2 = (long)(int)32767; static long r3 = (long)(int)32767;
      printf("1057: (long)(int)32767 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)32767; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)32767; static unsigned long r3 = (unsigned long)(int)32767;
      printf("1058: (unsigned long)(int)32767 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)32767; long long r1 = (long long)v; long long r2 = (long long)(int)32767; static long long r3 = (long long)(int)32767;
      printf("1059: (long long)(int)32767 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)32767; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)32767; static unsigned long long r3 = (unsigned long long)(int)32767;
      printf("1060: (unsigned long long)(int)32767 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)32767; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)32767; static _Bool r3 = (_Bool)(int)32767;
      printf("1061: (_Bool)(int)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)32767; float r1 = (float)v; float r2 = (float)(int)32767; static float r3 = (float)(int)32767;
      printf("1062: (float)(int)32767 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)32767; double r1 = (double)v; double r2 = (double)(int)32767; static double r3 = (double)(int)32767;
      printf("1063: (double)(int)32767 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)32767; long double r1 = (long double)v; long double r2 = (long double)(int)32767; static long double r3 = (long double)(int)32767;
      printf("1064: (long double)(int)32767 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)-32768; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)-32768; static signed char r3 = (signed char)(int)-32768;
      printf("1065: (signed char)(int)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-32768; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)-32768; static unsigned char r3 = (unsigned char)(int)-32768;
      printf("1066: (unsigned char)(int)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-32768; short r1 = (short)v; short r2 = (short)(int)-32768; static short r3 = (short)(int)-32768;
      printf("1067: (short)(int)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-32768; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)-32768; static unsigned short r3 = (unsigned short)(int)-32768;
      printf("1068: (unsigned short)(int)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-32768; int r1 = (int)v; int r2 = (int)(int)-32768; static int r3 = (int)(int)-32768;
      printf("1069: (int)(int)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-32768; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)-32768; static unsigned r3 = (unsigned)(int)-32768;
      printf("1070: (unsigned)(int)-32768 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)-32768; long r1 = (long)v; long r2 = (long)(int)-32768; static long r3 = (long)(int)-32768;
      printf("1071: (long)(int)-32768 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)-32768; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)-32768; static unsigned long r3 = (unsigned long)(int)-32768;
      printf("1072: (unsigned long)(int)-32768 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)-32768; long long r1 = (long long)v; long long r2 = (long long)(int)-32768; static long long r3 = (long long)(int)-32768;
      printf("1073: (long long)(int)-32768 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)-32768; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)-32768; static unsigned long long r3 = (unsigned long long)(int)-32768;
      printf("1074: (unsigned long long)(int)-32768 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)-32768; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)-32768; static _Bool r3 = (_Bool)(int)-32768;
      printf("1075: (_Bool)(int)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)-32768; float r1 = (float)v; float r2 = (float)(int)-32768; static float r3 = (float)(int)-32768;
      printf("1076: (float)(int)-32768 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)-32768; double r1 = (double)v; double r2 = (double)(int)-32768; static double r3 = (double)(int)-32768;
      printf("1077: (double)(int)-32768 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)-32768; long double r1 = (long double)v; long double r2 = (long double)(int)-32768; static long double r3 = (long double)(int)-32768;
      printf("1078: (long double)(int)-32768 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)65535; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)65535; static signed char r3 = (signed char)(int)65535;
      printf("1079: (signed char)(int)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)65535; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)65535; static unsigned char r3 = (unsigned char)(int)65535;
      printf("1080: (unsigned char)(int)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)65535; short r1 = (short)v; short r2 = (short)(int)65535; static short r3 = (short)(int)65535;
      printf("1081: (short)(int)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)65535; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)65535; static unsigned short r3 = (unsigned short)(int)65535;
      printf("1082: (unsigned short)(int)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)65535; int r1 = (int)v; int r2 = (int)(int)65535; static int r3 = (int)(int)65535;
      printf("1083: (int)(int)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)65535; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)65535; static unsigned r3 = (unsigned)(int)65535;
      printf("1084: (unsigned)(int)65535 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)65535; long r1 = (long)v; long r2 = (long)(int)65535; static long r3 = (long)(int)65535;
      printf("1085: (long)(int)65535 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)65535; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)65535; static unsigned long r3 = (unsigned long)(int)65535;
      printf("1086: (unsigned long)(int)65535 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)65535; long long r1 = (long long)v; long long r2 = (long long)(int)65535; static long long r3 = (long long)(int)65535;
      printf("1087: (long long)(int)65535 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)65535; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)65535; static unsigned long long r3 = (unsigned long long)(int)65535;
      printf("1088: (unsigned long long)(int)65535 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)65535; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)65535; static _Bool r3 = (_Bool)(int)65535;
      printf("1089: (_Bool)(int)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)65535; float r1 = (float)v; float r2 = (float)(int)65535; static float r3 = (float)(int)65535;
      printf("1090: (float)(int)65535 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)65535; double r1 = (double)v; double r2 = (double)(int)65535; static double r3 = (double)(int)65535;
      printf("1091: (double)(int)65535 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)65535; long double r1 = (long double)v; long double r2 = (long double)(int)65535; static long double r3 = (long double)(int)65535;
      printf("1092: (long double)(int)65535 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)2147483647; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)2147483647; static signed char r3 = (signed char)(int)2147483647;
      printf("1093: (signed char)(int)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)2147483647; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)2147483647; static unsigned char r3 = (unsigned char)(int)2147483647;
      printf("1094: (unsigned char)(int)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)2147483647; short r1 = (short)v; short r2 = (short)(int)2147483647; static short r3 = (short)(int)2147483647;
      printf("1095: (short)(int)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)2147483647; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)2147483647; static unsigned short r3 = (unsigned short)(int)2147483647;
      printf("1096: (unsigned short)(int)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)2147483647; int r1 = (int)v; int r2 = (int)(int)2147483647; static int r3 = (int)(int)2147483647;
      printf("1097: (int)(int)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)2147483647; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)2147483647; static unsigned r3 = (unsigned)(int)2147483647;
      printf("1098: (unsigned)(int)2147483647 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)2147483647; long r1 = (long)v; long r2 = (long)(int)2147483647; static long r3 = (long)(int)2147483647;
      printf("1099: (long)(int)2147483647 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)2147483647; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)2147483647; static unsigned long r3 = (unsigned long)(int)2147483647;
      printf("1100: (unsigned long)(int)2147483647 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)2147483647; long long r1 = (long long)v; long long r2 = (long long)(int)2147483647; static long long r3 = (long long)(int)2147483647;
      printf("1101: (long long)(int)2147483647 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)2147483647; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)2147483647; static unsigned long long r3 = (unsigned long long)(int)2147483647;
      printf("1102: (unsigned long long)(int)2147483647 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)2147483647; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)2147483647; static _Bool r3 = (_Bool)(int)2147483647;
      printf("1103: (_Bool)(int)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)2147483647; float r1 = (float)v; float r2 = (float)(int)2147483647; static float r3 = (float)(int)2147483647;
      printf("1104: (float)(int)2147483647 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)2147483647; double r1 = (double)v; double r2 = (double)(int)2147483647; static double r3 = (double)(int)2147483647;
      printf("1105: (double)(int)2147483647 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)2147483647; long double r1 = (long double)v; long double r2 = (long double)(int)2147483647; static long double r3 = (long double)(int)2147483647;
      printf("1106: (long double)(int)2147483647 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)(-2147483647-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(int)(-2147483647-1); static signed char r3 = (signed char)(int)(-2147483647-1);
      printf("1107: (signed char)(int)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)(-2147483647-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)(-2147483647-1); static unsigned char r3 = (unsigned char)(int)(-2147483647-1);
      printf("1108: (unsigned char)(int)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)(-2147483647-1); short r1 = (short)v; short r2 = (short)(int)(-2147483647-1); static short r3 = (short)(int)(-2147483647-1);
      printf("1109: (short)(int)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)(-2147483647-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)(-2147483647-1); static unsigned short r3 = (unsigned short)(int)(-2147483647-1);
      printf("1110: (unsigned short)(int)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)(-2147483647-1); int r1 = (int)v; int r2 = (int)(int)(-2147483647-1); static int r3 = (int)(int)(-2147483647-1);
      printf("1111: (int)(int)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)(-2147483647-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)(-2147483647-1); static unsigned r3 = (unsigned)(int)(-2147483647-1);
      printf("1112: (unsigned)(int)(-2147483647-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)(-2147483647-1); long r1 = (long)v; long r2 = (long)(int)(-2147483647-1); static long r3 = (long)(int)(-2147483647-1);
      printf("1113: (long)(int)(-2147483647-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)(-2147483647-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)(-2147483647-1); static unsigned long r3 = (unsigned long)(int)(-2147483647-1);
      printf("1114: (unsigned long)(int)(-2147483647-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)(-2147483647-1); long long r1 = (long long)v; long long r2 = (long long)(int)(-2147483647-1); static long long r3 = (long long)(int)(-2147483647-1);
      printf("1115: (long long)(int)(-2147483647-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)(-2147483647-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)(-2147483647-1); static unsigned long long r3 = (unsigned long long)(int)(-2147483647-1);
      printf("1116: (unsigned long long)(int)(-2147483647-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)(-2147483647-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)(-2147483647-1); static _Bool r3 = (_Bool)(int)(-2147483647-1);
      printf("1117: (_Bool)(int)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)(-2147483647-1); float r1 = (float)v; float r2 = (float)(int)(-2147483647-1); static float r3 = (float)(int)(-2147483647-1);
      printf("1118: (float)(int)(-2147483647-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)(-2147483647-1); double r1 = (double)v; double r2 = (double)(int)(-2147483647-1); static double r3 = (double)(int)(-2147483647-1);
      printf("1119: (double)(int)(-2147483647-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)(-2147483647-1); long double r1 = (long double)v; long double r2 = (long double)(int)(-2147483647-1); static long double r3 = (long double)(int)(-2147483647-1);
      printf("1120: (long double)(int)(-2147483647-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)4294967295LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)4294967295LL; static signed char r3 = (signed char)(int)4294967295LL;
      printf("1121: (signed char)(int)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)4294967295LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)4294967295LL; static unsigned char r3 = (unsigned char)(int)4294967295LL;
      printf("1122: (unsigned char)(int)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)4294967295LL; short r1 = (short)v; short r2 = (short)(int)4294967295LL; static short r3 = (short)(int)4294967295LL;
      printf("1123: (short)(int)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)4294967295LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)4294967295LL; static unsigned short r3 = (unsigned short)(int)4294967295LL;
      printf("1124: (unsigned short)(int)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)4294967295LL; int r1 = (int)v; int r2 = (int)(int)4294967295LL; static int r3 = (int)(int)4294967295LL;
      printf("1125: (int)(int)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)4294967295LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)4294967295LL; static unsigned r3 = (unsigned)(int)4294967295LL;
      printf("1126: (unsigned)(int)4294967295LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)4294967295LL; long r1 = (long)v; long r2 = (long)(int)4294967295LL; static long r3 = (long)(int)4294967295LL;
      printf("1127: (long)(int)4294967295LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)4294967295LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)4294967295LL; static unsigned long r3 = (unsigned long)(int)4294967295LL;
      printf("1128: (unsigned long)(int)4294967295LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)4294967295LL; long long r1 = (long long)v; long long r2 = (long long)(int)4294967295LL; static long long r3 = (long long)(int)4294967295LL;
      printf("1129: (long long)(int)4294967295LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)4294967295LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)4294967295LL; static unsigned long long r3 = (unsigned long long)(int)4294967295LL;
      printf("1130: (unsigned long long)(int)4294967295LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)4294967295LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)4294967295LL; static _Bool r3 = (_Bool)(int)4294967295LL;
      printf("1131: (_Bool)(int)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)4294967295LL; float r1 = (float)v; float r2 = (float)(int)4294967295LL; static float r3 = (float)(int)4294967295LL;
      printf("1132: (float)(int)4294967295LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)4294967295LL; double r1 = (double)v; double r2 = (double)(int)4294967295LL; static double r3 = (double)(int)4294967295LL;
      printf("1133: (double)(int)4294967295LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)4294967295LL; long double r1 = (long double)v; long double r2 = (long double)(int)4294967295LL; static long double r3 = (long double)(int)4294967295LL;
      printf("1134: (long double)(int)4294967295LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)9223372036854775807LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)9223372036854775807LL; static signed char r3 = (signed char)(int)9223372036854775807LL;
      printf("1135: (signed char)(int)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)9223372036854775807LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)9223372036854775807LL; static unsigned char r3 = (unsigned char)(int)9223372036854775807LL;
      printf("1136: (unsigned char)(int)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)9223372036854775807LL; short r1 = (short)v; short r2 = (short)(int)9223372036854775807LL; static short r3 = (short)(int)9223372036854775807LL;
      printf("1137: (short)(int)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)9223372036854775807LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)9223372036854775807LL; static unsigned short r3 = (unsigned short)(int)9223372036854775807LL;
      printf("1138: (unsigned short)(int)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)9223372036854775807LL; int r1 = (int)v; int r2 = (int)(int)9223372036854775807LL; static int r3 = (int)(int)9223372036854775807LL;
      printf("1139: (int)(int)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)9223372036854775807LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)9223372036854775807LL; static unsigned r3 = (unsigned)(int)9223372036854775807LL;
      printf("1140: (unsigned)(int)9223372036854775807LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)9223372036854775807LL; long r1 = (long)v; long r2 = (long)(int)9223372036854775807LL; static long r3 = (long)(int)9223372036854775807LL;
      printf("1141: (long)(int)9223372036854775807LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)9223372036854775807LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)9223372036854775807LL; static unsigned long r3 = (unsigned long)(int)9223372036854775807LL;
      printf("1142: (unsigned long)(int)9223372036854775807LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)9223372036854775807LL; long long r1 = (long long)v; long long r2 = (long long)(int)9223372036854775807LL; static long long r3 = (long long)(int)9223372036854775807LL;
      printf("1143: (long long)(int)9223372036854775807LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)9223372036854775807LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)9223372036854775807LL; static unsigned long long r3 = (unsigned long long)(int)9223372036854775807LL;
      printf("1144: (unsigned long long)(int)9223372036854775807LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)9223372036854775807LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)9223372036854775807LL; static _Bool r3 = (_Bool)(int)9223372036854775807LL;
      printf("1145: (_Bool)(int)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)9223372036854775807LL; float r1 = (float)v; float r2 = (float)(int)9223372036854775807LL; static float r3 = (float)(int)9223372036854775807LL;
      printf("1146: (float)(int)9223372036854775807LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)9223372036854775807LL; double r1 = (double)v; double r2 = (double)(int)9223372036854775807LL; static double r3 = (double)(int)9223372036854775807LL;
      printf("1147: (double)(int)9223372036854775807LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)9223372036854775807LL; long double r1 = (long double)v; long double r2 = (long double)(int)9223372036854775807LL; static long double r3 = (long double)(int)9223372036854775807LL;
      printf("1148: (long double)(int)9223372036854775807LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(int)(-9223372036854775807LL-1); static signed char r3 = (signed char)(int)(-9223372036854775807LL-1);
      printf("1149: (signed char)(int)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)(-9223372036854775807LL-1); static unsigned char r3 = (unsigned char)(int)(-9223372036854775807LL-1);
      printf("1150: (unsigned char)(int)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); short r1 = (short)v; short r2 = (short)(int)(-9223372036854775807LL-1); static short r3 = (short)(int)(-9223372036854775807LL-1);
      printf("1151: (short)(int)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)(-9223372036854775807LL-1); static unsigned short r3 = (unsigned short)(int)(-9223372036854775807LL-1);
      printf("1152: (unsigned short)(int)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); int r1 = (int)v; int r2 = (int)(int)(-9223372036854775807LL-1); static int r3 = (int)(int)(-9223372036854775807LL-1);
      printf("1153: (int)(int)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)(-9223372036854775807LL-1); static unsigned r3 = (unsigned)(int)(-9223372036854775807LL-1);
      printf("1154: (unsigned)(int)(-9223372036854775807LL-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); long r1 = (long)v; long r2 = (long)(int)(-9223372036854775807LL-1); static long r3 = (long)(int)(-9223372036854775807LL-1);
      printf("1155: (long)(int)(-9223372036854775807LL-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)(-9223372036854775807LL-1); static unsigned long r3 = (unsigned long)(int)(-9223372036854775807LL-1);
      printf("1156: (unsigned long)(int)(-9223372036854775807LL-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); long long r1 = (long long)v; long long r2 = (long long)(int)(-9223372036854775807LL-1); static long long r3 = (long long)(int)(-9223372036854775807LL-1);
      printf("1157: (long long)(int)(-9223372036854775807LL-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)(-9223372036854775807LL-1); static unsigned long long r3 = (unsigned long long)(int)(-9223372036854775807LL-1);
      printf("1158: (unsigned long long)(int)(-9223372036854775807LL-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)(-9223372036854775807LL-1); static _Bool r3 = (_Bool)(int)(-9223372036854775807LL-1);
      printf("1159: (_Bool)(int)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); float r1 = (float)v; float r2 = (float)(int)(-9223372036854775807LL-1); static float r3 = (float)(int)(-9223372036854775807LL-1);
      printf("1160: (float)(int)(-9223372036854775807LL-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); double r1 = (double)v; double r2 = (double)(int)(-9223372036854775807LL-1); static double r3 = (double)(int)(-9223372036854775807LL-1);
      printf("1161: (double)(int)(-9223372036854775807LL-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)(-9223372036854775807LL-1); long double r1 = (long double)v; long double r2 = (long double)(int)(-9223372036854775807LL-1); static long double r3 = (long double)(int)(-9223372036854775807LL-1);
      printf("1162: (long double)(int)(-9223372036854775807LL-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)0x8000000000000000ULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)0x8000000000000000ULL; static signed char r3 = (signed char)(int)0x8000000000000000ULL;
      printf("1163: (signed char)(int)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0x8000000000000000ULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)0x8000000000000000ULL; static unsigned char r3 = (unsigned char)(int)0x8000000000000000ULL;
      printf("1164: (unsigned char)(int)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0x8000000000000000ULL; short r1 = (short)v; short r2 = (short)(int)0x8000000000000000ULL; static short r3 = (short)(int)0x8000000000000000ULL;
      printf("1165: (short)(int)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0x8000000000000000ULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)0x8000000000000000ULL; static unsigned short r3 = (unsigned short)(int)0x8000000000000000ULL;
      printf("1166: (unsigned short)(int)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0x8000000000000000ULL; int r1 = (int)v; int r2 = (int)(int)0x8000000000000000ULL; static int r3 = (int)(int)0x8000000000000000ULL;
      printf("1167: (int)(int)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0x8000000000000000ULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)0x8000000000000000ULL; static unsigned r3 = (unsigned)(int)0x8000000000000000ULL;
      printf("1168: (unsigned)(int)0x8000000000000000ULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)0x8000000000000000ULL; long r1 = (long)v; long r2 = (long)(int)0x8000000000000000ULL; static long r3 = (long)(int)0x8000000000000000ULL;
      printf("1169: (long)(int)0x8000000000000000ULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)0x8000000000000000ULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)0x8000000000000000ULL; static unsigned long r3 = (unsigned long)(int)0x8000000000000000ULL;
      printf("1170: (unsigned long)(int)0x8000000000000000ULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)0x8000000000000000ULL; long long r1 = (long long)v; long long r2 = (long long)(int)0x8000000000000000ULL; static long long r3 = (long long)(int)0x8000000000000000ULL;
      printf("1171: (long long)(int)0x8000000000000000ULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)0x8000000000000000ULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)0x8000000000000000ULL; static unsigned long long r3 = (unsigned long long)(int)0x8000000000000000ULL;
      printf("1172: (unsigned long long)(int)0x8000000000000000ULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)0x8000000000000000ULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)0x8000000000000000ULL; static _Bool r3 = (_Bool)(int)0x8000000000000000ULL;
      printf("1173: (_Bool)(int)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0x8000000000000000ULL; float r1 = (float)v; float r2 = (float)(int)0x8000000000000000ULL; static float r3 = (float)(int)0x8000000000000000ULL;
      printf("1174: (float)(int)0x8000000000000000ULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)0x8000000000000000ULL; double r1 = (double)v; double r2 = (double)(int)0x8000000000000000ULL; static double r3 = (double)(int)0x8000000000000000ULL;
      printf("1175: (double)(int)0x8000000000000000ULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)0x8000000000000000ULL; long double r1 = (long double)v; long double r2 = (long double)(int)0x8000000000000000ULL; static long double r3 = (long double)(int)0x8000000000000000ULL;
      printf("1176: (long double)(int)0x8000000000000000ULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(int)0xffffffffffffffffULL; static signed char r3 = (signed char)(int)0xffffffffffffffffULL;
      printf("1177: (signed char)(int)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(int)0xffffffffffffffffULL; static unsigned char r3 = (unsigned char)(int)0xffffffffffffffffULL;
      printf("1178: (unsigned char)(int)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; short r1 = (short)v; short r2 = (short)(int)0xffffffffffffffffULL; static short r3 = (short)(int)0xffffffffffffffffULL;
      printf("1179: (short)(int)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(int)0xffffffffffffffffULL; static unsigned short r3 = (unsigned short)(int)0xffffffffffffffffULL;
      printf("1180: (unsigned short)(int)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; int r1 = (int)v; int r2 = (int)(int)0xffffffffffffffffULL; static int r3 = (int)(int)0xffffffffffffffffULL;
      printf("1181: (int)(int)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(int)0xffffffffffffffffULL; static unsigned r3 = (unsigned)(int)0xffffffffffffffffULL;
      printf("1182: (unsigned)(int)0xffffffffffffffffULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; long r1 = (long)v; long r2 = (long)(int)0xffffffffffffffffULL; static long r3 = (long)(int)0xffffffffffffffffULL;
      printf("1183: (long)(int)0xffffffffffffffffULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(int)0xffffffffffffffffULL; static unsigned long r3 = (unsigned long)(int)0xffffffffffffffffULL;
      printf("1184: (unsigned long)(int)0xffffffffffffffffULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; long long r1 = (long long)v; long long r2 = (long long)(int)0xffffffffffffffffULL; static long long r3 = (long long)(int)0xffffffffffffffffULL;
      printf("1185: (long long)(int)0xffffffffffffffffULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(int)0xffffffffffffffffULL; static unsigned long long r3 = (unsigned long long)(int)0xffffffffffffffffULL;
      printf("1186: (unsigned long long)(int)0xffffffffffffffffULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(int)0xffffffffffffffffULL; static _Bool r3 = (_Bool)(int)0xffffffffffffffffULL;
      printf("1187: (_Bool)(int)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; float r1 = (float)v; float r2 = (float)(int)0xffffffffffffffffULL; static float r3 = (float)(int)0xffffffffffffffffULL;
      printf("1188: (float)(int)0xffffffffffffffffULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; double r1 = (double)v; double r2 = (double)(int)0xffffffffffffffffULL; static double r3 = (double)(int)0xffffffffffffffffULL;
      printf("1189: (double)(int)0xffffffffffffffffULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int v = (int)0xffffffffffffffffULL; long double r1 = (long double)v; long double r2 = (long double)(int)0xffffffffffffffffULL; static long double r3 = (long double)(int)0xffffffffffffffffULL;
      printf("1190: (long double)(int)0xffffffffffffffffULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)0; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)0; static signed char r3 = (signed char)(unsigned)0;
      printf("1191: (signed char)(unsigned)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)0; static unsigned char r3 = (unsigned char)(unsigned)0;
      printf("1192: (unsigned char)(unsigned)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0; short r1 = (short)v; short r2 = (short)(unsigned)0; static short r3 = (short)(unsigned)0;
      printf("1193: (short)(unsigned)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)0; static unsigned short r3 = (unsigned short)(unsigned)0;
      printf("1194: (unsigned short)(unsigned)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0; int r1 = (int)v; int r2 = (int)(unsigned)0; static int r3 = (int)(unsigned)0;
      printf("1195: (int)(unsigned)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)0; static unsigned r3 = (unsigned)(unsigned)0;
      printf("1196: (unsigned)(unsigned)0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)0; long r1 = (long)v; long r2 = (long)(unsigned)0; static long r3 = (long)(unsigned)0;
      printf("1197: (long)(unsigned)0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)0; static unsigned long r3 = (unsigned long)(unsigned)0;
      printf("1198: (unsigned long)(unsigned)0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)0; long long r1 = (long long)v; long long r2 = (long long)(unsigned)0; static long long r3 = (long long)(unsigned)0;
      printf("1199: (long long)(unsigned)0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)0; static unsigned long long r3 = (unsigned long long)(unsigned)0;
      printf("1200: (unsigned long long)(unsigned)0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)0; static _Bool r3 = (_Bool)(unsigned)0;
      printf("1201: (_Bool)(unsigned)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0; float r1 = (float)v; float r2 = (float)(unsigned)0; static float r3 = (float)(unsigned)0;
      printf("1202: (float)(unsigned)0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)0; double r1 = (double)v; double r2 = (double)(unsigned)0; static double r3 = (double)(unsigned)0;
      printf("1203: (double)(unsigned)0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)0; long double r1 = (long double)v; long double r2 = (long double)(unsigned)0; static long double r3 = (long double)(unsigned)0;
      printf("1204: (long double)(unsigned)0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)1; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)1; static signed char r3 = (signed char)(unsigned)1;
      printf("1205: (signed char)(unsigned)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)1; static unsigned char r3 = (unsigned char)(unsigned)1;
      printf("1206: (unsigned char)(unsigned)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)1; short r1 = (short)v; short r2 = (short)(unsigned)1; static short r3 = (short)(unsigned)1;
      printf("1207: (short)(unsigned)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)1; static unsigned short r3 = (unsigned short)(unsigned)1;
      printf("1208: (unsigned short)(unsigned)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)1; int r1 = (int)v; int r2 = (int)(unsigned)1; static int r3 = (int)(unsigned)1;
      printf("1209: (int)(unsigned)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)1; static unsigned r3 = (unsigned)(unsigned)1;
      printf("1210: (unsigned)(unsigned)1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)1; long r1 = (long)v; long r2 = (long)(unsigned)1; static long r3 = (long)(unsigned)1;
      printf("1211: (long)(unsigned)1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)1; static unsigned long r3 = (unsigned long)(unsigned)1;
      printf("1212: (unsigned long)(unsigned)1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)1; long long r1 = (long long)v; long long r2 = (long long)(unsigned)1; static long long r3 = (long long)(unsigned)1;
      printf("1213: (long long)(unsigned)1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)1; static unsigned long long r3 = (unsigned long long)(unsigned)1;
      printf("1214: (unsigned long long)(unsigned)1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)1; static _Bool r3 = (_Bool)(unsigned)1;
      printf("1215: (_Bool)(unsigned)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)1; float r1 = (float)v; float r2 = (float)(unsigned)1; static float r3 = (float)(unsigned)1;
      printf("1216: (float)(unsigned)1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)1; double r1 = (double)v; double r2 = (double)(unsigned)1; static double r3 = (double)(unsigned)1;
      printf("1217: (double)(unsigned)1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)1; long double r1 = (long double)v; long double r2 = (long double)(unsigned)1; static long double r3 = (long double)(unsigned)1;
      printf("1218: (long double)(unsigned)1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)-1; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)-1; static signed char r3 = (signed char)(unsigned)-1;
      printf("1219: (signed char)(unsigned)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)-1; static unsigned char r3 = (unsigned char)(unsigned)-1;
      printf("1220: (unsigned char)(unsigned)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-1; short r1 = (short)v; short r2 = (short)(unsigned)-1; static short r3 = (short)(unsigned)-1;
      printf("1221: (short)(unsigned)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)-1; static unsigned short r3 = (unsigned short)(unsigned)-1;
      printf("1222: (unsigned short)(unsigned)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-1; int r1 = (int)v; int r2 = (int)(unsigned)-1; static int r3 = (int)(unsigned)-1;
      printf("1223: (int)(unsigned)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)-1; static unsigned r3 = (unsigned)(unsigned)-1;
      printf("1224: (unsigned)(unsigned)-1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)-1; long r1 = (long)v; long r2 = (long)(unsigned)-1; static long r3 = (long)(unsigned)-1;
      printf("1225: (long)(unsigned)-1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)-1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)-1; static unsigned long r3 = (unsigned long)(unsigned)-1;
      printf("1226: (unsigned long)(unsigned)-1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)-1; long long r1 = (long long)v; long long r2 = (long long)(unsigned)-1; static long long r3 = (long long)(unsigned)-1;
      printf("1227: (long long)(unsigned)-1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)-1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)-1; static unsigned long long r3 = (unsigned long long)(unsigned)-1;
      printf("1228: (unsigned long long)(unsigned)-1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)-1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)-1; static _Bool r3 = (_Bool)(unsigned)-1;
      printf("1229: (_Bool)(unsigned)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-1; float r1 = (float)v; float r2 = (float)(unsigned)-1; static float r3 = (float)(unsigned)-1;
      printf("1230: (float)(unsigned)-1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)-1; double r1 = (double)v; double r2 = (double)(unsigned)-1; static double r3 = (double)(unsigned)-1;
      printf("1231: (double)(unsigned)-1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)-1; long double r1 = (long double)v; long double r2 = (long double)(unsigned)-1; static long double r3 = (long double)(unsigned)-1;
      printf("1232: (long double)(unsigned)-1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)127; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)127; static signed char r3 = (signed char)(unsigned)127;
      printf("1233: (signed char)(unsigned)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)127; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)127; static unsigned char r3 = (unsigned char)(unsigned)127;
      printf("1234: (unsigned char)(unsigned)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)127; short r1 = (short)v; short r2 = (short)(unsigned)127; static short r3 = (short)(unsigned)127;
      printf("1235: (short)(unsigned)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)127; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)127; static unsigned short r3 = (unsigned short)(unsigned)127;
      printf("1236: (unsigned short)(unsigned)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)127; int r1 = (int)v; int r2 = (int)(unsigned)127; static int r3 = (int)(unsigned)127;
      printf("1237: (int)(unsigned)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)127; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)127; static unsigned r3 = (unsigned)(unsigned)127;
      printf("1238: (unsigned)(unsigned)127 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)127; long r1 = (long)v; long r2 = (long)(unsigned)127; static long r3 = (long)(unsigned)127;
      printf("1239: (long)(unsigned)127 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)127; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)127; static unsigned long r3 = (unsigned long)(unsigned)127;
      printf("1240: (unsigned long)(unsigned)127 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)127; long long r1 = (long long)v; long long r2 = (long long)(unsigned)127; static long long r3 = (long long)(unsigned)127;
      printf("1241: (long long)(unsigned)127 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)127; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)127; static unsigned long long r3 = (unsigned long long)(unsigned)127;
      printf("1242: (unsigned long long)(unsigned)127 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)127; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)127; static _Bool r3 = (_Bool)(unsigned)127;
      printf("1243: (_Bool)(unsigned)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)127; float r1 = (float)v; float r2 = (float)(unsigned)127; static float r3 = (float)(unsigned)127;
      printf("1244: (float)(unsigned)127 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)127; double r1 = (double)v; double r2 = (double)(unsigned)127; static double r3 = (double)(unsigned)127;
      printf("1245: (double)(unsigned)127 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)127; long double r1 = (long double)v; long double r2 = (long double)(unsigned)127; static long double r3 = (long double)(unsigned)127;
      printf("1246: (long double)(unsigned)127 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)-128; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)-128; static signed char r3 = (signed char)(unsigned)-128;
      printf("1247: (signed char)(unsigned)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-128; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)-128; static unsigned char r3 = (unsigned char)(unsigned)-128;
      printf("1248: (unsigned char)(unsigned)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-128; short r1 = (short)v; short r2 = (short)(unsigned)-128; static short r3 = (short)(unsigned)-128;
      printf("1249: (short)(unsigned)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-128; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)-128; static unsigned short r3 = (unsigned short)(unsigned)-128;
      printf("1250: (unsigned short)(unsigned)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-128; int r1 = (int)v; int r2 = (int)(unsigned)-128; static int r3 = (int)(unsigned)-128;
      printf("1251: (int)(unsigned)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-128; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)-128; static unsigned r3 = (unsigned)(unsigned)-128;
      printf("1252: (unsigned)(unsigned)-128 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)-128; long r1 = (long)v; long r2 = (long)(unsigned)-128; static long r3 = (long)(unsigned)-128;
      printf("1253: (long)(unsigned)-128 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)-128; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)-128; static unsigned long r3 = (unsigned long)(unsigned)-128;
      printf("1254: (unsigned long)(unsigned)-128 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)-128; long long r1 = (long long)v; long long r2 = (long long)(unsigned)-128; static long long r3 = (long long)(unsigned)-128;
      printf("1255: (long long)(unsigned)-128 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)-128; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)-128; static unsigned long long r3 = (unsigned long long)(unsigned)-128;
      printf("1256: (unsigned long long)(unsigned)-128 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)-128; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)-128; static _Bool r3 = (_Bool)(unsigned)-128;
      printf("1257: (_Bool)(unsigned)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-128; float r1 = (float)v; float r2 = (float)(unsigned)-128; static float r3 = (float)(unsigned)-128;
      printf("1258: (float)(unsigned)-128 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)-128; double r1 = (double)v; double r2 = (double)(unsigned)-128; static double r3 = (double)(unsigned)-128;
      printf("1259: (double)(unsigned)-128 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)-128; long double r1 = (long double)v; long double r2 = (long double)(unsigned)-128; static long double r3 = (long double)(unsigned)-128;
      printf("1260: (long double)(unsigned)-128 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)255; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)255; static signed char r3 = (signed char)(unsigned)255;
      printf("1261: (signed char)(unsigned)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)255; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)255; static unsigned char r3 = (unsigned char)(unsigned)255;
      printf("1262: (unsigned char)(unsigned)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)255; short r1 = (short)v; short r2 = (short)(unsigned)255; static short r3 = (short)(unsigned)255;
      printf("1263: (short)(unsigned)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)255; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)255; static unsigned short r3 = (unsigned short)(unsigned)255;
      printf("1264: (unsigned short)(unsigned)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)255; int r1 = (int)v; int r2 = (int)(unsigned)255; static int r3 = (int)(unsigned)255;
      printf("1265: (int)(unsigned)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)255; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)255; static unsigned r3 = (unsigned)(unsigned)255;
      printf("1266: (unsigned)(unsigned)255 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)255; long r1 = (long)v; long r2 = (long)(unsigned)255; static long r3 = (long)(unsigned)255;
      printf("1267: (long)(unsigned)255 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)255; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)255; static unsigned long r3 = (unsigned long)(unsigned)255;
      printf("1268: (unsigned long)(unsigned)255 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)255; long long r1 = (long long)v; long long r2 = (long long)(unsigned)255; static long long r3 = (long long)(unsigned)255;
      printf("1269: (long long)(unsigned)255 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)255; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)255; static unsigned long long r3 = (unsigned long long)(unsigned)255;
      printf("1270: (unsigned long long)(unsigned)255 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)255; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)255; static _Bool r3 = (_Bool)(unsigned)255;
      printf("1271: (_Bool)(unsigned)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)255; float r1 = (float)v; float r2 = (float)(unsigned)255; static float r3 = (float)(unsigned)255;
      printf("1272: (float)(unsigned)255 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)255; double r1 = (double)v; double r2 = (double)(unsigned)255; static double r3 = (double)(unsigned)255;
      printf("1273: (double)(unsigned)255 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)255; long double r1 = (long double)v; long double r2 = (long double)(unsigned)255; static long double r3 = (long double)(unsigned)255;
      printf("1274: (long double)(unsigned)255 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)256; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)256; static signed char r3 = (signed char)(unsigned)256;
      printf("1275: (signed char)(unsigned)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)256; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)256; static unsigned char r3 = (unsigned char)(unsigned)256;
      printf("1276: (unsigned char)(unsigned)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)256; short r1 = (short)v; short r2 = (short)(unsigned)256; static short r3 = (short)(unsigned)256;
      printf("1277: (short)(unsigned)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)256; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)256; static unsigned short r3 = (unsigned short)(unsigned)256;
      printf("1278: (unsigned short)(unsigned)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)256; int r1 = (int)v; int r2 = (int)(unsigned)256; static int r3 = (int)(unsigned)256;
      printf("1279: (int)(unsigned)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)256; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)256; static unsigned r3 = (unsigned)(unsigned)256;
      printf("1280: (unsigned)(unsigned)256 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)256; long r1 = (long)v; long r2 = (long)(unsigned)256; static long r3 = (long)(unsigned)256;
      printf("1281: (long)(unsigned)256 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)256; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)256; static unsigned long r3 = (unsigned long)(unsigned)256;
      printf("1282: (unsigned long)(unsigned)256 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)256; long long r1 = (long long)v; long long r2 = (long long)(unsigned)256; static long long r3 = (long long)(unsigned)256;
      printf("1283: (long long)(unsigned)256 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)256; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)256; static unsigned long long r3 = (unsigned long long)(unsigned)256;
      printf("1284: (unsigned long long)(unsigned)256 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)256; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)256; static _Bool r3 = (_Bool)(unsigned)256;
      printf("1285: (_Bool)(unsigned)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)256; float r1 = (float)v; float r2 = (float)(unsigned)256; static float r3 = (float)(unsigned)256;
      printf("1286: (float)(unsigned)256 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)256; double r1 = (double)v; double r2 = (double)(unsigned)256; static double r3 = (double)(unsigned)256;
      printf("1287: (double)(unsigned)256 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)256; long double r1 = (long double)v; long double r2 = (long double)(unsigned)256; static long double r3 = (long double)(unsigned)256;
      printf("1288: (long double)(unsigned)256 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)32767; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)32767; static signed char r3 = (signed char)(unsigned)32767;
      printf("1289: (signed char)(unsigned)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)32767; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)32767; static unsigned char r3 = (unsigned char)(unsigned)32767;
      printf("1290: (unsigned char)(unsigned)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)32767; short r1 = (short)v; short r2 = (short)(unsigned)32767; static short r3 = (short)(unsigned)32767;
      printf("1291: (short)(unsigned)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)32767; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)32767; static unsigned short r3 = (unsigned short)(unsigned)32767;
      printf("1292: (unsigned short)(unsigned)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)32767; int r1 = (int)v; int r2 = (int)(unsigned)32767; static int r3 = (int)(unsigned)32767;
      printf("1293: (int)(unsigned)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)32767; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)32767; static unsigned r3 = (unsigned)(unsigned)32767;
      printf("1294: (unsigned)(unsigned)32767 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)32767; long r1 = (long)v; long r2 = (long)(unsigned)32767; static long r3 = (long)(unsigned)32767;
      printf("1295: (long)(unsigned)32767 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)32767; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)32767; static unsigned long r3 = (unsigned long)(unsigned)32767;
      printf("1296: (unsigned long)(unsigned)32767 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)32767; long long r1 = (long long)v; long long r2 = (long long)(unsigned)32767; static long long r3 = (long long)(unsigned)32767;
      printf("1297: (long long)(unsigned)32767 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)32767; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)32767; static unsigned long long r3 = (unsigned long long)(unsigned)32767;
      printf("1298: (unsigned long long)(unsigned)32767 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)32767; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)32767; static _Bool r3 = (_Bool)(unsigned)32767;
      printf("1299: (_Bool)(unsigned)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)32767; float r1 = (float)v; float r2 = (float)(unsigned)32767; static float r3 = (float)(unsigned)32767;
      printf("1300: (float)(unsigned)32767 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)32767; double r1 = (double)v; double r2 = (double)(unsigned)32767; static double r3 = (double)(unsigned)32767;
      printf("1301: (double)(unsigned)32767 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)32767; long double r1 = (long double)v; long double r2 = (long double)(unsigned)32767; static long double r3 = (long double)(unsigned)32767;
      printf("1302: (long double)(unsigned)32767 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)-32768; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)-32768; static signed char r3 = (signed char)(unsigned)-32768;
      printf("1303: (signed char)(unsigned)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-32768; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)-32768; static unsigned char r3 = (unsigned char)(unsigned)-32768;
      printf("1304: (unsigned char)(unsigned)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-32768; short r1 = (short)v; short r2 = (short)(unsigned)-32768; static short r3 = (short)(unsigned)-32768;
      printf("1305: (short)(unsigned)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-32768; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)-32768; static unsigned short r3 = (unsigned short)(unsigned)-32768;
      printf("1306: (unsigned short)(unsigned)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-32768; int r1 = (int)v; int r2 = (int)(unsigned)-32768; static int r3 = (int)(unsigned)-32768;
      printf("1307: (int)(unsigned)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-32768; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)-32768; static unsigned r3 = (unsigned)(unsigned)-32768;
      printf("1308: (unsigned)(unsigned)-32768 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)-32768; long r1 = (long)v; long r2 = (long)(unsigned)-32768; static long r3 = (long)(unsigned)-32768;
      printf("1309: (long)(unsigned)-32768 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)-32768; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)-32768; static unsigned long r3 = (unsigned long)(unsigned)-32768;
      printf("1310: (unsigned long)(unsigned)-32768 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)-32768; long long r1 = (long long)v; long long r2 = (long long)(unsigned)-32768; static long long r3 = (long long)(unsigned)-32768;
      printf("1311: (long long)(unsigned)-32768 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)-32768; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)-32768; static unsigned long long r3 = (unsigned long long)(unsigned)-32768;
      printf("1312: (unsigned long long)(unsigned)-32768 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)-32768; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)-32768; static _Bool r3 = (_Bool)(unsigned)-32768;
      printf("1313: (_Bool)(unsigned)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)-32768; float r1 = (float)v; float r2 = (float)(unsigned)-32768; static float r3 = (float)(unsigned)-32768;
      printf("1314: (float)(unsigned)-32768 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)-32768; double r1 = (double)v; double r2 = (double)(unsigned)-32768; static double r3 = (double)(unsigned)-32768;
      printf("1315: (double)(unsigned)-32768 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)-32768; long double r1 = (long double)v; long double r2 = (long double)(unsigned)-32768; static long double r3 = (long double)(unsigned)-32768;
      printf("1316: (long double)(unsigned)-32768 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)65535; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)65535; static signed char r3 = (signed char)(unsigned)65535;
      printf("1317: (signed char)(unsigned)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)65535; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)65535; static unsigned char r3 = (unsigned char)(unsigned)65535;
      printf("1318: (unsigned char)(unsigned)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)65535; short r1 = (short)v; short r2 = (short)(unsigned)65535; static short r3 = (short)(unsigned)65535;
      printf("1319: (short)(unsigned)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)65535; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)65535; static unsigned short r3 = (unsigned short)(unsigned)65535;
      printf("1320: (unsigned short)(unsigned)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)65535; int r1 = (int)v; int r2 = (int)(unsigned)65535; static int r3 = (int)(unsigned)65535;
      printf("1321: (int)(unsigned)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)65535; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)65535; static unsigned r3 = (unsigned)(unsigned)65535;
      printf("1322: (unsigned)(unsigned)65535 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)65535; long r1 = (long)v; long r2 = (long)(unsigned)65535; static long r3 = (long)(unsigned)65535;
      printf("1323: (long)(unsigned)65535 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)65535; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)65535; static unsigned long r3 = (unsigned long)(unsigned)65535;
      printf("1324: (unsigned long)(unsigned)65535 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)65535; long long r1 = (long long)v; long long r2 = (long long)(unsigned)65535; static long long r3 = (long long)(unsigned)65535;
      printf("1325: (long long)(unsigned)65535 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)65535; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)65535; static unsigned long long r3 = (unsigned long long)(unsigned)65535;
      printf("1326: (unsigned long long)(unsigned)65535 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)65535; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)65535; static _Bool r3 = (_Bool)(unsigned)65535;
      printf("1327: (_Bool)(unsigned)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)65535; float r1 = (float)v; float r2 = (float)(unsigned)65535; static float r3 = (float)(unsigned)65535;
      printf("1328: (float)(unsigned)65535 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)65535; double r1 = (double)v; double r2 = (double)(unsigned)65535; static double r3 = (double)(unsigned)65535;
      printf("1329: (double)(unsigned)65535 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)65535; long double r1 = (long double)v; long double r2 = (long double)(unsigned)65535; static long double r3 = (long double)(unsigned)65535;
      printf("1330: (long double)(unsigned)65535 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)2147483647; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)2147483647; static signed char r3 = (signed char)(unsigned)2147483647;
      printf("1331: (signed char)(unsigned)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)2147483647; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)2147483647; static unsigned char r3 = (unsigned char)(unsigned)2147483647;
      printf("1332: (unsigned char)(unsigned)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)2147483647; short r1 = (short)v; short r2 = (short)(unsigned)2147483647; static short r3 = (short)(unsigned)2147483647;
      printf("1333: (short)(unsigned)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)2147483647; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)2147483647; static unsigned short r3 = (unsigned short)(unsigned)2147483647;
      printf("1334: (unsigned short)(unsigned)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)2147483647; int r1 = (int)v; int r2 = (int)(unsigned)2147483647; static int r3 = (int)(unsigned)2147483647;
      printf("1335: (int)(unsigned)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)2147483647; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)2147483647; static unsigned r3 = (unsigned)(unsigned)2147483647;
      printf("1336: (unsigned)(unsigned)2147483647 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)2147483647; long r1 = (long)v; long r2 = (long)(unsigned)2147483647; static long r3 = (long)(unsigned)2147483647;
      printf("1337: (long)(unsigned)2147483647 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)2147483647; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)2147483647; static unsigned long r3 = (unsigned long)(unsigned)2147483647;
      printf("1338: (unsigned long)(unsigned)2147483647 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)2147483647; long long r1 = (long long)v; long long r2 = (long long)(unsigned)2147483647; static long long r3 = (long long)(unsigned)2147483647;
      printf("1339: (long long)(unsigned)2147483647 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)2147483647; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)2147483647; static unsigned long long r3 = (unsigned long long)(unsigned)2147483647;
      printf("1340: (unsigned long long)(unsigned)2147483647 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)2147483647; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)2147483647; static _Bool r3 = (_Bool)(unsigned)2147483647;
      printf("1341: (_Bool)(unsigned)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)2147483647; float r1 = (float)v; float r2 = (float)(unsigned)2147483647; static float r3 = (float)(unsigned)2147483647;
      printf("1342: (float)(unsigned)2147483647 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)2147483647; double r1 = (double)v; double r2 = (double)(unsigned)2147483647; static double r3 = (double)(unsigned)2147483647;
      printf("1343: (double)(unsigned)2147483647 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)2147483647; long double r1 = (long double)v; long double r2 = (long double)(unsigned)2147483647; static long double r3 = (long double)(unsigned)2147483647;
      printf("1344: (long double)(unsigned)2147483647 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)(-2147483647-1); static signed char r3 = (signed char)(unsigned)(-2147483647-1);
      printf("1345: (signed char)(unsigned)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)(-2147483647-1); static unsigned char r3 = (unsigned char)(unsigned)(-2147483647-1);
      printf("1346: (unsigned char)(unsigned)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); short r1 = (short)v; short r2 = (short)(unsigned)(-2147483647-1); static short r3 = (short)(unsigned)(-2147483647-1);
      printf("1347: (short)(unsigned)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)(-2147483647-1); static unsigned short r3 = (unsigned short)(unsigned)(-2147483647-1);
      printf("1348: (unsigned short)(unsigned)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); int r1 = (int)v; int r2 = (int)(unsigned)(-2147483647-1); static int r3 = (int)(unsigned)(-2147483647-1);
      printf("1349: (int)(unsigned)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)(-2147483647-1); static unsigned r3 = (unsigned)(unsigned)(-2147483647-1);
      printf("1350: (unsigned)(unsigned)(-2147483647-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); long r1 = (long)v; long r2 = (long)(unsigned)(-2147483647-1); static long r3 = (long)(unsigned)(-2147483647-1);
      printf("1351: (long)(unsigned)(-2147483647-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)(-2147483647-1); static unsigned long r3 = (unsigned long)(unsigned)(-2147483647-1);
      printf("1352: (unsigned long)(unsigned)(-2147483647-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); long long r1 = (long long)v; long long r2 = (long long)(unsigned)(-2147483647-1); static long long r3 = (long long)(unsigned)(-2147483647-1);
      printf("1353: (long long)(unsigned)(-2147483647-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)(-2147483647-1); static unsigned long long r3 = (unsigned long long)(unsigned)(-2147483647-1);
      printf("1354: (unsigned long long)(unsigned)(-2147483647-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)(-2147483647-1); static _Bool r3 = (_Bool)(unsigned)(-2147483647-1);
      printf("1355: (_Bool)(unsigned)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); float r1 = (float)v; float r2 = (float)(unsigned)(-2147483647-1); static float r3 = (float)(unsigned)(-2147483647-1);
      printf("1356: (float)(unsigned)(-2147483647-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); double r1 = (double)v; double r2 = (double)(unsigned)(-2147483647-1); static double r3 = (double)(unsigned)(-2147483647-1);
      printf("1357: (double)(unsigned)(-2147483647-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)(-2147483647-1); long double r1 = (long double)v; long double r2 = (long double)(unsigned)(-2147483647-1); static long double r3 = (long double)(unsigned)(-2147483647-1);
      printf("1358: (long double)(unsigned)(-2147483647-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)4294967295LL; static signed char r3 = (signed char)(unsigned)4294967295LL;
      printf("1359: (signed char)(unsigned)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)4294967295LL; static unsigned char r3 = (unsigned char)(unsigned)4294967295LL;
      printf("1360: (unsigned char)(unsigned)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; short r1 = (short)v; short r2 = (short)(unsigned)4294967295LL; static short r3 = (short)(unsigned)4294967295LL;
      printf("1361: (short)(unsigned)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)4294967295LL; static unsigned short r3 = (unsigned short)(unsigned)4294967295LL;
      printf("1362: (unsigned short)(unsigned)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; int r1 = (int)v; int r2 = (int)(unsigned)4294967295LL; static int r3 = (int)(unsigned)4294967295LL;
      printf("1363: (int)(unsigned)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)4294967295LL; static unsigned r3 = (unsigned)(unsigned)4294967295LL;
      printf("1364: (unsigned)(unsigned)4294967295LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; long r1 = (long)v; long r2 = (long)(unsigned)4294967295LL; static long r3 = (long)(unsigned)4294967295LL;
      printf("1365: (long)(unsigned)4294967295LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)4294967295LL; static unsigned long r3 = (unsigned long)(unsigned)4294967295LL;
      printf("1366: (unsigned long)(unsigned)4294967295LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; long long r1 = (long long)v; long long r2 = (long long)(unsigned)4294967295LL; static long long r3 = (long long)(unsigned)4294967295LL;
      printf("1367: (long long)(unsigned)4294967295LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)4294967295LL; static unsigned long long r3 = (unsigned long long)(unsigned)4294967295LL;
      printf("1368: (unsigned long long)(unsigned)4294967295LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)4294967295LL; static _Bool r3 = (_Bool)(unsigned)4294967295LL;
      printf("1369: (_Bool)(unsigned)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; float r1 = (float)v; float r2 = (float)(unsigned)4294967295LL; static float r3 = (float)(unsigned)4294967295LL;
      printf("1370: (float)(unsigned)4294967295LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; double r1 = (double)v; double r2 = (double)(unsigned)4294967295LL; static double r3 = (double)(unsigned)4294967295LL;
      printf("1371: (double)(unsigned)4294967295LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)4294967295LL; long double r1 = (long double)v; long double r2 = (long double)(unsigned)4294967295LL; static long double r3 = (long double)(unsigned)4294967295LL;
      printf("1372: (long double)(unsigned)4294967295LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)9223372036854775807LL; static signed char r3 = (signed char)(unsigned)9223372036854775807LL;
      printf("1373: (signed char)(unsigned)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)9223372036854775807LL; static unsigned char r3 = (unsigned char)(unsigned)9223372036854775807LL;
      printf("1374: (unsigned char)(unsigned)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; short r1 = (short)v; short r2 = (short)(unsigned)9223372036854775807LL; static short r3 = (short)(unsigned)9223372036854775807LL;
      printf("1375: (short)(unsigned)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)9223372036854775807LL; static unsigned short r3 = (unsigned short)(unsigned)9223372036854775807LL;
      printf("1376: (unsigned short)(unsigned)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; int r1 = (int)v; int r2 = (int)(unsigned)9223372036854775807LL; static int r3 = (int)(unsigned)9223372036854775807LL;
      printf("1377: (int)(unsigned)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)9223372036854775807LL; static unsigned r3 = (unsigned)(unsigned)9223372036854775807LL;
      printf("1378: (unsigned)(unsigned)9223372036854775807LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; long r1 = (long)v; long r2 = (long)(unsigned)9223372036854775807LL; static long r3 = (long)(unsigned)9223372036854775807LL;
      printf("1379: (long)(unsigned)9223372036854775807LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)9223372036854775807LL; static unsigned long r3 = (unsigned long)(unsigned)9223372036854775807LL;
      printf("1380: (unsigned long)(unsigned)9223372036854775807LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; long long r1 = (long long)v; long long r2 = (long long)(unsigned)9223372036854775807LL; static long long r3 = (long long)(unsigned)9223372036854775807LL;
      printf("1381: (long long)(unsigned)9223372036854775807LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)9223372036854775807LL; static unsigned long long r3 = (unsigned long long)(unsigned)9223372036854775807LL;
      printf("1382: (unsigned long long)(unsigned)9223372036854775807LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)9223372036854775807LL; static _Bool r3 = (_Bool)(unsigned)9223372036854775807LL;
      printf("1383: (_Bool)(unsigned)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; float r1 = (float)v; float r2 = (float)(unsigned)9223372036854775807LL; static float r3 = (float)(unsigned)9223372036854775807LL;
      printf("1384: (float)(unsigned)9223372036854775807LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; double r1 = (double)v; double r2 = (double)(unsigned)9223372036854775807LL; static double r3 = (double)(unsigned)9223372036854775807LL;
      printf("1385: (double)(unsigned)9223372036854775807LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)9223372036854775807LL; long double r1 = (long double)v; long double r2 = (long double)(unsigned)9223372036854775807LL; static long double r3 = (long double)(unsigned)9223372036854775807LL;
      printf("1386: (long double)(unsigned)9223372036854775807LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)(-9223372036854775807LL-1); static signed char r3 = (signed char)(unsigned)(-9223372036854775807LL-1);
      printf("1387: (signed char)(unsigned)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)(-9223372036854775807LL-1); static unsigned char r3 = (unsigned char)(unsigned)(-9223372036854775807LL-1);
      printf("1388: (unsigned char)(unsigned)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); short r1 = (short)v; short r2 = (short)(unsigned)(-9223372036854775807LL-1); static short r3 = (short)(unsigned)(-9223372036854775807LL-1);
      printf("1389: (short)(unsigned)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)(-9223372036854775807LL-1); static unsigned short r3 = (unsigned short)(unsigned)(-9223372036854775807LL-1);
      printf("1390: (unsigned short)(unsigned)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); int r1 = (int)v; int r2 = (int)(unsigned)(-9223372036854775807LL-1); static int r3 = (int)(unsigned)(-9223372036854775807LL-1);
      printf("1391: (int)(unsigned)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)(-9223372036854775807LL-1); static unsigned r3 = (unsigned)(unsigned)(-9223372036854775807LL-1);
      printf("1392: (unsigned)(unsigned)(-9223372036854775807LL-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); long r1 = (long)v; long r2 = (long)(unsigned)(-9223372036854775807LL-1); static long r3 = (long)(unsigned)(-9223372036854775807LL-1);
      printf("1393: (long)(unsigned)(-9223372036854775807LL-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)(-9223372036854775807LL-1); static unsigned long r3 = (unsigned long)(unsigned)(-9223372036854775807LL-1);
      printf("1394: (unsigned long)(unsigned)(-9223372036854775807LL-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); long long r1 = (long long)v; long long r2 = (long long)(unsigned)(-9223372036854775807LL-1); static long long r3 = (long long)(unsigned)(-9223372036854775807LL-1);
      printf("1395: (long long)(unsigned)(-9223372036854775807LL-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)(-9223372036854775807LL-1); static unsigned long long r3 = (unsigned long long)(unsigned)(-9223372036854775807LL-1);
      printf("1396: (unsigned long long)(unsigned)(-9223372036854775807LL-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)(-9223372036854775807LL-1); static _Bool r3 = (_Bool)(unsigned)(-9223372036854775807LL-1);
      printf("1397: (_Bool)(unsigned)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); float r1 = (float)v; float r2 = (float)(unsigned)(-9223372036854775807LL-1); static float r3 = (float)(unsigned)(-9223372036854775807LL-1);
      printf("1398: (float)(unsigned)(-9223372036854775807LL-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); double r1 = (double)v; double r2 = (double)(unsigned)(-9223372036854775807LL-1); static double r3 = (double)(unsigned)(-9223372036854775807LL-1);
      printf("1399: (double)(unsigned)(-9223372036854775807LL-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)(-9223372036854775807LL-1); long double r1 = (long double)v; long double r2 = (long double)(unsigned)(-9223372036854775807LL-1); static long double r3 = (long double)(unsigned)(-9223372036854775807LL-1);
      printf("1400: (long double)(unsigned)(-9223372036854775807LL-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)0x8000000000000000ULL; static signed char r3 = (signed char)(unsigned)0x8000000000000000ULL;
      printf("1401: (signed char)(unsigned)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)0x8000000000000000ULL; static unsigned char r3 = (unsigned char)(unsigned)0x8000000000000000ULL;
      printf("1402: (unsigned char)(unsigned)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; short r1 = (short)v; short r2 = (short)(unsigned)0x8000000000000000ULL; static short r3 = (short)(unsigned)0x8000000000000000ULL;
      printf("1403: (short)(unsigned)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)0x8000000000000000ULL; static unsigned short r3 = (unsigned short)(unsigned)0x8000000000000000ULL;
      printf("1404: (unsigned short)(unsigned)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; int r1 = (int)v; int r2 = (int)(unsigned)0x8000000000000000ULL; static int r3 = (int)(unsigned)0x8000000000000000ULL;
      printf("1405: (int)(unsigned)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)0x8000000000000000ULL; static unsigned r3 = (unsigned)(unsigned)0x8000000000000000ULL;
      printf("1406: (unsigned)(unsigned)0x8000000000000000ULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; long r1 = (long)v; long r2 = (long)(unsigned)0x8000000000000000ULL; static long r3 = (long)(unsigned)0x8000000000000000ULL;
      printf("1407: (long)(unsigned)0x8000000000000000ULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)0x8000000000000000ULL; static unsigned long r3 = (unsigned long)(unsigned)0x8000000000000000ULL;
      printf("1408: (unsigned long)(unsigned)0x8000000000000000ULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; long long r1 = (long long)v; long long r2 = (long long)(unsigned)0x8000000000000000ULL; static long long r3 = (long long)(unsigned)0x8000000000000000ULL;
      printf("1409: (long long)(unsigned)0x8000000000000000ULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)0x8000000000000000ULL; static unsigned long long r3 = (unsigned long long)(unsigned)0x8000000000000000ULL;
      printf("1410: (unsigned long long)(unsigned)0x8000000000000000ULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)0x8000000000000000ULL; static _Bool r3 = (_Bool)(unsigned)0x8000000000000000ULL;
      printf("1411: (_Bool)(unsigned)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; float r1 = (float)v; float r2 = (float)(unsigned)0x8000000000000000ULL; static float r3 = (float)(unsigned)0x8000000000000000ULL;
      printf("1412: (float)(unsigned)0x8000000000000000ULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; double r1 = (double)v; double r2 = (double)(unsigned)0x8000000000000000ULL; static double r3 = (double)(unsigned)0x8000000000000000ULL;
      printf("1413: (double)(unsigned)0x8000000000000000ULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)0x8000000000000000ULL; long double r1 = (long double)v; long double r2 = (long double)(unsigned)0x8000000000000000ULL; static long double r3 = (long double)(unsigned)0x8000000000000000ULL;
      printf("1414: (long double)(unsigned)0x8000000000000000ULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned)0xffffffffffffffffULL; static signed char r3 = (signed char)(unsigned)0xffffffffffffffffULL;
      printf("1415: (signed char)(unsigned)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned)0xffffffffffffffffULL; static unsigned char r3 = (unsigned char)(unsigned)0xffffffffffffffffULL;
      printf("1416: (unsigned char)(unsigned)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; short r1 = (short)v; short r2 = (short)(unsigned)0xffffffffffffffffULL; static short r3 = (short)(unsigned)0xffffffffffffffffULL;
      printf("1417: (short)(unsigned)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned)0xffffffffffffffffULL; static unsigned short r3 = (unsigned short)(unsigned)0xffffffffffffffffULL;
      printf("1418: (unsigned short)(unsigned)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; int r1 = (int)v; int r2 = (int)(unsigned)0xffffffffffffffffULL; static int r3 = (int)(unsigned)0xffffffffffffffffULL;
      printf("1419: (int)(unsigned)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned)0xffffffffffffffffULL; static unsigned r3 = (unsigned)(unsigned)0xffffffffffffffffULL;
      printf("1420: (unsigned)(unsigned)0xffffffffffffffffULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; long r1 = (long)v; long r2 = (long)(unsigned)0xffffffffffffffffULL; static long r3 = (long)(unsigned)0xffffffffffffffffULL;
      printf("1421: (long)(unsigned)0xffffffffffffffffULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned)0xffffffffffffffffULL; static unsigned long r3 = (unsigned long)(unsigned)0xffffffffffffffffULL;
      printf("1422: (unsigned long)(unsigned)0xffffffffffffffffULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; long long r1 = (long long)v; long long r2 = (long long)(unsigned)0xffffffffffffffffULL; static long long r3 = (long long)(unsigned)0xffffffffffffffffULL;
      printf("1423: (long long)(unsigned)0xffffffffffffffffULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned)0xffffffffffffffffULL; static unsigned long long r3 = (unsigned long long)(unsigned)0xffffffffffffffffULL;
      printf("1424: (unsigned long long)(unsigned)0xffffffffffffffffULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned)0xffffffffffffffffULL; static _Bool r3 = (_Bool)(unsigned)0xffffffffffffffffULL;
      printf("1425: (_Bool)(unsigned)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; float r1 = (float)v; float r2 = (float)(unsigned)0xffffffffffffffffULL; static float r3 = (float)(unsigned)0xffffffffffffffffULL;
      printf("1426: (float)(unsigned)0xffffffffffffffffULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; double r1 = (double)v; double r2 = (double)(unsigned)0xffffffffffffffffULL; static double r3 = (double)(unsigned)0xffffffffffffffffULL;
      printf("1427: (double)(unsigned)0xffffffffffffffffULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned v = (unsigned)0xffffffffffffffffULL; long double r1 = (long double)v; long double r2 = (long double)(unsigned)0xffffffffffffffffULL; static long double r3 = (long double)(unsigned)0xffffffffffffffffULL;
      printf("1428: (long double)(unsigned)0xffffffffffffffffULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)0; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)0; static signed char r3 = (signed char)(long)0;
      printf("1429: (signed char)(long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)0; static unsigned char r3 = (unsigned char)(long)0;
      printf("1430: (unsigned char)(long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0; short r1 = (short)v; short r2 = (short)(long)0; static short r3 = (short)(long)0;
      printf("1431: (short)(long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)0; static unsigned short r3 = (unsigned short)(long)0;
      printf("1432: (unsigned short)(long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0; int r1 = (int)v; int r2 = (int)(long)0; static int r3 = (int)(long)0;
      printf("1433: (int)(long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)0; static unsigned r3 = (unsigned)(long)0;
      printf("1434: (unsigned)(long)0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)0; long r1 = (long)v; long r2 = (long)(long)0; static long r3 = (long)(long)0;
      printf("1435: (long)(long)0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)0; static unsigned long r3 = (unsigned long)(long)0;
      printf("1436: (unsigned long)(long)0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)0; long long r1 = (long long)v; long long r2 = (long long)(long)0; static long long r3 = (long long)(long)0;
      printf("1437: (long long)(long)0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)0; static unsigned long long r3 = (unsigned long long)(long)0;
      printf("1438: (unsigned long long)(long)0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)0; static _Bool r3 = (_Bool)(long)0;
      printf("1439: (_Bool)(long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0; float r1 = (float)v; float r2 = (float)(long)0; static float r3 = (float)(long)0;
      printf("1440: (float)(long)0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)0; double r1 = (double)v; double r2 = (double)(long)0; static double r3 = (double)(long)0;
      printf("1441: (double)(long)0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)0; long double r1 = (long double)v; long double r2 = (long double)(long)0; static long double r3 = (long double)(long)0;
      printf("1442: (long double)(long)0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)1; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)1; static signed char r3 = (signed char)(long)1;
      printf("1443: (signed char)(long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)1; static unsigned char r3 = (unsigned char)(long)1;
      printf("1444: (unsigned char)(long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)1; short r1 = (short)v; short r2 = (short)(long)1; static short r3 = (short)(long)1;
      printf("1445: (short)(long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)1; static unsigned short r3 = (unsigned short)(long)1;
      printf("1446: (unsigned short)(long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)1; int r1 = (int)v; int r2 = (int)(long)1; static int r3 = (int)(long)1;
      printf("1447: (int)(long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)1; static unsigned r3 = (unsigned)(long)1;
      printf("1448: (unsigned)(long)1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)1; long r1 = (long)v; long r2 = (long)(long)1; static long r3 = (long)(long)1;
      printf("1449: (long)(long)1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)1; static unsigned long r3 = (unsigned long)(long)1;
      printf("1450: (unsigned long)(long)1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)1; long long r1 = (long long)v; long long r2 = (long long)(long)1; static long long r3 = (long long)(long)1;
      printf("1451: (long long)(long)1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)1; static unsigned long long r3 = (unsigned long long)(long)1;
      printf("1452: (unsigned long long)(long)1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)1; static _Bool r3 = (_Bool)(long)1;
      printf("1453: (_Bool)(long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)1; float r1 = (float)v; float r2 = (float)(long)1; static float r3 = (float)(long)1;
      printf("1454: (float)(long)1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)1; double r1 = (double)v; double r2 = (double)(long)1; static double r3 = (double)(long)1;
      printf("1455: (double)(long)1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)1; long double r1 = (long double)v; long double r2 = (long double)(long)1; static long double r3 = (long double)(long)1;
      printf("1456: (long double)(long)1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)-1; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)-1; static signed char r3 = (signed char)(long)-1;
      printf("1457: (signed char)(long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)-1; static unsigned char r3 = (unsigned char)(long)-1;
      printf("1458: (unsigned char)(long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-1; short r1 = (short)v; short r2 = (short)(long)-1; static short r3 = (short)(long)-1;
      printf("1459: (short)(long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)-1; static unsigned short r3 = (unsigned short)(long)-1;
      printf("1460: (unsigned short)(long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-1; int r1 = (int)v; int r2 = (int)(long)-1; static int r3 = (int)(long)-1;
      printf("1461: (int)(long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)-1; static unsigned r3 = (unsigned)(long)-1;
      printf("1462: (unsigned)(long)-1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)-1; long r1 = (long)v; long r2 = (long)(long)-1; static long r3 = (long)(long)-1;
      printf("1463: (long)(long)-1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)-1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)-1; static unsigned long r3 = (unsigned long)(long)-1;
      printf("1464: (unsigned long)(long)-1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)-1; long long r1 = (long long)v; long long r2 = (long long)(long)-1; static long long r3 = (long long)(long)-1;
      printf("1465: (long long)(long)-1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)-1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)-1; static unsigned long long r3 = (unsigned long long)(long)-1;
      printf("1466: (unsigned long long)(long)-1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)-1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)-1; static _Bool r3 = (_Bool)(long)-1;
      printf("1467: (_Bool)(long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-1; float r1 = (float)v; float r2 = (float)(long)-1; static float r3 = (float)(long)-1;
      printf("1468: (float)(long)-1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)-1; double r1 = (double)v; double r2 = (double)(long)-1; static double r3 = (double)(long)-1;
      printf("1469: (double)(long)-1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)-1; long double r1 = (long double)v; long double r2 = (long double)(long)-1; static long double r3 = (long double)(long)-1;
      printf("1470: (long double)(long)-1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)127; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)127; static signed char r3 = (signed char)(long)127;
      printf("1471: (signed char)(long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)127; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)127; static unsigned char r3 = (unsigned char)(long)127;
      printf("1472: (unsigned char)(long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)127; short r1 = (short)v; short r2 = (short)(long)127; static short r3 = (short)(long)127;
      printf("1473: (short)(long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)127; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)127; static unsigned short r3 = (unsigned short)(long)127;
      printf("1474: (unsigned short)(long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)127; int r1 = (int)v; int r2 = (int)(long)127; static int r3 = (int)(long)127;
      printf("1475: (int)(long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)127; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)127; static unsigned r3 = (unsigned)(long)127;
      printf("1476: (unsigned)(long)127 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)127; long r1 = (long)v; long r2 = (long)(long)127; static long r3 = (long)(long)127;
      printf("1477: (long)(long)127 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)127; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)127; static unsigned long r3 = (unsigned long)(long)127;
      printf("1478: (unsigned long)(long)127 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)127; long long r1 = (long long)v; long long r2 = (long long)(long)127; static long long r3 = (long long)(long)127;
      printf("1479: (long long)(long)127 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)127; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)127; static unsigned long long r3 = (unsigned long long)(long)127;
      printf("1480: (unsigned long long)(long)127 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)127; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)127; static _Bool r3 = (_Bool)(long)127;
      printf("1481: (_Bool)(long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)127; float r1 = (float)v; float r2 = (float)(long)127; static float r3 = (float)(long)127;
      printf("1482: (float)(long)127 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)127; double r1 = (double)v; double r2 = (double)(long)127; static double r3 = (double)(long)127;
      printf("1483: (double)(long)127 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)127; long double r1 = (long double)v; long double r2 = (long double)(long)127; static long double r3 = (long double)(long)127;
      printf("1484: (long double)(long)127 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)-128; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)-128; static signed char r3 = (signed char)(long)-128;
      printf("1485: (signed char)(long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-128; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)-128; static unsigned char r3 = (unsigned char)(long)-128;
      printf("1486: (unsigned char)(long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-128; short r1 = (short)v; short r2 = (short)(long)-128; static short r3 = (short)(long)-128;
      printf("1487: (short)(long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-128; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)-128; static unsigned short r3 = (unsigned short)(long)-128;
      printf("1488: (unsigned short)(long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-128; int r1 = (int)v; int r2 = (int)(long)-128; static int r3 = (int)(long)-128;
      printf("1489: (int)(long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-128; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)-128; static unsigned r3 = (unsigned)(long)-128;
      printf("1490: (unsigned)(long)-128 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)-128; long r1 = (long)v; long r2 = (long)(long)-128; static long r3 = (long)(long)-128;
      printf("1491: (long)(long)-128 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)-128; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)-128; static unsigned long r3 = (unsigned long)(long)-128;
      printf("1492: (unsigned long)(long)-128 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)-128; long long r1 = (long long)v; long long r2 = (long long)(long)-128; static long long r3 = (long long)(long)-128;
      printf("1493: (long long)(long)-128 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)-128; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)-128; static unsigned long long r3 = (unsigned long long)(long)-128;
      printf("1494: (unsigned long long)(long)-128 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)-128; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)-128; static _Bool r3 = (_Bool)(long)-128;
      printf("1495: (_Bool)(long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-128; float r1 = (float)v; float r2 = (float)(long)-128; static float r3 = (float)(long)-128;
      printf("1496: (float)(long)-128 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)-128; double r1 = (double)v; double r2 = (double)(long)-128; static double r3 = (double)(long)-128;
      printf("1497: (double)(long)-128 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)-128; long double r1 = (long double)v; long double r2 = (long double)(long)-128; static long double r3 = (long double)(long)-128;
      printf("1498: (long double)(long)-128 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)255; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)255; static signed char r3 = (signed char)(long)255;
      printf("1499: (signed char)(long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)255; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)255; static unsigned char r3 = (unsigned char)(long)255;
      printf("1500: (unsigned char)(long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)255; short r1 = (short)v; short r2 = (short)(long)255; static short r3 = (short)(long)255;
      printf("1501: (short)(long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)255; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)255; static unsigned short r3 = (unsigned short)(long)255;
      printf("1502: (unsigned short)(long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)255; int r1 = (int)v; int r2 = (int)(long)255; static int r3 = (int)(long)255;
      printf("1503: (int)(long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)255; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)255; static unsigned r3 = (unsigned)(long)255;
      printf("1504: (unsigned)(long)255 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)255; long r1 = (long)v; long r2 = (long)(long)255; static long r3 = (long)(long)255;
      printf("1505: (long)(long)255 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)255; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)255; static unsigned long r3 = (unsigned long)(long)255;
      printf("1506: (unsigned long)(long)255 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)255; long long r1 = (long long)v; long long r2 = (long long)(long)255; static long long r3 = (long long)(long)255;
      printf("1507: (long long)(long)255 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)255; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)255; static unsigned long long r3 = (unsigned long long)(long)255;
      printf("1508: (unsigned long long)(long)255 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)255; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)255; static _Bool r3 = (_Bool)(long)255;
      printf("1509: (_Bool)(long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)255; float r1 = (float)v; float r2 = (float)(long)255; static float r3 = (float)(long)255;
      printf("1510: (float)(long)255 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)255; double r1 = (double)v; double r2 = (double)(long)255; static double r3 = (double)(long)255;
      printf("1511: (double)(long)255 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)255; long double r1 = (long double)v; long double r2 = (long double)(long)255; static long double r3 = (long double)(long)255;
      printf("1512: (long double)(long)255 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)256; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)256; static signed char r3 = (signed char)(long)256;
      printf("1513: (signed char)(long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)256; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)256; static unsigned char r3 = (unsigned char)(long)256;
      printf("1514: (unsigned char)(long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)256; short r1 = (short)v; short r2 = (short)(long)256; static short r3 = (short)(long)256;
      printf("1515: (short)(long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)256; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)256; static unsigned short r3 = (unsigned short)(long)256;
      printf("1516: (unsigned short)(long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)256; int r1 = (int)v; int r2 = (int)(long)256; static int r3 = (int)(long)256;
      printf("1517: (int)(long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)256; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)256; static unsigned r3 = (unsigned)(long)256;
      printf("1518: (unsigned)(long)256 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)256; long r1 = (long)v; long r2 = (long)(long)256; static long r3 = (long)(long)256;
      printf("1519: (long)(long)256 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)256; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)256; static unsigned long r3 = (unsigned long)(long)256;
      printf("1520: (unsigned long)(long)256 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)256; long long r1 = (long long)v; long long r2 = (long long)(long)256; static long long r3 = (long long)(long)256;
      printf("1521: (long long)(long)256 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)256; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)256; static unsigned long long r3 = (unsigned long long)(long)256;
      printf("1522: (unsigned long long)(long)256 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)256; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)256; static _Bool r3 = (_Bool)(long)256;
      printf("1523: (_Bool)(long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)256; float r1 = (float)v; float r2 = (float)(long)256; static float r3 = (float)(long)256;
      printf("1524: (float)(long)256 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)256; double r1 = (double)v; double r2 = (double)(long)256; static double r3 = (double)(long)256;
      printf("1525: (double)(long)256 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)256; long double r1 = (long double)v; long double r2 = (long double)(long)256; static long double r3 = (long double)(long)256;
      printf("1526: (long double)(long)256 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)32767; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)32767; static signed char r3 = (signed char)(long)32767;
      printf("1527: (signed char)(long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)32767; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)32767; static unsigned char r3 = (unsigned char)(long)32767;
      printf("1528: (unsigned char)(long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)32767; short r1 = (short)v; short r2 = (short)(long)32767; static short r3 = (short)(long)32767;
      printf("1529: (short)(long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)32767; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)32767; static unsigned short r3 = (unsigned short)(long)32767;
      printf("1530: (unsigned short)(long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)32767; int r1 = (int)v; int r2 = (int)(long)32767; static int r3 = (int)(long)32767;
      printf("1531: (int)(long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)32767; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)32767; static unsigned r3 = (unsigned)(long)32767;
      printf("1532: (unsigned)(long)32767 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)32767; long r1 = (long)v; long r2 = (long)(long)32767; static long r3 = (long)(long)32767;
      printf("1533: (long)(long)32767 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)32767; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)32767; static unsigned long r3 = (unsigned long)(long)32767;
      printf("1534: (unsigned long)(long)32767 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)32767; long long r1 = (long long)v; long long r2 = (long long)(long)32767; static long long r3 = (long long)(long)32767;
      printf("1535: (long long)(long)32767 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)32767; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)32767; static unsigned long long r3 = (unsigned long long)(long)32767;
      printf("1536: (unsigned long long)(long)32767 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)32767; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)32767; static _Bool r3 = (_Bool)(long)32767;
      printf("1537: (_Bool)(long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)32767; float r1 = (float)v; float r2 = (float)(long)32767; static float r3 = (float)(long)32767;
      printf("1538: (float)(long)32767 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)32767; double r1 = (double)v; double r2 = (double)(long)32767; static double r3 = (double)(long)32767;
      printf("1539: (double)(long)32767 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)32767; long double r1 = (long double)v; long double r2 = (long double)(long)32767; static long double r3 = (long double)(long)32767;
      printf("1540: (long double)(long)32767 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)-32768; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)-32768; static signed char r3 = (signed char)(long)-32768;
      printf("1541: (signed char)(long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-32768; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)-32768; static unsigned char r3 = (unsigned char)(long)-32768;
      printf("1542: (unsigned char)(long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-32768; short r1 = (short)v; short r2 = (short)(long)-32768; static short r3 = (short)(long)-32768;
      printf("1543: (short)(long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-32768; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)-32768; static unsigned short r3 = (unsigned short)(long)-32768;
      printf("1544: (unsigned short)(long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-32768; int r1 = (int)v; int r2 = (int)(long)-32768; static int r3 = (int)(long)-32768;
      printf("1545: (int)(long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-32768; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)-32768; static unsigned r3 = (unsigned)(long)-32768;
      printf("1546: (unsigned)(long)-32768 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)-32768; long r1 = (long)v; long r2 = (long)(long)-32768; static long r3 = (long)(long)-32768;
      printf("1547: (long)(long)-32768 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)-32768; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)-32768; static unsigned long r3 = (unsigned long)(long)-32768;
      printf("1548: (unsigned long)(long)-32768 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)-32768; long long r1 = (long long)v; long long r2 = (long long)(long)-32768; static long long r3 = (long long)(long)-32768;
      printf("1549: (long long)(long)-32768 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)-32768; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)-32768; static unsigned long long r3 = (unsigned long long)(long)-32768;
      printf("1550: (unsigned long long)(long)-32768 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)-32768; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)-32768; static _Bool r3 = (_Bool)(long)-32768;
      printf("1551: (_Bool)(long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)-32768; float r1 = (float)v; float r2 = (float)(long)-32768; static float r3 = (float)(long)-32768;
      printf("1552: (float)(long)-32768 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)-32768; double r1 = (double)v; double r2 = (double)(long)-32768; static double r3 = (double)(long)-32768;
      printf("1553: (double)(long)-32768 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)-32768; long double r1 = (long double)v; long double r2 = (long double)(long)-32768; static long double r3 = (long double)(long)-32768;
      printf("1554: (long double)(long)-32768 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)65535; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)65535; static signed char r3 = (signed char)(long)65535;
      printf("1555: (signed char)(long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)65535; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)65535; static unsigned char r3 = (unsigned char)(long)65535;
      printf("1556: (unsigned char)(long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)65535; short r1 = (short)v; short r2 = (short)(long)65535; static short r3 = (short)(long)65535;
      printf("1557: (short)(long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)65535; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)65535; static unsigned short r3 = (unsigned short)(long)65535;
      printf("1558: (unsigned short)(long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)65535; int r1 = (int)v; int r2 = (int)(long)65535; static int r3 = (int)(long)65535;
      printf("1559: (int)(long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)65535; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)65535; static unsigned r3 = (unsigned)(long)65535;
      printf("1560: (unsigned)(long)65535 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)65535; long r1 = (long)v; long r2 = (long)(long)65535; static long r3 = (long)(long)65535;
      printf("1561: (long)(long)65535 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)65535; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)65535; static unsigned long r3 = (unsigned long)(long)65535;
      printf("1562: (unsigned long)(long)65535 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)65535; long long r1 = (long long)v; long long r2 = (long long)(long)65535; static long long r3 = (long long)(long)65535;
      printf("1563: (long long)(long)65535 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)65535; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)65535; static unsigned long long r3 = (unsigned long long)(long)65535;
      printf("1564: (unsigned long long)(long)65535 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)65535; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)65535; static _Bool r3 = (_Bool)(long)65535;
      printf("1565: (_Bool)(long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)65535; float r1 = (float)v; float r2 = (float)(long)65535; static float r3 = (float)(long)65535;
      printf("1566: (float)(long)65535 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)65535; double r1 = (double)v; double r2 = (double)(long)65535; static double r3 = (double)(long)65535;
      printf("1567: (double)(long)65535 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)65535; long double r1 = (long double)v; long double r2 = (long double)(long)65535; static long double r3 = (long double)(long)65535;
      printf("1568: (long double)(long)65535 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)2147483647; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)2147483647; static signed char r3 = (signed char)(long)2147483647;
      printf("1569: (signed char)(long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)2147483647; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)2147483647; static unsigned char r3 = (unsigned char)(long)2147483647;
      printf("1570: (unsigned char)(long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)2147483647; short r1 = (short)v; short r2 = (short)(long)2147483647; static short r3 = (short)(long)2147483647;
      printf("1571: (short)(long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)2147483647; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)2147483647; static unsigned short r3 = (unsigned short)(long)2147483647;
      printf("1572: (unsigned short)(long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)2147483647; int r1 = (int)v; int r2 = (int)(long)2147483647; static int r3 = (int)(long)2147483647;
      printf("1573: (int)(long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)2147483647; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)2147483647; static unsigned r3 = (unsigned)(long)2147483647;
      printf("1574: (unsigned)(long)2147483647 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)2147483647; long r1 = (long)v; long r2 = (long)(long)2147483647; static long r3 = (long)(long)2147483647;
      printf("1575: (long)(long)2147483647 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)2147483647; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)2147483647; static unsigned long r3 = (unsigned long)(long)2147483647;
      printf("1576: (unsigned long)(long)2147483647 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)2147483647; long long r1 = (long long)v; long long r2 = (long long)(long)2147483647; static long long r3 = (long long)(long)2147483647;
      printf("1577: (long long)(long)2147483647 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)2147483647; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)2147483647; static unsigned long long r3 = (unsigned long long)(long)2147483647;
      printf("1578: (unsigned long long)(long)2147483647 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)2147483647; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)2147483647; static _Bool r3 = (_Bool)(long)2147483647;
      printf("1579: (_Bool)(long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)2147483647; float r1 = (float)v; float r2 = (float)(long)2147483647; static float r3 = (float)(long)2147483647;
      printf("1580: (float)(long)2147483647 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)2147483647; double r1 = (double)v; double r2 = (double)(long)2147483647; static double r3 = (double)(long)2147483647;
      printf("1581: (double)(long)2147483647 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)2147483647; long double r1 = (long double)v; long double r2 = (long double)(long)2147483647; static long double r3 = (long double)(long)2147483647;
      printf("1582: (long double)(long)2147483647 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)(-2147483647-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(long)(-2147483647-1); static signed char r3 = (signed char)(long)(-2147483647-1);
      printf("1583: (signed char)(long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)(-2147483647-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)(-2147483647-1); static unsigned char r3 = (unsigned char)(long)(-2147483647-1);
      printf("1584: (unsigned char)(long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)(-2147483647-1); short r1 = (short)v; short r2 = (short)(long)(-2147483647-1); static short r3 = (short)(long)(-2147483647-1);
      printf("1585: (short)(long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)(-2147483647-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)(-2147483647-1); static unsigned short r3 = (unsigned short)(long)(-2147483647-1);
      printf("1586: (unsigned short)(long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)(-2147483647-1); int r1 = (int)v; int r2 = (int)(long)(-2147483647-1); static int r3 = (int)(long)(-2147483647-1);
      printf("1587: (int)(long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)(-2147483647-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)(-2147483647-1); static unsigned r3 = (unsigned)(long)(-2147483647-1);
      printf("1588: (unsigned)(long)(-2147483647-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)(-2147483647-1); long r1 = (long)v; long r2 = (long)(long)(-2147483647-1); static long r3 = (long)(long)(-2147483647-1);
      printf("1589: (long)(long)(-2147483647-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)(-2147483647-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)(-2147483647-1); static unsigned long r3 = (unsigned long)(long)(-2147483647-1);
      printf("1590: (unsigned long)(long)(-2147483647-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)(-2147483647-1); long long r1 = (long long)v; long long r2 = (long long)(long)(-2147483647-1); static long long r3 = (long long)(long)(-2147483647-1);
      printf("1591: (long long)(long)(-2147483647-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)(-2147483647-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)(-2147483647-1); static unsigned long long r3 = (unsigned long long)(long)(-2147483647-1);
      printf("1592: (unsigned long long)(long)(-2147483647-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)(-2147483647-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)(-2147483647-1); static _Bool r3 = (_Bool)(long)(-2147483647-1);
      printf("1593: (_Bool)(long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)(-2147483647-1); float r1 = (float)v; float r2 = (float)(long)(-2147483647-1); static float r3 = (float)(long)(-2147483647-1);
      printf("1594: (float)(long)(-2147483647-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)(-2147483647-1); double r1 = (double)v; double r2 = (double)(long)(-2147483647-1); static double r3 = (double)(long)(-2147483647-1);
      printf("1595: (double)(long)(-2147483647-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)(-2147483647-1); long double r1 = (long double)v; long double r2 = (long double)(long)(-2147483647-1); static long double r3 = (long double)(long)(-2147483647-1);
      printf("1596: (long double)(long)(-2147483647-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)4294967295LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)4294967295LL; static signed char r3 = (signed char)(long)4294967295LL;
      printf("1597: (signed char)(long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)4294967295LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)4294967295LL; static unsigned char r3 = (unsigned char)(long)4294967295LL;
      printf("1598: (unsigned char)(long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)4294967295LL; short r1 = (short)v; short r2 = (short)(long)4294967295LL; static short r3 = (short)(long)4294967295LL;
      printf("1599: (short)(long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)4294967295LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)4294967295LL; static unsigned short r3 = (unsigned short)(long)4294967295LL;
      printf("1600: (unsigned short)(long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)4294967295LL; int r1 = (int)v; int r2 = (int)(long)4294967295LL; static int r3 = (int)(long)4294967295LL;
      printf("1601: (int)(long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)4294967295LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)4294967295LL; static unsigned r3 = (unsigned)(long)4294967295LL;
      printf("1602: (unsigned)(long)4294967295LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)4294967295LL; long r1 = (long)v; long r2 = (long)(long)4294967295LL; static long r3 = (long)(long)4294967295LL;
      printf("1603: (long)(long)4294967295LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)4294967295LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)4294967295LL; static unsigned long r3 = (unsigned long)(long)4294967295LL;
      printf("1604: (unsigned long)(long)4294967295LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)4294967295LL; long long r1 = (long long)v; long long r2 = (long long)(long)4294967295LL; static long long r3 = (long long)(long)4294967295LL;
      printf("1605: (long long)(long)4294967295LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)4294967295LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)4294967295LL; static unsigned long long r3 = (unsigned long long)(long)4294967295LL;
      printf("1606: (unsigned long long)(long)4294967295LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)4294967295LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)4294967295LL; static _Bool r3 = (_Bool)(long)4294967295LL;
      printf("1607: (_Bool)(long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)4294967295LL; float r1 = (float)v; float r2 = (float)(long)4294967295LL; static float r3 = (float)(long)4294967295LL;
      printf("1608: (float)(long)4294967295LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)4294967295LL; double r1 = (double)v; double r2 = (double)(long)4294967295LL; static double r3 = (double)(long)4294967295LL;
      printf("1609: (double)(long)4294967295LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)4294967295LL; long double r1 = (long double)v; long double r2 = (long double)(long)4294967295LL; static long double r3 = (long double)(long)4294967295LL;
      printf("1610: (long double)(long)4294967295LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)9223372036854775807LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)9223372036854775807LL; static signed char r3 = (signed char)(long)9223372036854775807LL;
      printf("1611: (signed char)(long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)9223372036854775807LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)9223372036854775807LL; static unsigned char r3 = (unsigned char)(long)9223372036854775807LL;
      printf("1612: (unsigned char)(long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)9223372036854775807LL; short r1 = (short)v; short r2 = (short)(long)9223372036854775807LL; static short r3 = (short)(long)9223372036854775807LL;
      printf("1613: (short)(long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)9223372036854775807LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)9223372036854775807LL; static unsigned short r3 = (unsigned short)(long)9223372036854775807LL;
      printf("1614: (unsigned short)(long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)9223372036854775807LL; int r1 = (int)v; int r2 = (int)(long)9223372036854775807LL; static int r3 = (int)(long)9223372036854775807LL;
      printf("1615: (int)(long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)9223372036854775807LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)9223372036854775807LL; static unsigned r3 = (unsigned)(long)9223372036854775807LL;
      printf("1616: (unsigned)(long)9223372036854775807LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)9223372036854775807LL; long r1 = (long)v; long r2 = (long)(long)9223372036854775807LL; static long r3 = (long)(long)9223372036854775807LL;
      printf("1617: (long)(long)9223372036854775807LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)9223372036854775807LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)9223372036854775807LL; static unsigned long r3 = (unsigned long)(long)9223372036854775807LL;
      printf("1618: (unsigned long)(long)9223372036854775807LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)9223372036854775807LL; long long r1 = (long long)v; long long r2 = (long long)(long)9223372036854775807LL; static long long r3 = (long long)(long)9223372036854775807LL;
      printf("1619: (long long)(long)9223372036854775807LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)9223372036854775807LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)9223372036854775807LL; static unsigned long long r3 = (unsigned long long)(long)9223372036854775807LL;
      printf("1620: (unsigned long long)(long)9223372036854775807LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)9223372036854775807LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)9223372036854775807LL; static _Bool r3 = (_Bool)(long)9223372036854775807LL;
      printf("1621: (_Bool)(long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)9223372036854775807LL; float r1 = (float)v; float r2 = (float)(long)9223372036854775807LL; static float r3 = (float)(long)9223372036854775807LL;
      printf("1622: (float)(long)9223372036854775807LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)9223372036854775807LL; double r1 = (double)v; double r2 = (double)(long)9223372036854775807LL; static double r3 = (double)(long)9223372036854775807LL;
      printf("1623: (double)(long)9223372036854775807LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)9223372036854775807LL; long double r1 = (long double)v; long double r2 = (long double)(long)9223372036854775807LL; static long double r3 = (long double)(long)9223372036854775807LL;
      printf("1624: (long double)(long)9223372036854775807LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(long)(-9223372036854775807LL-1); static signed char r3 = (signed char)(long)(-9223372036854775807LL-1);
      printf("1625: (signed char)(long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)(-9223372036854775807LL-1); static unsigned char r3 = (unsigned char)(long)(-9223372036854775807LL-1);
      printf("1626: (unsigned char)(long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); short r1 = (short)v; short r2 = (short)(long)(-9223372036854775807LL-1); static short r3 = (short)(long)(-9223372036854775807LL-1);
      printf("1627: (short)(long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)(-9223372036854775807LL-1); static unsigned short r3 = (unsigned short)(long)(-9223372036854775807LL-1);
      printf("1628: (unsigned short)(long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); int r1 = (int)v; int r2 = (int)(long)(-9223372036854775807LL-1); static int r3 = (int)(long)(-9223372036854775807LL-1);
      printf("1629: (int)(long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)(-9223372036854775807LL-1); static unsigned r3 = (unsigned)(long)(-9223372036854775807LL-1);
      printf("1630: (unsigned)(long)(-9223372036854775807LL-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); long r1 = (long)v; long r2 = (long)(long)(-9223372036854775807LL-1); static long r3 = (long)(long)(-9223372036854775807LL-1);
      printf("1631: (long)(long)(-9223372036854775807LL-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)(-9223372036854775807LL-1); static unsigned long r3 = (unsigned long)(long)(-9223372036854775807LL-1);
      printf("1632: (unsigned long)(long)(-9223372036854775807LL-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); long long r1 = (long long)v; long long r2 = (long long)(long)(-9223372036854775807LL-1); static long long r3 = (long long)(long)(-9223372036854775807LL-1);
      printf("1633: (long long)(long)(-9223372036854775807LL-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)(-9223372036854775807LL-1); static unsigned long long r3 = (unsigned long long)(long)(-9223372036854775807LL-1);
      printf("1634: (unsigned long long)(long)(-9223372036854775807LL-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)(-9223372036854775807LL-1); static _Bool r3 = (_Bool)(long)(-9223372036854775807LL-1);
      printf("1635: (_Bool)(long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); float r1 = (float)v; float r2 = (float)(long)(-9223372036854775807LL-1); static float r3 = (float)(long)(-9223372036854775807LL-1);
      printf("1636: (float)(long)(-9223372036854775807LL-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); double r1 = (double)v; double r2 = (double)(long)(-9223372036854775807LL-1); static double r3 = (double)(long)(-9223372036854775807LL-1);
      printf("1637: (double)(long)(-9223372036854775807LL-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)(-9223372036854775807LL-1); long double r1 = (long double)v; long double r2 = (long double)(long)(-9223372036854775807LL-1); static long double r3 = (long double)(long)(-9223372036854775807LL-1);
      printf("1638: (long double)(long)(-9223372036854775807LL-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)0x8000000000000000ULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)0x8000000000000000ULL; static signed char r3 = (signed char)(long)0x8000000000000000ULL;
      printf("1639: (signed char)(long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0x8000000000000000ULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)0x8000000000000000ULL; static unsigned char r3 = (unsigned char)(long)0x8000000000000000ULL;
      printf("1640: (unsigned char)(long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0x8000000000000000ULL; short r1 = (short)v; short r2 = (short)(long)0x8000000000000000ULL; static short r3 = (short)(long)0x8000000000000000ULL;
      printf("1641: (short)(long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0x8000000000000000ULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)0x8000000000000000ULL; static unsigned short r3 = (unsigned short)(long)0x8000000000000000ULL;
      printf("1642: (unsigned short)(long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0x8000000000000000ULL; int r1 = (int)v; int r2 = (int)(long)0x8000000000000000ULL; static int r3 = (int)(long)0x8000000000000000ULL;
      printf("1643: (int)(long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0x8000000000000000ULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)0x8000000000000000ULL; static unsigned r3 = (unsigned)(long)0x8000000000000000ULL;
      printf("1644: (unsigned)(long)0x8000000000000000ULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)0x8000000000000000ULL; long r1 = (long)v; long r2 = (long)(long)0x8000000000000000ULL; static long r3 = (long)(long)0x8000000000000000ULL;
      printf("1645: (long)(long)0x8000000000000000ULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)0x8000000000000000ULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)0x8000000000000000ULL; static unsigned long r3 = (unsigned long)(long)0x8000000000000000ULL;
      printf("1646: (unsigned long)(long)0x8000000000000000ULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)0x8000000000000000ULL; long long r1 = (long long)v; long long r2 = (long long)(long)0x8000000000000000ULL; static long long r3 = (long long)(long)0x8000000000000000ULL;
      printf("1647: (long long)(long)0x8000000000000000ULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)0x8000000000000000ULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)0x8000000000000000ULL; static unsigned long long r3 = (unsigned long long)(long)0x8000000000000000ULL;
      printf("1648: (unsigned long long)(long)0x8000000000000000ULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)0x8000000000000000ULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)0x8000000000000000ULL; static _Bool r3 = (_Bool)(long)0x8000000000000000ULL;
      printf("1649: (_Bool)(long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0x8000000000000000ULL; float r1 = (float)v; float r2 = (float)(long)0x8000000000000000ULL; static float r3 = (float)(long)0x8000000000000000ULL;
      printf("1650: (float)(long)0x8000000000000000ULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)0x8000000000000000ULL; double r1 = (double)v; double r2 = (double)(long)0x8000000000000000ULL; static double r3 = (double)(long)0x8000000000000000ULL;
      printf("1651: (double)(long)0x8000000000000000ULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)0x8000000000000000ULL; long double r1 = (long double)v; long double r2 = (long double)(long)0x8000000000000000ULL; static long double r3 = (long double)(long)0x8000000000000000ULL;
      printf("1652: (long double)(long)0x8000000000000000ULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(long)0xffffffffffffffffULL; static signed char r3 = (signed char)(long)0xffffffffffffffffULL;
      printf("1653: (signed char)(long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long)0xffffffffffffffffULL; static unsigned char r3 = (unsigned char)(long)0xffffffffffffffffULL;
      printf("1654: (unsigned char)(long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; short r1 = (short)v; short r2 = (short)(long)0xffffffffffffffffULL; static short r3 = (short)(long)0xffffffffffffffffULL;
      printf("1655: (short)(long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long)0xffffffffffffffffULL; static unsigned short r3 = (unsigned short)(long)0xffffffffffffffffULL;
      printf("1656: (unsigned short)(long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; int r1 = (int)v; int r2 = (int)(long)0xffffffffffffffffULL; static int r3 = (int)(long)0xffffffffffffffffULL;
      printf("1657: (int)(long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long)0xffffffffffffffffULL; static unsigned r3 = (unsigned)(long)0xffffffffffffffffULL;
      printf("1658: (unsigned)(long)0xffffffffffffffffULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; long r1 = (long)v; long r2 = (long)(long)0xffffffffffffffffULL; static long r3 = (long)(long)0xffffffffffffffffULL;
      printf("1659: (long)(long)0xffffffffffffffffULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long)0xffffffffffffffffULL; static unsigned long r3 = (unsigned long)(long)0xffffffffffffffffULL;
      printf("1660: (unsigned long)(long)0xffffffffffffffffULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; long long r1 = (long long)v; long long r2 = (long long)(long)0xffffffffffffffffULL; static long long r3 = (long long)(long)0xffffffffffffffffULL;
      printf("1661: (long long)(long)0xffffffffffffffffULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long)0xffffffffffffffffULL; static unsigned long long r3 = (unsigned long long)(long)0xffffffffffffffffULL;
      printf("1662: (unsigned long long)(long)0xffffffffffffffffULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long)0xffffffffffffffffULL; static _Bool r3 = (_Bool)(long)0xffffffffffffffffULL;
      printf("1663: (_Bool)(long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; float r1 = (float)v; float r2 = (float)(long)0xffffffffffffffffULL; static float r3 = (float)(long)0xffffffffffffffffULL;
      printf("1664: (float)(long)0xffffffffffffffffULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; double r1 = (double)v; double r2 = (double)(long)0xffffffffffffffffULL; static double r3 = (double)(long)0xffffffffffffffffULL;
      printf("1665: (double)(long)0xffffffffffffffffULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long v = (long)0xffffffffffffffffULL; long double r1 = (long double)v; long double r2 = (long double)(long)0xffffffffffffffffULL; static long double r3 = (long double)(long)0xffffffffffffffffULL;
      printf("1666: (long double)(long)0xffffffffffffffffULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)0; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)0; static signed char r3 = (signed char)(unsigned long)0;
      printf("1667: (signed char)(unsigned long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)0; static unsigned char r3 = (unsigned char)(unsigned long)0;
      printf("1668: (unsigned char)(unsigned long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0; short r1 = (short)v; short r2 = (short)(unsigned long)0; static short r3 = (short)(unsigned long)0;
      printf("1669: (short)(unsigned long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)0; static unsigned short r3 = (unsigned short)(unsigned long)0;
      printf("1670: (unsigned short)(unsigned long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0; int r1 = (int)v; int r2 = (int)(unsigned long)0; static int r3 = (int)(unsigned long)0;
      printf("1671: (int)(unsigned long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)0; static unsigned r3 = (unsigned)(unsigned long)0;
      printf("1672: (unsigned)(unsigned long)0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)0; long r1 = (long)v; long r2 = (long)(unsigned long)0; static long r3 = (long)(unsigned long)0;
      printf("1673: (long)(unsigned long)0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)0; static unsigned long r3 = (unsigned long)(unsigned long)0;
      printf("1674: (unsigned long)(unsigned long)0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)0; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)0; static long long r3 = (long long)(unsigned long)0;
      printf("1675: (long long)(unsigned long)0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)0; static unsigned long long r3 = (unsigned long long)(unsigned long)0;
      printf("1676: (unsigned long long)(unsigned long)0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)0; static _Bool r3 = (_Bool)(unsigned long)0;
      printf("1677: (_Bool)(unsigned long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0; float r1 = (float)v; float r2 = (float)(unsigned long)0; static float r3 = (float)(unsigned long)0;
      printf("1678: (float)(unsigned long)0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)0; double r1 = (double)v; double r2 = (double)(unsigned long)0; static double r3 = (double)(unsigned long)0;
      printf("1679: (double)(unsigned long)0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)0; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)0; static long double r3 = (long double)(unsigned long)0;
      printf("1680: (long double)(unsigned long)0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)1; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)1; static signed char r3 = (signed char)(unsigned long)1;
      printf("1681: (signed char)(unsigned long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)1; static unsigned char r3 = (unsigned char)(unsigned long)1;
      printf("1682: (unsigned char)(unsigned long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)1; short r1 = (short)v; short r2 = (short)(unsigned long)1; static short r3 = (short)(unsigned long)1;
      printf("1683: (short)(unsigned long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)1; static unsigned short r3 = (unsigned short)(unsigned long)1;
      printf("1684: (unsigned short)(unsigned long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)1; int r1 = (int)v; int r2 = (int)(unsigned long)1; static int r3 = (int)(unsigned long)1;
      printf("1685: (int)(unsigned long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)1; static unsigned r3 = (unsigned)(unsigned long)1;
      printf("1686: (unsigned)(unsigned long)1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)1; long r1 = (long)v; long r2 = (long)(unsigned long)1; static long r3 = (long)(unsigned long)1;
      printf("1687: (long)(unsigned long)1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)1; static unsigned long r3 = (unsigned long)(unsigned long)1;
      printf("1688: (unsigned long)(unsigned long)1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)1; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)1; static long long r3 = (long long)(unsigned long)1;
      printf("1689: (long long)(unsigned long)1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)1; static unsigned long long r3 = (unsigned long long)(unsigned long)1;
      printf("1690: (unsigned long long)(unsigned long)1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)1; static _Bool r3 = (_Bool)(unsigned long)1;
      printf("1691: (_Bool)(unsigned long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)1; float r1 = (float)v; float r2 = (float)(unsigned long)1; static float r3 = (float)(unsigned long)1;
      printf("1692: (float)(unsigned long)1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)1; double r1 = (double)v; double r2 = (double)(unsigned long)1; static double r3 = (double)(unsigned long)1;
      printf("1693: (double)(unsigned long)1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)1; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)1; static long double r3 = (long double)(unsigned long)1;
      printf("1694: (long double)(unsigned long)1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)-1; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)-1; static signed char r3 = (signed char)(unsigned long)-1;
      printf("1695: (signed char)(unsigned long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)-1; static unsigned char r3 = (unsigned char)(unsigned long)-1;
      printf("1696: (unsigned char)(unsigned long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-1; short r1 = (short)v; short r2 = (short)(unsigned long)-1; static short r3 = (short)(unsigned long)-1;
      printf("1697: (short)(unsigned long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)-1; static unsigned short r3 = (unsigned short)(unsigned long)-1;
      printf("1698: (unsigned short)(unsigned long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-1; int r1 = (int)v; int r2 = (int)(unsigned long)-1; static int r3 = (int)(unsigned long)-1;
      printf("1699: (int)(unsigned long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)-1; static unsigned r3 = (unsigned)(unsigned long)-1;
      printf("1700: (unsigned)(unsigned long)-1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)-1; long r1 = (long)v; long r2 = (long)(unsigned long)-1; static long r3 = (long)(unsigned long)-1;
      printf("1701: (long)(unsigned long)-1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)-1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)-1; static unsigned long r3 = (unsigned long)(unsigned long)-1;
      printf("1702: (unsigned long)(unsigned long)-1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)-1; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)-1; static long long r3 = (long long)(unsigned long)-1;
      printf("1703: (long long)(unsigned long)-1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)-1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)-1; static unsigned long long r3 = (unsigned long long)(unsigned long)-1;
      printf("1704: (unsigned long long)(unsigned long)-1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)-1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)-1; static _Bool r3 = (_Bool)(unsigned long)-1;
      printf("1705: (_Bool)(unsigned long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-1; float r1 = (float)v; float r2 = (float)(unsigned long)-1; static float r3 = (float)(unsigned long)-1;
      printf("1706: (float)(unsigned long)-1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)-1; double r1 = (double)v; double r2 = (double)(unsigned long)-1; static double r3 = (double)(unsigned long)-1;
      printf("1707: (double)(unsigned long)-1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)-1; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)-1; static long double r3 = (long double)(unsigned long)-1;
      printf("1708: (long double)(unsigned long)-1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)127; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)127; static signed char r3 = (signed char)(unsigned long)127;
      printf("1709: (signed char)(unsigned long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)127; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)127; static unsigned char r3 = (unsigned char)(unsigned long)127;
      printf("1710: (unsigned char)(unsigned long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)127; short r1 = (short)v; short r2 = (short)(unsigned long)127; static short r3 = (short)(unsigned long)127;
      printf("1711: (short)(unsigned long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)127; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)127; static unsigned short r3 = (unsigned short)(unsigned long)127;
      printf("1712: (unsigned short)(unsigned long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)127; int r1 = (int)v; int r2 = (int)(unsigned long)127; static int r3 = (int)(unsigned long)127;
      printf("1713: (int)(unsigned long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)127; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)127; static unsigned r3 = (unsigned)(unsigned long)127;
      printf("1714: (unsigned)(unsigned long)127 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)127; long r1 = (long)v; long r2 = (long)(unsigned long)127; static long r3 = (long)(unsigned long)127;
      printf("1715: (long)(unsigned long)127 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)127; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)127; static unsigned long r3 = (unsigned long)(unsigned long)127;
      printf("1716: (unsigned long)(unsigned long)127 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)127; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)127; static long long r3 = (long long)(unsigned long)127;
      printf("1717: (long long)(unsigned long)127 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)127; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)127; static unsigned long long r3 = (unsigned long long)(unsigned long)127;
      printf("1718: (unsigned long long)(unsigned long)127 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)127; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)127; static _Bool r3 = (_Bool)(unsigned long)127;
      printf("1719: (_Bool)(unsigned long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)127; float r1 = (float)v; float r2 = (float)(unsigned long)127; static float r3 = (float)(unsigned long)127;
      printf("1720: (float)(unsigned long)127 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)127; double r1 = (double)v; double r2 = (double)(unsigned long)127; static double r3 = (double)(unsigned long)127;
      printf("1721: (double)(unsigned long)127 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)127; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)127; static long double r3 = (long double)(unsigned long)127;
      printf("1722: (long double)(unsigned long)127 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)-128; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)-128; static signed char r3 = (signed char)(unsigned long)-128;
      printf("1723: (signed char)(unsigned long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-128; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)-128; static unsigned char r3 = (unsigned char)(unsigned long)-128;
      printf("1724: (unsigned char)(unsigned long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-128; short r1 = (short)v; short r2 = (short)(unsigned long)-128; static short r3 = (short)(unsigned long)-128;
      printf("1725: (short)(unsigned long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-128; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)-128; static unsigned short r3 = (unsigned short)(unsigned long)-128;
      printf("1726: (unsigned short)(unsigned long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-128; int r1 = (int)v; int r2 = (int)(unsigned long)-128; static int r3 = (int)(unsigned long)-128;
      printf("1727: (int)(unsigned long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-128; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)-128; static unsigned r3 = (unsigned)(unsigned long)-128;
      printf("1728: (unsigned)(unsigned long)-128 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)-128; long r1 = (long)v; long r2 = (long)(unsigned long)-128; static long r3 = (long)(unsigned long)-128;
      printf("1729: (long)(unsigned long)-128 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)-128; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)-128; static unsigned long r3 = (unsigned long)(unsigned long)-128;
      printf("1730: (unsigned long)(unsigned long)-128 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)-128; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)-128; static long long r3 = (long long)(unsigned long)-128;
      printf("1731: (long long)(unsigned long)-128 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)-128; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)-128; static unsigned long long r3 = (unsigned long long)(unsigned long)-128;
      printf("1732: (unsigned long long)(unsigned long)-128 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)-128; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)-128; static _Bool r3 = (_Bool)(unsigned long)-128;
      printf("1733: (_Bool)(unsigned long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-128; float r1 = (float)v; float r2 = (float)(unsigned long)-128; static float r3 = (float)(unsigned long)-128;
      printf("1734: (float)(unsigned long)-128 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)-128; double r1 = (double)v; double r2 = (double)(unsigned long)-128; static double r3 = (double)(unsigned long)-128;
      printf("1735: (double)(unsigned long)-128 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)-128; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)-128; static long double r3 = (long double)(unsigned long)-128;
      printf("1736: (long double)(unsigned long)-128 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)255; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)255; static signed char r3 = (signed char)(unsigned long)255;
      printf("1737: (signed char)(unsigned long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)255; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)255; static unsigned char r3 = (unsigned char)(unsigned long)255;
      printf("1738: (unsigned char)(unsigned long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)255; short r1 = (short)v; short r2 = (short)(unsigned long)255; static short r3 = (short)(unsigned long)255;
      printf("1739: (short)(unsigned long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)255; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)255; static unsigned short r3 = (unsigned short)(unsigned long)255;
      printf("1740: (unsigned short)(unsigned long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)255; int r1 = (int)v; int r2 = (int)(unsigned long)255; static int r3 = (int)(unsigned long)255;
      printf("1741: (int)(unsigned long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)255; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)255; static unsigned r3 = (unsigned)(unsigned long)255;
      printf("1742: (unsigned)(unsigned long)255 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)255; long r1 = (long)v; long r2 = (long)(unsigned long)255; static long r3 = (long)(unsigned long)255;
      printf("1743: (long)(unsigned long)255 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)255; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)255; static unsigned long r3 = (unsigned long)(unsigned long)255;
      printf("1744: (unsigned long)(unsigned long)255 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)255; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)255; static long long r3 = (long long)(unsigned long)255;
      printf("1745: (long long)(unsigned long)255 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)255; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)255; static unsigned long long r3 = (unsigned long long)(unsigned long)255;
      printf("1746: (unsigned long long)(unsigned long)255 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)255; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)255; static _Bool r3 = (_Bool)(unsigned long)255;
      printf("1747: (_Bool)(unsigned long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)255; float r1 = (float)v; float r2 = (float)(unsigned long)255; static float r3 = (float)(unsigned long)255;
      printf("1748: (float)(unsigned long)255 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)255; double r1 = (double)v; double r2 = (double)(unsigned long)255; static double r3 = (double)(unsigned long)255;
      printf("1749: (double)(unsigned long)255 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)255; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)255; static long double r3 = (long double)(unsigned long)255;
      printf("1750: (long double)(unsigned long)255 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)256; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)256; static signed char r3 = (signed char)(unsigned long)256;
      printf("1751: (signed char)(unsigned long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)256; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)256; static unsigned char r3 = (unsigned char)(unsigned long)256;
      printf("1752: (unsigned char)(unsigned long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)256; short r1 = (short)v; short r2 = (short)(unsigned long)256; static short r3 = (short)(unsigned long)256;
      printf("1753: (short)(unsigned long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)256; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)256; static unsigned short r3 = (unsigned short)(unsigned long)256;
      printf("1754: (unsigned short)(unsigned long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)256; int r1 = (int)v; int r2 = (int)(unsigned long)256; static int r3 = (int)(unsigned long)256;
      printf("1755: (int)(unsigned long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)256; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)256; static unsigned r3 = (unsigned)(unsigned long)256;
      printf("1756: (unsigned)(unsigned long)256 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)256; long r1 = (long)v; long r2 = (long)(unsigned long)256; static long r3 = (long)(unsigned long)256;
      printf("1757: (long)(unsigned long)256 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)256; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)256; static unsigned long r3 = (unsigned long)(unsigned long)256;
      printf("1758: (unsigned long)(unsigned long)256 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)256; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)256; static long long r3 = (long long)(unsigned long)256;
      printf("1759: (long long)(unsigned long)256 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)256; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)256; static unsigned long long r3 = (unsigned long long)(unsigned long)256;
      printf("1760: (unsigned long long)(unsigned long)256 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)256; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)256; static _Bool r3 = (_Bool)(unsigned long)256;
      printf("1761: (_Bool)(unsigned long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)256; float r1 = (float)v; float r2 = (float)(unsigned long)256; static float r3 = (float)(unsigned long)256;
      printf("1762: (float)(unsigned long)256 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)256; double r1 = (double)v; double r2 = (double)(unsigned long)256; static double r3 = (double)(unsigned long)256;
      printf("1763: (double)(unsigned long)256 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)256; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)256; static long double r3 = (long double)(unsigned long)256;
      printf("1764: (long double)(unsigned long)256 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)32767; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)32767; static signed char r3 = (signed char)(unsigned long)32767;
      printf("1765: (signed char)(unsigned long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)32767; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)32767; static unsigned char r3 = (unsigned char)(unsigned long)32767;
      printf("1766: (unsigned char)(unsigned long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)32767; short r1 = (short)v; short r2 = (short)(unsigned long)32767; static short r3 = (short)(unsigned long)32767;
      printf("1767: (short)(unsigned long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)32767; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)32767; static unsigned short r3 = (unsigned short)(unsigned long)32767;
      printf("1768: (unsigned short)(unsigned long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)32767; int r1 = (int)v; int r2 = (int)(unsigned long)32767; static int r3 = (int)(unsigned long)32767;
      printf("1769: (int)(unsigned long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)32767; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)32767; static unsigned r3 = (unsigned)(unsigned long)32767;
      printf("1770: (unsigned)(unsigned long)32767 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)32767; long r1 = (long)v; long r2 = (long)(unsigned long)32767; static long r3 = (long)(unsigned long)32767;
      printf("1771: (long)(unsigned long)32767 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)32767; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)32767; static unsigned long r3 = (unsigned long)(unsigned long)32767;
      printf("1772: (unsigned long)(unsigned long)32767 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)32767; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)32767; static long long r3 = (long long)(unsigned long)32767;
      printf("1773: (long long)(unsigned long)32767 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)32767; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)32767; static unsigned long long r3 = (unsigned long long)(unsigned long)32767;
      printf("1774: (unsigned long long)(unsigned long)32767 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)32767; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)32767; static _Bool r3 = (_Bool)(unsigned long)32767;
      printf("1775: (_Bool)(unsigned long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)32767; float r1 = (float)v; float r2 = (float)(unsigned long)32767; static float r3 = (float)(unsigned long)32767;
      printf("1776: (float)(unsigned long)32767 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)32767; double r1 = (double)v; double r2 = (double)(unsigned long)32767; static double r3 = (double)(unsigned long)32767;
      printf("1777: (double)(unsigned long)32767 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)32767; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)32767; static long double r3 = (long double)(unsigned long)32767;
      printf("1778: (long double)(unsigned long)32767 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)-32768; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)-32768; static signed char r3 = (signed char)(unsigned long)-32768;
      printf("1779: (signed char)(unsigned long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-32768; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)-32768; static unsigned char r3 = (unsigned char)(unsigned long)-32768;
      printf("1780: (unsigned char)(unsigned long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-32768; short r1 = (short)v; short r2 = (short)(unsigned long)-32768; static short r3 = (short)(unsigned long)-32768;
      printf("1781: (short)(unsigned long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-32768; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)-32768; static unsigned short r3 = (unsigned short)(unsigned long)-32768;
      printf("1782: (unsigned short)(unsigned long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-32768; int r1 = (int)v; int r2 = (int)(unsigned long)-32768; static int r3 = (int)(unsigned long)-32768;
      printf("1783: (int)(unsigned long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-32768; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)-32768; static unsigned r3 = (unsigned)(unsigned long)-32768;
      printf("1784: (unsigned)(unsigned long)-32768 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)-32768; long r1 = (long)v; long r2 = (long)(unsigned long)-32768; static long r3 = (long)(unsigned long)-32768;
      printf("1785: (long)(unsigned long)-32768 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)-32768; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)-32768; static unsigned long r3 = (unsigned long)(unsigned long)-32768;
      printf("1786: (unsigned long)(unsigned long)-32768 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)-32768; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)-32768; static long long r3 = (long long)(unsigned long)-32768;
      printf("1787: (long long)(unsigned long)-32768 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)-32768; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)-32768; static unsigned long long r3 = (unsigned long long)(unsigned long)-32768;
      printf("1788: (unsigned long long)(unsigned long)-32768 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)-32768; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)-32768; static _Bool r3 = (_Bool)(unsigned long)-32768;
      printf("1789: (_Bool)(unsigned long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)-32768; float r1 = (float)v; float r2 = (float)(unsigned long)-32768; static float r3 = (float)(unsigned long)-32768;
      printf("1790: (float)(unsigned long)-32768 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)-32768; double r1 = (double)v; double r2 = (double)(unsigned long)-32768; static double r3 = (double)(unsigned long)-32768;
      printf("1791: (double)(unsigned long)-32768 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)-32768; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)-32768; static long double r3 = (long double)(unsigned long)-32768;
      printf("1792: (long double)(unsigned long)-32768 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)65535; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)65535; static signed char r3 = (signed char)(unsigned long)65535;
      printf("1793: (signed char)(unsigned long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)65535; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)65535; static unsigned char r3 = (unsigned char)(unsigned long)65535;
      printf("1794: (unsigned char)(unsigned long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)65535; short r1 = (short)v; short r2 = (short)(unsigned long)65535; static short r3 = (short)(unsigned long)65535;
      printf("1795: (short)(unsigned long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)65535; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)65535; static unsigned short r3 = (unsigned short)(unsigned long)65535;
      printf("1796: (unsigned short)(unsigned long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)65535; int r1 = (int)v; int r2 = (int)(unsigned long)65535; static int r3 = (int)(unsigned long)65535;
      printf("1797: (int)(unsigned long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)65535; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)65535; static unsigned r3 = (unsigned)(unsigned long)65535;
      printf("1798: (unsigned)(unsigned long)65535 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)65535; long r1 = (long)v; long r2 = (long)(unsigned long)65535; static long r3 = (long)(unsigned long)65535;
      printf("1799: (long)(unsigned long)65535 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)65535; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)65535; static unsigned long r3 = (unsigned long)(unsigned long)65535;
      printf("1800: (unsigned long)(unsigned long)65535 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)65535; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)65535; static long long r3 = (long long)(unsigned long)65535;
      printf("1801: (long long)(unsigned long)65535 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)65535; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)65535; static unsigned long long r3 = (unsigned long long)(unsigned long)65535;
      printf("1802: (unsigned long long)(unsigned long)65535 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)65535; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)65535; static _Bool r3 = (_Bool)(unsigned long)65535;
      printf("1803: (_Bool)(unsigned long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)65535; float r1 = (float)v; float r2 = (float)(unsigned long)65535; static float r3 = (float)(unsigned long)65535;
      printf("1804: (float)(unsigned long)65535 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)65535; double r1 = (double)v; double r2 = (double)(unsigned long)65535; static double r3 = (double)(unsigned long)65535;
      printf("1805: (double)(unsigned long)65535 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)65535; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)65535; static long double r3 = (long double)(unsigned long)65535;
      printf("1806: (long double)(unsigned long)65535 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)2147483647; static signed char r3 = (signed char)(unsigned long)2147483647;
      printf("1807: (signed char)(unsigned long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)2147483647; static unsigned char r3 = (unsigned char)(unsigned long)2147483647;
      printf("1808: (unsigned char)(unsigned long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; short r1 = (short)v; short r2 = (short)(unsigned long)2147483647; static short r3 = (short)(unsigned long)2147483647;
      printf("1809: (short)(unsigned long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)2147483647; static unsigned short r3 = (unsigned short)(unsigned long)2147483647;
      printf("1810: (unsigned short)(unsigned long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; int r1 = (int)v; int r2 = (int)(unsigned long)2147483647; static int r3 = (int)(unsigned long)2147483647;
      printf("1811: (int)(unsigned long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)2147483647; static unsigned r3 = (unsigned)(unsigned long)2147483647;
      printf("1812: (unsigned)(unsigned long)2147483647 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; long r1 = (long)v; long r2 = (long)(unsigned long)2147483647; static long r3 = (long)(unsigned long)2147483647;
      printf("1813: (long)(unsigned long)2147483647 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)2147483647; static unsigned long r3 = (unsigned long)(unsigned long)2147483647;
      printf("1814: (unsigned long)(unsigned long)2147483647 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)2147483647; static long long r3 = (long long)(unsigned long)2147483647;
      printf("1815: (long long)(unsigned long)2147483647 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)2147483647; static unsigned long long r3 = (unsigned long long)(unsigned long)2147483647;
      printf("1816: (unsigned long long)(unsigned long)2147483647 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)2147483647; static _Bool r3 = (_Bool)(unsigned long)2147483647;
      printf("1817: (_Bool)(unsigned long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; float r1 = (float)v; float r2 = (float)(unsigned long)2147483647; static float r3 = (float)(unsigned long)2147483647;
      printf("1818: (float)(unsigned long)2147483647 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; double r1 = (double)v; double r2 = (double)(unsigned long)2147483647; static double r3 = (double)(unsigned long)2147483647;
      printf("1819: (double)(unsigned long)2147483647 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)2147483647; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)2147483647; static long double r3 = (long double)(unsigned long)2147483647;
      printf("1820: (long double)(unsigned long)2147483647 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)(-2147483647-1); static signed char r3 = (signed char)(unsigned long)(-2147483647-1);
      printf("1821: (signed char)(unsigned long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)(-2147483647-1); static unsigned char r3 = (unsigned char)(unsigned long)(-2147483647-1);
      printf("1822: (unsigned char)(unsigned long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); short r1 = (short)v; short r2 = (short)(unsigned long)(-2147483647-1); static short r3 = (short)(unsigned long)(-2147483647-1);
      printf("1823: (short)(unsigned long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)(-2147483647-1); static unsigned short r3 = (unsigned short)(unsigned long)(-2147483647-1);
      printf("1824: (unsigned short)(unsigned long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); int r1 = (int)v; int r2 = (int)(unsigned long)(-2147483647-1); static int r3 = (int)(unsigned long)(-2147483647-1);
      printf("1825: (int)(unsigned long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)(-2147483647-1); static unsigned r3 = (unsigned)(unsigned long)(-2147483647-1);
      printf("1826: (unsigned)(unsigned long)(-2147483647-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); long r1 = (long)v; long r2 = (long)(unsigned long)(-2147483647-1); static long r3 = (long)(unsigned long)(-2147483647-1);
      printf("1827: (long)(unsigned long)(-2147483647-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)(-2147483647-1); static unsigned long r3 = (unsigned long)(unsigned long)(-2147483647-1);
      printf("1828: (unsigned long)(unsigned long)(-2147483647-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); long long r1 = (long long)v; long long r2 = (long long)(unsigned long)(-2147483647-1); static long long r3 = (long long)(unsigned long)(-2147483647-1);
      printf("1829: (long long)(unsigned long)(-2147483647-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)(-2147483647-1); static unsigned long long r3 = (unsigned long long)(unsigned long)(-2147483647-1);
      printf("1830: (unsigned long long)(unsigned long)(-2147483647-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)(-2147483647-1); static _Bool r3 = (_Bool)(unsigned long)(-2147483647-1);
      printf("1831: (_Bool)(unsigned long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); float r1 = (float)v; float r2 = (float)(unsigned long)(-2147483647-1); static float r3 = (float)(unsigned long)(-2147483647-1);
      printf("1832: (float)(unsigned long)(-2147483647-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); double r1 = (double)v; double r2 = (double)(unsigned long)(-2147483647-1); static double r3 = (double)(unsigned long)(-2147483647-1);
      printf("1833: (double)(unsigned long)(-2147483647-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)(-2147483647-1); long double r1 = (long double)v; long double r2 = (long double)(unsigned long)(-2147483647-1); static long double r3 = (long double)(unsigned long)(-2147483647-1);
      printf("1834: (long double)(unsigned long)(-2147483647-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)4294967295LL; static signed char r3 = (signed char)(unsigned long)4294967295LL;
      printf("1835: (signed char)(unsigned long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)4294967295LL; static unsigned char r3 = (unsigned char)(unsigned long)4294967295LL;
      printf("1836: (unsigned char)(unsigned long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; short r1 = (short)v; short r2 = (short)(unsigned long)4294967295LL; static short r3 = (short)(unsigned long)4294967295LL;
      printf("1837: (short)(unsigned long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)4294967295LL; static unsigned short r3 = (unsigned short)(unsigned long)4294967295LL;
      printf("1838: (unsigned short)(unsigned long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; int r1 = (int)v; int r2 = (int)(unsigned long)4294967295LL; static int r3 = (int)(unsigned long)4294967295LL;
      printf("1839: (int)(unsigned long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)4294967295LL; static unsigned r3 = (unsigned)(unsigned long)4294967295LL;
      printf("1840: (unsigned)(unsigned long)4294967295LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; long r1 = (long)v; long r2 = (long)(unsigned long)4294967295LL; static long r3 = (long)(unsigned long)4294967295LL;
      printf("1841: (long)(unsigned long)4294967295LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)4294967295LL; static unsigned long r3 = (unsigned long)(unsigned long)4294967295LL;
      printf("1842: (unsigned long)(unsigned long)4294967295LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)4294967295LL; static long long r3 = (long long)(unsigned long)4294967295LL;
      printf("1843: (long long)(unsigned long)4294967295LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)4294967295LL; static unsigned long long r3 = (unsigned long long)(unsigned long)4294967295LL;
      printf("1844: (unsigned long long)(unsigned long)4294967295LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)4294967295LL; static _Bool r3 = (_Bool)(unsigned long)4294967295LL;
      printf("1845: (_Bool)(unsigned long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; float r1 = (float)v; float r2 = (float)(unsigned long)4294967295LL; static float r3 = (float)(unsigned long)4294967295LL;
      printf("1846: (float)(unsigned long)4294967295LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; double r1 = (double)v; double r2 = (double)(unsigned long)4294967295LL; static double r3 = (double)(unsigned long)4294967295LL;
      printf("1847: (double)(unsigned long)4294967295LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)4294967295LL; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)4294967295LL; static long double r3 = (long double)(unsigned long)4294967295LL;
      printf("1848: (long double)(unsigned long)4294967295LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)9223372036854775807LL; static signed char r3 = (signed char)(unsigned long)9223372036854775807LL;
      printf("1849: (signed char)(unsigned long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)9223372036854775807LL; static unsigned char r3 = (unsigned char)(unsigned long)9223372036854775807LL;
      printf("1850: (unsigned char)(unsigned long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; short r1 = (short)v; short r2 = (short)(unsigned long)9223372036854775807LL; static short r3 = (short)(unsigned long)9223372036854775807LL;
      printf("1851: (short)(unsigned long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)9223372036854775807LL; static unsigned short r3 = (unsigned short)(unsigned long)9223372036854775807LL;
      printf("1852: (unsigned short)(unsigned long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; int r1 = (int)v; int r2 = (int)(unsigned long)9223372036854775807LL; static int r3 = (int)(unsigned long)9223372036854775807LL;
      printf("1853: (int)(unsigned long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)9223372036854775807LL; static unsigned r3 = (unsigned)(unsigned long)9223372036854775807LL;
      printf("1854: (unsigned)(unsigned long)9223372036854775807LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; long r1 = (long)v; long r2 = (long)(unsigned long)9223372036854775807LL; static long r3 = (long)(unsigned long)9223372036854775807LL;
      printf("1855: (long)(unsigned long)9223372036854775807LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)9223372036854775807LL; static unsigned long r3 = (unsigned long)(unsigned long)9223372036854775807LL;
      printf("1856: (unsigned long)(unsigned long)9223372036854775807LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)9223372036854775807LL; static long long r3 = (long long)(unsigned long)9223372036854775807LL;
      printf("1857: (long long)(unsigned long)9223372036854775807LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)9223372036854775807LL; static unsigned long long r3 = (unsigned long long)(unsigned long)9223372036854775807LL;
      printf("1858: (unsigned long long)(unsigned long)9223372036854775807LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)9223372036854775807LL; static _Bool r3 = (_Bool)(unsigned long)9223372036854775807LL;
      printf("1859: (_Bool)(unsigned long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; float r1 = (float)v; float r2 = (float)(unsigned long)9223372036854775807LL; static float r3 = (float)(unsigned long)9223372036854775807LL;
      printf("1860: (float)(unsigned long)9223372036854775807LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; double r1 = (double)v; double r2 = (double)(unsigned long)9223372036854775807LL; static double r3 = (double)(unsigned long)9223372036854775807LL;
      printf("1861: (double)(unsigned long)9223372036854775807LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)9223372036854775807LL; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)9223372036854775807LL; static long double r3 = (long double)(unsigned long)9223372036854775807LL;
      printf("1862: (long double)(unsigned long)9223372036854775807LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)(-9223372036854775807LL-1); static signed char r3 = (signed char)(unsigned long)(-9223372036854775807LL-1);
      printf("1863: (signed char)(unsigned long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)(-9223372036854775807LL-1); static unsigned char r3 = (unsigned char)(unsigned long)(-9223372036854775807LL-1);
      printf("1864: (unsigned char)(unsigned long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); short r1 = (short)v; short r2 = (short)(unsigned long)(-9223372036854775807LL-1); static short r3 = (short)(unsigned long)(-9223372036854775807LL-1);
      printf("1865: (short)(unsigned long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)(-9223372036854775807LL-1); static unsigned short r3 = (unsigned short)(unsigned long)(-9223372036854775807LL-1);
      printf("1866: (unsigned short)(unsigned long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); int r1 = (int)v; int r2 = (int)(unsigned long)(-9223372036854775807LL-1); static int r3 = (int)(unsigned long)(-9223372036854775807LL-1);
      printf("1867: (int)(unsigned long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)(-9223372036854775807LL-1); static unsigned r3 = (unsigned)(unsigned long)(-9223372036854775807LL-1);
      printf("1868: (unsigned)(unsigned long)(-9223372036854775807LL-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); long r1 = (long)v; long r2 = (long)(unsigned long)(-9223372036854775807LL-1); static long r3 = (long)(unsigned long)(-9223372036854775807LL-1);
      printf("1869: (long)(unsigned long)(-9223372036854775807LL-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)(-9223372036854775807LL-1); static unsigned long r3 = (unsigned long)(unsigned long)(-9223372036854775807LL-1);
      printf("1870: (unsigned long)(unsigned long)(-9223372036854775807LL-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); long long r1 = (long long)v; long long r2 = (long long)(unsigned long)(-9223372036854775807LL-1); static long long r3 = (long long)(unsigned long)(-9223372036854775807LL-1);
      printf("1871: (long long)(unsigned long)(-9223372036854775807LL-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)(-9223372036854775807LL-1); static unsigned long long r3 = (unsigned long long)(unsigned long)(-9223372036854775807LL-1);
      printf("1872: (unsigned long long)(unsigned long)(-9223372036854775807LL-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)(-9223372036854775807LL-1); static _Bool r3 = (_Bool)(unsigned long)(-9223372036854775807LL-1);
      printf("1873: (_Bool)(unsigned long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); float r1 = (float)v; float r2 = (float)(unsigned long)(-9223372036854775807LL-1); static float r3 = (float)(unsigned long)(-9223372036854775807LL-1);
      printf("1874: (float)(unsigned long)(-9223372036854775807LL-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); double r1 = (double)v; double r2 = (double)(unsigned long)(-9223372036854775807LL-1); static double r3 = (double)(unsigned long)(-9223372036854775807LL-1);
      printf("1875: (double)(unsigned long)(-9223372036854775807LL-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)(-9223372036854775807LL-1); long double r1 = (long double)v; long double r2 = (long double)(unsigned long)(-9223372036854775807LL-1); static long double r3 = (long double)(unsigned long)(-9223372036854775807LL-1);
      printf("1876: (long double)(unsigned long)(-9223372036854775807LL-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)0x8000000000000000ULL; static signed char r3 = (signed char)(unsigned long)0x8000000000000000ULL;
      printf("1877: (signed char)(unsigned long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)0x8000000000000000ULL; static unsigned char r3 = (unsigned char)(unsigned long)0x8000000000000000ULL;
      printf("1878: (unsigned char)(unsigned long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; short r1 = (short)v; short r2 = (short)(unsigned long)0x8000000000000000ULL; static short r3 = (short)(unsigned long)0x8000000000000000ULL;
      printf("1879: (short)(unsigned long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)0x8000000000000000ULL; static unsigned short r3 = (unsigned short)(unsigned long)0x8000000000000000ULL;
      printf("1880: (unsigned short)(unsigned long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; int r1 = (int)v; int r2 = (int)(unsigned long)0x8000000000000000ULL; static int r3 = (int)(unsigned long)0x8000000000000000ULL;
      printf("1881: (int)(unsigned long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)0x8000000000000000ULL; static unsigned r3 = (unsigned)(unsigned long)0x8000000000000000ULL;
      printf("1882: (unsigned)(unsigned long)0x8000000000000000ULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; long r1 = (long)v; long r2 = (long)(unsigned long)0x8000000000000000ULL; static long r3 = (long)(unsigned long)0x8000000000000000ULL;
      printf("1883: (long)(unsigned long)0x8000000000000000ULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)0x8000000000000000ULL; static unsigned long r3 = (unsigned long)(unsigned long)0x8000000000000000ULL;
      printf("1884: (unsigned long)(unsigned long)0x8000000000000000ULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)0x8000000000000000ULL; static long long r3 = (long long)(unsigned long)0x8000000000000000ULL;
      printf("1885: (long long)(unsigned long)0x8000000000000000ULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)0x8000000000000000ULL; static unsigned long long r3 = (unsigned long long)(unsigned long)0x8000000000000000ULL;
      printf("1886: (unsigned long long)(unsigned long)0x8000000000000000ULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)0x8000000000000000ULL; static _Bool r3 = (_Bool)(unsigned long)0x8000000000000000ULL;
      printf("1887: (_Bool)(unsigned long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; float r1 = (float)v; float r2 = (float)(unsigned long)0x8000000000000000ULL; static float r3 = (float)(unsigned long)0x8000000000000000ULL;
      printf("1888: (float)(unsigned long)0x8000000000000000ULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; double r1 = (double)v; double r2 = (double)(unsigned long)0x8000000000000000ULL; static double r3 = (double)(unsigned long)0x8000000000000000ULL;
      printf("1889: (double)(unsigned long)0x8000000000000000ULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)0x8000000000000000ULL; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)0x8000000000000000ULL; static long double r3 = (long double)(unsigned long)0x8000000000000000ULL;
      printf("1890: (long double)(unsigned long)0x8000000000000000ULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long)0xffffffffffffffffULL; static signed char r3 = (signed char)(unsigned long)0xffffffffffffffffULL;
      printf("1891: (signed char)(unsigned long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long)0xffffffffffffffffULL; static unsigned char r3 = (unsigned char)(unsigned long)0xffffffffffffffffULL;
      printf("1892: (unsigned char)(unsigned long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; short r1 = (short)v; short r2 = (short)(unsigned long)0xffffffffffffffffULL; static short r3 = (short)(unsigned long)0xffffffffffffffffULL;
      printf("1893: (short)(unsigned long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long)0xffffffffffffffffULL; static unsigned short r3 = (unsigned short)(unsigned long)0xffffffffffffffffULL;
      printf("1894: (unsigned short)(unsigned long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; int r1 = (int)v; int r2 = (int)(unsigned long)0xffffffffffffffffULL; static int r3 = (int)(unsigned long)0xffffffffffffffffULL;
      printf("1895: (int)(unsigned long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long)0xffffffffffffffffULL; static unsigned r3 = (unsigned)(unsigned long)0xffffffffffffffffULL;
      printf("1896: (unsigned)(unsigned long)0xffffffffffffffffULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; long r1 = (long)v; long r2 = (long)(unsigned long)0xffffffffffffffffULL; static long r3 = (long)(unsigned long)0xffffffffffffffffULL;
      printf("1897: (long)(unsigned long)0xffffffffffffffffULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long)0xffffffffffffffffULL; static unsigned long r3 = (unsigned long)(unsigned long)0xffffffffffffffffULL;
      printf("1898: (unsigned long)(unsigned long)0xffffffffffffffffULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; long long r1 = (long long)v; long long r2 = (long long)(unsigned long)0xffffffffffffffffULL; static long long r3 = (long long)(unsigned long)0xffffffffffffffffULL;
      printf("1899: (long long)(unsigned long)0xffffffffffffffffULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long)0xffffffffffffffffULL; static unsigned long long r3 = (unsigned long long)(unsigned long)0xffffffffffffffffULL;
      printf("1900: (unsigned long long)(unsigned long)0xffffffffffffffffULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long)0xffffffffffffffffULL; static _Bool r3 = (_Bool)(unsigned long)0xffffffffffffffffULL;
      printf("1901: (_Bool)(unsigned long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; float r1 = (float)v; float r2 = (float)(unsigned long)0xffffffffffffffffULL; static float r3 = (float)(unsigned long)0xffffffffffffffffULL;
      printf("1902: (float)(unsigned long)0xffffffffffffffffULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; double r1 = (double)v; double r2 = (double)(unsigned long)0xffffffffffffffffULL; static double r3 = (double)(unsigned long)0xffffffffffffffffULL;
      printf("1903: (double)(unsigned long)0xffffffffffffffffULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long v = (unsigned long)0xffffffffffffffffULL; long double r1 = (long double)v; long double r2 = (long double)(unsigned long)0xffffffffffffffffULL; static long double r3 = (long double)(unsigned long)0xffffffffffffffffULL;
      printf("1904: (long double)(unsigned long)0xffffffffffffffffULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)0; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)0; static signed char r3 = (signed char)(long long)0;
      printf("1905: (signed char)(long long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)0; static unsigned char r3 = (unsigned char)(long long)0;
      printf("1906: (unsigned char)(long long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0; short r1 = (short)v; short r2 = (short)(long long)0; static short r3 = (short)(long long)0;
      printf("1907: (short)(long long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)0; static unsigned short r3 = (unsigned short)(long long)0;
      printf("1908: (unsigned short)(long long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0; int r1 = (int)v; int r2 = (int)(long long)0; static int r3 = (int)(long long)0;
      printf("1909: (int)(long long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)0; static unsigned r3 = (unsigned)(long long)0;
      printf("1910: (unsigned)(long long)0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)0; long r1 = (long)v; long r2 = (long)(long long)0; static long r3 = (long)(long long)0;
      printf("1911: (long)(long long)0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)0; static unsigned long r3 = (unsigned long)(long long)0;
      printf("1912: (unsigned long)(long long)0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)0; long long r1 = (long long)v; long long r2 = (long long)(long long)0; static long long r3 = (long long)(long long)0;
      printf("1913: (long long)(long long)0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)0; static unsigned long long r3 = (unsigned long long)(long long)0;
      printf("1914: (unsigned long long)(long long)0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)0; static _Bool r3 = (_Bool)(long long)0;
      printf("1915: (_Bool)(long long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0; float r1 = (float)v; float r2 = (float)(long long)0; static float r3 = (float)(long long)0;
      printf("1916: (float)(long long)0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)0; double r1 = (double)v; double r2 = (double)(long long)0; static double r3 = (double)(long long)0;
      printf("1917: (double)(long long)0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)0; long double r1 = (long double)v; long double r2 = (long double)(long long)0; static long double r3 = (long double)(long long)0;
      printf("1918: (long double)(long long)0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)1; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)1; static signed char r3 = (signed char)(long long)1;
      printf("1919: (signed char)(long long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)1; static unsigned char r3 = (unsigned char)(long long)1;
      printf("1920: (unsigned char)(long long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)1; short r1 = (short)v; short r2 = (short)(long long)1; static short r3 = (short)(long long)1;
      printf("1921: (short)(long long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)1; static unsigned short r3 = (unsigned short)(long long)1;
      printf("1922: (unsigned short)(long long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)1; int r1 = (int)v; int r2 = (int)(long long)1; static int r3 = (int)(long long)1;
      printf("1923: (int)(long long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)1; static unsigned r3 = (unsigned)(long long)1;
      printf("1924: (unsigned)(long long)1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)1; long r1 = (long)v; long r2 = (long)(long long)1; static long r3 = (long)(long long)1;
      printf("1925: (long)(long long)1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)1; static unsigned long r3 = (unsigned long)(long long)1;
      printf("1926: (unsigned long)(long long)1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)1; long long r1 = (long long)v; long long r2 = (long long)(long long)1; static long long r3 = (long long)(long long)1;
      printf("1927: (long long)(long long)1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)1; static unsigned long long r3 = (unsigned long long)(long long)1;
      printf("1928: (unsigned long long)(long long)1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)1; static _Bool r3 = (_Bool)(long long)1;
      printf("1929: (_Bool)(long long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)1; float r1 = (float)v; float r2 = (float)(long long)1; static float r3 = (float)(long long)1;
      printf("1930: (float)(long long)1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)1; double r1 = (double)v; double r2 = (double)(long long)1; static double r3 = (double)(long long)1;
      printf("1931: (double)(long long)1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)1; long double r1 = (long double)v; long double r2 = (long double)(long long)1; static long double r3 = (long double)(long long)1;
      printf("1932: (long double)(long long)1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)-1; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)-1; static signed char r3 = (signed char)(long long)-1;
      printf("1933: (signed char)(long long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)-1; static unsigned char r3 = (unsigned char)(long long)-1;
      printf("1934: (unsigned char)(long long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-1; short r1 = (short)v; short r2 = (short)(long long)-1; static short r3 = (short)(long long)-1;
      printf("1935: (short)(long long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)-1; static unsigned short r3 = (unsigned short)(long long)-1;
      printf("1936: (unsigned short)(long long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-1; int r1 = (int)v; int r2 = (int)(long long)-1; static int r3 = (int)(long long)-1;
      printf("1937: (int)(long long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)-1; static unsigned r3 = (unsigned)(long long)-1;
      printf("1938: (unsigned)(long long)-1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)-1; long r1 = (long)v; long r2 = (long)(long long)-1; static long r3 = (long)(long long)-1;
      printf("1939: (long)(long long)-1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)-1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)-1; static unsigned long r3 = (unsigned long)(long long)-1;
      printf("1940: (unsigned long)(long long)-1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)-1; long long r1 = (long long)v; long long r2 = (long long)(long long)-1; static long long r3 = (long long)(long long)-1;
      printf("1941: (long long)(long long)-1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)-1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)-1; static unsigned long long r3 = (unsigned long long)(long long)-1;
      printf("1942: (unsigned long long)(long long)-1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)-1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)-1; static _Bool r3 = (_Bool)(long long)-1;
      printf("1943: (_Bool)(long long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-1; float r1 = (float)v; float r2 = (float)(long long)-1; static float r3 = (float)(long long)-1;
      printf("1944: (float)(long long)-1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)-1; double r1 = (double)v; double r2 = (double)(long long)-1; static double r3 = (double)(long long)-1;
      printf("1945: (double)(long long)-1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)-1; long double r1 = (long double)v; long double r2 = (long double)(long long)-1; static long double r3 = (long double)(long long)-1;
      printf("1946: (long double)(long long)-1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)127; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)127; static signed char r3 = (signed char)(long long)127;
      printf("1947: (signed char)(long long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)127; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)127; static unsigned char r3 = (unsigned char)(long long)127;
      printf("1948: (unsigned char)(long long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)127; short r1 = (short)v; short r2 = (short)(long long)127; static short r3 = (short)(long long)127;
      printf("1949: (short)(long long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)127; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)127; static unsigned short r3 = (unsigned short)(long long)127;
      printf("1950: (unsigned short)(long long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)127; int r1 = (int)v; int r2 = (int)(long long)127; static int r3 = (int)(long long)127;
      printf("1951: (int)(long long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)127; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)127; static unsigned r3 = (unsigned)(long long)127;
      printf("1952: (unsigned)(long long)127 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)127; long r1 = (long)v; long r2 = (long)(long long)127; static long r3 = (long)(long long)127;
      printf("1953: (long)(long long)127 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)127; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)127; static unsigned long r3 = (unsigned long)(long long)127;
      printf("1954: (unsigned long)(long long)127 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)127; long long r1 = (long long)v; long long r2 = (long long)(long long)127; static long long r3 = (long long)(long long)127;
      printf("1955: (long long)(long long)127 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)127; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)127; static unsigned long long r3 = (unsigned long long)(long long)127;
      printf("1956: (unsigned long long)(long long)127 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)127; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)127; static _Bool r3 = (_Bool)(long long)127;
      printf("1957: (_Bool)(long long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)127; float r1 = (float)v; float r2 = (float)(long long)127; static float r3 = (float)(long long)127;
      printf("1958: (float)(long long)127 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)127; double r1 = (double)v; double r2 = (double)(long long)127; static double r3 = (double)(long long)127;
      printf("1959: (double)(long long)127 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)127; long double r1 = (long double)v; long double r2 = (long double)(long long)127; static long double r3 = (long double)(long long)127;
      printf("1960: (long double)(long long)127 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)-128; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)-128; static signed char r3 = (signed char)(long long)-128;
      printf("1961: (signed char)(long long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-128; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)-128; static unsigned char r3 = (unsigned char)(long long)-128;
      printf("1962: (unsigned char)(long long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-128; short r1 = (short)v; short r2 = (short)(long long)-128; static short r3 = (short)(long long)-128;
      printf("1963: (short)(long long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-128; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)-128; static unsigned short r3 = (unsigned short)(long long)-128;
      printf("1964: (unsigned short)(long long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-128; int r1 = (int)v; int r2 = (int)(long long)-128; static int r3 = (int)(long long)-128;
      printf("1965: (int)(long long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-128; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)-128; static unsigned r3 = (unsigned)(long long)-128;
      printf("1966: (unsigned)(long long)-128 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)-128; long r1 = (long)v; long r2 = (long)(long long)-128; static long r3 = (long)(long long)-128;
      printf("1967: (long)(long long)-128 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)-128; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)-128; static unsigned long r3 = (unsigned long)(long long)-128;
      printf("1968: (unsigned long)(long long)-128 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)-128; long long r1 = (long long)v; long long r2 = (long long)(long long)-128; static long long r3 = (long long)(long long)-128;
      printf("1969: (long long)(long long)-128 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)-128; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)-128; static unsigned long long r3 = (unsigned long long)(long long)-128;
      printf("1970: (unsigned long long)(long long)-128 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)-128; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)-128; static _Bool r3 = (_Bool)(long long)-128;
      printf("1971: (_Bool)(long long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-128; float r1 = (float)v; float r2 = (float)(long long)-128; static float r3 = (float)(long long)-128;
      printf("1972: (float)(long long)-128 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)-128; double r1 = (double)v; double r2 = (double)(long long)-128; static double r3 = (double)(long long)-128;
      printf("1973: (double)(long long)-128 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)-128; long double r1 = (long double)v; long double r2 = (long double)(long long)-128; static long double r3 = (long double)(long long)-128;
      printf("1974: (long double)(long long)-128 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)255; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)255; static signed char r3 = (signed char)(long long)255;
      printf("1975: (signed char)(long long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)255; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)255; static unsigned char r3 = (unsigned char)(long long)255;
      printf("1976: (unsigned char)(long long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)255; short r1 = (short)v; short r2 = (short)(long long)255; static short r3 = (short)(long long)255;
      printf("1977: (short)(long long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)255; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)255; static unsigned short r3 = (unsigned short)(long long)255;
      printf("1978: (unsigned short)(long long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)255; int r1 = (int)v; int r2 = (int)(long long)255; static int r3 = (int)(long long)255;
      printf("1979: (int)(long long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)255; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)255; static unsigned r3 = (unsigned)(long long)255;
      printf("1980: (unsigned)(long long)255 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)255; long r1 = (long)v; long r2 = (long)(long long)255; static long r3 = (long)(long long)255;
      printf("1981: (long)(long long)255 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)255; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)255; static unsigned long r3 = (unsigned long)(long long)255;
      printf("1982: (unsigned long)(long long)255 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)255; long long r1 = (long long)v; long long r2 = (long long)(long long)255; static long long r3 = (long long)(long long)255;
      printf("1983: (long long)(long long)255 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)255; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)255; static unsigned long long r3 = (unsigned long long)(long long)255;
      printf("1984: (unsigned long long)(long long)255 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)255; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)255; static _Bool r3 = (_Bool)(long long)255;
      printf("1985: (_Bool)(long long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)255; float r1 = (float)v; float r2 = (float)(long long)255; static float r3 = (float)(long long)255;
      printf("1986: (float)(long long)255 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)255; double r1 = (double)v; double r2 = (double)(long long)255; static double r3 = (double)(long long)255;
      printf("1987: (double)(long long)255 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)255; long double r1 = (long double)v; long double r2 = (long double)(long long)255; static long double r3 = (long double)(long long)255;
      printf("1988: (long double)(long long)255 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)256; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)256; static signed char r3 = (signed char)(long long)256;
      printf("1989: (signed char)(long long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)256; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)256; static unsigned char r3 = (unsigned char)(long long)256;
      printf("1990: (unsigned char)(long long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)256; short r1 = (short)v; short r2 = (short)(long long)256; static short r3 = (short)(long long)256;
      printf("1991: (short)(long long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)256; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)256; static unsigned short r3 = (unsigned short)(long long)256;
      printf("1992: (unsigned short)(long long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)256; int r1 = (int)v; int r2 = (int)(long long)256; static int r3 = (int)(long long)256;
      printf("1993: (int)(long long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)256; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)256; static unsigned r3 = (unsigned)(long long)256;
      printf("1994: (unsigned)(long long)256 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)256; long r1 = (long)v; long r2 = (long)(long long)256; static long r3 = (long)(long long)256;
      printf("1995: (long)(long long)256 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)256; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)256; static unsigned long r3 = (unsigned long)(long long)256;
      printf("1996: (unsigned long)(long long)256 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)256; long long r1 = (long long)v; long long r2 = (long long)(long long)256; static long long r3 = (long long)(long long)256;
      printf("1997: (long long)(long long)256 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)256; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)256; static unsigned long long r3 = (unsigned long long)(long long)256;
      printf("1998: (unsigned long long)(long long)256 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)256; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)256; static _Bool r3 = (_Bool)(long long)256;
      printf("1999: (_Bool)(long long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)256; float r1 = (float)v; float r2 = (float)(long long)256; static float r3 = (float)(long long)256;
      printf("2000: (float)(long long)256 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)256; double r1 = (double)v; double r2 = (double)(long long)256; static double r3 = (double)(long long)256;
      printf("2001: (double)(long long)256 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)256; long double r1 = (long double)v; long double r2 = (long double)(long long)256; static long double r3 = (long double)(long long)256;
      printf("2002: (long double)(long long)256 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)32767; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)32767; static signed char r3 = (signed char)(long long)32767;
      printf("2003: (signed char)(long long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)32767; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)32767; static unsigned char r3 = (unsigned char)(long long)32767;
      printf("2004: (unsigned char)(long long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)32767; short r1 = (short)v; short r2 = (short)(long long)32767; static short r3 = (short)(long long)32767;
      printf("2005: (short)(long long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)32767; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)32767; static unsigned short r3 = (unsigned short)(long long)32767;
      printf("2006: (unsigned short)(long long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)32767; int r1 = (int)v; int r2 = (int)(long long)32767; static int r3 = (int)(long long)32767;
      printf("2007: (int)(long long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)32767; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)32767; static unsigned r3 = (unsigned)(long long)32767;
      printf("2008: (unsigned)(long long)32767 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)32767; long r1 = (long)v; long r2 = (long)(long long)32767; static long r3 = (long)(long long)32767;
      printf("2009: (long)(long long)32767 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)32767; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)32767; static unsigned long r3 = (unsigned long)(long long)32767;
      printf("2010: (unsigned long)(long long)32767 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)32767; long long r1 = (long long)v; long long r2 = (long long)(long long)32767; static long long r3 = (long long)(long long)32767;
      printf("2011: (long long)(long long)32767 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)32767; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)32767; static unsigned long long r3 = (unsigned long long)(long long)32767;
      printf("2012: (unsigned long long)(long long)32767 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)32767; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)32767; static _Bool r3 = (_Bool)(long long)32767;
      printf("2013: (_Bool)(long long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)32767; float r1 = (float)v; float r2 = (float)(long long)32767; static float r3 = (float)(long long)32767;
      printf("2014: (float)(long long)32767 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)32767; double r1 = (double)v; double r2 = (double)(long long)32767; static double r3 = (double)(long long)32767;
      printf("2015: (double)(long long)32767 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)32767; long double r1 = (long double)v; long double r2 = (long double)(long long)32767; static long double r3 = (long double)(long long)32767;
      printf("2016: (long double)(long long)32767 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)-32768; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)-32768; static signed char r3 = (signed char)(long long)-32768;
      printf("2017: (signed char)(long long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-32768; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)-32768; static unsigned char r3 = (unsigned char)(long long)-32768;
      printf("2018: (unsigned char)(long long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-32768; short r1 = (short)v; short r2 = (short)(long long)-32768; static short r3 = (short)(long long)-32768;
      printf("2019: (short)(long long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-32768; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)-32768; static unsigned short r3 = (unsigned short)(long long)-32768;
      printf("2020: (unsigned short)(long long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-32768; int r1 = (int)v; int r2 = (int)(long long)-32768; static int r3 = (int)(long long)-32768;
      printf("2021: (int)(long long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-32768; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)-32768; static unsigned r3 = (unsigned)(long long)-32768;
      printf("2022: (unsigned)(long long)-32768 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)-32768; long r1 = (long)v; long r2 = (long)(long long)-32768; static long r3 = (long)(long long)-32768;
      printf("2023: (long)(long long)-32768 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)-32768; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)-32768; static unsigned long r3 = (unsigned long)(long long)-32768;
      printf("2024: (unsigned long)(long long)-32768 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)-32768; long long r1 = (long long)v; long long r2 = (long long)(long long)-32768; static long long r3 = (long long)(long long)-32768;
      printf("2025: (long long)(long long)-32768 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)-32768; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)-32768; static unsigned long long r3 = (unsigned long long)(long long)-32768;
      printf("2026: (unsigned long long)(long long)-32768 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)-32768; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)-32768; static _Bool r3 = (_Bool)(long long)-32768;
      printf("2027: (_Bool)(long long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)-32768; float r1 = (float)v; float r2 = (float)(long long)-32768; static float r3 = (float)(long long)-32768;
      printf("2028: (float)(long long)-32768 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)-32768; double r1 = (double)v; double r2 = (double)(long long)-32768; static double r3 = (double)(long long)-32768;
      printf("2029: (double)(long long)-32768 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)-32768; long double r1 = (long double)v; long double r2 = (long double)(long long)-32768; static long double r3 = (long double)(long long)-32768;
      printf("2030: (long double)(long long)-32768 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)65535; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)65535; static signed char r3 = (signed char)(long long)65535;
      printf("2031: (signed char)(long long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)65535; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)65535; static unsigned char r3 = (unsigned char)(long long)65535;
      printf("2032: (unsigned char)(long long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)65535; short r1 = (short)v; short r2 = (short)(long long)65535; static short r3 = (short)(long long)65535;
      printf("2033: (short)(long long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)65535; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)65535; static unsigned short r3 = (unsigned short)(long long)65535;
      printf("2034: (unsigned short)(long long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)65535; int r1 = (int)v; int r2 = (int)(long long)65535; static int r3 = (int)(long long)65535;
      printf("2035: (int)(long long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)65535; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)65535; static unsigned r3 = (unsigned)(long long)65535;
      printf("2036: (unsigned)(long long)65535 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)65535; long r1 = (long)v; long r2 = (long)(long long)65535; static long r3 = (long)(long long)65535;
      printf("2037: (long)(long long)65535 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)65535; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)65535; static unsigned long r3 = (unsigned long)(long long)65535;
      printf("2038: (unsigned long)(long long)65535 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)65535; long long r1 = (long long)v; long long r2 = (long long)(long long)65535; static long long r3 = (long long)(long long)65535;
      printf("2039: (long long)(long long)65535 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)65535; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)65535; static unsigned long long r3 = (unsigned long long)(long long)65535;
      printf("2040: (unsigned long long)(long long)65535 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)65535; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)65535; static _Bool r3 = (_Bool)(long long)65535;
      printf("2041: (_Bool)(long long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)65535; float r1 = (float)v; float r2 = (float)(long long)65535; static float r3 = (float)(long long)65535;
      printf("2042: (float)(long long)65535 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)65535; double r1 = (double)v; double r2 = (double)(long long)65535; static double r3 = (double)(long long)65535;
      printf("2043: (double)(long long)65535 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)65535; long double r1 = (long double)v; long double r2 = (long double)(long long)65535; static long double r3 = (long double)(long long)65535;
      printf("2044: (long double)(long long)65535 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)2147483647; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)2147483647; static signed char r3 = (signed char)(long long)2147483647;
      printf("2045: (signed char)(long long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)2147483647; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)2147483647; static unsigned char r3 = (unsigned char)(long long)2147483647;
      printf("2046: (unsigned char)(long long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)2147483647; short r1 = (short)v; short r2 = (short)(long long)2147483647; static short r3 = (short)(long long)2147483647;
      printf("2047: (short)(long long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)2147483647; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)2147483647; static unsigned short r3 = (unsigned short)(long long)2147483647;
      printf("2048: (unsigned short)(long long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)2147483647; int r1 = (int)v; int r2 = (int)(long long)2147483647; static int r3 = (int)(long long)2147483647;
      printf("2049: (int)(long long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)2147483647; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)2147483647; static unsigned r3 = (unsigned)(long long)2147483647;
      printf("2050: (unsigned)(long long)2147483647 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)2147483647; long r1 = (long)v; long r2 = (long)(long long)2147483647; static long r3 = (long)(long long)2147483647;
      printf("2051: (long)(long long)2147483647 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)2147483647; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)2147483647; static unsigned long r3 = (unsigned long)(long long)2147483647;
      printf("2052: (unsigned long)(long long)2147483647 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)2147483647; long long r1 = (long long)v; long long r2 = (long long)(long long)2147483647; static long long r3 = (long long)(long long)2147483647;
      printf("2053: (long long)(long long)2147483647 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)2147483647; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)2147483647; static unsigned long long r3 = (unsigned long long)(long long)2147483647;
      printf("2054: (unsigned long long)(long long)2147483647 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)2147483647; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)2147483647; static _Bool r3 = (_Bool)(long long)2147483647;
      printf("2055: (_Bool)(long long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)2147483647; float r1 = (float)v; float r2 = (float)(long long)2147483647; static float r3 = (float)(long long)2147483647;
      printf("2056: (float)(long long)2147483647 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)2147483647; double r1 = (double)v; double r2 = (double)(long long)2147483647; static double r3 = (double)(long long)2147483647;
      printf("2057: (double)(long long)2147483647 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)2147483647; long double r1 = (long double)v; long double r2 = (long double)(long long)2147483647; static long double r3 = (long double)(long long)2147483647;
      printf("2058: (long double)(long long)2147483647 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)(-2147483647-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)(-2147483647-1); static signed char r3 = (signed char)(long long)(-2147483647-1);
      printf("2059: (signed char)(long long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)(-2147483647-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)(-2147483647-1); static unsigned char r3 = (unsigned char)(long long)(-2147483647-1);
      printf("2060: (unsigned char)(long long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)(-2147483647-1); short r1 = (short)v; short r2 = (short)(long long)(-2147483647-1); static short r3 = (short)(long long)(-2147483647-1);
      printf("2061: (short)(long long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)(-2147483647-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)(-2147483647-1); static unsigned short r3 = (unsigned short)(long long)(-2147483647-1);
      printf("2062: (unsigned short)(long long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)(-2147483647-1); int r1 = (int)v; int r2 = (int)(long long)(-2147483647-1); static int r3 = (int)(long long)(-2147483647-1);
      printf("2063: (int)(long long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)(-2147483647-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)(-2147483647-1); static unsigned r3 = (unsigned)(long long)(-2147483647-1);
      printf("2064: (unsigned)(long long)(-2147483647-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)(-2147483647-1); long r1 = (long)v; long r2 = (long)(long long)(-2147483647-1); static long r3 = (long)(long long)(-2147483647-1);
      printf("2065: (long)(long long)(-2147483647-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)(-2147483647-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)(-2147483647-1); static unsigned long r3 = (unsigned long)(long long)(-2147483647-1);
      printf("2066: (unsigned long)(long long)(-2147483647-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)(-2147483647-1); long long r1 = (long long)v; long long r2 = (long long)(long long)(-2147483647-1); static long long r3 = (long long)(long long)(-2147483647-1);
      printf("2067: (long long)(long long)(-2147483647-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)(-2147483647-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)(-2147483647-1); static unsigned long long r3 = (unsigned long long)(long long)(-2147483647-1);
      printf("2068: (unsigned long long)(long long)(-2147483647-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)(-2147483647-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)(-2147483647-1); static _Bool r3 = (_Bool)(long long)(-2147483647-1);
      printf("2069: (_Bool)(long long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)(-2147483647-1); float r1 = (float)v; float r2 = (float)(long long)(-2147483647-1); static float r3 = (float)(long long)(-2147483647-1);
      printf("2070: (float)(long long)(-2147483647-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)(-2147483647-1); double r1 = (double)v; double r2 = (double)(long long)(-2147483647-1); static double r3 = (double)(long long)(-2147483647-1);
      printf("2071: (double)(long long)(-2147483647-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)(-2147483647-1); long double r1 = (long double)v; long double r2 = (long double)(long long)(-2147483647-1); static long double r3 = (long double)(long long)(-2147483647-1);
      printf("2072: (long double)(long long)(-2147483647-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)4294967295LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)4294967295LL; static signed char r3 = (signed char)(long long)4294967295LL;
      printf("2073: (signed char)(long long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)4294967295LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)4294967295LL; static unsigned char r3 = (unsigned char)(long long)4294967295LL;
      printf("2074: (unsigned char)(long long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)4294967295LL; short r1 = (short)v; short r2 = (short)(long long)4294967295LL; static short r3 = (short)(long long)4294967295LL;
      printf("2075: (short)(long long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)4294967295LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)4294967295LL; static unsigned short r3 = (unsigned short)(long long)4294967295LL;
      printf("2076: (unsigned short)(long long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)4294967295LL; int r1 = (int)v; int r2 = (int)(long long)4294967295LL; static int r3 = (int)(long long)4294967295LL;
      printf("2077: (int)(long long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)4294967295LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)4294967295LL; static unsigned r3 = (unsigned)(long long)4294967295LL;
      printf("2078: (unsigned)(long long)4294967295LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)4294967295LL; long r1 = (long)v; long r2 = (long)(long long)4294967295LL; static long r3 = (long)(long long)4294967295LL;
      printf("2079: (long)(long long)4294967295LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)4294967295LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)4294967295LL; static unsigned long r3 = (unsigned long)(long long)4294967295LL;
      printf("2080: (unsigned long)(long long)4294967295LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)4294967295LL; long long r1 = (long long)v; long long r2 = (long long)(long long)4294967295LL; static long long r3 = (long long)(long long)4294967295LL;
      printf("2081: (long long)(long long)4294967295LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)4294967295LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)4294967295LL; static unsigned long long r3 = (unsigned long long)(long long)4294967295LL;
      printf("2082: (unsigned long long)(long long)4294967295LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)4294967295LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)4294967295LL; static _Bool r3 = (_Bool)(long long)4294967295LL;
      printf("2083: (_Bool)(long long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)4294967295LL; float r1 = (float)v; float r2 = (float)(long long)4294967295LL; static float r3 = (float)(long long)4294967295LL;
      printf("2084: (float)(long long)4294967295LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)4294967295LL; double r1 = (double)v; double r2 = (double)(long long)4294967295LL; static double r3 = (double)(long long)4294967295LL;
      printf("2085: (double)(long long)4294967295LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)4294967295LL; long double r1 = (long double)v; long double r2 = (long double)(long long)4294967295LL; static long double r3 = (long double)(long long)4294967295LL;
      printf("2086: (long double)(long long)4294967295LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)9223372036854775807LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)9223372036854775807LL; static signed char r3 = (signed char)(long long)9223372036854775807LL;
      printf("2087: (signed char)(long long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)9223372036854775807LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)9223372036854775807LL; static unsigned char r3 = (unsigned char)(long long)9223372036854775807LL;
      printf("2088: (unsigned char)(long long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)9223372036854775807LL; short r1 = (short)v; short r2 = (short)(long long)9223372036854775807LL; static short r3 = (short)(long long)9223372036854775807LL;
      printf("2089: (short)(long long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)9223372036854775807LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)9223372036854775807LL; static unsigned short r3 = (unsigned short)(long long)9223372036854775807LL;
      printf("2090: (unsigned short)(long long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)9223372036854775807LL; int r1 = (int)v; int r2 = (int)(long long)9223372036854775807LL; static int r3 = (int)(long long)9223372036854775807LL;
      printf("2091: (int)(long long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)9223372036854775807LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)9223372036854775807LL; static unsigned r3 = (unsigned)(long long)9223372036854775807LL;
      printf("2092: (unsigned)(long long)9223372036854775807LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)9223372036854775807LL; long r1 = (long)v; long r2 = (long)(long long)9223372036854775807LL; static long r3 = (long)(long long)9223372036854775807LL;
      printf("2093: (long)(long long)9223372036854775807LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)9223372036854775807LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)9223372036854775807LL; static unsigned long r3 = (unsigned long)(long long)9223372036854775807LL;
      printf("2094: (unsigned long)(long long)9223372036854775807LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)9223372036854775807LL; long long r1 = (long long)v; long long r2 = (long long)(long long)9223372036854775807LL; static long long r3 = (long long)(long long)9223372036854775807LL;
      printf("2095: (long long)(long long)9223372036854775807LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)9223372036854775807LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)9223372036854775807LL; static unsigned long long r3 = (unsigned long long)(long long)9223372036854775807LL;
      printf("2096: (unsigned long long)(long long)9223372036854775807LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)9223372036854775807LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)9223372036854775807LL; static _Bool r3 = (_Bool)(long long)9223372036854775807LL;
      printf("2097: (_Bool)(long long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)9223372036854775807LL; float r1 = (float)v; float r2 = (float)(long long)9223372036854775807LL; static float r3 = (float)(long long)9223372036854775807LL;
      printf("2098: (float)(long long)9223372036854775807LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)9223372036854775807LL; double r1 = (double)v; double r2 = (double)(long long)9223372036854775807LL; static double r3 = (double)(long long)9223372036854775807LL;
      printf("2099: (double)(long long)9223372036854775807LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)9223372036854775807LL; long double r1 = (long double)v; long double r2 = (long double)(long long)9223372036854775807LL; static long double r3 = (long double)(long long)9223372036854775807LL;
      printf("2100: (long double)(long long)9223372036854775807LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)(-9223372036854775807LL-1); static signed char r3 = (signed char)(long long)(-9223372036854775807LL-1);
      printf("2101: (signed char)(long long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)(-9223372036854775807LL-1); static unsigned char r3 = (unsigned char)(long long)(-9223372036854775807LL-1);
      printf("2102: (unsigned char)(long long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); short r1 = (short)v; short r2 = (short)(long long)(-9223372036854775807LL-1); static short r3 = (short)(long long)(-9223372036854775807LL-1);
      printf("2103: (short)(long long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)(-9223372036854775807LL-1); static unsigned short r3 = (unsigned short)(long long)(-9223372036854775807LL-1);
      printf("2104: (unsigned short)(long long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); int r1 = (int)v; int r2 = (int)(long long)(-9223372036854775807LL-1); static int r3 = (int)(long long)(-9223372036854775807LL-1);
      printf("2105: (int)(long long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)(-9223372036854775807LL-1); static unsigned r3 = (unsigned)(long long)(-9223372036854775807LL-1);
      printf("2106: (unsigned)(long long)(-9223372036854775807LL-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); long r1 = (long)v; long r2 = (long)(long long)(-9223372036854775807LL-1); static long r3 = (long)(long long)(-9223372036854775807LL-1);
      printf("2107: (long)(long long)(-9223372036854775807LL-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)(-9223372036854775807LL-1); static unsigned long r3 = (unsigned long)(long long)(-9223372036854775807LL-1);
      printf("2108: (unsigned long)(long long)(-9223372036854775807LL-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); long long r1 = (long long)v; long long r2 = (long long)(long long)(-9223372036854775807LL-1); static long long r3 = (long long)(long long)(-9223372036854775807LL-1);
      printf("2109: (long long)(long long)(-9223372036854775807LL-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)(-9223372036854775807LL-1); static unsigned long long r3 = (unsigned long long)(long long)(-9223372036854775807LL-1);
      printf("2110: (unsigned long long)(long long)(-9223372036854775807LL-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)(-9223372036854775807LL-1); static _Bool r3 = (_Bool)(long long)(-9223372036854775807LL-1);
      printf("2111: (_Bool)(long long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); float r1 = (float)v; float r2 = (float)(long long)(-9223372036854775807LL-1); static float r3 = (float)(long long)(-9223372036854775807LL-1);
      printf("2112: (float)(long long)(-9223372036854775807LL-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); double r1 = (double)v; double r2 = (double)(long long)(-9223372036854775807LL-1); static double r3 = (double)(long long)(-9223372036854775807LL-1);
      printf("2113: (double)(long long)(-9223372036854775807LL-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)(-9223372036854775807LL-1); long double r1 = (long double)v; long double r2 = (long double)(long long)(-9223372036854775807LL-1); static long double r3 = (long double)(long long)(-9223372036854775807LL-1);
      printf("2114: (long double)(long long)(-9223372036854775807LL-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)0x8000000000000000ULL; static signed char r3 = (signed char)(long long)0x8000000000000000ULL;
      printf("2115: (signed char)(long long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)0x8000000000000000ULL; static unsigned char r3 = (unsigned char)(long long)0x8000000000000000ULL;
      printf("2116: (unsigned char)(long long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; short r1 = (short)v; short r2 = (short)(long long)0x8000000000000000ULL; static short r3 = (short)(long long)0x8000000000000000ULL;
      printf("2117: (short)(long long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)0x8000000000000000ULL; static unsigned short r3 = (unsigned short)(long long)0x8000000000000000ULL;
      printf("2118: (unsigned short)(long long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; int r1 = (int)v; int r2 = (int)(long long)0x8000000000000000ULL; static int r3 = (int)(long long)0x8000000000000000ULL;
      printf("2119: (int)(long long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)0x8000000000000000ULL; static unsigned r3 = (unsigned)(long long)0x8000000000000000ULL;
      printf("2120: (unsigned)(long long)0x8000000000000000ULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; long r1 = (long)v; long r2 = (long)(long long)0x8000000000000000ULL; static long r3 = (long)(long long)0x8000000000000000ULL;
      printf("2121: (long)(long long)0x8000000000000000ULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)0x8000000000000000ULL; static unsigned long r3 = (unsigned long)(long long)0x8000000000000000ULL;
      printf("2122: (unsigned long)(long long)0x8000000000000000ULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; long long r1 = (long long)v; long long r2 = (long long)(long long)0x8000000000000000ULL; static long long r3 = (long long)(long long)0x8000000000000000ULL;
      printf("2123: (long long)(long long)0x8000000000000000ULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)0x8000000000000000ULL; static unsigned long long r3 = (unsigned long long)(long long)0x8000000000000000ULL;
      printf("2124: (unsigned long long)(long long)0x8000000000000000ULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)0x8000000000000000ULL; static _Bool r3 = (_Bool)(long long)0x8000000000000000ULL;
      printf("2125: (_Bool)(long long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; float r1 = (float)v; float r2 = (float)(long long)0x8000000000000000ULL; static float r3 = (float)(long long)0x8000000000000000ULL;
      printf("2126: (float)(long long)0x8000000000000000ULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; double r1 = (double)v; double r2 = (double)(long long)0x8000000000000000ULL; static double r3 = (double)(long long)0x8000000000000000ULL;
      printf("2127: (double)(long long)0x8000000000000000ULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)0x8000000000000000ULL; long double r1 = (long double)v; long double r2 = (long double)(long long)0x8000000000000000ULL; static long double r3 = (long double)(long long)0x8000000000000000ULL;
      printf("2128: (long double)(long long)0x8000000000000000ULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(long long)0xffffffffffffffffULL; static signed char r3 = (signed char)(long long)0xffffffffffffffffULL;
      printf("2129: (signed char)(long long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long long)0xffffffffffffffffULL; static unsigned char r3 = (unsigned char)(long long)0xffffffffffffffffULL;
      printf("2130: (unsigned char)(long long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; short r1 = (short)v; short r2 = (short)(long long)0xffffffffffffffffULL; static short r3 = (short)(long long)0xffffffffffffffffULL;
      printf("2131: (short)(long long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long long)0xffffffffffffffffULL; static unsigned short r3 = (unsigned short)(long long)0xffffffffffffffffULL;
      printf("2132: (unsigned short)(long long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; int r1 = (int)v; int r2 = (int)(long long)0xffffffffffffffffULL; static int r3 = (int)(long long)0xffffffffffffffffULL;
      printf("2133: (int)(long long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long long)0xffffffffffffffffULL; static unsigned r3 = (unsigned)(long long)0xffffffffffffffffULL;
      printf("2134: (unsigned)(long long)0xffffffffffffffffULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; long r1 = (long)v; long r2 = (long)(long long)0xffffffffffffffffULL; static long r3 = (long)(long long)0xffffffffffffffffULL;
      printf("2135: (long)(long long)0xffffffffffffffffULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long long)0xffffffffffffffffULL; static unsigned long r3 = (unsigned long)(long long)0xffffffffffffffffULL;
      printf("2136: (unsigned long)(long long)0xffffffffffffffffULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; long long r1 = (long long)v; long long r2 = (long long)(long long)0xffffffffffffffffULL; static long long r3 = (long long)(long long)0xffffffffffffffffULL;
      printf("2137: (long long)(long long)0xffffffffffffffffULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long long)0xffffffffffffffffULL; static unsigned long long r3 = (unsigned long long)(long long)0xffffffffffffffffULL;
      printf("2138: (unsigned long long)(long long)0xffffffffffffffffULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long long)0xffffffffffffffffULL; static _Bool r3 = (_Bool)(long long)0xffffffffffffffffULL;
      printf("2139: (_Bool)(long long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; float r1 = (float)v; float r2 = (float)(long long)0xffffffffffffffffULL; static float r3 = (float)(long long)0xffffffffffffffffULL;
      printf("2140: (float)(long long)0xffffffffffffffffULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; double r1 = (double)v; double r2 = (double)(long long)0xffffffffffffffffULL; static double r3 = (double)(long long)0xffffffffffffffffULL;
      printf("2141: (double)(long long)0xffffffffffffffffULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long v = (long long)0xffffffffffffffffULL; long double r1 = (long double)v; long double r2 = (long double)(long long)0xffffffffffffffffULL; static long double r3 = (long double)(long long)0xffffffffffffffffULL;
      printf("2142: (long double)(long long)0xffffffffffffffffULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)0; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)0; static signed char r3 = (signed char)(unsigned long long)0;
      printf("2143: (signed char)(unsigned long long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)0; static unsigned char r3 = (unsigned char)(unsigned long long)0;
      printf("2144: (unsigned char)(unsigned long long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0; short r1 = (short)v; short r2 = (short)(unsigned long long)0; static short r3 = (short)(unsigned long long)0;
      printf("2145: (short)(unsigned long long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)0; static unsigned short r3 = (unsigned short)(unsigned long long)0;
      printf("2146: (unsigned short)(unsigned long long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0; int r1 = (int)v; int r2 = (int)(unsigned long long)0; static int r3 = (int)(unsigned long long)0;
      printf("2147: (int)(unsigned long long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)0; static unsigned r3 = (unsigned)(unsigned long long)0;
      printf("2148: (unsigned)(unsigned long long)0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)0; long r1 = (long)v; long r2 = (long)(unsigned long long)0; static long r3 = (long)(unsigned long long)0;
      printf("2149: (long)(unsigned long long)0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)0; static unsigned long r3 = (unsigned long)(unsigned long long)0;
      printf("2150: (unsigned long)(unsigned long long)0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)0; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)0; static long long r3 = (long long)(unsigned long long)0;
      printf("2151: (long long)(unsigned long long)0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)0; static unsigned long long r3 = (unsigned long long)(unsigned long long)0;
      printf("2152: (unsigned long long)(unsigned long long)0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)0; static _Bool r3 = (_Bool)(unsigned long long)0;
      printf("2153: (_Bool)(unsigned long long)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0; float r1 = (float)v; float r2 = (float)(unsigned long long)0; static float r3 = (float)(unsigned long long)0;
      printf("2154: (float)(unsigned long long)0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)0; double r1 = (double)v; double r2 = (double)(unsigned long long)0; static double r3 = (double)(unsigned long long)0;
      printf("2155: (double)(unsigned long long)0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)0; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)0; static long double r3 = (long double)(unsigned long long)0;
      printf("2156: (long double)(unsigned long long)0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)1; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)1; static signed char r3 = (signed char)(unsigned long long)1;
      printf("2157: (signed char)(unsigned long long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)1; static unsigned char r3 = (unsigned char)(unsigned long long)1;
      printf("2158: (unsigned char)(unsigned long long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)1; short r1 = (short)v; short r2 = (short)(unsigned long long)1; static short r3 = (short)(unsigned long long)1;
      printf("2159: (short)(unsigned long long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)1; static unsigned short r3 = (unsigned short)(unsigned long long)1;
      printf("2160: (unsigned short)(unsigned long long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)1; int r1 = (int)v; int r2 = (int)(unsigned long long)1; static int r3 = (int)(unsigned long long)1;
      printf("2161: (int)(unsigned long long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)1; static unsigned r3 = (unsigned)(unsigned long long)1;
      printf("2162: (unsigned)(unsigned long long)1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)1; long r1 = (long)v; long r2 = (long)(unsigned long long)1; static long r3 = (long)(unsigned long long)1;
      printf("2163: (long)(unsigned long long)1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)1; static unsigned long r3 = (unsigned long)(unsigned long long)1;
      printf("2164: (unsigned long)(unsigned long long)1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)1; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)1; static long long r3 = (long long)(unsigned long long)1;
      printf("2165: (long long)(unsigned long long)1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)1; static unsigned long long r3 = (unsigned long long)(unsigned long long)1;
      printf("2166: (unsigned long long)(unsigned long long)1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)1; static _Bool r3 = (_Bool)(unsigned long long)1;
      printf("2167: (_Bool)(unsigned long long)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)1; float r1 = (float)v; float r2 = (float)(unsigned long long)1; static float r3 = (float)(unsigned long long)1;
      printf("2168: (float)(unsigned long long)1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)1; double r1 = (double)v; double r2 = (double)(unsigned long long)1; static double r3 = (double)(unsigned long long)1;
      printf("2169: (double)(unsigned long long)1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)1; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)1; static long double r3 = (long double)(unsigned long long)1;
      printf("2170: (long double)(unsigned long long)1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)-1; static signed char r3 = (signed char)(unsigned long long)-1;
      printf("2171: (signed char)(unsigned long long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)-1; static unsigned char r3 = (unsigned char)(unsigned long long)-1;
      printf("2172: (unsigned char)(unsigned long long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; short r1 = (short)v; short r2 = (short)(unsigned long long)-1; static short r3 = (short)(unsigned long long)-1;
      printf("2173: (short)(unsigned long long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)-1; static unsigned short r3 = (unsigned short)(unsigned long long)-1;
      printf("2174: (unsigned short)(unsigned long long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; int r1 = (int)v; int r2 = (int)(unsigned long long)-1; static int r3 = (int)(unsigned long long)-1;
      printf("2175: (int)(unsigned long long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)-1; static unsigned r3 = (unsigned)(unsigned long long)-1;
      printf("2176: (unsigned)(unsigned long long)-1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; long r1 = (long)v; long r2 = (long)(unsigned long long)-1; static long r3 = (long)(unsigned long long)-1;
      printf("2177: (long)(unsigned long long)-1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)-1; static unsigned long r3 = (unsigned long)(unsigned long long)-1;
      printf("2178: (unsigned long)(unsigned long long)-1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)-1; static long long r3 = (long long)(unsigned long long)-1;
      printf("2179: (long long)(unsigned long long)-1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)-1; static unsigned long long r3 = (unsigned long long)(unsigned long long)-1;
      printf("2180: (unsigned long long)(unsigned long long)-1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)-1; static _Bool r3 = (_Bool)(unsigned long long)-1;
      printf("2181: (_Bool)(unsigned long long)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; float r1 = (float)v; float r2 = (float)(unsigned long long)-1; static float r3 = (float)(unsigned long long)-1;
      printf("2182: (float)(unsigned long long)-1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; double r1 = (double)v; double r2 = (double)(unsigned long long)-1; static double r3 = (double)(unsigned long long)-1;
      printf("2183: (double)(unsigned long long)-1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)-1; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)-1; static long double r3 = (long double)(unsigned long long)-1;
      printf("2184: (long double)(unsigned long long)-1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)127; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)127; static signed char r3 = (signed char)(unsigned long long)127;
      printf("2185: (signed char)(unsigned long long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)127; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)127; static unsigned char r3 = (unsigned char)(unsigned long long)127;
      printf("2186: (unsigned char)(unsigned long long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)127; short r1 = (short)v; short r2 = (short)(unsigned long long)127; static short r3 = (short)(unsigned long long)127;
      printf("2187: (short)(unsigned long long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)127; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)127; static unsigned short r3 = (unsigned short)(unsigned long long)127;
      printf("2188: (unsigned short)(unsigned long long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)127; int r1 = (int)v; int r2 = (int)(unsigned long long)127; static int r3 = (int)(unsigned long long)127;
      printf("2189: (int)(unsigned long long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)127; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)127; static unsigned r3 = (unsigned)(unsigned long long)127;
      printf("2190: (unsigned)(unsigned long long)127 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)127; long r1 = (long)v; long r2 = (long)(unsigned long long)127; static long r3 = (long)(unsigned long long)127;
      printf("2191: (long)(unsigned long long)127 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)127; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)127; static unsigned long r3 = (unsigned long)(unsigned long long)127;
      printf("2192: (unsigned long)(unsigned long long)127 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)127; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)127; static long long r3 = (long long)(unsigned long long)127;
      printf("2193: (long long)(unsigned long long)127 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)127; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)127; static unsigned long long r3 = (unsigned long long)(unsigned long long)127;
      printf("2194: (unsigned long long)(unsigned long long)127 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)127; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)127; static _Bool r3 = (_Bool)(unsigned long long)127;
      printf("2195: (_Bool)(unsigned long long)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)127; float r1 = (float)v; float r2 = (float)(unsigned long long)127; static float r3 = (float)(unsigned long long)127;
      printf("2196: (float)(unsigned long long)127 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)127; double r1 = (double)v; double r2 = (double)(unsigned long long)127; static double r3 = (double)(unsigned long long)127;
      printf("2197: (double)(unsigned long long)127 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)127; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)127; static long double r3 = (long double)(unsigned long long)127;
      printf("2198: (long double)(unsigned long long)127 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)-128; static signed char r3 = (signed char)(unsigned long long)-128;
      printf("2199: (signed char)(unsigned long long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)-128; static unsigned char r3 = (unsigned char)(unsigned long long)-128;
      printf("2200: (unsigned char)(unsigned long long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; short r1 = (short)v; short r2 = (short)(unsigned long long)-128; static short r3 = (short)(unsigned long long)-128;
      printf("2201: (short)(unsigned long long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)-128; static unsigned short r3 = (unsigned short)(unsigned long long)-128;
      printf("2202: (unsigned short)(unsigned long long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; int r1 = (int)v; int r2 = (int)(unsigned long long)-128; static int r3 = (int)(unsigned long long)-128;
      printf("2203: (int)(unsigned long long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)-128; static unsigned r3 = (unsigned)(unsigned long long)-128;
      printf("2204: (unsigned)(unsigned long long)-128 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; long r1 = (long)v; long r2 = (long)(unsigned long long)-128; static long r3 = (long)(unsigned long long)-128;
      printf("2205: (long)(unsigned long long)-128 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)-128; static unsigned long r3 = (unsigned long)(unsigned long long)-128;
      printf("2206: (unsigned long)(unsigned long long)-128 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)-128; static long long r3 = (long long)(unsigned long long)-128;
      printf("2207: (long long)(unsigned long long)-128 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)-128; static unsigned long long r3 = (unsigned long long)(unsigned long long)-128;
      printf("2208: (unsigned long long)(unsigned long long)-128 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)-128; static _Bool r3 = (_Bool)(unsigned long long)-128;
      printf("2209: (_Bool)(unsigned long long)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; float r1 = (float)v; float r2 = (float)(unsigned long long)-128; static float r3 = (float)(unsigned long long)-128;
      printf("2210: (float)(unsigned long long)-128 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; double r1 = (double)v; double r2 = (double)(unsigned long long)-128; static double r3 = (double)(unsigned long long)-128;
      printf("2211: (double)(unsigned long long)-128 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)-128; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)-128; static long double r3 = (long double)(unsigned long long)-128;
      printf("2212: (long double)(unsigned long long)-128 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)255; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)255; static signed char r3 = (signed char)(unsigned long long)255;
      printf("2213: (signed char)(unsigned long long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)255; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)255; static unsigned char r3 = (unsigned char)(unsigned long long)255;
      printf("2214: (unsigned char)(unsigned long long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)255; short r1 = (short)v; short r2 = (short)(unsigned long long)255; static short r3 = (short)(unsigned long long)255;
      printf("2215: (short)(unsigned long long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)255; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)255; static unsigned short r3 = (unsigned short)(unsigned long long)255;
      printf("2216: (unsigned short)(unsigned long long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)255; int r1 = (int)v; int r2 = (int)(unsigned long long)255; static int r3 = (int)(unsigned long long)255;
      printf("2217: (int)(unsigned long long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)255; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)255; static unsigned r3 = (unsigned)(unsigned long long)255;
      printf("2218: (unsigned)(unsigned long long)255 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)255; long r1 = (long)v; long r2 = (long)(unsigned long long)255; static long r3 = (long)(unsigned long long)255;
      printf("2219: (long)(unsigned long long)255 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)255; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)255; static unsigned long r3 = (unsigned long)(unsigned long long)255;
      printf("2220: (unsigned long)(unsigned long long)255 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)255; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)255; static long long r3 = (long long)(unsigned long long)255;
      printf("2221: (long long)(unsigned long long)255 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)255; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)255; static unsigned long long r3 = (unsigned long long)(unsigned long long)255;
      printf("2222: (unsigned long long)(unsigned long long)255 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)255; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)255; static _Bool r3 = (_Bool)(unsigned long long)255;
      printf("2223: (_Bool)(unsigned long long)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)255; float r1 = (float)v; float r2 = (float)(unsigned long long)255; static float r3 = (float)(unsigned long long)255;
      printf("2224: (float)(unsigned long long)255 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)255; double r1 = (double)v; double r2 = (double)(unsigned long long)255; static double r3 = (double)(unsigned long long)255;
      printf("2225: (double)(unsigned long long)255 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)255; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)255; static long double r3 = (long double)(unsigned long long)255;
      printf("2226: (long double)(unsigned long long)255 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)256; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)256; static signed char r3 = (signed char)(unsigned long long)256;
      printf("2227: (signed char)(unsigned long long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)256; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)256; static unsigned char r3 = (unsigned char)(unsigned long long)256;
      printf("2228: (unsigned char)(unsigned long long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)256; short r1 = (short)v; short r2 = (short)(unsigned long long)256; static short r3 = (short)(unsigned long long)256;
      printf("2229: (short)(unsigned long long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)256; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)256; static unsigned short r3 = (unsigned short)(unsigned long long)256;
      printf("2230: (unsigned short)(unsigned long long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)256; int r1 = (int)v; int r2 = (int)(unsigned long long)256; static int r3 = (int)(unsigned long long)256;
      printf("2231: (int)(unsigned long long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)256; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)256; static unsigned r3 = (unsigned)(unsigned long long)256;
      printf("2232: (unsigned)(unsigned long long)256 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)256; long r1 = (long)v; long r2 = (long)(unsigned long long)256; static long r3 = (long)(unsigned long long)256;
      printf("2233: (long)(unsigned long long)256 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)256; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)256; static unsigned long r3 = (unsigned long)(unsigned long long)256;
      printf("2234: (unsigned long)(unsigned long long)256 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)256; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)256; static long long r3 = (long long)(unsigned long long)256;
      printf("2235: (long long)(unsigned long long)256 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)256; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)256; static unsigned long long r3 = (unsigned long long)(unsigned long long)256;
      printf("2236: (unsigned long long)(unsigned long long)256 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)256; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)256; static _Bool r3 = (_Bool)(unsigned long long)256;
      printf("2237: (_Bool)(unsigned long long)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)256; float r1 = (float)v; float r2 = (float)(unsigned long long)256; static float r3 = (float)(unsigned long long)256;
      printf("2238: (float)(unsigned long long)256 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)256; double r1 = (double)v; double r2 = (double)(unsigned long long)256; static double r3 = (double)(unsigned long long)256;
      printf("2239: (double)(unsigned long long)256 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)256; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)256; static long double r3 = (long double)(unsigned long long)256;
      printf("2240: (long double)(unsigned long long)256 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)32767; static signed char r3 = (signed char)(unsigned long long)32767;
      printf("2241: (signed char)(unsigned long long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)32767; static unsigned char r3 = (unsigned char)(unsigned long long)32767;
      printf("2242: (unsigned char)(unsigned long long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; short r1 = (short)v; short r2 = (short)(unsigned long long)32767; static short r3 = (short)(unsigned long long)32767;
      printf("2243: (short)(unsigned long long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)32767; static unsigned short r3 = (unsigned short)(unsigned long long)32767;
      printf("2244: (unsigned short)(unsigned long long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; int r1 = (int)v; int r2 = (int)(unsigned long long)32767; static int r3 = (int)(unsigned long long)32767;
      printf("2245: (int)(unsigned long long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)32767; static unsigned r3 = (unsigned)(unsigned long long)32767;
      printf("2246: (unsigned)(unsigned long long)32767 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; long r1 = (long)v; long r2 = (long)(unsigned long long)32767; static long r3 = (long)(unsigned long long)32767;
      printf("2247: (long)(unsigned long long)32767 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)32767; static unsigned long r3 = (unsigned long)(unsigned long long)32767;
      printf("2248: (unsigned long)(unsigned long long)32767 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)32767; static long long r3 = (long long)(unsigned long long)32767;
      printf("2249: (long long)(unsigned long long)32767 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)32767; static unsigned long long r3 = (unsigned long long)(unsigned long long)32767;
      printf("2250: (unsigned long long)(unsigned long long)32767 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)32767; static _Bool r3 = (_Bool)(unsigned long long)32767;
      printf("2251: (_Bool)(unsigned long long)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; float r1 = (float)v; float r2 = (float)(unsigned long long)32767; static float r3 = (float)(unsigned long long)32767;
      printf("2252: (float)(unsigned long long)32767 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; double r1 = (double)v; double r2 = (double)(unsigned long long)32767; static double r3 = (double)(unsigned long long)32767;
      printf("2253: (double)(unsigned long long)32767 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)32767; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)32767; static long double r3 = (long double)(unsigned long long)32767;
      printf("2254: (long double)(unsigned long long)32767 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)-32768; static signed char r3 = (signed char)(unsigned long long)-32768;
      printf("2255: (signed char)(unsigned long long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)-32768; static unsigned char r3 = (unsigned char)(unsigned long long)-32768;
      printf("2256: (unsigned char)(unsigned long long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; short r1 = (short)v; short r2 = (short)(unsigned long long)-32768; static short r3 = (short)(unsigned long long)-32768;
      printf("2257: (short)(unsigned long long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)-32768; static unsigned short r3 = (unsigned short)(unsigned long long)-32768;
      printf("2258: (unsigned short)(unsigned long long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; int r1 = (int)v; int r2 = (int)(unsigned long long)-32768; static int r3 = (int)(unsigned long long)-32768;
      printf("2259: (int)(unsigned long long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)-32768; static unsigned r3 = (unsigned)(unsigned long long)-32768;
      printf("2260: (unsigned)(unsigned long long)-32768 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; long r1 = (long)v; long r2 = (long)(unsigned long long)-32768; static long r3 = (long)(unsigned long long)-32768;
      printf("2261: (long)(unsigned long long)-32768 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)-32768; static unsigned long r3 = (unsigned long)(unsigned long long)-32768;
      printf("2262: (unsigned long)(unsigned long long)-32768 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)-32768; static long long r3 = (long long)(unsigned long long)-32768;
      printf("2263: (long long)(unsigned long long)-32768 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)-32768; static unsigned long long r3 = (unsigned long long)(unsigned long long)-32768;
      printf("2264: (unsigned long long)(unsigned long long)-32768 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)-32768; static _Bool r3 = (_Bool)(unsigned long long)-32768;
      printf("2265: (_Bool)(unsigned long long)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; float r1 = (float)v; float r2 = (float)(unsigned long long)-32768; static float r3 = (float)(unsigned long long)-32768;
      printf("2266: (float)(unsigned long long)-32768 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; double r1 = (double)v; double r2 = (double)(unsigned long long)-32768; static double r3 = (double)(unsigned long long)-32768;
      printf("2267: (double)(unsigned long long)-32768 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)-32768; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)-32768; static long double r3 = (long double)(unsigned long long)-32768;
      printf("2268: (long double)(unsigned long long)-32768 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)65535; static signed char r3 = (signed char)(unsigned long long)65535;
      printf("2269: (signed char)(unsigned long long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)65535; static unsigned char r3 = (unsigned char)(unsigned long long)65535;
      printf("2270: (unsigned char)(unsigned long long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; short r1 = (short)v; short r2 = (short)(unsigned long long)65535; static short r3 = (short)(unsigned long long)65535;
      printf("2271: (short)(unsigned long long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)65535; static unsigned short r3 = (unsigned short)(unsigned long long)65535;
      printf("2272: (unsigned short)(unsigned long long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; int r1 = (int)v; int r2 = (int)(unsigned long long)65535; static int r3 = (int)(unsigned long long)65535;
      printf("2273: (int)(unsigned long long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)65535; static unsigned r3 = (unsigned)(unsigned long long)65535;
      printf("2274: (unsigned)(unsigned long long)65535 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; long r1 = (long)v; long r2 = (long)(unsigned long long)65535; static long r3 = (long)(unsigned long long)65535;
      printf("2275: (long)(unsigned long long)65535 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)65535; static unsigned long r3 = (unsigned long)(unsigned long long)65535;
      printf("2276: (unsigned long)(unsigned long long)65535 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)65535; static long long r3 = (long long)(unsigned long long)65535;
      printf("2277: (long long)(unsigned long long)65535 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)65535; static unsigned long long r3 = (unsigned long long)(unsigned long long)65535;
      printf("2278: (unsigned long long)(unsigned long long)65535 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)65535; static _Bool r3 = (_Bool)(unsigned long long)65535;
      printf("2279: (_Bool)(unsigned long long)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; float r1 = (float)v; float r2 = (float)(unsigned long long)65535; static float r3 = (float)(unsigned long long)65535;
      printf("2280: (float)(unsigned long long)65535 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; double r1 = (double)v; double r2 = (double)(unsigned long long)65535; static double r3 = (double)(unsigned long long)65535;
      printf("2281: (double)(unsigned long long)65535 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)65535; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)65535; static long double r3 = (long double)(unsigned long long)65535;
      printf("2282: (long double)(unsigned long long)65535 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)2147483647; static signed char r3 = (signed char)(unsigned long long)2147483647;
      printf("2283: (signed char)(unsigned long long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)2147483647; static unsigned char r3 = (unsigned char)(unsigned long long)2147483647;
      printf("2284: (unsigned char)(unsigned long long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; short r1 = (short)v; short r2 = (short)(unsigned long long)2147483647; static short r3 = (short)(unsigned long long)2147483647;
      printf("2285: (short)(unsigned long long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)2147483647; static unsigned short r3 = (unsigned short)(unsigned long long)2147483647;
      printf("2286: (unsigned short)(unsigned long long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; int r1 = (int)v; int r2 = (int)(unsigned long long)2147483647; static int r3 = (int)(unsigned long long)2147483647;
      printf("2287: (int)(unsigned long long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)2147483647; static unsigned r3 = (unsigned)(unsigned long long)2147483647;
      printf("2288: (unsigned)(unsigned long long)2147483647 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; long r1 = (long)v; long r2 = (long)(unsigned long long)2147483647; static long r3 = (long)(unsigned long long)2147483647;
      printf("2289: (long)(unsigned long long)2147483647 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)2147483647; static unsigned long r3 = (unsigned long)(unsigned long long)2147483647;
      printf("2290: (unsigned long)(unsigned long long)2147483647 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)2147483647; static long long r3 = (long long)(unsigned long long)2147483647;
      printf("2291: (long long)(unsigned long long)2147483647 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)2147483647; static unsigned long long r3 = (unsigned long long)(unsigned long long)2147483647;
      printf("2292: (unsigned long long)(unsigned long long)2147483647 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)2147483647; static _Bool r3 = (_Bool)(unsigned long long)2147483647;
      printf("2293: (_Bool)(unsigned long long)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; float r1 = (float)v; float r2 = (float)(unsigned long long)2147483647; static float r3 = (float)(unsigned long long)2147483647;
      printf("2294: (float)(unsigned long long)2147483647 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; double r1 = (double)v; double r2 = (double)(unsigned long long)2147483647; static double r3 = (double)(unsigned long long)2147483647;
      printf("2295: (double)(unsigned long long)2147483647 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)2147483647; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)2147483647; static long double r3 = (long double)(unsigned long long)2147483647;
      printf("2296: (long double)(unsigned long long)2147483647 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)(-2147483647-1); static signed char r3 = (signed char)(unsigned long long)(-2147483647-1);
      printf("2297: (signed char)(unsigned long long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)(-2147483647-1); static unsigned char r3 = (unsigned char)(unsigned long long)(-2147483647-1);
      printf("2298: (unsigned char)(unsigned long long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); short r1 = (short)v; short r2 = (short)(unsigned long long)(-2147483647-1); static short r3 = (short)(unsigned long long)(-2147483647-1);
      printf("2299: (short)(unsigned long long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)(-2147483647-1); static unsigned short r3 = (unsigned short)(unsigned long long)(-2147483647-1);
      printf("2300: (unsigned short)(unsigned long long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); int r1 = (int)v; int r2 = (int)(unsigned long long)(-2147483647-1); static int r3 = (int)(unsigned long long)(-2147483647-1);
      printf("2301: (int)(unsigned long long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)(-2147483647-1); static unsigned r3 = (unsigned)(unsigned long long)(-2147483647-1);
      printf("2302: (unsigned)(unsigned long long)(-2147483647-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); long r1 = (long)v; long r2 = (long)(unsigned long long)(-2147483647-1); static long r3 = (long)(unsigned long long)(-2147483647-1);
      printf("2303: (long)(unsigned long long)(-2147483647-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)(-2147483647-1); static unsigned long r3 = (unsigned long)(unsigned long long)(-2147483647-1);
      printf("2304: (unsigned long)(unsigned long long)(-2147483647-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)(-2147483647-1); static long long r3 = (long long)(unsigned long long)(-2147483647-1);
      printf("2305: (long long)(unsigned long long)(-2147483647-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)(-2147483647-1); static unsigned long long r3 = (unsigned long long)(unsigned long long)(-2147483647-1);
      printf("2306: (unsigned long long)(unsigned long long)(-2147483647-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)(-2147483647-1); static _Bool r3 = (_Bool)(unsigned long long)(-2147483647-1);
      printf("2307: (_Bool)(unsigned long long)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); float r1 = (float)v; float r2 = (float)(unsigned long long)(-2147483647-1); static float r3 = (float)(unsigned long long)(-2147483647-1);
      printf("2308: (float)(unsigned long long)(-2147483647-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); double r1 = (double)v; double r2 = (double)(unsigned long long)(-2147483647-1); static double r3 = (double)(unsigned long long)(-2147483647-1);
      printf("2309: (double)(unsigned long long)(-2147483647-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)(-2147483647-1); long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)(-2147483647-1); static long double r3 = (long double)(unsigned long long)(-2147483647-1);
      printf("2310: (long double)(unsigned long long)(-2147483647-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)4294967295LL; static signed char r3 = (signed char)(unsigned long long)4294967295LL;
      printf("2311: (signed char)(unsigned long long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)4294967295LL; static unsigned char r3 = (unsigned char)(unsigned long long)4294967295LL;
      printf("2312: (unsigned char)(unsigned long long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; short r1 = (short)v; short r2 = (short)(unsigned long long)4294967295LL; static short r3 = (short)(unsigned long long)4294967295LL;
      printf("2313: (short)(unsigned long long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)4294967295LL; static unsigned short r3 = (unsigned short)(unsigned long long)4294967295LL;
      printf("2314: (unsigned short)(unsigned long long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; int r1 = (int)v; int r2 = (int)(unsigned long long)4294967295LL; static int r3 = (int)(unsigned long long)4294967295LL;
      printf("2315: (int)(unsigned long long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)4294967295LL; static unsigned r3 = (unsigned)(unsigned long long)4294967295LL;
      printf("2316: (unsigned)(unsigned long long)4294967295LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; long r1 = (long)v; long r2 = (long)(unsigned long long)4294967295LL; static long r3 = (long)(unsigned long long)4294967295LL;
      printf("2317: (long)(unsigned long long)4294967295LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)4294967295LL; static unsigned long r3 = (unsigned long)(unsigned long long)4294967295LL;
      printf("2318: (unsigned long)(unsigned long long)4294967295LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)4294967295LL; static long long r3 = (long long)(unsigned long long)4294967295LL;
      printf("2319: (long long)(unsigned long long)4294967295LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)4294967295LL; static unsigned long long r3 = (unsigned long long)(unsigned long long)4294967295LL;
      printf("2320: (unsigned long long)(unsigned long long)4294967295LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)4294967295LL; static _Bool r3 = (_Bool)(unsigned long long)4294967295LL;
      printf("2321: (_Bool)(unsigned long long)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; float r1 = (float)v; float r2 = (float)(unsigned long long)4294967295LL; static float r3 = (float)(unsigned long long)4294967295LL;
      printf("2322: (float)(unsigned long long)4294967295LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; double r1 = (double)v; double r2 = (double)(unsigned long long)4294967295LL; static double r3 = (double)(unsigned long long)4294967295LL;
      printf("2323: (double)(unsigned long long)4294967295LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)4294967295LL; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)4294967295LL; static long double r3 = (long double)(unsigned long long)4294967295LL;
      printf("2324: (long double)(unsigned long long)4294967295LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)9223372036854775807LL; static signed char r3 = (signed char)(unsigned long long)9223372036854775807LL;
      printf("2325: (signed char)(unsigned long long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)9223372036854775807LL; static unsigned char r3 = (unsigned char)(unsigned long long)9223372036854775807LL;
      printf("2326: (unsigned char)(unsigned long long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; short r1 = (short)v; short r2 = (short)(unsigned long long)9223372036854775807LL; static short r3 = (short)(unsigned long long)9223372036854775807LL;
      printf("2327: (short)(unsigned long long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)9223372036854775807LL; static unsigned short r3 = (unsigned short)(unsigned long long)9223372036854775807LL;
      printf("2328: (unsigned short)(unsigned long long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; int r1 = (int)v; int r2 = (int)(unsigned long long)9223372036854775807LL; static int r3 = (int)(unsigned long long)9223372036854775807LL;
      printf("2329: (int)(unsigned long long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)9223372036854775807LL; static unsigned r3 = (unsigned)(unsigned long long)9223372036854775807LL;
      printf("2330: (unsigned)(unsigned long long)9223372036854775807LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; long r1 = (long)v; long r2 = (long)(unsigned long long)9223372036854775807LL; static long r3 = (long)(unsigned long long)9223372036854775807LL;
      printf("2331: (long)(unsigned long long)9223372036854775807LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)9223372036854775807LL; static unsigned long r3 = (unsigned long)(unsigned long long)9223372036854775807LL;
      printf("2332: (unsigned long)(unsigned long long)9223372036854775807LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)9223372036854775807LL; static long long r3 = (long long)(unsigned long long)9223372036854775807LL;
      printf("2333: (long long)(unsigned long long)9223372036854775807LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)9223372036854775807LL; static unsigned long long r3 = (unsigned long long)(unsigned long long)9223372036854775807LL;
      printf("2334: (unsigned long long)(unsigned long long)9223372036854775807LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)9223372036854775807LL; static _Bool r3 = (_Bool)(unsigned long long)9223372036854775807LL;
      printf("2335: (_Bool)(unsigned long long)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; float r1 = (float)v; float r2 = (float)(unsigned long long)9223372036854775807LL; static float r3 = (float)(unsigned long long)9223372036854775807LL;
      printf("2336: (float)(unsigned long long)9223372036854775807LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; double r1 = (double)v; double r2 = (double)(unsigned long long)9223372036854775807LL; static double r3 = (double)(unsigned long long)9223372036854775807LL;
      printf("2337: (double)(unsigned long long)9223372036854775807LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)9223372036854775807LL; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)9223372036854775807LL; static long double r3 = (long double)(unsigned long long)9223372036854775807LL;
      printf("2338: (long double)(unsigned long long)9223372036854775807LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)(-9223372036854775807LL-1); static signed char r3 = (signed char)(unsigned long long)(-9223372036854775807LL-1);
      printf("2339: (signed char)(unsigned long long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)(-9223372036854775807LL-1); static unsigned char r3 = (unsigned char)(unsigned long long)(-9223372036854775807LL-1);
      printf("2340: (unsigned char)(unsigned long long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); short r1 = (short)v; short r2 = (short)(unsigned long long)(-9223372036854775807LL-1); static short r3 = (short)(unsigned long long)(-9223372036854775807LL-1);
      printf("2341: (short)(unsigned long long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)(-9223372036854775807LL-1); static unsigned short r3 = (unsigned short)(unsigned long long)(-9223372036854775807LL-1);
      printf("2342: (unsigned short)(unsigned long long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); int r1 = (int)v; int r2 = (int)(unsigned long long)(-9223372036854775807LL-1); static int r3 = (int)(unsigned long long)(-9223372036854775807LL-1);
      printf("2343: (int)(unsigned long long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)(-9223372036854775807LL-1); static unsigned r3 = (unsigned)(unsigned long long)(-9223372036854775807LL-1);
      printf("2344: (unsigned)(unsigned long long)(-9223372036854775807LL-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); long r1 = (long)v; long r2 = (long)(unsigned long long)(-9223372036854775807LL-1); static long r3 = (long)(unsigned long long)(-9223372036854775807LL-1);
      printf("2345: (long)(unsigned long long)(-9223372036854775807LL-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)(-9223372036854775807LL-1); static unsigned long r3 = (unsigned long)(unsigned long long)(-9223372036854775807LL-1);
      printf("2346: (unsigned long)(unsigned long long)(-9223372036854775807LL-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)(-9223372036854775807LL-1); static long long r3 = (long long)(unsigned long long)(-9223372036854775807LL-1);
      printf("2347: (long long)(unsigned long long)(-9223372036854775807LL-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)(-9223372036854775807LL-1); static unsigned long long r3 = (unsigned long long)(unsigned long long)(-9223372036854775807LL-1);
      printf("2348: (unsigned long long)(unsigned long long)(-9223372036854775807LL-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)(-9223372036854775807LL-1); static _Bool r3 = (_Bool)(unsigned long long)(-9223372036854775807LL-1);
      printf("2349: (_Bool)(unsigned long long)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); float r1 = (float)v; float r2 = (float)(unsigned long long)(-9223372036854775807LL-1); static float r3 = (float)(unsigned long long)(-9223372036854775807LL-1);
      printf("2350: (float)(unsigned long long)(-9223372036854775807LL-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); double r1 = (double)v; double r2 = (double)(unsigned long long)(-9223372036854775807LL-1); static double r3 = (double)(unsigned long long)(-9223372036854775807LL-1);
      printf("2351: (double)(unsigned long long)(-9223372036854775807LL-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)(-9223372036854775807LL-1); long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)(-9223372036854775807LL-1); static long double r3 = (long double)(unsigned long long)(-9223372036854775807LL-1);
      printf("2352: (long double)(unsigned long long)(-9223372036854775807LL-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)0x8000000000000000ULL; static signed char r3 = (signed char)(unsigned long long)0x8000000000000000ULL;
      printf("2353: (signed char)(unsigned long long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)0x8000000000000000ULL; static unsigned char r3 = (unsigned char)(unsigned long long)0x8000000000000000ULL;
      printf("2354: (unsigned char)(unsigned long long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; short r1 = (short)v; short r2 = (short)(unsigned long long)0x8000000000000000ULL; static short r3 = (short)(unsigned long long)0x8000000000000000ULL;
      printf("2355: (short)(unsigned long long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)0x8000000000000000ULL; static unsigned short r3 = (unsigned short)(unsigned long long)0x8000000000000000ULL;
      printf("2356: (unsigned short)(unsigned long long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; int r1 = (int)v; int r2 = (int)(unsigned long long)0x8000000000000000ULL; static int r3 = (int)(unsigned long long)0x8000000000000000ULL;
      printf("2357: (int)(unsigned long long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)0x8000000000000000ULL; static unsigned r3 = (unsigned)(unsigned long long)0x8000000000000000ULL;
      printf("2358: (unsigned)(unsigned long long)0x8000000000000000ULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; long r1 = (long)v; long r2 = (long)(unsigned long long)0x8000000000000000ULL; static long r3 = (long)(unsigned long long)0x8000000000000000ULL;
      printf("2359: (long)(unsigned long long)0x8000000000000000ULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)0x8000000000000000ULL; static unsigned long r3 = (unsigned long)(unsigned long long)0x8000000000000000ULL;
      printf("2360: (unsigned long)(unsigned long long)0x8000000000000000ULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)0x8000000000000000ULL; static long long r3 = (long long)(unsigned long long)0x8000000000000000ULL;
      printf("2361: (long long)(unsigned long long)0x8000000000000000ULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)0x8000000000000000ULL; static unsigned long long r3 = (unsigned long long)(unsigned long long)0x8000000000000000ULL;
      printf("2362: (unsigned long long)(unsigned long long)0x8000000000000000ULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)0x8000000000000000ULL; static _Bool r3 = (_Bool)(unsigned long long)0x8000000000000000ULL;
      printf("2363: (_Bool)(unsigned long long)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; float r1 = (float)v; float r2 = (float)(unsigned long long)0x8000000000000000ULL; static float r3 = (float)(unsigned long long)0x8000000000000000ULL;
      printf("2364: (float)(unsigned long long)0x8000000000000000ULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; double r1 = (double)v; double r2 = (double)(unsigned long long)0x8000000000000000ULL; static double r3 = (double)(unsigned long long)0x8000000000000000ULL;
      printf("2365: (double)(unsigned long long)0x8000000000000000ULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)0x8000000000000000ULL; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)0x8000000000000000ULL; static long double r3 = (long double)(unsigned long long)0x8000000000000000ULL;
      printf("2366: (long double)(unsigned long long)0x8000000000000000ULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(unsigned long long)0xffffffffffffffffULL; static signed char r3 = (signed char)(unsigned long long)0xffffffffffffffffULL;
      printf("2367: (signed char)(unsigned long long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(unsigned long long)0xffffffffffffffffULL; static unsigned char r3 = (unsigned char)(unsigned long long)0xffffffffffffffffULL;
      printf("2368: (unsigned char)(unsigned long long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; short r1 = (short)v; short r2 = (short)(unsigned long long)0xffffffffffffffffULL; static short r3 = (short)(unsigned long long)0xffffffffffffffffULL;
      printf("2369: (short)(unsigned long long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(unsigned long long)0xffffffffffffffffULL; static unsigned short r3 = (unsigned short)(unsigned long long)0xffffffffffffffffULL;
      printf("2370: (unsigned short)(unsigned long long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; int r1 = (int)v; int r2 = (int)(unsigned long long)0xffffffffffffffffULL; static int r3 = (int)(unsigned long long)0xffffffffffffffffULL;
      printf("2371: (int)(unsigned long long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(unsigned long long)0xffffffffffffffffULL; static unsigned r3 = (unsigned)(unsigned long long)0xffffffffffffffffULL;
      printf("2372: (unsigned)(unsigned long long)0xffffffffffffffffULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; long r1 = (long)v; long r2 = (long)(unsigned long long)0xffffffffffffffffULL; static long r3 = (long)(unsigned long long)0xffffffffffffffffULL;
      printf("2373: (long)(unsigned long long)0xffffffffffffffffULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(unsigned long long)0xffffffffffffffffULL; static unsigned long r3 = (unsigned long)(unsigned long long)0xffffffffffffffffULL;
      printf("2374: (unsigned long)(unsigned long long)0xffffffffffffffffULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; long long r1 = (long long)v; long long r2 = (long long)(unsigned long long)0xffffffffffffffffULL; static long long r3 = (long long)(unsigned long long)0xffffffffffffffffULL;
      printf("2375: (long long)(unsigned long long)0xffffffffffffffffULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(unsigned long long)0xffffffffffffffffULL; static unsigned long long r3 = (unsigned long long)(unsigned long long)0xffffffffffffffffULL;
      printf("2376: (unsigned long long)(unsigned long long)0xffffffffffffffffULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(unsigned long long)0xffffffffffffffffULL; static _Bool r3 = (_Bool)(unsigned long long)0xffffffffffffffffULL;
      printf("2377: (_Bool)(unsigned long long)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; float r1 = (float)v; float r2 = (float)(unsigned long long)0xffffffffffffffffULL; static float r3 = (float)(unsigned long long)0xffffffffffffffffULL;
      printf("2378: (float)(unsigned long long)0xffffffffffffffffULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; double r1 = (double)v; double r2 = (double)(unsigned long long)0xffffffffffffffffULL; static double r3 = (double)(unsigned long long)0xffffffffffffffffULL;
      printf("2379: (double)(unsigned long long)0xffffffffffffffffULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long v = (unsigned long long)0xffffffffffffffffULL; long double r1 = (long double)v; long double r2 = (long double)(unsigned long long)0xffffffffffffffffULL; static long double r3 = (long double)(unsigned long long)0xffffffffffffffffULL;
      printf("2380: (long double)(unsigned long long)0xffffffffffffffffULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)0; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)0; static signed char r3 = (signed char)(_Bool)0;
      printf("2381: (signed char)(_Bool)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)0; static unsigned char r3 = (unsigned char)(_Bool)0;
      printf("2382: (unsigned char)(_Bool)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0; short r1 = (short)v; short r2 = (short)(_Bool)0; static short r3 = (short)(_Bool)0;
      printf("2383: (short)(_Bool)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)0; static unsigned short r3 = (unsigned short)(_Bool)0;
      printf("2384: (unsigned short)(_Bool)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0; int r1 = (int)v; int r2 = (int)(_Bool)0; static int r3 = (int)(_Bool)0;
      printf("2385: (int)(_Bool)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)0; static unsigned r3 = (unsigned)(_Bool)0;
      printf("2386: (unsigned)(_Bool)0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)0; long r1 = (long)v; long r2 = (long)(_Bool)0; static long r3 = (long)(_Bool)0;
      printf("2387: (long)(_Bool)0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)0; static unsigned long r3 = (unsigned long)(_Bool)0;
      printf("2388: (unsigned long)(_Bool)0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)0; long long r1 = (long long)v; long long r2 = (long long)(_Bool)0; static long long r3 = (long long)(_Bool)0;
      printf("2389: (long long)(_Bool)0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)0; static unsigned long long r3 = (unsigned long long)(_Bool)0;
      printf("2390: (unsigned long long)(_Bool)0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)0; static _Bool r3 = (_Bool)(_Bool)0;
      printf("2391: (_Bool)(_Bool)0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0; float r1 = (float)v; float r2 = (float)(_Bool)0; static float r3 = (float)(_Bool)0;
      printf("2392: (float)(_Bool)0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)0; double r1 = (double)v; double r2 = (double)(_Bool)0; static double r3 = (double)(_Bool)0;
      printf("2393: (double)(_Bool)0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)0; long double r1 = (long double)v; long double r2 = (long double)(_Bool)0; static long double r3 = (long double)(_Bool)0;
      printf("2394: (long double)(_Bool)0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)1; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)1; static signed char r3 = (signed char)(_Bool)1;
      printf("2395: (signed char)(_Bool)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)1; static unsigned char r3 = (unsigned char)(_Bool)1;
      printf("2396: (unsigned char)(_Bool)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)1; short r1 = (short)v; short r2 = (short)(_Bool)1; static short r3 = (short)(_Bool)1;
      printf("2397: (short)(_Bool)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)1; static unsigned short r3 = (unsigned short)(_Bool)1;
      printf("2398: (unsigned short)(_Bool)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)1; int r1 = (int)v; int r2 = (int)(_Bool)1; static int r3 = (int)(_Bool)1;
      printf("2399: (int)(_Bool)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)1; static unsigned r3 = (unsigned)(_Bool)1;
      printf("2400: (unsigned)(_Bool)1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)1; long r1 = (long)v; long r2 = (long)(_Bool)1; static long r3 = (long)(_Bool)1;
      printf("2401: (long)(_Bool)1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)1; static unsigned long r3 = (unsigned long)(_Bool)1;
      printf("2402: (unsigned long)(_Bool)1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)1; long long r1 = (long long)v; long long r2 = (long long)(_Bool)1; static long long r3 = (long long)(_Bool)1;
      printf("2403: (long long)(_Bool)1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)1; static unsigned long long r3 = (unsigned long long)(_Bool)1;
      printf("2404: (unsigned long long)(_Bool)1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)1; static _Bool r3 = (_Bool)(_Bool)1;
      printf("2405: (_Bool)(_Bool)1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)1; float r1 = (float)v; float r2 = (float)(_Bool)1; static float r3 = (float)(_Bool)1;
      printf("2406: (float)(_Bool)1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)1; double r1 = (double)v; double r2 = (double)(_Bool)1; static double r3 = (double)(_Bool)1;
      printf("2407: (double)(_Bool)1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)1; long double r1 = (long double)v; long double r2 = (long double)(_Bool)1; static long double r3 = (long double)(_Bool)1;
      printf("2408: (long double)(_Bool)1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)-1; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)-1; static signed char r3 = (signed char)(_Bool)-1;
      printf("2409: (signed char)(_Bool)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-1; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)-1; static unsigned char r3 = (unsigned char)(_Bool)-1;
      printf("2410: (unsigned char)(_Bool)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-1; short r1 = (short)v; short r2 = (short)(_Bool)-1; static short r3 = (short)(_Bool)-1;
      printf("2411: (short)(_Bool)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-1; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)-1; static unsigned short r3 = (unsigned short)(_Bool)-1;
      printf("2412: (unsigned short)(_Bool)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-1; int r1 = (int)v; int r2 = (int)(_Bool)-1; static int r3 = (int)(_Bool)-1;
      printf("2413: (int)(_Bool)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-1; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)-1; static unsigned r3 = (unsigned)(_Bool)-1;
      printf("2414: (unsigned)(_Bool)-1 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)-1; long r1 = (long)v; long r2 = (long)(_Bool)-1; static long r3 = (long)(_Bool)-1;
      printf("2415: (long)(_Bool)-1 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)-1; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)-1; static unsigned long r3 = (unsigned long)(_Bool)-1;
      printf("2416: (unsigned long)(_Bool)-1 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)-1; long long r1 = (long long)v; long long r2 = (long long)(_Bool)-1; static long long r3 = (long long)(_Bool)-1;
      printf("2417: (long long)(_Bool)-1 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)-1; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)-1; static unsigned long long r3 = (unsigned long long)(_Bool)-1;
      printf("2418: (unsigned long long)(_Bool)-1 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)-1; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)-1; static _Bool r3 = (_Bool)(_Bool)-1;
      printf("2419: (_Bool)(_Bool)-1 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-1; float r1 = (float)v; float r2 = (float)(_Bool)-1; static float r3 = (float)(_Bool)-1;
      printf("2420: (float)(_Bool)-1 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)-1; double r1 = (double)v; double r2 = (double)(_Bool)-1; static double r3 = (double)(_Bool)-1;
      printf("2421: (double)(_Bool)-1 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)-1; long double r1 = (long double)v; long double r2 = (long double)(_Bool)-1; static long double r3 = (long double)(_Bool)-1;
      printf("2422: (long double)(_Bool)-1 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)127; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)127; static signed char r3 = (signed char)(_Bool)127;
      printf("2423: (signed char)(_Bool)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)127; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)127; static unsigned char r3 = (unsigned char)(_Bool)127;
      printf("2424: (unsigned char)(_Bool)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)127; short r1 = (short)v; short r2 = (short)(_Bool)127; static short r3 = (short)(_Bool)127;
      printf("2425: (short)(_Bool)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)127; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)127; static unsigned short r3 = (unsigned short)(_Bool)127;
      printf("2426: (unsigned short)(_Bool)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)127; int r1 = (int)v; int r2 = (int)(_Bool)127; static int r3 = (int)(_Bool)127;
      printf("2427: (int)(_Bool)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)127; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)127; static unsigned r3 = (unsigned)(_Bool)127;
      printf("2428: (unsigned)(_Bool)127 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)127; long r1 = (long)v; long r2 = (long)(_Bool)127; static long r3 = (long)(_Bool)127;
      printf("2429: (long)(_Bool)127 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)127; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)127; static unsigned long r3 = (unsigned long)(_Bool)127;
      printf("2430: (unsigned long)(_Bool)127 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)127; long long r1 = (long long)v; long long r2 = (long long)(_Bool)127; static long long r3 = (long long)(_Bool)127;
      printf("2431: (long long)(_Bool)127 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)127; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)127; static unsigned long long r3 = (unsigned long long)(_Bool)127;
      printf("2432: (unsigned long long)(_Bool)127 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)127; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)127; static _Bool r3 = (_Bool)(_Bool)127;
      printf("2433: (_Bool)(_Bool)127 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)127; float r1 = (float)v; float r2 = (float)(_Bool)127; static float r3 = (float)(_Bool)127;
      printf("2434: (float)(_Bool)127 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)127; double r1 = (double)v; double r2 = (double)(_Bool)127; static double r3 = (double)(_Bool)127;
      printf("2435: (double)(_Bool)127 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)127; long double r1 = (long double)v; long double r2 = (long double)(_Bool)127; static long double r3 = (long double)(_Bool)127;
      printf("2436: (long double)(_Bool)127 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)-128; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)-128; static signed char r3 = (signed char)(_Bool)-128;
      printf("2437: (signed char)(_Bool)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-128; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)-128; static unsigned char r3 = (unsigned char)(_Bool)-128;
      printf("2438: (unsigned char)(_Bool)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-128; short r1 = (short)v; short r2 = (short)(_Bool)-128; static short r3 = (short)(_Bool)-128;
      printf("2439: (short)(_Bool)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-128; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)-128; static unsigned short r3 = (unsigned short)(_Bool)-128;
      printf("2440: (unsigned short)(_Bool)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-128; int r1 = (int)v; int r2 = (int)(_Bool)-128; static int r3 = (int)(_Bool)-128;
      printf("2441: (int)(_Bool)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-128; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)-128; static unsigned r3 = (unsigned)(_Bool)-128;
      printf("2442: (unsigned)(_Bool)-128 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)-128; long r1 = (long)v; long r2 = (long)(_Bool)-128; static long r3 = (long)(_Bool)-128;
      printf("2443: (long)(_Bool)-128 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)-128; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)-128; static unsigned long r3 = (unsigned long)(_Bool)-128;
      printf("2444: (unsigned long)(_Bool)-128 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)-128; long long r1 = (long long)v; long long r2 = (long long)(_Bool)-128; static long long r3 = (long long)(_Bool)-128;
      printf("2445: (long long)(_Bool)-128 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)-128; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)-128; static unsigned long long r3 = (unsigned long long)(_Bool)-128;
      printf("2446: (unsigned long long)(_Bool)-128 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)-128; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)-128; static _Bool r3 = (_Bool)(_Bool)-128;
      printf("2447: (_Bool)(_Bool)-128 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-128; float r1 = (float)v; float r2 = (float)(_Bool)-128; static float r3 = (float)(_Bool)-128;
      printf("2448: (float)(_Bool)-128 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)-128; double r1 = (double)v; double r2 = (double)(_Bool)-128; static double r3 = (double)(_Bool)-128;
      printf("2449: (double)(_Bool)-128 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)-128; long double r1 = (long double)v; long double r2 = (long double)(_Bool)-128; static long double r3 = (long double)(_Bool)-128;
      printf("2450: (long double)(_Bool)-128 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)255; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)255; static signed char r3 = (signed char)(_Bool)255;
      printf("2451: (signed char)(_Bool)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)255; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)255; static unsigned char r3 = (unsigned char)(_Bool)255;
      printf("2452: (unsigned char)(_Bool)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)255; short r1 = (short)v; short r2 = (short)(_Bool)255; static short r3 = (short)(_Bool)255;
      printf("2453: (short)(_Bool)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)255; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)255; static unsigned short r3 = (unsigned short)(_Bool)255;
      printf("2454: (unsigned short)(_Bool)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)255; int r1 = (int)v; int r2 = (int)(_Bool)255; static int r3 = (int)(_Bool)255;
      printf("2455: (int)(_Bool)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)255; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)255; static unsigned r3 = (unsigned)(_Bool)255;
      printf("2456: (unsigned)(_Bool)255 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)255; long r1 = (long)v; long r2 = (long)(_Bool)255; static long r3 = (long)(_Bool)255;
      printf("2457: (long)(_Bool)255 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)255; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)255; static unsigned long r3 = (unsigned long)(_Bool)255;
      printf("2458: (unsigned long)(_Bool)255 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)255; long long r1 = (long long)v; long long r2 = (long long)(_Bool)255; static long long r3 = (long long)(_Bool)255;
      printf("2459: (long long)(_Bool)255 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)255; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)255; static unsigned long long r3 = (unsigned long long)(_Bool)255;
      printf("2460: (unsigned long long)(_Bool)255 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)255; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)255; static _Bool r3 = (_Bool)(_Bool)255;
      printf("2461: (_Bool)(_Bool)255 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)255; float r1 = (float)v; float r2 = (float)(_Bool)255; static float r3 = (float)(_Bool)255;
      printf("2462: (float)(_Bool)255 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)255; double r1 = (double)v; double r2 = (double)(_Bool)255; static double r3 = (double)(_Bool)255;
      printf("2463: (double)(_Bool)255 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)255; long double r1 = (long double)v; long double r2 = (long double)(_Bool)255; static long double r3 = (long double)(_Bool)255;
      printf("2464: (long double)(_Bool)255 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)256; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)256; static signed char r3 = (signed char)(_Bool)256;
      printf("2465: (signed char)(_Bool)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)256; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)256; static unsigned char r3 = (unsigned char)(_Bool)256;
      printf("2466: (unsigned char)(_Bool)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)256; short r1 = (short)v; short r2 = (short)(_Bool)256; static short r3 = (short)(_Bool)256;
      printf("2467: (short)(_Bool)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)256; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)256; static unsigned short r3 = (unsigned short)(_Bool)256;
      printf("2468: (unsigned short)(_Bool)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)256; int r1 = (int)v; int r2 = (int)(_Bool)256; static int r3 = (int)(_Bool)256;
      printf("2469: (int)(_Bool)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)256; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)256; static unsigned r3 = (unsigned)(_Bool)256;
      printf("2470: (unsigned)(_Bool)256 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)256; long r1 = (long)v; long r2 = (long)(_Bool)256; static long r3 = (long)(_Bool)256;
      printf("2471: (long)(_Bool)256 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)256; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)256; static unsigned long r3 = (unsigned long)(_Bool)256;
      printf("2472: (unsigned long)(_Bool)256 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)256; long long r1 = (long long)v; long long r2 = (long long)(_Bool)256; static long long r3 = (long long)(_Bool)256;
      printf("2473: (long long)(_Bool)256 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)256; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)256; static unsigned long long r3 = (unsigned long long)(_Bool)256;
      printf("2474: (unsigned long long)(_Bool)256 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)256; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)256; static _Bool r3 = (_Bool)(_Bool)256;
      printf("2475: (_Bool)(_Bool)256 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)256; float r1 = (float)v; float r2 = (float)(_Bool)256; static float r3 = (float)(_Bool)256;
      printf("2476: (float)(_Bool)256 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)256; double r1 = (double)v; double r2 = (double)(_Bool)256; static double r3 = (double)(_Bool)256;
      printf("2477: (double)(_Bool)256 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)256; long double r1 = (long double)v; long double r2 = (long double)(_Bool)256; static long double r3 = (long double)(_Bool)256;
      printf("2478: (long double)(_Bool)256 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)32767; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)32767; static signed char r3 = (signed char)(_Bool)32767;
      printf("2479: (signed char)(_Bool)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)32767; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)32767; static unsigned char r3 = (unsigned char)(_Bool)32767;
      printf("2480: (unsigned char)(_Bool)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)32767; short r1 = (short)v; short r2 = (short)(_Bool)32767; static short r3 = (short)(_Bool)32767;
      printf("2481: (short)(_Bool)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)32767; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)32767; static unsigned short r3 = (unsigned short)(_Bool)32767;
      printf("2482: (unsigned short)(_Bool)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)32767; int r1 = (int)v; int r2 = (int)(_Bool)32767; static int r3 = (int)(_Bool)32767;
      printf("2483: (int)(_Bool)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)32767; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)32767; static unsigned r3 = (unsigned)(_Bool)32767;
      printf("2484: (unsigned)(_Bool)32767 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)32767; long r1 = (long)v; long r2 = (long)(_Bool)32767; static long r3 = (long)(_Bool)32767;
      printf("2485: (long)(_Bool)32767 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)32767; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)32767; static unsigned long r3 = (unsigned long)(_Bool)32767;
      printf("2486: (unsigned long)(_Bool)32767 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)32767; long long r1 = (long long)v; long long r2 = (long long)(_Bool)32767; static long long r3 = (long long)(_Bool)32767;
      printf("2487: (long long)(_Bool)32767 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)32767; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)32767; static unsigned long long r3 = (unsigned long long)(_Bool)32767;
      printf("2488: (unsigned long long)(_Bool)32767 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)32767; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)32767; static _Bool r3 = (_Bool)(_Bool)32767;
      printf("2489: (_Bool)(_Bool)32767 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)32767; float r1 = (float)v; float r2 = (float)(_Bool)32767; static float r3 = (float)(_Bool)32767;
      printf("2490: (float)(_Bool)32767 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)32767; double r1 = (double)v; double r2 = (double)(_Bool)32767; static double r3 = (double)(_Bool)32767;
      printf("2491: (double)(_Bool)32767 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)32767; long double r1 = (long double)v; long double r2 = (long double)(_Bool)32767; static long double r3 = (long double)(_Bool)32767;
      printf("2492: (long double)(_Bool)32767 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)-32768; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)-32768; static signed char r3 = (signed char)(_Bool)-32768;
      printf("2493: (signed char)(_Bool)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-32768; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)-32768; static unsigned char r3 = (unsigned char)(_Bool)-32768;
      printf("2494: (unsigned char)(_Bool)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-32768; short r1 = (short)v; short r2 = (short)(_Bool)-32768; static short r3 = (short)(_Bool)-32768;
      printf("2495: (short)(_Bool)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-32768; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)-32768; static unsigned short r3 = (unsigned short)(_Bool)-32768;
      printf("2496: (unsigned short)(_Bool)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-32768; int r1 = (int)v; int r2 = (int)(_Bool)-32768; static int r3 = (int)(_Bool)-32768;
      printf("2497: (int)(_Bool)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-32768; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)-32768; static unsigned r3 = (unsigned)(_Bool)-32768;
      printf("2498: (unsigned)(_Bool)-32768 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)-32768; long r1 = (long)v; long r2 = (long)(_Bool)-32768; static long r3 = (long)(_Bool)-32768;
      printf("2499: (long)(_Bool)-32768 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)-32768; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)-32768; static unsigned long r3 = (unsigned long)(_Bool)-32768;
      printf("2500: (unsigned long)(_Bool)-32768 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)-32768; long long r1 = (long long)v; long long r2 = (long long)(_Bool)-32768; static long long r3 = (long long)(_Bool)-32768;
      printf("2501: (long long)(_Bool)-32768 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)-32768; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)-32768; static unsigned long long r3 = (unsigned long long)(_Bool)-32768;
      printf("2502: (unsigned long long)(_Bool)-32768 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)-32768; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)-32768; static _Bool r3 = (_Bool)(_Bool)-32768;
      printf("2503: (_Bool)(_Bool)-32768 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)-32768; float r1 = (float)v; float r2 = (float)(_Bool)-32768; static float r3 = (float)(_Bool)-32768;
      printf("2504: (float)(_Bool)-32768 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)-32768; double r1 = (double)v; double r2 = (double)(_Bool)-32768; static double r3 = (double)(_Bool)-32768;
      printf("2505: (double)(_Bool)-32768 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)-32768; long double r1 = (long double)v; long double r2 = (long double)(_Bool)-32768; static long double r3 = (long double)(_Bool)-32768;
      printf("2506: (long double)(_Bool)-32768 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)65535; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)65535; static signed char r3 = (signed char)(_Bool)65535;
      printf("2507: (signed char)(_Bool)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)65535; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)65535; static unsigned char r3 = (unsigned char)(_Bool)65535;
      printf("2508: (unsigned char)(_Bool)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)65535; short r1 = (short)v; short r2 = (short)(_Bool)65535; static short r3 = (short)(_Bool)65535;
      printf("2509: (short)(_Bool)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)65535; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)65535; static unsigned short r3 = (unsigned short)(_Bool)65535;
      printf("2510: (unsigned short)(_Bool)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)65535; int r1 = (int)v; int r2 = (int)(_Bool)65535; static int r3 = (int)(_Bool)65535;
      printf("2511: (int)(_Bool)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)65535; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)65535; static unsigned r3 = (unsigned)(_Bool)65535;
      printf("2512: (unsigned)(_Bool)65535 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)65535; long r1 = (long)v; long r2 = (long)(_Bool)65535; static long r3 = (long)(_Bool)65535;
      printf("2513: (long)(_Bool)65535 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)65535; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)65535; static unsigned long r3 = (unsigned long)(_Bool)65535;
      printf("2514: (unsigned long)(_Bool)65535 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)65535; long long r1 = (long long)v; long long r2 = (long long)(_Bool)65535; static long long r3 = (long long)(_Bool)65535;
      printf("2515: (long long)(_Bool)65535 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)65535; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)65535; static unsigned long long r3 = (unsigned long long)(_Bool)65535;
      printf("2516: (unsigned long long)(_Bool)65535 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)65535; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)65535; static _Bool r3 = (_Bool)(_Bool)65535;
      printf("2517: (_Bool)(_Bool)65535 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)65535; float r1 = (float)v; float r2 = (float)(_Bool)65535; static float r3 = (float)(_Bool)65535;
      printf("2518: (float)(_Bool)65535 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)65535; double r1 = (double)v; double r2 = (double)(_Bool)65535; static double r3 = (double)(_Bool)65535;
      printf("2519: (double)(_Bool)65535 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)65535; long double r1 = (long double)v; long double r2 = (long double)(_Bool)65535; static long double r3 = (long double)(_Bool)65535;
      printf("2520: (long double)(_Bool)65535 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)2147483647; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)2147483647; static signed char r3 = (signed char)(_Bool)2147483647;
      printf("2521: (signed char)(_Bool)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)2147483647; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)2147483647; static unsigned char r3 = (unsigned char)(_Bool)2147483647;
      printf("2522: (unsigned char)(_Bool)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)2147483647; short r1 = (short)v; short r2 = (short)(_Bool)2147483647; static short r3 = (short)(_Bool)2147483647;
      printf("2523: (short)(_Bool)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)2147483647; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)2147483647; static unsigned short r3 = (unsigned short)(_Bool)2147483647;
      printf("2524: (unsigned short)(_Bool)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)2147483647; int r1 = (int)v; int r2 = (int)(_Bool)2147483647; static int r3 = (int)(_Bool)2147483647;
      printf("2525: (int)(_Bool)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)2147483647; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)2147483647; static unsigned r3 = (unsigned)(_Bool)2147483647;
      printf("2526: (unsigned)(_Bool)2147483647 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)2147483647; long r1 = (long)v; long r2 = (long)(_Bool)2147483647; static long r3 = (long)(_Bool)2147483647;
      printf("2527: (long)(_Bool)2147483647 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)2147483647; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)2147483647; static unsigned long r3 = (unsigned long)(_Bool)2147483647;
      printf("2528: (unsigned long)(_Bool)2147483647 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)2147483647; long long r1 = (long long)v; long long r2 = (long long)(_Bool)2147483647; static long long r3 = (long long)(_Bool)2147483647;
      printf("2529: (long long)(_Bool)2147483647 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)2147483647; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)2147483647; static unsigned long long r3 = (unsigned long long)(_Bool)2147483647;
      printf("2530: (unsigned long long)(_Bool)2147483647 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)2147483647; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)2147483647; static _Bool r3 = (_Bool)(_Bool)2147483647;
      printf("2531: (_Bool)(_Bool)2147483647 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)2147483647; float r1 = (float)v; float r2 = (float)(_Bool)2147483647; static float r3 = (float)(_Bool)2147483647;
      printf("2532: (float)(_Bool)2147483647 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)2147483647; double r1 = (double)v; double r2 = (double)(_Bool)2147483647; static double r3 = (double)(_Bool)2147483647;
      printf("2533: (double)(_Bool)2147483647 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)2147483647; long double r1 = (long double)v; long double r2 = (long double)(_Bool)2147483647; static long double r3 = (long double)(_Bool)2147483647;
      printf("2534: (long double)(_Bool)2147483647 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)(-2147483647-1); static signed char r3 = (signed char)(_Bool)(-2147483647-1);
      printf("2535: (signed char)(_Bool)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)(-2147483647-1); static unsigned char r3 = (unsigned char)(_Bool)(-2147483647-1);
      printf("2536: (unsigned char)(_Bool)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); short r1 = (short)v; short r2 = (short)(_Bool)(-2147483647-1); static short r3 = (short)(_Bool)(-2147483647-1);
      printf("2537: (short)(_Bool)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)(-2147483647-1); static unsigned short r3 = (unsigned short)(_Bool)(-2147483647-1);
      printf("2538: (unsigned short)(_Bool)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); int r1 = (int)v; int r2 = (int)(_Bool)(-2147483647-1); static int r3 = (int)(_Bool)(-2147483647-1);
      printf("2539: (int)(_Bool)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)(-2147483647-1); static unsigned r3 = (unsigned)(_Bool)(-2147483647-1);
      printf("2540: (unsigned)(_Bool)(-2147483647-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); long r1 = (long)v; long r2 = (long)(_Bool)(-2147483647-1); static long r3 = (long)(_Bool)(-2147483647-1);
      printf("2541: (long)(_Bool)(-2147483647-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)(-2147483647-1); static unsigned long r3 = (unsigned long)(_Bool)(-2147483647-1);
      printf("2542: (unsigned long)(_Bool)(-2147483647-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); long long r1 = (long long)v; long long r2 = (long long)(_Bool)(-2147483647-1); static long long r3 = (long long)(_Bool)(-2147483647-1);
      printf("2543: (long long)(_Bool)(-2147483647-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)(-2147483647-1); static unsigned long long r3 = (unsigned long long)(_Bool)(-2147483647-1);
      printf("2544: (unsigned long long)(_Bool)(-2147483647-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)(-2147483647-1); static _Bool r3 = (_Bool)(_Bool)(-2147483647-1);
      printf("2545: (_Bool)(_Bool)(-2147483647-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); float r1 = (float)v; float r2 = (float)(_Bool)(-2147483647-1); static float r3 = (float)(_Bool)(-2147483647-1);
      printf("2546: (float)(_Bool)(-2147483647-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); double r1 = (double)v; double r2 = (double)(_Bool)(-2147483647-1); static double r3 = (double)(_Bool)(-2147483647-1);
      printf("2547: (double)(_Bool)(-2147483647-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)(-2147483647-1); long double r1 = (long double)v; long double r2 = (long double)(_Bool)(-2147483647-1); static long double r3 = (long double)(_Bool)(-2147483647-1);
      printf("2548: (long double)(_Bool)(-2147483647-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)4294967295LL; static signed char r3 = (signed char)(_Bool)4294967295LL;
      printf("2549: (signed char)(_Bool)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)4294967295LL; static unsigned char r3 = (unsigned char)(_Bool)4294967295LL;
      printf("2550: (unsigned char)(_Bool)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; short r1 = (short)v; short r2 = (short)(_Bool)4294967295LL; static short r3 = (short)(_Bool)4294967295LL;
      printf("2551: (short)(_Bool)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)4294967295LL; static unsigned short r3 = (unsigned short)(_Bool)4294967295LL;
      printf("2552: (unsigned short)(_Bool)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; int r1 = (int)v; int r2 = (int)(_Bool)4294967295LL; static int r3 = (int)(_Bool)4294967295LL;
      printf("2553: (int)(_Bool)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)4294967295LL; static unsigned r3 = (unsigned)(_Bool)4294967295LL;
      printf("2554: (unsigned)(_Bool)4294967295LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; long r1 = (long)v; long r2 = (long)(_Bool)4294967295LL; static long r3 = (long)(_Bool)4294967295LL;
      printf("2555: (long)(_Bool)4294967295LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)4294967295LL; static unsigned long r3 = (unsigned long)(_Bool)4294967295LL;
      printf("2556: (unsigned long)(_Bool)4294967295LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; long long r1 = (long long)v; long long r2 = (long long)(_Bool)4294967295LL; static long long r3 = (long long)(_Bool)4294967295LL;
      printf("2557: (long long)(_Bool)4294967295LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)4294967295LL; static unsigned long long r3 = (unsigned long long)(_Bool)4294967295LL;
      printf("2558: (unsigned long long)(_Bool)4294967295LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)4294967295LL; static _Bool r3 = (_Bool)(_Bool)4294967295LL;
      printf("2559: (_Bool)(_Bool)4294967295LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; float r1 = (float)v; float r2 = (float)(_Bool)4294967295LL; static float r3 = (float)(_Bool)4294967295LL;
      printf("2560: (float)(_Bool)4294967295LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; double r1 = (double)v; double r2 = (double)(_Bool)4294967295LL; static double r3 = (double)(_Bool)4294967295LL;
      printf("2561: (double)(_Bool)4294967295LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)4294967295LL; long double r1 = (long double)v; long double r2 = (long double)(_Bool)4294967295LL; static long double r3 = (long double)(_Bool)4294967295LL;
      printf("2562: (long double)(_Bool)4294967295LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)9223372036854775807LL; static signed char r3 = (signed char)(_Bool)9223372036854775807LL;
      printf("2563: (signed char)(_Bool)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)9223372036854775807LL; static unsigned char r3 = (unsigned char)(_Bool)9223372036854775807LL;
      printf("2564: (unsigned char)(_Bool)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; short r1 = (short)v; short r2 = (short)(_Bool)9223372036854775807LL; static short r3 = (short)(_Bool)9223372036854775807LL;
      printf("2565: (short)(_Bool)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)9223372036854775807LL; static unsigned short r3 = (unsigned short)(_Bool)9223372036854775807LL;
      printf("2566: (unsigned short)(_Bool)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; int r1 = (int)v; int r2 = (int)(_Bool)9223372036854775807LL; static int r3 = (int)(_Bool)9223372036854775807LL;
      printf("2567: (int)(_Bool)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)9223372036854775807LL; static unsigned r3 = (unsigned)(_Bool)9223372036854775807LL;
      printf("2568: (unsigned)(_Bool)9223372036854775807LL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; long r1 = (long)v; long r2 = (long)(_Bool)9223372036854775807LL; static long r3 = (long)(_Bool)9223372036854775807LL;
      printf("2569: (long)(_Bool)9223372036854775807LL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)9223372036854775807LL; static unsigned long r3 = (unsigned long)(_Bool)9223372036854775807LL;
      printf("2570: (unsigned long)(_Bool)9223372036854775807LL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; long long r1 = (long long)v; long long r2 = (long long)(_Bool)9223372036854775807LL; static long long r3 = (long long)(_Bool)9223372036854775807LL;
      printf("2571: (long long)(_Bool)9223372036854775807LL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)9223372036854775807LL; static unsigned long long r3 = (unsigned long long)(_Bool)9223372036854775807LL;
      printf("2572: (unsigned long long)(_Bool)9223372036854775807LL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)9223372036854775807LL; static _Bool r3 = (_Bool)(_Bool)9223372036854775807LL;
      printf("2573: (_Bool)(_Bool)9223372036854775807LL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; float r1 = (float)v; float r2 = (float)(_Bool)9223372036854775807LL; static float r3 = (float)(_Bool)9223372036854775807LL;
      printf("2574: (float)(_Bool)9223372036854775807LL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; double r1 = (double)v; double r2 = (double)(_Bool)9223372036854775807LL; static double r3 = (double)(_Bool)9223372036854775807LL;
      printf("2575: (double)(_Bool)9223372036854775807LL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)9223372036854775807LL; long double r1 = (long double)v; long double r2 = (long double)(_Bool)9223372036854775807LL; static long double r3 = (long double)(_Bool)9223372036854775807LL;
      printf("2576: (long double)(_Bool)9223372036854775807LL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)(-9223372036854775807LL-1); static signed char r3 = (signed char)(_Bool)(-9223372036854775807LL-1);
      printf("2577: (signed char)(_Bool)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)(-9223372036854775807LL-1); static unsigned char r3 = (unsigned char)(_Bool)(-9223372036854775807LL-1);
      printf("2578: (unsigned char)(_Bool)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); short r1 = (short)v; short r2 = (short)(_Bool)(-9223372036854775807LL-1); static short r3 = (short)(_Bool)(-9223372036854775807LL-1);
      printf("2579: (short)(_Bool)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)(-9223372036854775807LL-1); static unsigned short r3 = (unsigned short)(_Bool)(-9223372036854775807LL-1);
      printf("2580: (unsigned short)(_Bool)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); int r1 = (int)v; int r2 = (int)(_Bool)(-9223372036854775807LL-1); static int r3 = (int)(_Bool)(-9223372036854775807LL-1);
      printf("2581: (int)(_Bool)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)(-9223372036854775807LL-1); static unsigned r3 = (unsigned)(_Bool)(-9223372036854775807LL-1);
      printf("2582: (unsigned)(_Bool)(-9223372036854775807LL-1) = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); long r1 = (long)v; long r2 = (long)(_Bool)(-9223372036854775807LL-1); static long r3 = (long)(_Bool)(-9223372036854775807LL-1);
      printf("2583: (long)(_Bool)(-9223372036854775807LL-1) = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)(-9223372036854775807LL-1); static unsigned long r3 = (unsigned long)(_Bool)(-9223372036854775807LL-1);
      printf("2584: (unsigned long)(_Bool)(-9223372036854775807LL-1) = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); long long r1 = (long long)v; long long r2 = (long long)(_Bool)(-9223372036854775807LL-1); static long long r3 = (long long)(_Bool)(-9223372036854775807LL-1);
      printf("2585: (long long)(_Bool)(-9223372036854775807LL-1) = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)(-9223372036854775807LL-1); static unsigned long long r3 = (unsigned long long)(_Bool)(-9223372036854775807LL-1);
      printf("2586: (unsigned long long)(_Bool)(-9223372036854775807LL-1) = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)(-9223372036854775807LL-1); static _Bool r3 = (_Bool)(_Bool)(-9223372036854775807LL-1);
      printf("2587: (_Bool)(_Bool)(-9223372036854775807LL-1) = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); float r1 = (float)v; float r2 = (float)(_Bool)(-9223372036854775807LL-1); static float r3 = (float)(_Bool)(-9223372036854775807LL-1);
      printf("2588: (float)(_Bool)(-9223372036854775807LL-1) = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); double r1 = (double)v; double r2 = (double)(_Bool)(-9223372036854775807LL-1); static double r3 = (double)(_Bool)(-9223372036854775807LL-1);
      printf("2589: (double)(_Bool)(-9223372036854775807LL-1) = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)(-9223372036854775807LL-1); long double r1 = (long double)v; long double r2 = (long double)(_Bool)(-9223372036854775807LL-1); static long double r3 = (long double)(_Bool)(-9223372036854775807LL-1);
      printf("2590: (long double)(_Bool)(-9223372036854775807LL-1) = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)0x8000000000000000ULL; static signed char r3 = (signed char)(_Bool)0x8000000000000000ULL;
      printf("2591: (signed char)(_Bool)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)0x8000000000000000ULL; static unsigned char r3 = (unsigned char)(_Bool)0x8000000000000000ULL;
      printf("2592: (unsigned char)(_Bool)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; short r1 = (short)v; short r2 = (short)(_Bool)0x8000000000000000ULL; static short r3 = (short)(_Bool)0x8000000000000000ULL;
      printf("2593: (short)(_Bool)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)0x8000000000000000ULL; static unsigned short r3 = (unsigned short)(_Bool)0x8000000000000000ULL;
      printf("2594: (unsigned short)(_Bool)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; int r1 = (int)v; int r2 = (int)(_Bool)0x8000000000000000ULL; static int r3 = (int)(_Bool)0x8000000000000000ULL;
      printf("2595: (int)(_Bool)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)0x8000000000000000ULL; static unsigned r3 = (unsigned)(_Bool)0x8000000000000000ULL;
      printf("2596: (unsigned)(_Bool)0x8000000000000000ULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; long r1 = (long)v; long r2 = (long)(_Bool)0x8000000000000000ULL; static long r3 = (long)(_Bool)0x8000000000000000ULL;
      printf("2597: (long)(_Bool)0x8000000000000000ULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)0x8000000000000000ULL; static unsigned long r3 = (unsigned long)(_Bool)0x8000000000000000ULL;
      printf("2598: (unsigned long)(_Bool)0x8000000000000000ULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; long long r1 = (long long)v; long long r2 = (long long)(_Bool)0x8000000000000000ULL; static long long r3 = (long long)(_Bool)0x8000000000000000ULL;
      printf("2599: (long long)(_Bool)0x8000000000000000ULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)0x8000000000000000ULL; static unsigned long long r3 = (unsigned long long)(_Bool)0x8000000000000000ULL;
      printf("2600: (unsigned long long)(_Bool)0x8000000000000000ULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)0x8000000000000000ULL; static _Bool r3 = (_Bool)(_Bool)0x8000000000000000ULL;
      printf("2601: (_Bool)(_Bool)0x8000000000000000ULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; float r1 = (float)v; float r2 = (float)(_Bool)0x8000000000000000ULL; static float r3 = (float)(_Bool)0x8000000000000000ULL;
      printf("2602: (float)(_Bool)0x8000000000000000ULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; double r1 = (double)v; double r2 = (double)(_Bool)0x8000000000000000ULL; static double r3 = (double)(_Bool)0x8000000000000000ULL;
      printf("2603: (double)(_Bool)0x8000000000000000ULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)0x8000000000000000ULL; long double r1 = (long double)v; long double r2 = (long double)(_Bool)0x8000000000000000ULL; static long double r3 = (long double)(_Bool)0x8000000000000000ULL;
      printf("2604: (long double)(_Bool)0x8000000000000000ULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; signed char r1 = (signed char)v; signed char r2 = (signed char)(_Bool)0xffffffffffffffffULL; static signed char r3 = (signed char)(_Bool)0xffffffffffffffffULL;
      printf("2605: (signed char)(_Bool)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(_Bool)0xffffffffffffffffULL; static unsigned char r3 = (unsigned char)(_Bool)0xffffffffffffffffULL;
      printf("2606: (unsigned char)(_Bool)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; short r1 = (short)v; short r2 = (short)(_Bool)0xffffffffffffffffULL; static short r3 = (short)(_Bool)0xffffffffffffffffULL;
      printf("2607: (short)(_Bool)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(_Bool)0xffffffffffffffffULL; static unsigned short r3 = (unsigned short)(_Bool)0xffffffffffffffffULL;
      printf("2608: (unsigned short)(_Bool)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; int r1 = (int)v; int r2 = (int)(_Bool)0xffffffffffffffffULL; static int r3 = (int)(_Bool)0xffffffffffffffffULL;
      printf("2609: (int)(_Bool)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(_Bool)0xffffffffffffffffULL; static unsigned r3 = (unsigned)(_Bool)0xffffffffffffffffULL;
      printf("2610: (unsigned)(_Bool)0xffffffffffffffffULL = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; long r1 = (long)v; long r2 = (long)(_Bool)0xffffffffffffffffULL; static long r3 = (long)(_Bool)0xffffffffffffffffULL;
      printf("2611: (long)(_Bool)0xffffffffffffffffULL = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(_Bool)0xffffffffffffffffULL; static unsigned long r3 = (unsigned long)(_Bool)0xffffffffffffffffULL;
      printf("2612: (unsigned long)(_Bool)0xffffffffffffffffULL = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; long long r1 = (long long)v; long long r2 = (long long)(_Bool)0xffffffffffffffffULL; static long long r3 = (long long)(_Bool)0xffffffffffffffffULL;
      printf("2613: (long long)(_Bool)0xffffffffffffffffULL = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(_Bool)0xffffffffffffffffULL; static unsigned long long r3 = (unsigned long long)(_Bool)0xffffffffffffffffULL;
      printf("2614: (unsigned long long)(_Bool)0xffffffffffffffffULL = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(_Bool)0xffffffffffffffffULL; static _Bool r3 = (_Bool)(_Bool)0xffffffffffffffffULL;
      printf("2615: (_Bool)(_Bool)0xffffffffffffffffULL = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; float r1 = (float)v; float r2 = (float)(_Bool)0xffffffffffffffffULL; static float r3 = (float)(_Bool)0xffffffffffffffffULL;
      printf("2616: (float)(_Bool)0xffffffffffffffffULL = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; double r1 = (double)v; double r2 = (double)(_Bool)0xffffffffffffffffULL; static double r3 = (double)(_Bool)0xffffffffffffffffULL;
      printf("2617: (double)(_Bool)0xffffffffffffffffULL = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile _Bool v = (_Bool)0xffffffffffffffffULL; long double r1 = (long double)v; long double r2 = (long double)(_Bool)0xffffffffffffffffULL; static long double r3 = (long double)(_Bool)0xffffffffffffffffULL;
      printf("2618: (long double)(_Bool)0xffffffffffffffffULL = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)0.0; signed char r1 = (signed char)v; signed char r2 = (signed char)(float)0.0; static signed char r3 = (signed char)(float)0.0;
      printf("2619: (signed char)(float)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)0.0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(float)0.0; static unsigned char r3 = (unsigned char)(float)0.0;
      printf("2620: (unsigned char)(float)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)0.0; short r1 = (short)v; short r2 = (short)(float)0.0; static short r3 = (short)(float)0.0;
      printf("2621: (short)(float)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)0.0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(float)0.0; static unsigned short r3 = (unsigned short)(float)0.0;
      printf("2622: (unsigned short)(float)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)0.0; int r1 = (int)v; int r2 = (int)(float)0.0; static int r3 = (int)(float)0.0;
      printf("2623: (int)(float)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)0.0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(float)0.0; static unsigned r3 = (unsigned)(float)0.0;
      printf("2624: (unsigned)(float)0.0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile float v = (float)0.0; long r1 = (long)v; long r2 = (long)(float)0.0; static long r3 = (long)(float)0.0;
      printf("2625: (long)(float)0.0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)0.0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)0.0; static unsigned long r3 = (unsigned long)(float)0.0;
      printf("2626: (unsigned long)(float)0.0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)0.0; long long r1 = (long long)v; long long r2 = (long long)(float)0.0; static long long r3 = (long long)(float)0.0;
      printf("2627: (long long)(float)0.0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)0.0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)0.0; static unsigned long long r3 = (unsigned long long)(float)0.0;
      printf("2628: (unsigned long long)(float)0.0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)0.0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)0.0; static _Bool r3 = (_Bool)(float)0.0;
      printf("2629: (_Bool)(float)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)0.0; float r1 = (float)v; float r2 = (float)(float)0.0; static float r3 = (float)(float)0.0;
      printf("2630: (float)(float)0.0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)0.0; double r1 = (double)v; double r2 = (double)(float)0.0; static double r3 = (double)(float)0.0;
      printf("2631: (double)(float)0.0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)0.0; long double r1 = (long double)v; long double r2 = (long double)(float)0.0; static long double r3 = (long double)(float)0.0;
      printf("2632: (long double)(float)0.0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)-0.0; signed char r1 = (signed char)v; signed char r2 = (signed char)(float)-0.0; static signed char r3 = (signed char)(float)-0.0;
      printf("2633: (signed char)(float)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-0.0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(float)-0.0; static unsigned char r3 = (unsigned char)(float)-0.0;
      printf("2634: (unsigned char)(float)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-0.0; short r1 = (short)v; short r2 = (short)(float)-0.0; static short r3 = (short)(float)-0.0;
      printf("2635: (short)(float)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-0.0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(float)-0.0; static unsigned short r3 = (unsigned short)(float)-0.0;
      printf("2636: (unsigned short)(float)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-0.0; int r1 = (int)v; int r2 = (int)(float)-0.0; static int r3 = (int)(float)-0.0;
      printf("2637: (int)(float)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-0.0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(float)-0.0; static unsigned r3 = (unsigned)(float)-0.0;
      printf("2638: (unsigned)(float)-0.0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile float v = (float)-0.0; long r1 = (long)v; long r2 = (long)(float)-0.0; static long r3 = (long)(float)-0.0;
      printf("2639: (long)(float)-0.0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)-0.0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)-0.0; static unsigned long r3 = (unsigned long)(float)-0.0;
      printf("2640: (unsigned long)(float)-0.0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)-0.0; long long r1 = (long long)v; long long r2 = (long long)(float)-0.0; static long long r3 = (long long)(float)-0.0;
      printf("2641: (long long)(float)-0.0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)-0.0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)-0.0; static unsigned long long r3 = (unsigned long long)(float)-0.0;
      printf("2642: (unsigned long long)(float)-0.0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)-0.0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)-0.0; static _Bool r3 = (_Bool)(float)-0.0;
      printf("2643: (_Bool)(float)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-0.0; float r1 = (float)v; float r2 = (float)(float)-0.0; static float r3 = (float)(float)-0.0;
      printf("2644: (float)(float)-0.0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)-0.0; double r1 = (double)v; double r2 = (double)(float)-0.0; static double r3 = (double)(float)-0.0;
      printf("2645: (double)(float)-0.0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)-0.0; long double r1 = (long double)v; long double r2 = (long double)(float)-0.0; static long double r3 = (long double)(float)-0.0;
      printf("2646: (long double)(float)-0.0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)0.5; signed char r1 = (signed char)v; signed char r2 = (signed char)(float)0.5; static signed char r3 = (signed char)(float)0.5;
      printf("2647: (signed char)(float)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)0.5; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(float)0.5; static unsigned char r3 = (unsigned char)(float)0.5;
      printf("2648: (unsigned char)(float)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)0.5; short r1 = (short)v; short r2 = (short)(float)0.5; static short r3 = (short)(float)0.5;
      printf("2649: (short)(float)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)0.5; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(float)0.5; static unsigned short r3 = (unsigned short)(float)0.5;
      printf("2650: (unsigned short)(float)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)0.5; int r1 = (int)v; int r2 = (int)(float)0.5; static int r3 = (int)(float)0.5;
      printf("2651: (int)(float)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)0.5; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(float)0.5; static unsigned r3 = (unsigned)(float)0.5;
      printf("2652: (unsigned)(float)0.5 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile float v = (float)0.5; long r1 = (long)v; long r2 = (long)(float)0.5; static long r3 = (long)(float)0.5;
      printf("2653: (long)(float)0.5 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)0.5; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)0.5; static unsigned long r3 = (unsigned long)(float)0.5;
      printf("2654: (unsigned long)(float)0.5 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)0.5; long long r1 = (long long)v; long long r2 = (long long)(float)0.5; static long long r3 = (long long)(float)0.5;
      printf("2655: (long long)(float)0.5 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)0.5; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)0.5; static unsigned long long r3 = (unsigned long long)(float)0.5;
      printf("2656: (unsigned long long)(float)0.5 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)0.5; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)0.5; static _Bool r3 = (_Bool)(float)0.5;
      printf("2657: (_Bool)(float)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)0.5; float r1 = (float)v; float r2 = (float)(float)0.5; static float r3 = (float)(float)0.5;
      printf("2658: (float)(float)0.5 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)0.5; double r1 = (double)v; double r2 = (double)(float)0.5; static double r3 = (double)(float)0.5;
      printf("2659: (double)(float)0.5 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)0.5; long double r1 = (long double)v; long double r2 = (long double)(float)0.5; static long double r3 = (long double)(float)0.5;
      printf("2660: (long double)(float)0.5 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)-0.5; signed char r1 = (signed char)v; signed char r2 = (signed char)(float)-0.5; static signed char r3 = (signed char)(float)-0.5;
      printf("2661: (signed char)(float)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-0.5; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(float)-0.5; static unsigned char r3 = (unsigned char)(float)-0.5;
      printf("2662: (unsigned char)(float)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-0.5; short r1 = (short)v; short r2 = (short)(float)-0.5; static short r3 = (short)(float)-0.5;
      printf("2663: (short)(float)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-0.5; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(float)-0.5; static unsigned short r3 = (unsigned short)(float)-0.5;
      printf("2664: (unsigned short)(float)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-0.5; int r1 = (int)v; int r2 = (int)(float)-0.5; static int r3 = (int)(float)-0.5;
      printf("2665: (int)(float)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-0.5; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(float)-0.5; static unsigned r3 = (unsigned)(float)-0.5;
      printf("2666: (unsigned)(float)-0.5 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile float v = (float)-0.5; long r1 = (long)v; long r2 = (long)(float)-0.5; static long r3 = (long)(float)-0.5;
      printf("2667: (long)(float)-0.5 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)-0.5; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)-0.5; static unsigned long r3 = (unsigned long)(float)-0.5;
      printf("2668: (unsigned long)(float)-0.5 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)-0.5; long long r1 = (long long)v; long long r2 = (long long)(float)-0.5; static long long r3 = (long long)(float)-0.5;
      printf("2669: (long long)(float)-0.5 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)-0.5; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)-0.5; static unsigned long long r3 = (unsigned long long)(float)-0.5;
      printf("2670: (unsigned long long)(float)-0.5 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)-0.5; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)-0.5; static _Bool r3 = (_Bool)(float)-0.5;
      printf("2671: (_Bool)(float)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-0.5; float r1 = (float)v; float r2 = (float)(float)-0.5; static float r3 = (float)(float)-0.5;
      printf("2672: (float)(float)-0.5 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)-0.5; double r1 = (double)v; double r2 = (double)(float)-0.5; static double r3 = (double)(float)-0.5;
      printf("2673: (double)(float)-0.5 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)-0.5; long double r1 = (long double)v; long double r2 = (long double)(float)-0.5; static long double r3 = (long double)(float)-0.5;
      printf("2674: (long double)(float)-0.5 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)1.5; signed char r1 = (signed char)v; signed char r2 = (signed char)(float)1.5; static signed char r3 = (signed char)(float)1.5;
      printf("2675: (signed char)(float)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1.5; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(float)1.5; static unsigned char r3 = (unsigned char)(float)1.5;
      printf("2676: (unsigned char)(float)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1.5; short r1 = (short)v; short r2 = (short)(float)1.5; static short r3 = (short)(float)1.5;
      printf("2677: (short)(float)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1.5; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(float)1.5; static unsigned short r3 = (unsigned short)(float)1.5;
      printf("2678: (unsigned short)(float)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1.5; int r1 = (int)v; int r2 = (int)(float)1.5; static int r3 = (int)(float)1.5;
      printf("2679: (int)(float)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1.5; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(float)1.5; static unsigned r3 = (unsigned)(float)1.5;
      printf("2680: (unsigned)(float)1.5 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile float v = (float)1.5; long r1 = (long)v; long r2 = (long)(float)1.5; static long r3 = (long)(float)1.5;
      printf("2681: (long)(float)1.5 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)1.5; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)1.5; static unsigned long r3 = (unsigned long)(float)1.5;
      printf("2682: (unsigned long)(float)1.5 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)1.5; long long r1 = (long long)v; long long r2 = (long long)(float)1.5; static long long r3 = (long long)(float)1.5;
      printf("2683: (long long)(float)1.5 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)1.5; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)1.5; static unsigned long long r3 = (unsigned long long)(float)1.5;
      printf("2684: (unsigned long long)(float)1.5 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)1.5; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)1.5; static _Bool r3 = (_Bool)(float)1.5;
      printf("2685: (_Bool)(float)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1.5; float r1 = (float)v; float r2 = (float)(float)1.5; static float r3 = (float)(float)1.5;
      printf("2686: (float)(float)1.5 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)1.5; double r1 = (double)v; double r2 = (double)(float)1.5; static double r3 = (double)(float)1.5;
      printf("2687: (double)(float)1.5 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)1.5; long double r1 = (long double)v; long double r2 = (long double)(float)1.5; static long double r3 = (long double)(float)1.5;
      printf("2688: (long double)(float)1.5 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)-1.5; signed char r1 = (signed char)v; signed char r2 = (signed char)(float)-1.5; static signed char r3 = (signed char)(float)-1.5;
      printf("2689: (signed char)(float)-1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-1.5; short r1 = (short)v; short r2 = (short)(float)-1.5; static short r3 = (short)(float)-1.5;
      printf("2690: (short)(float)-1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-1.5; int r1 = (int)v; int r2 = (int)(float)-1.5; static int r3 = (int)(float)-1.5;
      printf("2691: (int)(float)-1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-1.5; long r1 = (long)v; long r2 = (long)(float)-1.5; static long r3 = (long)(float)-1.5;
      printf("2692: (long)(float)-1.5 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)-1.5; long long r1 = (long long)v; long long r2 = (long long)(float)-1.5; static long long r3 = (long long)(float)-1.5;
      printf("2693: (long long)(float)-1.5 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)-1.5; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)-1.5; static _Bool r3 = (_Bool)(float)-1.5;
      printf("2694: (_Bool)(float)-1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-1.5; float r1 = (float)v; float r2 = (float)(float)-1.5; static float r3 = (float)(float)-1.5;
      printf("2695: (float)(float)-1.5 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)-1.5; double r1 = (double)v; double r2 = (double)(float)-1.5; static double r3 = (double)(float)-1.5;
      printf("2696: (double)(float)-1.5 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)-1.5; long double r1 = (long double)v; long double r2 = (long double)(float)-1.5; static long double r3 = (long double)(float)-1.5;
      printf("2697: (long double)(float)-1.5 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)100.75; signed char r1 = (signed char)v; signed char r2 = (signed char)(float)100.75; static signed char r3 = (signed char)(float)100.75;
      printf("2698: (signed char)(float)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)100.75; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(float)100.75; static unsigned char r3 = (unsigned char)(float)100.75;
      printf("2699: (unsigned char)(float)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)100.75; short r1 = (short)v; short r2 = (short)(float)100.75; static short r3 = (short)(float)100.75;
      printf("2700: (short)(float)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)100.75; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(float)100.75; static unsigned short r3 = (unsigned short)(float)100.75;
      printf("2701: (unsigned short)(float)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)100.75; int r1 = (int)v; int r2 = (int)(float)100.75; static int r3 = (int)(float)100.75;
      printf("2702: (int)(float)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)100.75; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(float)100.75; static unsigned r3 = (unsigned)(float)100.75;
      printf("2703: (unsigned)(float)100.75 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile float v = (float)100.75; long r1 = (long)v; long r2 = (long)(float)100.75; static long r3 = (long)(float)100.75;
      printf("2704: (long)(float)100.75 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)100.75; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)100.75; static unsigned long r3 = (unsigned long)(float)100.75;
      printf("2705: (unsigned long)(float)100.75 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)100.75; long long r1 = (long long)v; long long r2 = (long long)(float)100.75; static long long r3 = (long long)(float)100.75;
      printf("2706: (long long)(float)100.75 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)100.75; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)100.75; static unsigned long long r3 = (unsigned long long)(float)100.75;
      printf("2707: (unsigned long long)(float)100.75 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)100.75; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)100.75; static _Bool r3 = (_Bool)(float)100.75;
      printf("2708: (_Bool)(float)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)100.75; float r1 = (float)v; float r2 = (float)(float)100.75; static float r3 = (float)(float)100.75;
      printf("2709: (float)(float)100.75 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)100.75; double r1 = (double)v; double r2 = (double)(float)100.75; static double r3 = (double)(float)100.75;
      printf("2710: (double)(float)100.75 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)100.75; long double r1 = (long double)v; long double r2 = (long double)(float)100.75; static long double r3 = (long double)(float)100.75;
      printf("2711: (long double)(float)100.75 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)-100.75; signed char r1 = (signed char)v; signed char r2 = (signed char)(float)-100.75; static signed char r3 = (signed char)(float)-100.75;
      printf("2712: (signed char)(float)-100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-100.75; short r1 = (short)v; short r2 = (short)(float)-100.75; static short r3 = (short)(float)-100.75;
      printf("2713: (short)(float)-100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-100.75; int r1 = (int)v; int r2 = (int)(float)-100.75; static int r3 = (int)(float)-100.75;
      printf("2714: (int)(float)-100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-100.75; long r1 = (long)v; long r2 = (long)(float)-100.75; static long r3 = (long)(float)-100.75;
      printf("2715: (long)(float)-100.75 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)-100.75; long long r1 = (long long)v; long long r2 = (long long)(float)-100.75; static long long r3 = (long long)(float)-100.75;
      printf("2716: (long long)(float)-100.75 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)-100.75; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)-100.75; static _Bool r3 = (_Bool)(float)-100.75;
      printf("2717: (_Bool)(float)-100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-100.75; float r1 = (float)v; float r2 = (float)(float)-100.75; static float r3 = (float)(float)-100.75;
      printf("2718: (float)(float)-100.75 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)-100.75; double r1 = (double)v; double r2 = (double)(float)-100.75; static double r3 = (double)(float)-100.75;
      printf("2719: (double)(float)-100.75 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)-100.75; long double r1 = (long double)v; long double r2 = (long double)(float)-100.75; static long double r3 = (long double)(float)-100.75;
      printf("2720: (long double)(float)-100.75 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)1e9; int r1 = (int)v; int r2 = (int)(float)1e9; static int r3 = (int)(float)1e9;
      printf("2721: (int)(float)1e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1e9; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(float)1e9; static unsigned r3 = (unsigned)(float)1e9;
      printf("2722: (unsigned)(float)1e9 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile float v = (float)1e9; long r1 = (long)v; long r2 = (long)(float)1e9; static long r3 = (long)(float)1e9;
      printf("2723: (long)(float)1e9 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)1e9; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)1e9; static unsigned long r3 = (unsigned long)(float)1e9;
      printf("2724: (unsigned long)(float)1e9 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)1e9; long long r1 = (long long)v; long long r2 = (long long)(float)1e9; static long long r3 = (long long)(float)1e9;
      printf("2725: (long long)(float)1e9 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)1e9; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)1e9; static unsigned long long r3 = (unsigned long long)(float)1e9;
      printf("2726: (unsigned long long)(float)1e9 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)1e9; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)1e9; static _Bool r3 = (_Bool)(float)1e9;
      printf("2727: (_Bool)(float)1e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1e9; float r1 = (float)v; float r2 = (float)(float)1e9; static float r3 = (float)(float)1e9;
      printf("2728: (float)(float)1e9 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)1e9; double r1 = (double)v; double r2 = (double)(float)1e9; static double r3 = (double)(float)1e9;
      printf("2729: (double)(float)1e9 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)1e9; long double r1 = (long double)v; long double r2 = (long double)(float)1e9; static long double r3 = (long double)(float)1e9;
      printf("2730: (long double)(float)1e9 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)-1e9; int r1 = (int)v; int r2 = (int)(float)-1e9; static int r3 = (int)(float)-1e9;
      printf("2731: (int)(float)-1e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-1e9; long r1 = (long)v; long r2 = (long)(float)-1e9; static long r3 = (long)(float)-1e9;
      printf("2732: (long)(float)-1e9 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)-1e9; long long r1 = (long long)v; long long r2 = (long long)(float)-1e9; static long long r3 = (long long)(float)-1e9;
      printf("2733: (long long)(float)-1e9 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)-1e9; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)-1e9; static _Bool r3 = (_Bool)(float)-1e9;
      printf("2734: (_Bool)(float)-1e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)-1e9; float r1 = (float)v; float r2 = (float)(float)-1e9; static float r3 = (float)(float)-1e9;
      printf("2735: (float)(float)-1e9 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)-1e9; double r1 = (double)v; double r2 = (double)(float)-1e9; static double r3 = (double)(float)-1e9;
      printf("2736: (double)(float)-1e9 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)-1e9; long double r1 = (long double)v; long double r2 = (long double)(float)-1e9; static long double r3 = (long double)(float)-1e9;
      printf("2737: (long double)(float)-1e9 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)3e9; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(float)3e9; static unsigned r3 = (unsigned)(float)3e9;
      printf("2738: (unsigned)(float)3e9 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile float v = (float)3e9; long r1 = (long)v; long r2 = (long)(float)3e9; static long r3 = (long)(float)3e9;
      printf("2739: (long)(float)3e9 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)3e9; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)3e9; static unsigned long r3 = (unsigned long)(float)3e9;
      printf("2740: (unsigned long)(float)3e9 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)3e9; long long r1 = (long long)v; long long r2 = (long long)(float)3e9; static long long r3 = (long long)(float)3e9;
      printf("2741: (long long)(float)3e9 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)3e9; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)3e9; static unsigned long long r3 = (unsigned long long)(float)3e9;
      printf("2742: (unsigned long long)(float)3e9 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)3e9; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)3e9; static _Bool r3 = (_Bool)(float)3e9;
      printf("2743: (_Bool)(float)3e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)3e9; float r1 = (float)v; float r2 = (float)(float)3e9; static float r3 = (float)(float)3e9;
      printf("2744: (float)(float)3e9 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)3e9; double r1 = (double)v; double r2 = (double)(float)3e9; static double r3 = (double)(float)3e9;
      printf("2745: (double)(float)3e9 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)3e9; long double r1 = (long double)v; long double r2 = (long double)(float)3e9; static long double r3 = (long double)(float)3e9;
      printf("2746: (long double)(float)3e9 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)1e18; long r1 = (long)v; long r2 = (long)(float)1e18; static long r3 = (long)(float)1e18;
      printf("2747: (long)(float)1e18 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)1e18; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)1e18; static unsigned long r3 = (unsigned long)(float)1e18;
      printf("2748: (unsigned long)(float)1e18 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)1e18; long long r1 = (long long)v; long long r2 = (long long)(float)1e18; static long long r3 = (long long)(float)1e18;
      printf("2749: (long long)(float)1e18 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)1e18; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)1e18; static unsigned long long r3 = (unsigned long long)(float)1e18;
      printf("2750: (unsigned long long)(float)1e18 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)1e18; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)1e18; static _Bool r3 = (_Bool)(float)1e18;
      printf("2751: (_Bool)(float)1e18 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1e18; float r1 = (float)v; float r2 = (float)(float)1e18; static float r3 = (float)(float)1e18;
      printf("2752: (float)(float)1e18 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)1e18; double r1 = (double)v; double r2 = (double)(float)1e18; static double r3 = (double)(float)1e18;
      printf("2753: (double)(float)1e18 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)1e18; long double r1 = (long double)v; long double r2 = (long double)(float)1e18; static long double r3 = (long double)(float)1e18;
      printf("2754: (long double)(float)1e18 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)9.2e18; long r1 = (long)v; long r2 = (long)(float)9.2e18; static long r3 = (long)(float)9.2e18;
      printf("2755: (long)(float)9.2e18 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)9.2e18; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)9.2e18; static unsigned long r3 = (unsigned long)(float)9.2e18;
      printf("2756: (unsigned long)(float)9.2e18 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)9.2e18; long long r1 = (long long)v; long long r2 = (long long)(float)9.2e18; static long long r3 = (long long)(float)9.2e18;
      printf("2757: (long long)(float)9.2e18 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)9.2e18; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)9.2e18; static unsigned long long r3 = (unsigned long long)(float)9.2e18;
      printf("2758: (unsigned long long)(float)9.2e18 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)9.2e18; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)9.2e18; static _Bool r3 = (_Bool)(float)9.2e18;
      printf("2759: (_Bool)(float)9.2e18 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)9.2e18; float r1 = (float)v; float r2 = (float)(float)9.2e18; static float r3 = (float)(float)9.2e18;
      printf("2760: (float)(float)9.2e18 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)9.2e18; double r1 = (double)v; double r2 = (double)(float)9.2e18; static double r3 = (double)(float)9.2e18;
      printf("2761: (double)(float)9.2e18 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)9.2e18; long double r1 = (long double)v; long double r2 = (long double)(float)9.2e18; static long double r3 = (long double)(float)9.2e18;
      printf("2762: (long double)(float)9.2e18 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)1.8e19; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)1.8e19; static unsigned long r3 = (unsigned long)(float)1.8e19;
      printf("2763: (unsigned long)(float)1.8e19 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)1.8e19; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)1.8e19; static unsigned long long r3 = (unsigned long long)(float)1.8e19;
      printf("2764: (unsigned long long)(float)1.8e19 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)1.8e19; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)1.8e19; static _Bool r3 = (_Bool)(float)1.8e19;
      printf("2765: (_Bool)(float)1.8e19 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1.8e19; float r1 = (float)v; float r2 = (float)(float)1.8e19; static float r3 = (float)(float)1.8e19;
      printf("2766: (float)(float)1.8e19 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)1.8e19; double r1 = (double)v; double r2 = (double)(float)1.8e19; static double r3 = (double)(float)1.8e19;
      printf("2767: (double)(float)1.8e19 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)1.8e19; long double r1 = (long double)v; long double r2 = (long double)(float)1.8e19; static long double r3 = (long double)(float)1.8e19;
      printf("2768: (long double)(float)1.8e19 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)1e-30; signed char r1 = (signed char)v; signed char r2 = (signed char)(float)1e-30; static signed char r3 = (signed char)(float)1e-30;
      printf("2769: (signed char)(float)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1e-30; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(float)1e-30; static unsigned char r3 = (unsigned char)(float)1e-30;
      printf("2770: (unsigned char)(float)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1e-30; short r1 = (short)v; short r2 = (short)(float)1e-30; static short r3 = (short)(float)1e-30;
      printf("2771: (short)(float)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1e-30; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(float)1e-30; static unsigned short r3 = (unsigned short)(float)1e-30;
      printf("2772: (unsigned short)(float)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1e-30; int r1 = (int)v; int r2 = (int)(float)1e-30; static int r3 = (int)(float)1e-30;
      printf("2773: (int)(float)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1e-30; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(float)1e-30; static unsigned r3 = (unsigned)(float)1e-30;
      printf("2774: (unsigned)(float)1e-30 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile float v = (float)1e-30; long r1 = (long)v; long r2 = (long)(float)1e-30; static long r3 = (long)(float)1e-30;
      printf("2775: (long)(float)1e-30 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)1e-30; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)1e-30; static unsigned long r3 = (unsigned long)(float)1e-30;
      printf("2776: (unsigned long)(float)1e-30 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)1e-30; long long r1 = (long long)v; long long r2 = (long long)(float)1e-30; static long long r3 = (long long)(float)1e-30;
      printf("2777: (long long)(float)1e-30 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)1e-30; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)1e-30; static unsigned long long r3 = (unsigned long long)(float)1e-30;
      printf("2778: (unsigned long long)(float)1e-30 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)1e-30; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)1e-30; static _Bool r3 = (_Bool)(float)1e-30;
      printf("2779: (_Bool)(float)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)1e-30; float r1 = (float)v; float r2 = (float)(float)1e-30; static float r3 = (float)(float)1e-30;
      printf("2780: (float)(float)1e-30 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)1e-30; double r1 = (double)v; double r2 = (double)(float)1e-30; static double r3 = (double)(float)1e-30;
      printf("2781: (double)(float)1e-30 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)1e-30; long double r1 = (long double)v; long double r2 = (long double)(float)1e-30; static long double r3 = (long double)(float)1e-30;
      printf("2782: (long double)(float)1e-30 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)16777217.0; int r1 = (int)v; int r2 = (int)(float)16777217.0; static int r3 = (int)(float)16777217.0;
      printf("2783: (int)(float)16777217.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)16777217.0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(float)16777217.0; static unsigned r3 = (unsigned)(float)16777217.0;
      printf("2784: (unsigned)(float)16777217.0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile float v = (float)16777217.0; long r1 = (long)v; long r2 = (long)(float)16777217.0; static long r3 = (long)(float)16777217.0;
      printf("2785: (long)(float)16777217.0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)16777217.0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)16777217.0; static unsigned long r3 = (unsigned long)(float)16777217.0;
      printf("2786: (unsigned long)(float)16777217.0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)16777217.0; long long r1 = (long long)v; long long r2 = (long long)(float)16777217.0; static long long r3 = (long long)(float)16777217.0;
      printf("2787: (long long)(float)16777217.0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)16777217.0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)16777217.0; static unsigned long long r3 = (unsigned long long)(float)16777217.0;
      printf("2788: (unsigned long long)(float)16777217.0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)16777217.0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)16777217.0; static _Bool r3 = (_Bool)(float)16777217.0;
      printf("2789: (_Bool)(float)16777217.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)16777217.0; float r1 = (float)v; float r2 = (float)(float)16777217.0; static float r3 = (float)(float)16777217.0;
      printf("2790: (float)(float)16777217.0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)16777217.0; double r1 = (double)v; double r2 = (double)(float)16777217.0; static double r3 = (double)(float)16777217.0;
      printf("2791: (double)(float)16777217.0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)16777217.0; long double r1 = (long double)v; long double r2 = (long double)(float)16777217.0; static long double r3 = (long double)(float)16777217.0;
      printf("2792: (long double)(float)16777217.0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float v = (float)9007199254740993.0; long r1 = (long)v; long r2 = (long)(float)9007199254740993.0; static long r3 = (long)(float)9007199254740993.0;
      printf("2793: (long)(float)9007199254740993.0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile float v = (float)9007199254740993.0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(float)9007199254740993.0; static unsigned long r3 = (unsigned long)(float)9007199254740993.0;
      printf("2794: (unsigned long)(float)9007199254740993.0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile float v = (float)9007199254740993.0; long long r1 = (long long)v; long long r2 = (long long)(float)9007199254740993.0; static long long r3 = (long long)(float)9007199254740993.0;
      printf("2795: (long long)(float)9007199254740993.0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile float v = (float)9007199254740993.0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(float)9007199254740993.0; static unsigned long long r3 = (unsigned long long)(float)9007199254740993.0;
      printf("2796: (unsigned long long)(float)9007199254740993.0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile float v = (float)9007199254740993.0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(float)9007199254740993.0; static _Bool r3 = (_Bool)(float)9007199254740993.0;
      printf("2797: (_Bool)(float)9007199254740993.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float v = (float)9007199254740993.0; float r1 = (float)v; float r2 = (float)(float)9007199254740993.0; static float r3 = (float)(float)9007199254740993.0;
      printf("2798: (float)(float)9007199254740993.0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)9007199254740993.0; double r1 = (double)v; double r2 = (double)(float)9007199254740993.0; static double r3 = (double)(float)9007199254740993.0;
      printf("2799: (double)(float)9007199254740993.0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float v = (float)9007199254740993.0; long double r1 = (long double)v; long double r2 = (long double)(float)9007199254740993.0; static long double r3 = (long double)(float)9007199254740993.0;
      printf("2800: (long double)(float)9007199254740993.0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)0.0; signed char r1 = (signed char)v; signed char r2 = (signed char)(double)0.0; static signed char r3 = (signed char)(double)0.0;
      printf("2801: (signed char)(double)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)0.0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(double)0.0; static unsigned char r3 = (unsigned char)(double)0.0;
      printf("2802: (unsigned char)(double)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)0.0; short r1 = (short)v; short r2 = (short)(double)0.0; static short r3 = (short)(double)0.0;
      printf("2803: (short)(double)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)0.0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(double)0.0; static unsigned short r3 = (unsigned short)(double)0.0;
      printf("2804: (unsigned short)(double)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)0.0; int r1 = (int)v; int r2 = (int)(double)0.0; static int r3 = (int)(double)0.0;
      printf("2805: (int)(double)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)0.0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(double)0.0; static unsigned r3 = (unsigned)(double)0.0;
      printf("2806: (unsigned)(double)0.0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile double v = (double)0.0; long r1 = (long)v; long r2 = (long)(double)0.0; static long r3 = (long)(double)0.0;
      printf("2807: (long)(double)0.0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)0.0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)0.0; static unsigned long r3 = (unsigned long)(double)0.0;
      printf("2808: (unsigned long)(double)0.0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)0.0; long long r1 = (long long)v; long long r2 = (long long)(double)0.0; static long long r3 = (long long)(double)0.0;
      printf("2809: (long long)(double)0.0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)0.0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)0.0; static unsigned long long r3 = (unsigned long long)(double)0.0;
      printf("2810: (unsigned long long)(double)0.0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)0.0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)0.0; static _Bool r3 = (_Bool)(double)0.0;
      printf("2811: (_Bool)(double)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)0.0; float r1 = (float)v; float r2 = (float)(double)0.0; static float r3 = (float)(double)0.0;
      printf("2812: (float)(double)0.0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)0.0; double r1 = (double)v; double r2 = (double)(double)0.0; static double r3 = (double)(double)0.0;
      printf("2813: (double)(double)0.0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)0.0; long double r1 = (long double)v; long double r2 = (long double)(double)0.0; static long double r3 = (long double)(double)0.0;
      printf("2814: (long double)(double)0.0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)-0.0; signed char r1 = (signed char)v; signed char r2 = (signed char)(double)-0.0; static signed char r3 = (signed char)(double)-0.0;
      printf("2815: (signed char)(double)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-0.0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(double)-0.0; static unsigned char r3 = (unsigned char)(double)-0.0;
      printf("2816: (unsigned char)(double)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-0.0; short r1 = (short)v; short r2 = (short)(double)-0.0; static short r3 = (short)(double)-0.0;
      printf("2817: (short)(double)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-0.0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(double)-0.0; static unsigned short r3 = (unsigned short)(double)-0.0;
      printf("2818: (unsigned short)(double)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-0.0; int r1 = (int)v; int r2 = (int)(double)-0.0; static int r3 = (int)(double)-0.0;
      printf("2819: (int)(double)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-0.0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(double)-0.0; static unsigned r3 = (unsigned)(double)-0.0;
      printf("2820: (unsigned)(double)-0.0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile double v = (double)-0.0; long r1 = (long)v; long r2 = (long)(double)-0.0; static long r3 = (long)(double)-0.0;
      printf("2821: (long)(double)-0.0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)-0.0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)-0.0; static unsigned long r3 = (unsigned long)(double)-0.0;
      printf("2822: (unsigned long)(double)-0.0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)-0.0; long long r1 = (long long)v; long long r2 = (long long)(double)-0.0; static long long r3 = (long long)(double)-0.0;
      printf("2823: (long long)(double)-0.0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)-0.0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)-0.0; static unsigned long long r3 = (unsigned long long)(double)-0.0;
      printf("2824: (unsigned long long)(double)-0.0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)-0.0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)-0.0; static _Bool r3 = (_Bool)(double)-0.0;
      printf("2825: (_Bool)(double)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-0.0; float r1 = (float)v; float r2 = (float)(double)-0.0; static float r3 = (float)(double)-0.0;
      printf("2826: (float)(double)-0.0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)-0.0; double r1 = (double)v; double r2 = (double)(double)-0.0; static double r3 = (double)(double)-0.0;
      printf("2827: (double)(double)-0.0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)-0.0; long double r1 = (long double)v; long double r2 = (long double)(double)-0.0; static long double r3 = (long double)(double)-0.0;
      printf("2828: (long double)(double)-0.0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)0.5; signed char r1 = (signed char)v; signed char r2 = (signed char)(double)0.5; static signed char r3 = (signed char)(double)0.5;
      printf("2829: (signed char)(double)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)0.5; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(double)0.5; static unsigned char r3 = (unsigned char)(double)0.5;
      printf("2830: (unsigned char)(double)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)0.5; short r1 = (short)v; short r2 = (short)(double)0.5; static short r3 = (short)(double)0.5;
      printf("2831: (short)(double)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)0.5; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(double)0.5; static unsigned short r3 = (unsigned short)(double)0.5;
      printf("2832: (unsigned short)(double)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)0.5; int r1 = (int)v; int r2 = (int)(double)0.5; static int r3 = (int)(double)0.5;
      printf("2833: (int)(double)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)0.5; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(double)0.5; static unsigned r3 = (unsigned)(double)0.5;
      printf("2834: (unsigned)(double)0.5 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile double v = (double)0.5; long r1 = (long)v; long r2 = (long)(double)0.5; static long r3 = (long)(double)0.5;
      printf("2835: (long)(double)0.5 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)0.5; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)0.5; static unsigned long r3 = (unsigned long)(double)0.5;
      printf("2836: (unsigned long)(double)0.5 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)0.5; long long r1 = (long long)v; long long r2 = (long long)(double)0.5; static long long r3 = (long long)(double)0.5;
      printf("2837: (long long)(double)0.5 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)0.5; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)0.5; static unsigned long long r3 = (unsigned long long)(double)0.5;
      printf("2838: (unsigned long long)(double)0.5 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)0.5; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)0.5; static _Bool r3 = (_Bool)(double)0.5;
      printf("2839: (_Bool)(double)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)0.5; float r1 = (float)v; float r2 = (float)(double)0.5; static float r3 = (float)(double)0.5;
      printf("2840: (float)(double)0.5 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)0.5; double r1 = (double)v; double r2 = (double)(double)0.5; static double r3 = (double)(double)0.5;
      printf("2841: (double)(double)0.5 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)0.5; long double r1 = (long double)v; long double r2 = (long double)(double)0.5; static long double r3 = (long double)(double)0.5;
      printf("2842: (long double)(double)0.5 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)-0.5; signed char r1 = (signed char)v; signed char r2 = (signed char)(double)-0.5; static signed char r3 = (signed char)(double)-0.5;
      printf("2843: (signed char)(double)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-0.5; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(double)-0.5; static unsigned char r3 = (unsigned char)(double)-0.5;
      printf("2844: (unsigned char)(double)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-0.5; short r1 = (short)v; short r2 = (short)(double)-0.5; static short r3 = (short)(double)-0.5;
      printf("2845: (short)(double)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-0.5; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(double)-0.5; static unsigned short r3 = (unsigned short)(double)-0.5;
      printf("2846: (unsigned short)(double)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-0.5; int r1 = (int)v; int r2 = (int)(double)-0.5; static int r3 = (int)(double)-0.5;
      printf("2847: (int)(double)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-0.5; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(double)-0.5; static unsigned r3 = (unsigned)(double)-0.5;
      printf("2848: (unsigned)(double)-0.5 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile double v = (double)-0.5; long r1 = (long)v; long r2 = (long)(double)-0.5; static long r3 = (long)(double)-0.5;
      printf("2849: (long)(double)-0.5 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)-0.5; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)-0.5; static unsigned long r3 = (unsigned long)(double)-0.5;
      printf("2850: (unsigned long)(double)-0.5 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)-0.5; long long r1 = (long long)v; long long r2 = (long long)(double)-0.5; static long long r3 = (long long)(double)-0.5;
      printf("2851: (long long)(double)-0.5 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)-0.5; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)-0.5; static unsigned long long r3 = (unsigned long long)(double)-0.5;
      printf("2852: (unsigned long long)(double)-0.5 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)-0.5; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)-0.5; static _Bool r3 = (_Bool)(double)-0.5;
      printf("2853: (_Bool)(double)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-0.5; float r1 = (float)v; float r2 = (float)(double)-0.5; static float r3 = (float)(double)-0.5;
      printf("2854: (float)(double)-0.5 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)-0.5; double r1 = (double)v; double r2 = (double)(double)-0.5; static double r3 = (double)(double)-0.5;
      printf("2855: (double)(double)-0.5 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)-0.5; long double r1 = (long double)v; long double r2 = (long double)(double)-0.5; static long double r3 = (long double)(double)-0.5;
      printf("2856: (long double)(double)-0.5 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)1.5; signed char r1 = (signed char)v; signed char r2 = (signed char)(double)1.5; static signed char r3 = (signed char)(double)1.5;
      printf("2857: (signed char)(double)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1.5; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(double)1.5; static unsigned char r3 = (unsigned char)(double)1.5;
      printf("2858: (unsigned char)(double)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1.5; short r1 = (short)v; short r2 = (short)(double)1.5; static short r3 = (short)(double)1.5;
      printf("2859: (short)(double)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1.5; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(double)1.5; static unsigned short r3 = (unsigned short)(double)1.5;
      printf("2860: (unsigned short)(double)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1.5; int r1 = (int)v; int r2 = (int)(double)1.5; static int r3 = (int)(double)1.5;
      printf("2861: (int)(double)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1.5; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(double)1.5; static unsigned r3 = (unsigned)(double)1.5;
      printf("2862: (unsigned)(double)1.5 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile double v = (double)1.5; long r1 = (long)v; long r2 = (long)(double)1.5; static long r3 = (long)(double)1.5;
      printf("2863: (long)(double)1.5 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)1.5; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)1.5; static unsigned long r3 = (unsigned long)(double)1.5;
      printf("2864: (unsigned long)(double)1.5 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)1.5; long long r1 = (long long)v; long long r2 = (long long)(double)1.5; static long long r3 = (long long)(double)1.5;
      printf("2865: (long long)(double)1.5 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)1.5; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)1.5; static unsigned long long r3 = (unsigned long long)(double)1.5;
      printf("2866: (unsigned long long)(double)1.5 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)1.5; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)1.5; static _Bool r3 = (_Bool)(double)1.5;
      printf("2867: (_Bool)(double)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1.5; float r1 = (float)v; float r2 = (float)(double)1.5; static float r3 = (float)(double)1.5;
      printf("2868: (float)(double)1.5 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)1.5; double r1 = (double)v; double r2 = (double)(double)1.5; static double r3 = (double)(double)1.5;
      printf("2869: (double)(double)1.5 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)1.5; long double r1 = (long double)v; long double r2 = (long double)(double)1.5; static long double r3 = (long double)(double)1.5;
      printf("2870: (long double)(double)1.5 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)-1.5; signed char r1 = (signed char)v; signed char r2 = (signed char)(double)-1.5; static signed char r3 = (signed char)(double)-1.5;
      printf("2871: (signed char)(double)-1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-1.5; short r1 = (short)v; short r2 = (short)(double)-1.5; static short r3 = (short)(double)-1.5;
      printf("2872: (short)(double)-1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-1.5; int r1 = (int)v; int r2 = (int)(double)-1.5; static int r3 = (int)(double)-1.5;
      printf("2873: (int)(double)-1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-1.5; long r1 = (long)v; long r2 = (long)(double)-1.5; static long r3 = (long)(double)-1.5;
      printf("2874: (long)(double)-1.5 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)-1.5; long long r1 = (long long)v; long long r2 = (long long)(double)-1.5; static long long r3 = (long long)(double)-1.5;
      printf("2875: (long long)(double)-1.5 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)-1.5; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)-1.5; static _Bool r3 = (_Bool)(double)-1.5;
      printf("2876: (_Bool)(double)-1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-1.5; float r1 = (float)v; float r2 = (float)(double)-1.5; static float r3 = (float)(double)-1.5;
      printf("2877: (float)(double)-1.5 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)-1.5; double r1 = (double)v; double r2 = (double)(double)-1.5; static double r3 = (double)(double)-1.5;
      printf("2878: (double)(double)-1.5 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)-1.5; long double r1 = (long double)v; long double r2 = (long double)(double)-1.5; static long double r3 = (long double)(double)-1.5;
      printf("2879: (long double)(double)-1.5 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)100.75; signed char r1 = (signed char)v; signed char r2 = (signed char)(double)100.75; static signed char r3 = (signed char)(double)100.75;
      printf("2880: (signed char)(double)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)100.75; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(double)100.75; static unsigned char r3 = (unsigned char)(double)100.75;
      printf("2881: (unsigned char)(double)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)100.75; short r1 = (short)v; short r2 = (short)(double)100.75; static short r3 = (short)(double)100.75;
      printf("2882: (short)(double)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)100.75; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(double)100.75; static unsigned short r3 = (unsigned short)(double)100.75;
      printf("2883: (unsigned short)(double)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)100.75; int r1 = (int)v; int r2 = (int)(double)100.75; static int r3 = (int)(double)100.75;
      printf("2884: (int)(double)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)100.75; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(double)100.75; static unsigned r3 = (unsigned)(double)100.75;
      printf("2885: (unsigned)(double)100.75 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile double v = (double)100.75; long r1 = (long)v; long r2 = (long)(double)100.75; static long r3 = (long)(double)100.75;
      printf("2886: (long)(double)100.75 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)100.75; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)100.75; static unsigned long r3 = (unsigned long)(double)100.75;
      printf("2887: (unsigned long)(double)100.75 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)100.75; long long r1 = (long long)v; long long r2 = (long long)(double)100.75; static long long r3 = (long long)(double)100.75;
      printf("2888: (long long)(double)100.75 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)100.75; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)100.75; static unsigned long long r3 = (unsigned long long)(double)100.75;
      printf("2889: (unsigned long long)(double)100.75 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)100.75; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)100.75; static _Bool r3 = (_Bool)(double)100.75;
      printf("2890: (_Bool)(double)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)100.75; float r1 = (float)v; float r2 = (float)(double)100.75; static float r3 = (float)(double)100.75;
      printf("2891: (float)(double)100.75 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)100.75; double r1 = (double)v; double r2 = (double)(double)100.75; static double r3 = (double)(double)100.75;
      printf("2892: (double)(double)100.75 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)100.75; long double r1 = (long double)v; long double r2 = (long double)(double)100.75; static long double r3 = (long double)(double)100.75;
      printf("2893: (long double)(double)100.75 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)-100.75; signed char r1 = (signed char)v; signed char r2 = (signed char)(double)-100.75; static signed char r3 = (signed char)(double)-100.75;
      printf("2894: (signed char)(double)-100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-100.75; short r1 = (short)v; short r2 = (short)(double)-100.75; static short r3 = (short)(double)-100.75;
      printf("2895: (short)(double)-100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-100.75; int r1 = (int)v; int r2 = (int)(double)-100.75; static int r3 = (int)(double)-100.75;
      printf("2896: (int)(double)-100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-100.75; long r1 = (long)v; long r2 = (long)(double)-100.75; static long r3 = (long)(double)-100.75;
      printf("2897: (long)(double)-100.75 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)-100.75; long long r1 = (long long)v; long long r2 = (long long)(double)-100.75; static long long r3 = (long long)(double)-100.75;
      printf("2898: (long long)(double)-100.75 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)-100.75; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)-100.75; static _Bool r3 = (_Bool)(double)-100.75;
      printf("2899: (_Bool)(double)-100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-100.75; float r1 = (float)v; float r2 = (float)(double)-100.75; static float r3 = (float)(double)-100.75;
      printf("2900: (float)(double)-100.75 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)-100.75; double r1 = (double)v; double r2 = (double)(double)-100.75; static double r3 = (double)(double)-100.75;
      printf("2901: (double)(double)-100.75 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)-100.75; long double r1 = (long double)v; long double r2 = (long double)(double)-100.75; static long double r3 = (long double)(double)-100.75;
      printf("2902: (long double)(double)-100.75 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)1e9; int r1 = (int)v; int r2 = (int)(double)1e9; static int r3 = (int)(double)1e9;
      printf("2903: (int)(double)1e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1e9; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(double)1e9; static unsigned r3 = (unsigned)(double)1e9;
      printf("2904: (unsigned)(double)1e9 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile double v = (double)1e9; long r1 = (long)v; long r2 = (long)(double)1e9; static long r3 = (long)(double)1e9;
      printf("2905: (long)(double)1e9 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)1e9; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)1e9; static unsigned long r3 = (unsigned long)(double)1e9;
      printf("2906: (unsigned long)(double)1e9 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)1e9; long long r1 = (long long)v; long long r2 = (long long)(double)1e9; static long long r3 = (long long)(double)1e9;
      printf("2907: (long long)(double)1e9 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)1e9; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)1e9; static unsigned long long r3 = (unsigned long long)(double)1e9;
      printf("2908: (unsigned long long)(double)1e9 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)1e9; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)1e9; static _Bool r3 = (_Bool)(double)1e9;
      printf("2909: (_Bool)(double)1e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1e9; float r1 = (float)v; float r2 = (float)(double)1e9; static float r3 = (float)(double)1e9;
      printf("2910: (float)(double)1e9 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)1e9; double r1 = (double)v; double r2 = (double)(double)1e9; static double r3 = (double)(double)1e9;
      printf("2911: (double)(double)1e9 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)1e9; long double r1 = (long double)v; long double r2 = (long double)(double)1e9; static long double r3 = (long double)(double)1e9;
      printf("2912: (long double)(double)1e9 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)-1e9; int r1 = (int)v; int r2 = (int)(double)-1e9; static int r3 = (int)(double)-1e9;
      printf("2913: (int)(double)-1e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-1e9; long r1 = (long)v; long r2 = (long)(double)-1e9; static long r3 = (long)(double)-1e9;
      printf("2914: (long)(double)-1e9 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)-1e9; long long r1 = (long long)v; long long r2 = (long long)(double)-1e9; static long long r3 = (long long)(double)-1e9;
      printf("2915: (long long)(double)-1e9 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)-1e9; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)-1e9; static _Bool r3 = (_Bool)(double)-1e9;
      printf("2916: (_Bool)(double)-1e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)-1e9; float r1 = (float)v; float r2 = (float)(double)-1e9; static float r3 = (float)(double)-1e9;
      printf("2917: (float)(double)-1e9 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)-1e9; double r1 = (double)v; double r2 = (double)(double)-1e9; static double r3 = (double)(double)-1e9;
      printf("2918: (double)(double)-1e9 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)-1e9; long double r1 = (long double)v; long double r2 = (long double)(double)-1e9; static long double r3 = (long double)(double)-1e9;
      printf("2919: (long double)(double)-1e9 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)3e9; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(double)3e9; static unsigned r3 = (unsigned)(double)3e9;
      printf("2920: (unsigned)(double)3e9 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile double v = (double)3e9; long r1 = (long)v; long r2 = (long)(double)3e9; static long r3 = (long)(double)3e9;
      printf("2921: (long)(double)3e9 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)3e9; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)3e9; static unsigned long r3 = (unsigned long)(double)3e9;
      printf("2922: (unsigned long)(double)3e9 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)3e9; long long r1 = (long long)v; long long r2 = (long long)(double)3e9; static long long r3 = (long long)(double)3e9;
      printf("2923: (long long)(double)3e9 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)3e9; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)3e9; static unsigned long long r3 = (unsigned long long)(double)3e9;
      printf("2924: (unsigned long long)(double)3e9 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)3e9; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)3e9; static _Bool r3 = (_Bool)(double)3e9;
      printf("2925: (_Bool)(double)3e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)3e9; float r1 = (float)v; float r2 = (float)(double)3e9; static float r3 = (float)(double)3e9;
      printf("2926: (float)(double)3e9 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)3e9; double r1 = (double)v; double r2 = (double)(double)3e9; static double r3 = (double)(double)3e9;
      printf("2927: (double)(double)3e9 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)3e9; long double r1 = (long double)v; long double r2 = (long double)(double)3e9; static long double r3 = (long double)(double)3e9;
      printf("2928: (long double)(double)3e9 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)1e18; long r1 = (long)v; long r2 = (long)(double)1e18; static long r3 = (long)(double)1e18;
      printf("2929: (long)(double)1e18 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)1e18; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)1e18; static unsigned long r3 = (unsigned long)(double)1e18;
      printf("2930: (unsigned long)(double)1e18 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)1e18; long long r1 = (long long)v; long long r2 = (long long)(double)1e18; static long long r3 = (long long)(double)1e18;
      printf("2931: (long long)(double)1e18 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)1e18; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)1e18; static unsigned long long r3 = (unsigned long long)(double)1e18;
      printf("2932: (unsigned long long)(double)1e18 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)1e18; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)1e18; static _Bool r3 = (_Bool)(double)1e18;
      printf("2933: (_Bool)(double)1e18 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1e18; float r1 = (float)v; float r2 = (float)(double)1e18; static float r3 = (float)(double)1e18;
      printf("2934: (float)(double)1e18 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)1e18; double r1 = (double)v; double r2 = (double)(double)1e18; static double r3 = (double)(double)1e18;
      printf("2935: (double)(double)1e18 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)1e18; long double r1 = (long double)v; long double r2 = (long double)(double)1e18; static long double r3 = (long double)(double)1e18;
      printf("2936: (long double)(double)1e18 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)9.2e18; long r1 = (long)v; long r2 = (long)(double)9.2e18; static long r3 = (long)(double)9.2e18;
      printf("2937: (long)(double)9.2e18 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)9.2e18; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)9.2e18; static unsigned long r3 = (unsigned long)(double)9.2e18;
      printf("2938: (unsigned long)(double)9.2e18 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)9.2e18; long long r1 = (long long)v; long long r2 = (long long)(double)9.2e18; static long long r3 = (long long)(double)9.2e18;
      printf("2939: (long long)(double)9.2e18 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)9.2e18; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)9.2e18; static unsigned long long r3 = (unsigned long long)(double)9.2e18;
      printf("2940: (unsigned long long)(double)9.2e18 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)9.2e18; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)9.2e18; static _Bool r3 = (_Bool)(double)9.2e18;
      printf("2941: (_Bool)(double)9.2e18 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)9.2e18; float r1 = (float)v; float r2 = (float)(double)9.2e18; static float r3 = (float)(double)9.2e18;
      printf("2942: (float)(double)9.2e18 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)9.2e18; double r1 = (double)v; double r2 = (double)(double)9.2e18; static double r3 = (double)(double)9.2e18;
      printf("2943: (double)(double)9.2e18 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)9.2e18; long double r1 = (long double)v; long double r2 = (long double)(double)9.2e18; static long double r3 = (long double)(double)9.2e18;
      printf("2944: (long double)(double)9.2e18 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)1.8e19; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)1.8e19; static unsigned long r3 = (unsigned long)(double)1.8e19;
      printf("2945: (unsigned long)(double)1.8e19 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)1.8e19; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)1.8e19; static unsigned long long r3 = (unsigned long long)(double)1.8e19;
      printf("2946: (unsigned long long)(double)1.8e19 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)1.8e19; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)1.8e19; static _Bool r3 = (_Bool)(double)1.8e19;
      printf("2947: (_Bool)(double)1.8e19 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1.8e19; float r1 = (float)v; float r2 = (float)(double)1.8e19; static float r3 = (float)(double)1.8e19;
      printf("2948: (float)(double)1.8e19 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)1.8e19; double r1 = (double)v; double r2 = (double)(double)1.8e19; static double r3 = (double)(double)1.8e19;
      printf("2949: (double)(double)1.8e19 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)1.8e19; long double r1 = (long double)v; long double r2 = (long double)(double)1.8e19; static long double r3 = (long double)(double)1.8e19;
      printf("2950: (long double)(double)1.8e19 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)1e-30; signed char r1 = (signed char)v; signed char r2 = (signed char)(double)1e-30; static signed char r3 = (signed char)(double)1e-30;
      printf("2951: (signed char)(double)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1e-30; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(double)1e-30; static unsigned char r3 = (unsigned char)(double)1e-30;
      printf("2952: (unsigned char)(double)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1e-30; short r1 = (short)v; short r2 = (short)(double)1e-30; static short r3 = (short)(double)1e-30;
      printf("2953: (short)(double)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1e-30; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(double)1e-30; static unsigned short r3 = (unsigned short)(double)1e-30;
      printf("2954: (unsigned short)(double)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1e-30; int r1 = (int)v; int r2 = (int)(double)1e-30; static int r3 = (int)(double)1e-30;
      printf("2955: (int)(double)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1e-30; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(double)1e-30; static unsigned r3 = (unsigned)(double)1e-30;
      printf("2956: (unsigned)(double)1e-30 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile double v = (double)1e-30; long r1 = (long)v; long r2 = (long)(double)1e-30; static long r3 = (long)(double)1e-30;
      printf("2957: (long)(double)1e-30 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)1e-30; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)1e-30; static unsigned long r3 = (unsigned long)(double)1e-30;
      printf("2958: (unsigned long)(double)1e-30 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)1e-30; long long r1 = (long long)v; long long r2 = (long long)(double)1e-30; static long long r3 = (long long)(double)1e-30;
      printf("2959: (long long)(double)1e-30 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)1e-30; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)1e-30; static unsigned long long r3 = (unsigned long long)(double)1e-30;
      printf("2960: (unsigned long long)(double)1e-30 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)1e-30; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)1e-30; static _Bool r3 = (_Bool)(double)1e-30;
      printf("2961: (_Bool)(double)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)1e-30; float r1 = (float)v; float r2 = (float)(double)1e-30; static float r3 = (float)(double)1e-30;
      printf("2962: (float)(double)1e-30 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)1e-30; double r1 = (double)v; double r2 = (double)(double)1e-30; static double r3 = (double)(double)1e-30;
      printf("2963: (double)(double)1e-30 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)1e-30; long double r1 = (long double)v; long double r2 = (long double)(double)1e-30; static long double r3 = (long double)(double)1e-30;
      printf("2964: (long double)(double)1e-30 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)16777217.0; int r1 = (int)v; int r2 = (int)(double)16777217.0; static int r3 = (int)(double)16777217.0;
      printf("2965: (int)(double)16777217.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)16777217.0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(double)16777217.0; static unsigned r3 = (unsigned)(double)16777217.0;
      printf("2966: (unsigned)(double)16777217.0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile double v = (double)16777217.0; long r1 = (long)v; long r2 = (long)(double)16777217.0; static long r3 = (long)(double)16777217.0;
      printf("2967: (long)(double)16777217.0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)16777217.0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)16777217.0; static unsigned long r3 = (unsigned long)(double)16777217.0;
      printf("2968: (unsigned long)(double)16777217.0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)16777217.0; long long r1 = (long long)v; long long r2 = (long long)(double)16777217.0; static long long r3 = (long long)(double)16777217.0;
      printf("2969: (long long)(double)16777217.0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)16777217.0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)16777217.0; static unsigned long long r3 = (unsigned long long)(double)16777217.0;
      printf("2970: (unsigned long long)(double)16777217.0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)16777217.0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)16777217.0; static _Bool r3 = (_Bool)(double)16777217.0;
      printf("2971: (_Bool)(double)16777217.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)16777217.0; float r1 = (float)v; float r2 = (float)(double)16777217.0; static float r3 = (float)(double)16777217.0;
      printf("2972: (float)(double)16777217.0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)16777217.0; double r1 = (double)v; double r2 = (double)(double)16777217.0; static double r3 = (double)(double)16777217.0;
      printf("2973: (double)(double)16777217.0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)16777217.0; long double r1 = (long double)v; long double r2 = (long double)(double)16777217.0; static long double r3 = (long double)(double)16777217.0;
      printf("2974: (long double)(double)16777217.0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double v = (double)9007199254740993.0; long r1 = (long)v; long r2 = (long)(double)9007199254740993.0; static long r3 = (long)(double)9007199254740993.0;
      printf("2975: (long)(double)9007199254740993.0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile double v = (double)9007199254740993.0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(double)9007199254740993.0; static unsigned long r3 = (unsigned long)(double)9007199254740993.0;
      printf("2976: (unsigned long)(double)9007199254740993.0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile double v = (double)9007199254740993.0; long long r1 = (long long)v; long long r2 = (long long)(double)9007199254740993.0; static long long r3 = (long long)(double)9007199254740993.0;
      printf("2977: (long long)(double)9007199254740993.0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile double v = (double)9007199254740993.0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(double)9007199254740993.0; static unsigned long long r3 = (unsigned long long)(double)9007199254740993.0;
      printf("2978: (unsigned long long)(double)9007199254740993.0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile double v = (double)9007199254740993.0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(double)9007199254740993.0; static _Bool r3 = (_Bool)(double)9007199254740993.0;
      printf("2979: (_Bool)(double)9007199254740993.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double v = (double)9007199254740993.0; float r1 = (float)v; float r2 = (float)(double)9007199254740993.0; static float r3 = (float)(double)9007199254740993.0;
      printf("2980: (float)(double)9007199254740993.0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)9007199254740993.0; double r1 = (double)v; double r2 = (double)(double)9007199254740993.0; static double r3 = (double)(double)9007199254740993.0;
      printf("2981: (double)(double)9007199254740993.0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double v = (double)9007199254740993.0; long double r1 = (long double)v; long double r2 = (long double)(double)9007199254740993.0; static long double r3 = (long double)(double)9007199254740993.0;
      printf("2982: (long double)(double)9007199254740993.0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)0.0; signed char r1 = (signed char)v; signed char r2 = (signed char)(long double)0.0; static signed char r3 = (signed char)(long double)0.0;
      printf("2983: (signed char)(long double)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)0.0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long double)0.0; static unsigned char r3 = (unsigned char)(long double)0.0;
      printf("2984: (unsigned char)(long double)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)0.0; short r1 = (short)v; short r2 = (short)(long double)0.0; static short r3 = (short)(long double)0.0;
      printf("2985: (short)(long double)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)0.0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long double)0.0; static unsigned short r3 = (unsigned short)(long double)0.0;
      printf("2986: (unsigned short)(long double)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)0.0; int r1 = (int)v; int r2 = (int)(long double)0.0; static int r3 = (int)(long double)0.0;
      printf("2987: (int)(long double)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)0.0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long double)0.0; static unsigned r3 = (unsigned)(long double)0.0;
      printf("2988: (unsigned)(long double)0.0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long double v = (long double)0.0; long r1 = (long)v; long r2 = (long)(long double)0.0; static long r3 = (long)(long double)0.0;
      printf("2989: (long)(long double)0.0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)0.0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)0.0; static unsigned long r3 = (unsigned long)(long double)0.0;
      printf("2990: (unsigned long)(long double)0.0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)0.0; long long r1 = (long long)v; long long r2 = (long long)(long double)0.0; static long long r3 = (long long)(long double)0.0;
      printf("2991: (long long)(long double)0.0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)0.0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)0.0; static unsigned long long r3 = (unsigned long long)(long double)0.0;
      printf("2992: (unsigned long long)(long double)0.0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)0.0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)0.0; static _Bool r3 = (_Bool)(long double)0.0;
      printf("2993: (_Bool)(long double)0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)0.0; float r1 = (float)v; float r2 = (float)(long double)0.0; static float r3 = (float)(long double)0.0;
      printf("2994: (float)(long double)0.0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)0.0; double r1 = (double)v; double r2 = (double)(long double)0.0; static double r3 = (double)(long double)0.0;
      printf("2995: (double)(long double)0.0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)0.0; long double r1 = (long double)v; long double r2 = (long double)(long double)0.0; static long double r3 = (long double)(long double)0.0;
      printf("2996: (long double)(long double)0.0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)-0.0; signed char r1 = (signed char)v; signed char r2 = (signed char)(long double)-0.0; static signed char r3 = (signed char)(long double)-0.0;
      printf("2997: (signed char)(long double)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-0.0; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long double)-0.0; static unsigned char r3 = (unsigned char)(long double)-0.0;
      printf("2998: (unsigned char)(long double)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-0.0; short r1 = (short)v; short r2 = (short)(long double)-0.0; static short r3 = (short)(long double)-0.0;
      printf("2999: (short)(long double)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-0.0; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long double)-0.0; static unsigned short r3 = (unsigned short)(long double)-0.0;
      printf("3000: (unsigned short)(long double)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-0.0; int r1 = (int)v; int r2 = (int)(long double)-0.0; static int r3 = (int)(long double)-0.0;
      printf("3001: (int)(long double)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-0.0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long double)-0.0; static unsigned r3 = (unsigned)(long double)-0.0;
      printf("3002: (unsigned)(long double)-0.0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long double v = (long double)-0.0; long r1 = (long)v; long r2 = (long)(long double)-0.0; static long r3 = (long)(long double)-0.0;
      printf("3003: (long)(long double)-0.0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)-0.0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)-0.0; static unsigned long r3 = (unsigned long)(long double)-0.0;
      printf("3004: (unsigned long)(long double)-0.0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)-0.0; long long r1 = (long long)v; long long r2 = (long long)(long double)-0.0; static long long r3 = (long long)(long double)-0.0;
      printf("3005: (long long)(long double)-0.0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)-0.0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)-0.0; static unsigned long long r3 = (unsigned long long)(long double)-0.0;
      printf("3006: (unsigned long long)(long double)-0.0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)-0.0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)-0.0; static _Bool r3 = (_Bool)(long double)-0.0;
      printf("3007: (_Bool)(long double)-0.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-0.0; float r1 = (float)v; float r2 = (float)(long double)-0.0; static float r3 = (float)(long double)-0.0;
      printf("3008: (float)(long double)-0.0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)-0.0; double r1 = (double)v; double r2 = (double)(long double)-0.0; static double r3 = (double)(long double)-0.0;
      printf("3009: (double)(long double)-0.0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)-0.0; long double r1 = (long double)v; long double r2 = (long double)(long double)-0.0; static long double r3 = (long double)(long double)-0.0;
      printf("3010: (long double)(long double)-0.0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)0.5; signed char r1 = (signed char)v; signed char r2 = (signed char)(long double)0.5; static signed char r3 = (signed char)(long double)0.5;
      printf("3011: (signed char)(long double)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)0.5; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long double)0.5; static unsigned char r3 = (unsigned char)(long double)0.5;
      printf("3012: (unsigned char)(long double)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)0.5; short r1 = (short)v; short r2 = (short)(long double)0.5; static short r3 = (short)(long double)0.5;
      printf("3013: (short)(long double)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)0.5; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long double)0.5; static unsigned short r3 = (unsigned short)(long double)0.5;
      printf("3014: (unsigned short)(long double)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)0.5; int r1 = (int)v; int r2 = (int)(long double)0.5; static int r3 = (int)(long double)0.5;
      printf("3015: (int)(long double)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)0.5; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long double)0.5; static unsigned r3 = (unsigned)(long double)0.5;
      printf("3016: (unsigned)(long double)0.5 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long double v = (long double)0.5; long r1 = (long)v; long r2 = (long)(long double)0.5; static long r3 = (long)(long double)0.5;
      printf("3017: (long)(long double)0.5 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)0.5; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)0.5; static unsigned long r3 = (unsigned long)(long double)0.5;
      printf("3018: (unsigned long)(long double)0.5 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)0.5; long long r1 = (long long)v; long long r2 = (long long)(long double)0.5; static long long r3 = (long long)(long double)0.5;
      printf("3019: (long long)(long double)0.5 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)0.5; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)0.5; static unsigned long long r3 = (unsigned long long)(long double)0.5;
      printf("3020: (unsigned long long)(long double)0.5 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)0.5; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)0.5; static _Bool r3 = (_Bool)(long double)0.5;
      printf("3021: (_Bool)(long double)0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)0.5; float r1 = (float)v; float r2 = (float)(long double)0.5; static float r3 = (float)(long double)0.5;
      printf("3022: (float)(long double)0.5 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)0.5; double r1 = (double)v; double r2 = (double)(long double)0.5; static double r3 = (double)(long double)0.5;
      printf("3023: (double)(long double)0.5 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)0.5; long double r1 = (long double)v; long double r2 = (long double)(long double)0.5; static long double r3 = (long double)(long double)0.5;
      printf("3024: (long double)(long double)0.5 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)-0.5; signed char r1 = (signed char)v; signed char r2 = (signed char)(long double)-0.5; static signed char r3 = (signed char)(long double)-0.5;
      printf("3025: (signed char)(long double)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-0.5; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long double)-0.5; static unsigned char r3 = (unsigned char)(long double)-0.5;
      printf("3026: (unsigned char)(long double)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-0.5; short r1 = (short)v; short r2 = (short)(long double)-0.5; static short r3 = (short)(long double)-0.5;
      printf("3027: (short)(long double)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-0.5; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long double)-0.5; static unsigned short r3 = (unsigned short)(long double)-0.5;
      printf("3028: (unsigned short)(long double)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-0.5; int r1 = (int)v; int r2 = (int)(long double)-0.5; static int r3 = (int)(long double)-0.5;
      printf("3029: (int)(long double)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-0.5; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long double)-0.5; static unsigned r3 = (unsigned)(long double)-0.5;
      printf("3030: (unsigned)(long double)-0.5 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long double v = (long double)-0.5; long r1 = (long)v; long r2 = (long)(long double)-0.5; static long r3 = (long)(long double)-0.5;
      printf("3031: (long)(long double)-0.5 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)-0.5; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)-0.5; static unsigned long r3 = (unsigned long)(long double)-0.5;
      printf("3032: (unsigned long)(long double)-0.5 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)-0.5; long long r1 = (long long)v; long long r2 = (long long)(long double)-0.5; static long long r3 = (long long)(long double)-0.5;
      printf("3033: (long long)(long double)-0.5 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)-0.5; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)-0.5; static unsigned long long r3 = (unsigned long long)(long double)-0.5;
      printf("3034: (unsigned long long)(long double)-0.5 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)-0.5; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)-0.5; static _Bool r3 = (_Bool)(long double)-0.5;
      printf("3035: (_Bool)(long double)-0.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-0.5; float r1 = (float)v; float r2 = (float)(long double)-0.5; static float r3 = (float)(long double)-0.5;
      printf("3036: (float)(long double)-0.5 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)-0.5; double r1 = (double)v; double r2 = (double)(long double)-0.5; static double r3 = (double)(long double)-0.5;
      printf("3037: (double)(long double)-0.5 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)-0.5; long double r1 = (long double)v; long double r2 = (long double)(long double)-0.5; static long double r3 = (long double)(long double)-0.5;
      printf("3038: (long double)(long double)-0.5 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)1.5; signed char r1 = (signed char)v; signed char r2 = (signed char)(long double)1.5; static signed char r3 = (signed char)(long double)1.5;
      printf("3039: (signed char)(long double)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1.5; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long double)1.5; static unsigned char r3 = (unsigned char)(long double)1.5;
      printf("3040: (unsigned char)(long double)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1.5; short r1 = (short)v; short r2 = (short)(long double)1.5; static short r3 = (short)(long double)1.5;
      printf("3041: (short)(long double)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1.5; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long double)1.5; static unsigned short r3 = (unsigned short)(long double)1.5;
      printf("3042: (unsigned short)(long double)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1.5; int r1 = (int)v; int r2 = (int)(long double)1.5; static int r3 = (int)(long double)1.5;
      printf("3043: (int)(long double)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1.5; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long double)1.5; static unsigned r3 = (unsigned)(long double)1.5;
      printf("3044: (unsigned)(long double)1.5 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long double v = (long double)1.5; long r1 = (long)v; long r2 = (long)(long double)1.5; static long r3 = (long)(long double)1.5;
      printf("3045: (long)(long double)1.5 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)1.5; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)1.5; static unsigned long r3 = (unsigned long)(long double)1.5;
      printf("3046: (unsigned long)(long double)1.5 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)1.5; long long r1 = (long long)v; long long r2 = (long long)(long double)1.5; static long long r3 = (long long)(long double)1.5;
      printf("3047: (long long)(long double)1.5 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)1.5; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)1.5; static unsigned long long r3 = (unsigned long long)(long double)1.5;
      printf("3048: (unsigned long long)(long double)1.5 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)1.5; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)1.5; static _Bool r3 = (_Bool)(long double)1.5;
      printf("3049: (_Bool)(long double)1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1.5; float r1 = (float)v; float r2 = (float)(long double)1.5; static float r3 = (float)(long double)1.5;
      printf("3050: (float)(long double)1.5 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)1.5; double r1 = (double)v; double r2 = (double)(long double)1.5; static double r3 = (double)(long double)1.5;
      printf("3051: (double)(long double)1.5 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)1.5; long double r1 = (long double)v; long double r2 = (long double)(long double)1.5; static long double r3 = (long double)(long double)1.5;
      printf("3052: (long double)(long double)1.5 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)-1.5; signed char r1 = (signed char)v; signed char r2 = (signed char)(long double)-1.5; static signed char r3 = (signed char)(long double)-1.5;
      printf("3053: (signed char)(long double)-1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-1.5; short r1 = (short)v; short r2 = (short)(long double)-1.5; static short r3 = (short)(long double)-1.5;
      printf("3054: (short)(long double)-1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-1.5; int r1 = (int)v; int r2 = (int)(long double)-1.5; static int r3 = (int)(long double)-1.5;
      printf("3055: (int)(long double)-1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-1.5; long r1 = (long)v; long r2 = (long)(long double)-1.5; static long r3 = (long)(long double)-1.5;
      printf("3056: (long)(long double)-1.5 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)-1.5; long long r1 = (long long)v; long long r2 = (long long)(long double)-1.5; static long long r3 = (long long)(long double)-1.5;
      printf("3057: (long long)(long double)-1.5 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)-1.5; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)-1.5; static _Bool r3 = (_Bool)(long double)-1.5;
      printf("3058: (_Bool)(long double)-1.5 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-1.5; float r1 = (float)v; float r2 = (float)(long double)-1.5; static float r3 = (float)(long double)-1.5;
      printf("3059: (float)(long double)-1.5 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)-1.5; double r1 = (double)v; double r2 = (double)(long double)-1.5; static double r3 = (double)(long double)-1.5;
      printf("3060: (double)(long double)-1.5 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)-1.5; long double r1 = (long double)v; long double r2 = (long double)(long double)-1.5; static long double r3 = (long double)(long double)-1.5;
      printf("3061: (long double)(long double)-1.5 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)100.75; signed char r1 = (signed char)v; signed char r2 = (signed char)(long double)100.75; static signed char r3 = (signed char)(long double)100.75;
      printf("3062: (signed char)(long double)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)100.75; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long double)100.75; static unsigned char r3 = (unsigned char)(long double)100.75;
      printf("3063: (unsigned char)(long double)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)100.75; short r1 = (short)v; short r2 = (short)(long double)100.75; static short r3 = (short)(long double)100.75;
      printf("3064: (short)(long double)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)100.75; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long double)100.75; static unsigned short r3 = (unsigned short)(long double)100.75;
      printf("3065: (unsigned short)(long double)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)100.75; int r1 = (int)v; int r2 = (int)(long double)100.75; static int r3 = (int)(long double)100.75;
      printf("3066: (int)(long double)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)100.75; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long double)100.75; static unsigned r3 = (unsigned)(long double)100.75;
      printf("3067: (unsigned)(long double)100.75 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long double v = (long double)100.75; long r1 = (long)v; long r2 = (long)(long double)100.75; static long r3 = (long)(long double)100.75;
      printf("3068: (long)(long double)100.75 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)100.75; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)100.75; static unsigned long r3 = (unsigned long)(long double)100.75;
      printf("3069: (unsigned long)(long double)100.75 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)100.75; long long r1 = (long long)v; long long r2 = (long long)(long double)100.75; static long long r3 = (long long)(long double)100.75;
      printf("3070: (long long)(long double)100.75 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)100.75; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)100.75; static unsigned long long r3 = (unsigned long long)(long double)100.75;
      printf("3071: (unsigned long long)(long double)100.75 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)100.75; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)100.75; static _Bool r3 = (_Bool)(long double)100.75;
      printf("3072: (_Bool)(long double)100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)100.75; float r1 = (float)v; float r2 = (float)(long double)100.75; static float r3 = (float)(long double)100.75;
      printf("3073: (float)(long double)100.75 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)100.75; double r1 = (double)v; double r2 = (double)(long double)100.75; static double r3 = (double)(long double)100.75;
      printf("3074: (double)(long double)100.75 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)100.75; long double r1 = (long double)v; long double r2 = (long double)(long double)100.75; static long double r3 = (long double)(long double)100.75;
      printf("3075: (long double)(long double)100.75 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)-100.75; signed char r1 = (signed char)v; signed char r2 = (signed char)(long double)-100.75; static signed char r3 = (signed char)(long double)-100.75;
      printf("3076: (signed char)(long double)-100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-100.75; short r1 = (short)v; short r2 = (short)(long double)-100.75; static short r3 = (short)(long double)-100.75;
      printf("3077: (short)(long double)-100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-100.75; int r1 = (int)v; int r2 = (int)(long double)-100.75; static int r3 = (int)(long double)-100.75;
      printf("3078: (int)(long double)-100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-100.75; long r1 = (long)v; long r2 = (long)(long double)-100.75; static long r3 = (long)(long double)-100.75;
      printf("3079: (long)(long double)-100.75 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)-100.75; long long r1 = (long long)v; long long r2 = (long long)(long double)-100.75; static long long r3 = (long long)(long double)-100.75;
      printf("3080: (long long)(long double)-100.75 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)-100.75; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)-100.75; static _Bool r3 = (_Bool)(long double)-100.75;
      printf("3081: (_Bool)(long double)-100.75 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-100.75; float r1 = (float)v; float r2 = (float)(long double)-100.75; static float r3 = (float)(long double)-100.75;
      printf("3082: (float)(long double)-100.75 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)-100.75; double r1 = (double)v; double r2 = (double)(long double)-100.75; static double r3 = (double)(long double)-100.75;
      printf("3083: (double)(long double)-100.75 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)-100.75; long double r1 = (long double)v; long double r2 = (long double)(long double)-100.75; static long double r3 = (long double)(long double)-100.75;
      printf("3084: (long double)(long double)-100.75 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)1e9; int r1 = (int)v; int r2 = (int)(long double)1e9; static int r3 = (int)(long double)1e9;
      printf("3085: (int)(long double)1e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1e9; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long double)1e9; static unsigned r3 = (unsigned)(long double)1e9;
      printf("3086: (unsigned)(long double)1e9 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long double v = (long double)1e9; long r1 = (long)v; long r2 = (long)(long double)1e9; static long r3 = (long)(long double)1e9;
      printf("3087: (long)(long double)1e9 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)1e9; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)1e9; static unsigned long r3 = (unsigned long)(long double)1e9;
      printf("3088: (unsigned long)(long double)1e9 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)1e9; long long r1 = (long long)v; long long r2 = (long long)(long double)1e9; static long long r3 = (long long)(long double)1e9;
      printf("3089: (long long)(long double)1e9 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)1e9; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)1e9; static unsigned long long r3 = (unsigned long long)(long double)1e9;
      printf("3090: (unsigned long long)(long double)1e9 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)1e9; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)1e9; static _Bool r3 = (_Bool)(long double)1e9;
      printf("3091: (_Bool)(long double)1e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1e9; float r1 = (float)v; float r2 = (float)(long double)1e9; static float r3 = (float)(long double)1e9;
      printf("3092: (float)(long double)1e9 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)1e9; double r1 = (double)v; double r2 = (double)(long double)1e9; static double r3 = (double)(long double)1e9;
      printf("3093: (double)(long double)1e9 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)1e9; long double r1 = (long double)v; long double r2 = (long double)(long double)1e9; static long double r3 = (long double)(long double)1e9;
      printf("3094: (long double)(long double)1e9 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)-1e9; int r1 = (int)v; int r2 = (int)(long double)-1e9; static int r3 = (int)(long double)-1e9;
      printf("3095: (int)(long double)-1e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-1e9; long r1 = (long)v; long r2 = (long)(long double)-1e9; static long r3 = (long)(long double)-1e9;
      printf("3096: (long)(long double)-1e9 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)-1e9; long long r1 = (long long)v; long long r2 = (long long)(long double)-1e9; static long long r3 = (long long)(long double)-1e9;
      printf("3097: (long long)(long double)-1e9 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)-1e9; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)-1e9; static _Bool r3 = (_Bool)(long double)-1e9;
      printf("3098: (_Bool)(long double)-1e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)-1e9; float r1 = (float)v; float r2 = (float)(long double)-1e9; static float r3 = (float)(long double)-1e9;
      printf("3099: (float)(long double)-1e9 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)-1e9; double r1 = (double)v; double r2 = (double)(long double)-1e9; static double r3 = (double)(long double)-1e9;
      printf("3100: (double)(long double)-1e9 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)-1e9; long double r1 = (long double)v; long double r2 = (long double)(long double)-1e9; static long double r3 = (long double)(long double)-1e9;
      printf("3101: (long double)(long double)-1e9 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)3e9; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long double)3e9; static unsigned r3 = (unsigned)(long double)3e9;
      printf("3102: (unsigned)(long double)3e9 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long double v = (long double)3e9; long r1 = (long)v; long r2 = (long)(long double)3e9; static long r3 = (long)(long double)3e9;
      printf("3103: (long)(long double)3e9 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)3e9; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)3e9; static unsigned long r3 = (unsigned long)(long double)3e9;
      printf("3104: (unsigned long)(long double)3e9 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)3e9; long long r1 = (long long)v; long long r2 = (long long)(long double)3e9; static long long r3 = (long long)(long double)3e9;
      printf("3105: (long long)(long double)3e9 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)3e9; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)3e9; static unsigned long long r3 = (unsigned long long)(long double)3e9;
      printf("3106: (unsigned long long)(long double)3e9 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)3e9; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)3e9; static _Bool r3 = (_Bool)(long double)3e9;
      printf("3107: (_Bool)(long double)3e9 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)3e9; float r1 = (float)v; float r2 = (float)(long double)3e9; static float r3 = (float)(long double)3e9;
      printf("3108: (float)(long double)3e9 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)3e9; double r1 = (double)v; double r2 = (double)(long double)3e9; static double r3 = (double)(long double)3e9;
      printf("3109: (double)(long double)3e9 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)3e9; long double r1 = (long double)v; long double r2 = (long double)(long double)3e9; static long double r3 = (long double)(long double)3e9;
      printf("3110: (long double)(long double)3e9 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)1e18; long r1 = (long)v; long r2 = (long)(long double)1e18; static long r3 = (long)(long double)1e18;
      printf("3111: (long)(long double)1e18 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)1e18; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)1e18; static unsigned long r3 = (unsigned long)(long double)1e18;
      printf("3112: (unsigned long)(long double)1e18 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)1e18; long long r1 = (long long)v; long long r2 = (long long)(long double)1e18; static long long r3 = (long long)(long double)1e18;
      printf("3113: (long long)(long double)1e18 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)1e18; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)1e18; static unsigned long long r3 = (unsigned long long)(long double)1e18;
      printf("3114: (unsigned long long)(long double)1e18 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)1e18; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)1e18; static _Bool r3 = (_Bool)(long double)1e18;
      printf("3115: (_Bool)(long double)1e18 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1e18; float r1 = (float)v; float r2 = (float)(long double)1e18; static float r3 = (float)(long double)1e18;
      printf("3116: (float)(long double)1e18 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)1e18; double r1 = (double)v; double r2 = (double)(long double)1e18; static double r3 = (double)(long double)1e18;
      printf("3117: (double)(long double)1e18 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)1e18; long double r1 = (long double)v; long double r2 = (long double)(long double)1e18; static long double r3 = (long double)(long double)1e18;
      printf("3118: (long double)(long double)1e18 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)9.2e18; long r1 = (long)v; long r2 = (long)(long double)9.2e18; static long r3 = (long)(long double)9.2e18;
      printf("3119: (long)(long double)9.2e18 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)9.2e18; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)9.2e18; static unsigned long r3 = (unsigned long)(long double)9.2e18;
      printf("3120: (unsigned long)(long double)9.2e18 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)9.2e18; long long r1 = (long long)v; long long r2 = (long long)(long double)9.2e18; static long long r3 = (long long)(long double)9.2e18;
      printf("3121: (long long)(long double)9.2e18 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)9.2e18; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)9.2e18; static unsigned long long r3 = (unsigned long long)(long double)9.2e18;
      printf("3122: (unsigned long long)(long double)9.2e18 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)9.2e18; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)9.2e18; static _Bool r3 = (_Bool)(long double)9.2e18;
      printf("3123: (_Bool)(long double)9.2e18 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)9.2e18; float r1 = (float)v; float r2 = (float)(long double)9.2e18; static float r3 = (float)(long double)9.2e18;
      printf("3124: (float)(long double)9.2e18 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)9.2e18; double r1 = (double)v; double r2 = (double)(long double)9.2e18; static double r3 = (double)(long double)9.2e18;
      printf("3125: (double)(long double)9.2e18 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)9.2e18; long double r1 = (long double)v; long double r2 = (long double)(long double)9.2e18; static long double r3 = (long double)(long double)9.2e18;
      printf("3126: (long double)(long double)9.2e18 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)1.8e19; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)1.8e19; static unsigned long r3 = (unsigned long)(long double)1.8e19;
      printf("3127: (unsigned long)(long double)1.8e19 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)1.8e19; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)1.8e19; static unsigned long long r3 = (unsigned long long)(long double)1.8e19;
      printf("3128: (unsigned long long)(long double)1.8e19 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)1.8e19; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)1.8e19; static _Bool r3 = (_Bool)(long double)1.8e19;
      printf("3129: (_Bool)(long double)1.8e19 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1.8e19; float r1 = (float)v; float r2 = (float)(long double)1.8e19; static float r3 = (float)(long double)1.8e19;
      printf("3130: (float)(long double)1.8e19 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)1.8e19; double r1 = (double)v; double r2 = (double)(long double)1.8e19; static double r3 = (double)(long double)1.8e19;
      printf("3131: (double)(long double)1.8e19 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)1.8e19; long double r1 = (long double)v; long double r2 = (long double)(long double)1.8e19; static long double r3 = (long double)(long double)1.8e19;
      printf("3132: (long double)(long double)1.8e19 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)1e-30; signed char r1 = (signed char)v; signed char r2 = (signed char)(long double)1e-30; static signed char r3 = (signed char)(long double)1e-30;
      printf("3133: (signed char)(long double)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1e-30; unsigned char r1 = (unsigned char)v; unsigned char r2 = (unsigned char)(long double)1e-30; static unsigned char r3 = (unsigned char)(long double)1e-30;
      printf("3134: (unsigned char)(long double)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1e-30; short r1 = (short)v; short r2 = (short)(long double)1e-30; static short r3 = (short)(long double)1e-30;
      printf("3135: (short)(long double)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1e-30; unsigned short r1 = (unsigned short)v; unsigned short r2 = (unsigned short)(long double)1e-30; static unsigned short r3 = (unsigned short)(long double)1e-30;
      printf("3136: (unsigned short)(long double)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1e-30; int r1 = (int)v; int r2 = (int)(long double)1e-30; static int r3 = (int)(long double)1e-30;
      printf("3137: (int)(long double)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1e-30; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long double)1e-30; static unsigned r3 = (unsigned)(long double)1e-30;
      printf("3138: (unsigned)(long double)1e-30 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long double v = (long double)1e-30; long r1 = (long)v; long r2 = (long)(long double)1e-30; static long r3 = (long)(long double)1e-30;
      printf("3139: (long)(long double)1e-30 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)1e-30; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)1e-30; static unsigned long r3 = (unsigned long)(long double)1e-30;
      printf("3140: (unsigned long)(long double)1e-30 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)1e-30; long long r1 = (long long)v; long long r2 = (long long)(long double)1e-30; static long long r3 = (long long)(long double)1e-30;
      printf("3141: (long long)(long double)1e-30 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)1e-30; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)1e-30; static unsigned long long r3 = (unsigned long long)(long double)1e-30;
      printf("3142: (unsigned long long)(long double)1e-30 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)1e-30; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)1e-30; static _Bool r3 = (_Bool)(long double)1e-30;
      printf("3143: (_Bool)(long double)1e-30 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)1e-30; float r1 = (float)v; float r2 = (float)(long double)1e-30; static float r3 = (float)(long double)1e-30;
      printf("3144: (float)(long double)1e-30 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)1e-30; double r1 = (double)v; double r2 = (double)(long double)1e-30; static double r3 = (double)(long double)1e-30;
      printf("3145: (double)(long double)1e-30 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)1e-30; long double r1 = (long double)v; long double r2 = (long double)(long double)1e-30; static long double r3 = (long double)(long double)1e-30;
      printf("3146: (long double)(long double)1e-30 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)16777217.0; int r1 = (int)v; int r2 = (int)(long double)16777217.0; static int r3 = (int)(long double)16777217.0;
      printf("3147: (int)(long double)16777217.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)16777217.0; unsigned r1 = (unsigned)v; unsigned r2 = (unsigned)(long double)16777217.0; static unsigned r3 = (unsigned)(long double)16777217.0;
      printf("3148: (unsigned)(long double)16777217.0 = %u %u %u\n", (unsigned)r1, (unsigned)r2, (unsigned)r3); }
    { volatile long double v = (long double)16777217.0; long r1 = (long)v; long r2 = (long)(long double)16777217.0; static long r3 = (long)(long double)16777217.0;
      printf("3149: (long)(long double)16777217.0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)16777217.0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)16777217.0; static unsigned long r3 = (unsigned long)(long double)16777217.0;
      printf("3150: (unsigned long)(long double)16777217.0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)16777217.0; long long r1 = (long long)v; long long r2 = (long long)(long double)16777217.0; static long long r3 = (long long)(long double)16777217.0;
      printf("3151: (long long)(long double)16777217.0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)16777217.0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)16777217.0; static unsigned long long r3 = (unsigned long long)(long double)16777217.0;
      printf("3152: (unsigned long long)(long double)16777217.0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)16777217.0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)16777217.0; static _Bool r3 = (_Bool)(long double)16777217.0;
      printf("3153: (_Bool)(long double)16777217.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)16777217.0; float r1 = (float)v; float r2 = (float)(long double)16777217.0; static float r3 = (float)(long double)16777217.0;
      printf("3154: (float)(long double)16777217.0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)16777217.0; double r1 = (double)v; double r2 = (double)(long double)16777217.0; static double r3 = (double)(long double)16777217.0;
      printf("3155: (double)(long double)16777217.0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)16777217.0; long double r1 = (long double)v; long double r2 = (long double)(long double)16777217.0; static long double r3 = (long double)(long double)16777217.0;
      printf("3156: (long double)(long double)16777217.0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double v = (long double)9007199254740993.0; long r1 = (long)v; long r2 = (long)(long double)9007199254740993.0; static long r3 = (long)(long double)9007199254740993.0;
      printf("3157: (long)(long double)9007199254740993.0 = %ld %ld %ld\n", (long)r1, (long)r2, (long)r3); }
    { volatile long double v = (long double)9007199254740993.0; unsigned long r1 = (unsigned long)v; unsigned long r2 = (unsigned long)(long double)9007199254740993.0; static unsigned long r3 = (unsigned long)(long double)9007199254740993.0;
      printf("3158: (unsigned long)(long double)9007199254740993.0 = %lu %lu %lu\n", (unsigned long)r1, (unsigned long)r2, (unsigned long)r3); }
    { volatile long double v = (long double)9007199254740993.0; long long r1 = (long long)v; long long r2 = (long long)(long double)9007199254740993.0; static long long r3 = (long long)(long double)9007199254740993.0;
      printf("3159: (long long)(long double)9007199254740993.0 = %lld %lld %lld\n", (long long)r1, (long long)r2, (long long)r3); }
    { volatile long double v = (long double)9007199254740993.0; unsigned long long r1 = (unsigned long long)v; unsigned long long r2 = (unsigned long long)(long double)9007199254740993.0; static unsigned long long r3 = (unsigned long long)(long double)9007199254740993.0;
      printf("3160: (unsigned long long)(long double)9007199254740993.0 = %llu %llu %llu\n", (unsigned long long)r1, (unsigned long long)r2, (unsigned long long)r3); }
    { volatile long double v = (long double)9007199254740993.0; _Bool r1 = (_Bool)v; _Bool r2 = (_Bool)(long double)9007199254740993.0; static _Bool r3 = (_Bool)(long double)9007199254740993.0;
      printf("3161: (_Bool)(long double)9007199254740993.0 = %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double v = (long double)9007199254740993.0; float r1 = (float)v; float r2 = (float)(long double)9007199254740993.0; static float r3 = (float)(long double)9007199254740993.0;
      printf("3162: (float)(long double)9007199254740993.0 = %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)9007199254740993.0; double r1 = (double)v; double r2 = (double)(long double)9007199254740993.0; static double r3 = (double)(long double)9007199254740993.0;
      printf("3163: (double)(long double)9007199254740993.0 = %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long double v = (long double)9007199254740993.0; long double r1 = (long double)v; long double r2 = (long double)(long double)9007199254740993.0; static long double r3 = (long double)(long double)9007199254740993.0;
      printf("3164: (long double)(long double)9007199254740993.0 = %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    return 0;
}
