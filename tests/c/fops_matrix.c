#include <stdio.h>
int main(void) {
    { volatile float vx = ((float)16777217.0f); volatile float vy = ((float)123456.789f); float r1 = vx + vy; float r2 = ((float)16777217.0f) + ((float)123456.789f); static float r3 = ((float)16777217.0f) + ((float)123456.789f);
      printf("1: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)123456.789f); volatile float vy = ((float)16777217.0f); float r1 = vx + vy; float r2 = ((float)123456.789f) + ((float)16777217.0f); static float r3 = ((float)123456.789f) + ((float)16777217.0f);
      printf("2: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile float vy = ((float)16777217.0f); float r1 = vx + vy; float r2 = ((float)1e-5f) + ((float)16777217.0f); static float r3 = ((float)1e-5f) + ((float)16777217.0f);
      printf("3: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile float vy = ((float)1e-5f); float r1 = vx + vy; float r2 = ((float)16777217.0f) + ((float)1e-5f); static float r3 = ((float)16777217.0f) + ((float)1e-5f);
      printf("4: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile float vy = ((float)1.0f); float r1 = vx + vy; float r2 = ((float)3.0f) + ((float)1.0f); static float r3 = ((float)3.0f) + ((float)1.0f);
      printf("5: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1.0f); volatile float vy = ((float)3.0f); float r1 = vx + vy; float r2 = ((float)1.0f) + ((float)3.0f); static float r3 = ((float)1.0f) + ((float)3.0f);
      printf("6: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile float vy = ((float)1e-5f); float r1 = vx + vy; float r2 = ((float)16777217.0f) + ((float)1e-5f); static float r3 = ((float)16777217.0f) + ((float)1e-5f);
      printf("7: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile float vy = ((float)16777217.0f); float r1 = vx + vy; float r2 = ((float)1e-5f) + ((float)16777217.0f); static float r3 = ((float)1e-5f) + ((float)16777217.0f);
      printf("8: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e30f); volatile float vy = ((float)123456.789f); float r1 = vx - vy; float r2 = ((float)1e30f) - ((float)123456.789f); static float r3 = ((float)1e30f) - ((float)123456.789f);
      printf("9: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)123456.789f); volatile float vy = ((float)1e30f); float r1 = vx - vy; float r2 = ((float)123456.789f) - ((float)1e30f); static float r3 = ((float)123456.789f) - ((float)1e30f);
      printf("10: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile float vy = ((float)1e-5f); float r1 = vx - vy; float r2 = ((float)-0.1f) - ((float)1e-5f); static float r3 = ((float)-0.1f) - ((float)1e-5f);
      printf("11: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile float vy = ((float)-0.1f); float r1 = vx - vy; float r2 = ((float)1e-5f) - ((float)-0.1f); static float r3 = ((float)1e-5f) - ((float)-0.1f);
      printf("12: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1.0f); volatile float vy = ((float)-0.1f); float r1 = vx - vy; float r2 = ((float)1.0f) - ((float)-0.1f); static float r3 = ((float)1.0f) - ((float)-0.1f);
      printf("13: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile float vy = ((float)1.0f); float r1 = vx - vy; float r2 = ((float)-0.1f) - ((float)1.0f); static float r3 = ((float)-0.1f) - ((float)1.0f);
      printf("14: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)123456.789f); volatile float vy = ((float)1e-40f); float r1 = vx - vy; float r2 = ((float)123456.789f) - ((float)1e-40f); static float r3 = ((float)123456.789f) - ((float)1e-40f);
      printf("15: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-40f); volatile float vy = ((float)123456.789f); float r1 = vx - vy; float r2 = ((float)1e-40f) - ((float)123456.789f); static float r3 = ((float)1e-40f) - ((float)123456.789f);
      printf("16: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile float vy = ((float)1e30f); float r1 = vx * vy; float r2 = ((float)0.1f) * ((float)1e30f); static float r3 = ((float)0.1f) * ((float)1e30f);
      printf("17: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e30f); volatile float vy = ((float)0.1f); float r1 = vx * vy; float r2 = ((float)1e30f) * ((float)0.1f); static float r3 = ((float)1e30f) * ((float)0.1f);
      printf("18: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile float vy = ((float)7.0f); float r1 = vx * vy; float r2 = ((float)1e-5f) * ((float)7.0f); static float r3 = ((float)1e-5f) * ((float)7.0f);
      printf("19: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)7.0f); volatile float vy = ((float)1e-5f); float r1 = vx * vy; float r2 = ((float)7.0f) * ((float)1e-5f); static float r3 = ((float)7.0f) * ((float)1e-5f);
      printf("20: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e30f); volatile float vy = ((float)7.0f); float r1 = vx * vy; float r2 = ((float)1e30f) * ((float)7.0f); static float r3 = ((float)1e30f) * ((float)7.0f);
      printf("21: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)7.0f); volatile float vy = ((float)1e30f); float r1 = vx * vy; float r2 = ((float)7.0f) * ((float)1e30f); static float r3 = ((float)7.0f) * ((float)1e30f);
      printf("22: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e30f); volatile float vy = ((float)0.1f); float r1 = vx * vy; float r2 = ((float)1e30f) * ((float)0.1f); static float r3 = ((float)1e30f) * ((float)0.1f);
      printf("23: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile float vy = ((float)1e30f); float r1 = vx * vy; float r2 = ((float)0.1f) * ((float)1e30f); static float r3 = ((float)0.1f) * ((float)1e30f);
      printf("24: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile float vy = ((float)0.1f); float r1 = vx / vy; float r2 = ((float)-0.1f) / ((float)0.1f); static float r3 = ((float)-0.1f) / ((float)0.1f);
      printf("25: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile float vy = ((float)-0.1f); float r1 = vx / vy; float r2 = ((float)0.1f) / ((float)-0.1f); static float r3 = ((float)0.1f) / ((float)-0.1f);
      printf("26: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile float vy = ((float)3.0f); float r1 = vx / vy; float r2 = ((float)3.0f) / ((float)3.0f); static float r3 = ((float)3.0f) / ((float)3.0f);
      printf("27: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile float vy = ((float)3.0f); float r1 = vx / vy; float r2 = ((float)3.0f) / ((float)3.0f); static float r3 = ((float)3.0f) / ((float)3.0f);
      printf("28: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile float vy = ((float)123456.789f); float r1 = vx / vy; float r2 = ((float)0.1f) / ((float)123456.789f); static float r3 = ((float)0.1f) / ((float)123456.789f);
      printf("29: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)123456.789f); volatile float vy = ((float)0.1f); float r1 = vx / vy; float r2 = ((float)123456.789f) / ((float)0.1f); static float r3 = ((float)123456.789f) / ((float)0.1f);
      printf("30: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-2.5f); volatile float vy = ((float)1e-5f); float r1 = vx / vy; float r2 = ((float)-2.5f) / ((float)1e-5f); static float r3 = ((float)-2.5f) / ((float)1e-5f);
      printf("31: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile float vy = ((float)-2.5f); float r1 = vx / vy; float r2 = ((float)1e-5f) / ((float)-2.5f); static float r3 = ((float)1e-5f) / ((float)-2.5f);
      printf("32: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile float vy = ((float)16777217.0f); int r1 = vx < vy; int r2 = ((float)3.0f) < ((float)16777217.0f); static int r3 = ((float)3.0f) < ((float)16777217.0f);
      printf("33: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)16777217.0f); volatile float vy = ((float)3.0f); int r1 = vx < vy; int r2 = ((float)16777217.0f) < ((float)3.0f); static int r3 = ((float)16777217.0f) < ((float)3.0f);
      printf("34: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)7.0f); volatile float vy = ((float)1e10f); int r1 = vx < vy; int r2 = ((float)7.0f) < ((float)1e10f); static int r3 = ((float)7.0f) < ((float)1e10f);
      printf("35: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e10f); volatile float vy = ((float)7.0f); int r1 = vx < vy; int r2 = ((float)1e10f) < ((float)7.0f); static int r3 = ((float)1e10f) < ((float)7.0f);
      printf("36: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.1f); volatile float vy = ((float)7.0f); int r1 = vx < vy; int r2 = ((float)0.1f) < ((float)7.0f); static int r3 = ((float)0.1f) < ((float)7.0f);
      printf("37: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)7.0f); volatile float vy = ((float)0.1f); int r1 = vx < vy; int r2 = ((float)7.0f) < ((float)0.1f); static int r3 = ((float)7.0f) < ((float)0.1f);
      printf("38: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-5f); volatile float vy = ((float)7.0f); int r1 = vx < vy; int r2 = ((float)1e-5f) < ((float)7.0f); static int r3 = ((float)1e-5f) < ((float)7.0f);
      printf("39: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)7.0f); volatile float vy = ((float)1e-5f); int r1 = vx < vy; int r2 = ((float)7.0f) < ((float)1e-5f); static int r3 = ((float)7.0f) < ((float)1e-5f);
      printf("40: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile float vy = ((float)16777217.0f); int r1 = vx == vy; int r2 = ((float)0.3333333333333333f) == ((float)16777217.0f); static int r3 = ((float)0.3333333333333333f) == ((float)16777217.0f);
      printf("41: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)16777217.0f); volatile float vy = ((float)0.3333333333333333f); int r1 = vx == vy; int r2 = ((float)16777217.0f) == ((float)0.3333333333333333f); static int r3 = ((float)16777217.0f) == ((float)0.3333333333333333f);
      printf("42: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-40f); volatile float vy = ((float)1e10f); int r1 = vx == vy; int r2 = ((float)1e-40f) == ((float)1e10f); static int r3 = ((float)1e-40f) == ((float)1e10f);
      printf("43: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e10f); volatile float vy = ((float)1e-40f); int r1 = vx == vy; int r2 = ((float)1e10f) == ((float)1e-40f); static int r3 = ((float)1e10f) == ((float)1e-40f);
      printf("44: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)123456.789f); volatile float vy = ((float)3.0f); int r1 = vx == vy; int r2 = ((float)123456.789f) == ((float)3.0f); static int r3 = ((float)123456.789f) == ((float)3.0f);
      printf("45: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile float vy = ((float)123456.789f); int r1 = vx == vy; int r2 = ((float)3.0f) == ((float)123456.789f); static int r3 = ((float)3.0f) == ((float)123456.789f);
      printf("46: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e10f); volatile float vy = ((float)0.1f); int r1 = vx == vy; int r2 = ((float)1e10f) == ((float)0.1f); static int r3 = ((float)1e10f) == ((float)0.1f);
      printf("47: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.1f); volatile float vy = ((float)1e10f); int r1 = vx == vy; int r2 = ((float)0.1f) == ((float)1e10f); static int r3 = ((float)0.1f) == ((float)1e10f);
      printf("48: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile float vy = ((float)123456.789f); int r1 = vx >= vy; int r2 = ((float)1e30f) >= ((float)123456.789f); static int r3 = ((float)1e30f) >= ((float)123456.789f);
      printf("49: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)123456.789f); volatile float vy = ((float)1e30f); int r1 = vx >= vy; int r2 = ((float)123456.789f) >= ((float)1e30f); static int r3 = ((float)123456.789f) >= ((float)1e30f);
      printf("50: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile float vy = ((float)-0.1f); int r1 = vx >= vy; int r2 = ((float)0.3333333333333333f) >= ((float)-0.1f); static int r3 = ((float)0.3333333333333333f) >= ((float)-0.1f);
      printf("51: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile float vy = ((float)0.3333333333333333f); int r1 = vx >= vy; int r2 = ((float)-0.1f) >= ((float)0.3333333333333333f); static int r3 = ((float)-0.1f) >= ((float)0.3333333333333333f);
      printf("52: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile float vy = ((float)-0.1f); int r1 = vx >= vy; int r2 = ((float)0.3333333333333333f) >= ((float)-0.1f); static int r3 = ((float)0.3333333333333333f) >= ((float)-0.1f);
      printf("53: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile float vy = ((float)0.3333333333333333f); int r1 = vx >= vy; int r2 = ((float)-0.1f) >= ((float)0.3333333333333333f); static int r3 = ((float)-0.1f) >= ((float)0.3333333333333333f);
      printf("54: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)7.0f); volatile float vy = ((float)0.1f); int r1 = vx >= vy; int r2 = ((float)7.0f) >= ((float)0.1f); static int r3 = ((float)7.0f) >= ((float)0.1f);
      printf("55: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.1f); volatile float vy = ((float)7.0f); int r1 = vx >= vy; int r2 = ((float)0.1f) >= ((float)7.0f); static int r3 = ((float)0.1f) >= ((float)7.0f);
      printf("56: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile double vy = ((double)0.1); double r1 = vx + vy; double r2 = ((float)3.0f) + ((double)0.1); static double r3 = ((float)3.0f) + ((double)0.1);
      printf("57: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.1); volatile float vy = ((float)3.0f); double r1 = vx + vy; double r2 = ((double)0.1) + ((float)3.0f); static double r3 = ((double)0.1) + ((float)3.0f);
      printf("58: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile double vy = ((double)1e-40); double r1 = vx + vy; double r2 = ((float)0.3333333333333333f) + ((double)1e-40); static double r3 = ((float)0.3333333333333333f) + ((double)1e-40);
      printf("59: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile float vy = ((float)0.3333333333333333f); double r1 = vx + vy; double r2 = ((double)1e-40) + ((float)0.3333333333333333f); static double r3 = ((double)1e-40) + ((float)0.3333333333333333f);
      printf("60: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile double vy = ((double)-0.1); double r1 = vx + vy; double r2 = ((float)0.3333333333333333f) + ((double)-0.1); static double r3 = ((float)0.3333333333333333f) + ((double)-0.1);
      printf("61: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-0.1); volatile float vy = ((float)0.3333333333333333f); double r1 = vx + vy; double r2 = ((double)-0.1) + ((float)0.3333333333333333f); static double r3 = ((double)-0.1) + ((float)0.3333333333333333f);
      printf("62: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)7.0f); volatile double vy = ((double)3.0); double r1 = vx + vy; double r2 = ((float)7.0f) + ((double)3.0); static double r3 = ((float)7.0f) + ((double)3.0);
      printf("63: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile float vy = ((float)7.0f); double r1 = vx + vy; double r2 = ((double)3.0) + ((float)7.0f); static double r3 = ((double)3.0) + ((float)7.0f);
      printf("64: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)7.0f); volatile double vy = ((double)1e10); double r1 = vx - vy; double r2 = ((float)7.0f) - ((double)1e10); static double r3 = ((float)7.0f) - ((double)1e10);
      printf("65: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile float vy = ((float)7.0f); double r1 = vx - vy; double r2 = ((double)1e10) - ((float)7.0f); static double r3 = ((double)1e10) - ((float)7.0f);
      printf("66: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile double vy = ((double)1e10); double r1 = vx - vy; double r2 = ((float)-0.1f) - ((double)1e10); static double r3 = ((float)-0.1f) - ((double)1e10);
      printf("67: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile float vy = ((float)-0.1f); double r1 = vx - vy; double r2 = ((double)1e10) - ((float)-0.1f); static double r3 = ((double)1e10) - ((float)-0.1f);
      printf("68: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile double vy = ((double)0.3333333333333333); double r1 = vx - vy; double r2 = ((float)0.1f) - ((double)0.3333333333333333); static double r3 = ((float)0.1f) - ((double)0.3333333333333333);
      printf("69: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile float vy = ((float)0.1f); double r1 = vx - vy; double r2 = ((double)0.3333333333333333) - ((float)0.1f); static double r3 = ((double)0.3333333333333333) - ((float)0.1f);
      printf("70: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile double vy = ((double)1.0); double r1 = vx - vy; double r2 = ((float)-0.1f) - ((double)1.0); static double r3 = ((float)-0.1f) - ((double)1.0);
      printf("71: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile float vy = ((float)-0.1f); double r1 = vx - vy; double r2 = ((double)1.0) - ((float)-0.1f); static double r3 = ((double)1.0) - ((float)-0.1f);
      printf("72: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-40f); volatile double vy = ((double)-0.1); double r1 = vx * vy; double r2 = ((float)1e-40f) * ((double)-0.1); static double r3 = ((float)1e-40f) * ((double)-0.1);
      printf("73: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-0.1); volatile float vy = ((float)1e-40f); double r1 = vx * vy; double r2 = ((double)-0.1) * ((float)1e-40f); static double r3 = ((double)-0.1) * ((float)1e-40f);
      printf("74: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile double vy = ((double)1e-5); double r1 = vx * vy; double r2 = ((float)0.1f) * ((double)1e-5); static double r3 = ((float)0.1f) * ((double)1e-5);
      printf("75: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-5); volatile float vy = ((float)0.1f); double r1 = vx * vy; double r2 = ((double)1e-5) * ((float)0.1f); static double r3 = ((double)1e-5) * ((float)0.1f);
      printf("76: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile double vy = ((double)1.0); double r1 = vx * vy; double r2 = ((float)1e-5f) * ((double)1.0); static double r3 = ((float)1e-5f) * ((double)1.0);
      printf("77: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile float vy = ((float)1e-5f); double r1 = vx * vy; double r2 = ((double)1.0) * ((float)1e-5f); static double r3 = ((double)1.0) * ((float)1e-5f);
      printf("78: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile double vy = ((double)3.0); double r1 = vx * vy; double r2 = ((float)16777217.0f) * ((double)3.0); static double r3 = ((float)16777217.0f) * ((double)3.0);
      printf("79: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile float vy = ((float)16777217.0f); double r1 = vx * vy; double r2 = ((double)3.0) * ((float)16777217.0f); static double r3 = ((double)3.0) * ((float)16777217.0f);
      printf("80: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile double vy = ((double)3.0); double r1 = vx / vy; double r2 = ((float)16777217.0f) / ((double)3.0); static double r3 = ((float)16777217.0f) / ((double)3.0);
      printf("81: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile float vy = ((float)16777217.0f); double r1 = vx / vy; double r2 = ((double)3.0) / ((float)16777217.0f); static double r3 = ((double)3.0) / ((float)16777217.0f);
      printf("82: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)123456.789f); volatile double vy = ((double)1.0); double r1 = vx / vy; double r2 = ((float)123456.789f) / ((double)1.0); static double r3 = ((float)123456.789f) / ((double)1.0);
      printf("83: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile float vy = ((float)123456.789f); double r1 = vx / vy; double r2 = ((double)1.0) / ((float)123456.789f); static double r3 = ((double)1.0) / ((float)123456.789f);
      printf("84: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)7.0f); volatile double vy = ((double)0.3333333333333333); double r1 = vx / vy; double r2 = ((float)7.0f) / ((double)0.3333333333333333); static double r3 = ((float)7.0f) / ((double)0.3333333333333333);
      printf("85: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile float vy = ((float)7.0f); double r1 = vx / vy; double r2 = ((double)0.3333333333333333) / ((float)7.0f); static double r3 = ((double)0.3333333333333333) / ((float)7.0f);
      printf("86: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile double vy = ((double)0.3333333333333333); double r1 = vx / vy; double r2 = ((float)0.3333333333333333f) / ((double)0.3333333333333333); static double r3 = ((float)0.3333333333333333f) / ((double)0.3333333333333333);
      printf("87: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile float vy = ((float)0.3333333333333333f); double r1 = vx / vy; double r2 = ((double)0.3333333333333333) / ((float)0.3333333333333333f); static double r3 = ((double)0.3333333333333333) / ((float)0.3333333333333333f);
      printf("88: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile double vy = ((double)1e10); int r1 = vx < vy; int r2 = ((float)0.1f) < ((double)1e10); static int r3 = ((float)0.1f) < ((double)1e10);
      printf("89: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e10); volatile float vy = ((float)0.1f); int r1 = vx < vy; int r2 = ((double)1e10) < ((float)0.1f); static int r3 = ((double)1e10) < ((float)0.1f);
      printf("90: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile double vy = ((double)1e10); int r1 = vx < vy; int r2 = ((float)1e30f) < ((double)1e10); static int r3 = ((float)1e30f) < ((double)1e10);
      printf("91: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e10); volatile float vy = ((float)1e30f); int r1 = vx < vy; int r2 = ((double)1e10) < ((float)1e30f); static int r3 = ((double)1e10) < ((float)1e30f);
      printf("92: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile double vy = ((double)1.0); int r1 = vx < vy; int r2 = ((float)3.0f) < ((double)1.0); static int r3 = ((float)3.0f) < ((double)1.0);
      printf("93: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1.0); volatile float vy = ((float)3.0f); int r1 = vx < vy; int r2 = ((double)1.0) < ((float)3.0f); static int r3 = ((double)1.0) < ((float)3.0f);
      printf("94: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile double vy = ((double)7.0); int r1 = vx < vy; int r2 = ((float)1e30f) < ((double)7.0); static int r3 = ((float)1e30f) < ((double)7.0);
      printf("95: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)7.0); volatile float vy = ((float)1e30f); int r1 = vx < vy; int r2 = ((double)7.0) < ((float)1e30f); static int r3 = ((double)7.0) < ((float)1e30f);
      printf("96: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile double vy = ((double)0.1); int r1 = vx == vy; int r2 = ((float)-0.1f) == ((double)0.1); static int r3 = ((float)-0.1f) == ((double)0.1);
      printf("97: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.1); volatile float vy = ((float)-0.1f); int r1 = vx == vy; int r2 = ((double)0.1) == ((float)-0.1f); static int r3 = ((double)0.1) == ((float)-0.1f);
      printf("98: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile double vy = ((double)1e-5); int r1 = vx == vy; int r2 = ((float)3.0f) == ((double)1e-5); static int r3 = ((float)3.0f) == ((double)1e-5);
      printf("99: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile float vy = ((float)3.0f); int r1 = vx == vy; int r2 = ((double)1e-5) == ((float)3.0f); static int r3 = ((double)1e-5) == ((float)3.0f);
      printf("100: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.1f); volatile double vy = ((double)123456.789); int r1 = vx == vy; int r2 = ((float)0.1f) == ((double)123456.789); static int r3 = ((float)0.1f) == ((double)123456.789);
      printf("101: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile float vy = ((float)0.1f); int r1 = vx == vy; int r2 = ((double)123456.789) == ((float)0.1f); static int r3 = ((double)123456.789) == ((float)0.1f);
      printf("102: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-2.5f); volatile double vy = ((double)1e10); int r1 = vx == vy; int r2 = ((float)-2.5f) == ((double)1e10); static int r3 = ((float)-2.5f) == ((double)1e10);
      printf("103: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e10); volatile float vy = ((float)-2.5f); int r1 = vx == vy; int r2 = ((double)1e10) == ((float)-2.5f); static int r3 = ((double)1e10) == ((float)-2.5f);
      printf("104: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile double vy = ((double)-0.1); int r1 = vx >= vy; int r2 = ((float)-0.1f) >= ((double)-0.1); static int r3 = ((float)-0.1f) >= ((double)-0.1);
      printf("105: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-0.1); volatile float vy = ((float)-0.1f); int r1 = vx >= vy; int r2 = ((double)-0.1) >= ((float)-0.1f); static int r3 = ((double)-0.1) >= ((float)-0.1f);
      printf("106: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile double vy = ((double)1e30); int r1 = vx >= vy; int r2 = ((float)3.0f) >= ((double)1e30); static int r3 = ((float)3.0f) >= ((double)1e30);
      printf("107: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e30); volatile float vy = ((float)3.0f); int r1 = vx >= vy; int r2 = ((double)1e30) >= ((float)3.0f); static int r3 = ((double)1e30) >= ((float)3.0f);
      printf("108: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile double vy = ((double)0.1); int r1 = vx >= vy; int r2 = ((float)3.0f) >= ((double)0.1); static int r3 = ((float)3.0f) >= ((double)0.1);
      printf("109: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.1); volatile float vy = ((float)3.0f); int r1 = vx >= vy; int r2 = ((double)0.1) >= ((float)3.0f); static int r3 = ((double)0.1) >= ((float)3.0f);
      printf("110: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-2.5f); volatile double vy = ((double)-2.5); int r1 = vx >= vy; int r2 = ((float)-2.5f) >= ((double)-2.5); static int r3 = ((float)-2.5f) >= ((double)-2.5);
      printf("111: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-2.5); volatile float vy = ((float)-2.5f); int r1 = vx >= vy; int r2 = ((double)-2.5) >= ((float)-2.5f); static int r3 = ((double)-2.5) >= ((float)-2.5f);
      printf("112: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-5f); volatile long double vy = ((long double)1.0L); long double r1 = vx + vy; long double r2 = ((float)1e-5f) + ((long double)1.0L); static long double r3 = ((float)1e-5f) + ((long double)1.0L);
      printf("113: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile float vy = ((float)1e-5f); long double r1 = vx + vy; long double r2 = ((long double)1.0L) + ((float)1e-5f); static long double r3 = ((long double)1.0L) + ((float)1e-5f);
      printf("114: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)1e-5f); volatile long double vy = ((long double)1e30L); long double r1 = vx + vy; long double r2 = ((float)1e-5f) + ((long double)1e30L); static long double r3 = ((float)1e-5f) + ((long double)1e30L);
      printf("115: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile float vy = ((float)1e-5f); long double r1 = vx + vy; long double r2 = ((long double)1e30L) + ((float)1e-5f); static long double r3 = ((long double)1e30L) + ((float)1e-5f);
      printf("116: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile long double vy = ((long double)1.0L); long double r1 = vx + vy; long double r2 = ((float)0.3333333333333333f) + ((long double)1.0L); static long double r3 = ((float)0.3333333333333333f) + ((long double)1.0L);
      printf("117: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile float vy = ((float)0.3333333333333333f); long double r1 = vx + vy; long double r2 = ((long double)1.0L) + ((float)0.3333333333333333f); static long double r3 = ((long double)1.0L) + ((float)0.3333333333333333f);
      printf("118: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)1.0f); volatile long double vy = ((long double)1e10L); long double r1 = vx + vy; long double r2 = ((float)1.0f) + ((long double)1e10L); static long double r3 = ((float)1.0f) + ((long double)1e10L);
      printf("119: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile float vy = ((float)1.0f); long double r1 = vx + vy; long double r2 = ((long double)1e10L) + ((float)1.0f); static long double r3 = ((long double)1e10L) + ((float)1.0f);
      printf("120: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)1e30f); volatile long double vy = ((long double)3.0L); long double r1 = vx - vy; long double r2 = ((float)1e30f) - ((long double)3.0L); static long double r3 = ((float)1e30f) - ((long double)3.0L);
      printf("121: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)3.0L); volatile float vy = ((float)1e30f); long double r1 = vx - vy; long double r2 = ((long double)3.0L) - ((float)1e30f); static long double r3 = ((long double)3.0L) - ((float)1e30f);
      printf("122: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)3.0f); volatile long double vy = ((long double)1.0L); long double r1 = vx - vy; long double r2 = ((float)3.0f) - ((long double)1.0L); static long double r3 = ((float)3.0f) - ((long double)1.0L);
      printf("123: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile float vy = ((float)3.0f); long double r1 = vx - vy; long double r2 = ((long double)1.0L) - ((float)3.0f); static long double r3 = ((long double)1.0L) - ((float)3.0f);
      printf("124: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)7.0f); volatile long double vy = ((long double)16777217.0L); long double r1 = vx - vy; long double r2 = ((float)7.0f) - ((long double)16777217.0L); static long double r3 = ((float)7.0f) - ((long double)16777217.0L);
      printf("125: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile float vy = ((float)7.0f); long double r1 = vx - vy; long double r2 = ((long double)16777217.0L) - ((float)7.0f); static long double r3 = ((long double)16777217.0L) - ((float)7.0f);
      printf("126: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)7.0f); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx - vy; long double r2 = ((float)7.0f) - ((long double)0.3333333333333333L); static long double r3 = ((float)7.0f) - ((long double)0.3333333333333333L);
      printf("127: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile float vy = ((float)7.0f); long double r1 = vx - vy; long double r2 = ((long double)0.3333333333333333L) - ((float)7.0f); static long double r3 = ((long double)0.3333333333333333L) - ((float)7.0f);
      printf("128: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)1e30f); volatile long double vy = ((long double)-0.1L); long double r1 = vx * vy; long double r2 = ((float)1e30f) * ((long double)-0.1L); static long double r3 = ((float)1e30f) * ((long double)-0.1L);
      printf("129: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile float vy = ((float)1e30f); long double r1 = vx * vy; long double r2 = ((long double)-0.1L) * ((float)1e30f); static long double r3 = ((long double)-0.1L) * ((float)1e30f);
      printf("130: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)0.1f); volatile long double vy = ((long double)-0.1L); long double r1 = vx * vy; long double r2 = ((float)0.1f) * ((long double)-0.1L); static long double r3 = ((float)0.1f) * ((long double)-0.1L);
      printf("131: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile float vy = ((float)0.1f); long double r1 = vx * vy; long double r2 = ((long double)-0.1L) * ((float)0.1f); static long double r3 = ((long double)-0.1L) * ((float)0.1f);
      printf("132: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)0.1f); volatile long double vy = ((long double)16777217.0L); long double r1 = vx * vy; long double r2 = ((float)0.1f) * ((long double)16777217.0L); static long double r3 = ((float)0.1f) * ((long double)16777217.0L);
      printf("133: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile float vy = ((float)0.1f); long double r1 = vx * vy; long double r2 = ((long double)16777217.0L) * ((float)0.1f); static long double r3 = ((long double)16777217.0L) * ((float)0.1f);
      printf("134: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)3.0f); volatile long double vy = ((long double)1e-40L); long double r1 = vx * vy; long double r2 = ((float)3.0f) * ((long double)1e-40L); static long double r3 = ((float)3.0f) * ((long double)1e-40L);
      printf("135: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile float vy = ((float)3.0f); long double r1 = vx * vy; long double r2 = ((long double)1e-40L) * ((float)3.0f); static long double r3 = ((long double)1e-40L) * ((float)3.0f);
      printf("136: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile long double vy = ((long double)1e10L); long double r1 = vx / vy; long double r2 = ((float)0.3333333333333333f) / ((long double)1e10L); static long double r3 = ((float)0.3333333333333333f) / ((long double)1e10L);
      printf("137: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile float vy = ((float)0.3333333333333333f); long double r1 = vx / vy; long double r2 = ((long double)1e10L) / ((float)0.3333333333333333f); static long double r3 = ((long double)1e10L) / ((float)0.3333333333333333f);
      printf("138: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)1e30f); volatile long double vy = ((long double)1e-5L); long double r1 = vx / vy; long double r2 = ((float)1e30f) / ((long double)1e-5L); static long double r3 = ((float)1e30f) / ((long double)1e-5L);
      printf("139: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile float vy = ((float)1e30f); long double r1 = vx / vy; long double r2 = ((long double)1e-5L) / ((float)1e30f); static long double r3 = ((long double)1e-5L) / ((float)1e30f);
      printf("140: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile long double vy = ((long double)1.0L); long double r1 = vx / vy; long double r2 = ((float)16777217.0f) / ((long double)1.0L); static long double r3 = ((float)16777217.0f) / ((long double)1.0L);
      printf("141: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile float vy = ((float)16777217.0f); long double r1 = vx / vy; long double r2 = ((long double)1.0L) / ((float)16777217.0f); static long double r3 = ((long double)1.0L) / ((float)16777217.0f);
      printf("142: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)-0.1f); volatile long double vy = ((long double)1.0L); long double r1 = vx / vy; long double r2 = ((float)-0.1f) / ((long double)1.0L); static long double r3 = ((float)-0.1f) / ((long double)1.0L);
      printf("143: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile float vy = ((float)-0.1f); long double r1 = vx / vy; long double r2 = ((long double)1.0L) / ((float)-0.1f); static long double r3 = ((long double)1.0L) / ((float)-0.1f);
      printf("144: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)1e-5f); volatile long double vy = ((long double)16777217.0L); int r1 = vx < vy; int r2 = ((float)1e-5f) < ((long double)16777217.0L); static int r3 = ((float)1e-5f) < ((long double)16777217.0L);
      printf("145: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile float vy = ((float)1e-5f); int r1 = vx < vy; int r2 = ((long double)16777217.0L) < ((float)1e-5f); static int r3 = ((long double)16777217.0L) < ((float)1e-5f);
      printf("146: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile long double vy = ((long double)1e30L); int r1 = vx < vy; int r2 = ((float)1e30f) < ((long double)1e30L); static int r3 = ((float)1e30f) < ((long double)1e30L);
      printf("147: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e30L); volatile float vy = ((float)1e30f); int r1 = vx < vy; int r2 = ((long double)1e30L) < ((float)1e30f); static int r3 = ((long double)1e30L) < ((float)1e30f);
      printf("148: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e10f); volatile long double vy = ((long double)3.0L); int r1 = vx < vy; int r2 = ((float)1e10f) < ((long double)3.0L); static int r3 = ((float)1e10f) < ((long double)3.0L);
      printf("149: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)3.0L); volatile float vy = ((float)1e10f); int r1 = vx < vy; int r2 = ((long double)3.0L) < ((float)1e10f); static int r3 = ((long double)3.0L) < ((float)1e10f);
      printf("150: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-40f); volatile long double vy = ((long double)0.1L); int r1 = vx < vy; int r2 = ((float)1e-40f) < ((long double)0.1L); static int r3 = ((float)1e-40f) < ((long double)0.1L);
      printf("151: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.1L); volatile float vy = ((float)1e-40f); int r1 = vx < vy; int r2 = ((long double)0.1L) < ((float)1e-40f); static int r3 = ((long double)0.1L) < ((float)1e-40f);
      printf("152: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e10f); volatile long double vy = ((long double)0.3333333333333333L); int r1 = vx == vy; int r2 = ((float)1e10f) == ((long double)0.3333333333333333L); static int r3 = ((float)1e10f) == ((long double)0.3333333333333333L);
      printf("153: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile float vy = ((float)1e10f); int r1 = vx == vy; int r2 = ((long double)0.3333333333333333L) == ((float)1e10f); static int r3 = ((long double)0.3333333333333333L) == ((float)1e10f);
      printf("154: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile long double vy = ((long double)0.1L); int r1 = vx == vy; int r2 = ((float)3.0f) == ((long double)0.1L); static int r3 = ((float)3.0f) == ((long double)0.1L);
      printf("155: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.1L); volatile float vy = ((float)3.0f); int r1 = vx == vy; int r2 = ((long double)0.1L) == ((float)3.0f); static int r3 = ((long double)0.1L) == ((float)3.0f);
      printf("156: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1.0f); volatile long double vy = ((long double)1e10L); int r1 = vx == vy; int r2 = ((float)1.0f) == ((long double)1e10L); static int r3 = ((float)1.0f) == ((long double)1e10L);
      printf("157: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e10L); volatile float vy = ((float)1.0f); int r1 = vx == vy; int r2 = ((long double)1e10L) == ((float)1.0f); static int r3 = ((long double)1e10L) == ((float)1.0f);
      printf("158: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)16777217.0f); volatile long double vy = ((long double)1e30L); int r1 = vx == vy; int r2 = ((float)16777217.0f) == ((long double)1e30L); static int r3 = ((float)16777217.0f) == ((long double)1e30L);
      printf("159: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e30L); volatile float vy = ((float)16777217.0f); int r1 = vx == vy; int r2 = ((long double)1e30L) == ((float)16777217.0f); static int r3 = ((long double)1e30L) == ((float)16777217.0f);
      printf("160: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile long double vy = ((long double)-2.5L); int r1 = vx >= vy; int r2 = ((float)-0.1f) >= ((long double)-2.5L); static int r3 = ((float)-0.1f) >= ((long double)-2.5L);
      printf("161: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile float vy = ((float)-0.1f); int r1 = vx >= vy; int r2 = ((long double)-2.5L) >= ((float)-0.1f); static int r3 = ((long double)-2.5L) >= ((float)-0.1f);
      printf("162: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-5f); volatile long double vy = ((long double)-2.5L); int r1 = vx >= vy; int r2 = ((float)1e-5f) >= ((long double)-2.5L); static int r3 = ((float)1e-5f) >= ((long double)-2.5L);
      printf("163: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile float vy = ((float)1e-5f); int r1 = vx >= vy; int r2 = ((long double)-2.5L) >= ((float)1e-5f); static int r3 = ((long double)-2.5L) >= ((float)1e-5f);
      printf("164: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-40f); volatile long double vy = ((long double)1e-40L); int r1 = vx >= vy; int r2 = ((float)1e-40f) >= ((long double)1e-40L); static int r3 = ((float)1e-40f) >= ((long double)1e-40L);
      printf("165: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile float vy = ((float)1e-40f); int r1 = vx >= vy; int r2 = ((long double)1e-40L) >= ((float)1e-40f); static int r3 = ((long double)1e-40L) >= ((float)1e-40f);
      printf("166: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile long double vy = ((long double)1.0L); int r1 = vx >= vy; int r2 = ((float)1e30f) >= ((long double)1.0L); static int r3 = ((float)1e30f) >= ((long double)1.0L);
      printf("167: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1.0L); volatile float vy = ((float)1e30f); int r1 = vx >= vy; int r2 = ((long double)1.0L) >= ((float)1e30f); static int r3 = ((long double)1.0L) >= ((float)1e30f);
      printf("168: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.1f); volatile int vy = ((int)3); float r1 = vx + vy; float r2 = ((float)0.1f) + ((int)3); static float r3 = ((float)0.1f) + ((int)3);
      printf("169: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)3); volatile float vy = ((float)0.1f); float r1 = vx + vy; float r2 = ((int)3) + ((float)0.1f); static float r3 = ((int)3) + ((float)0.1f);
      printf("170: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-2.5f); volatile int vy = ((int)100000); float r1 = vx + vy; float r2 = ((float)-2.5f) + ((int)100000); static float r3 = ((float)-2.5f) + ((int)100000);
      printf("171: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)100000); volatile float vy = ((float)-2.5f); float r1 = vx + vy; float r2 = ((int)100000) + ((float)-2.5f); static float r3 = ((int)100000) + ((float)-2.5f);
      printf("172: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile int vy = ((int)3); float r1 = vx + vy; float r2 = ((float)0.1f) + ((int)3); static float r3 = ((float)0.1f) + ((int)3);
      printf("173: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)3); volatile float vy = ((float)0.1f); float r1 = vx + vy; float r2 = ((int)3) + ((float)0.1f); static float r3 = ((int)3) + ((float)0.1f);
      printf("174: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)7.0f); volatile int vy = ((int)3); float r1 = vx + vy; float r2 = ((float)7.0f) + ((int)3); static float r3 = ((float)7.0f) + ((int)3);
      printf("175: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)3); volatile float vy = ((float)7.0f); float r1 = vx + vy; float r2 = ((int)3) + ((float)7.0f); static float r3 = ((int)3) + ((float)7.0f);
      printf("176: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile int vy = ((int)100000); float r1 = vx - vy; float r2 = ((float)-0.1f) - ((int)100000); static float r3 = ((float)-0.1f) - ((int)100000);
      printf("177: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)100000); volatile float vy = ((float)-0.1f); float r1 = vx - vy; float r2 = ((int)100000) - ((float)-0.1f); static float r3 = ((int)100000) - ((float)-0.1f);
      printf("178: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-2.5f); volatile int vy = ((int)3); float r1 = vx - vy; float r2 = ((float)-2.5f) - ((int)3); static float r3 = ((float)-2.5f) - ((int)3);
      printf("179: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)3); volatile float vy = ((float)-2.5f); float r1 = vx - vy; float r2 = ((int)3) - ((float)-2.5f); static float r3 = ((int)3) - ((float)-2.5f);
      printf("180: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile int vy = ((int)-7); float r1 = vx - vy; float r2 = ((float)-0.1f) - ((int)-7); static float r3 = ((float)-0.1f) - ((int)-7);
      printf("181: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)-7); volatile float vy = ((float)-0.1f); float r1 = vx - vy; float r2 = ((int)-7) - ((float)-0.1f); static float r3 = ((int)-7) - ((float)-0.1f);
      printf("182: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-2.5f); volatile int vy = ((int)100000); float r1 = vx - vy; float r2 = ((float)-2.5f) - ((int)100000); static float r3 = ((float)-2.5f) - ((int)100000);
      printf("183: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)100000); volatile float vy = ((float)-2.5f); float r1 = vx - vy; float r2 = ((int)100000) - ((float)-2.5f); static float r3 = ((int)100000) - ((float)-2.5f);
      printf("184: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-40f); volatile int vy = ((int)100000); float r1 = vx * vy; float r2 = ((float)1e-40f) * ((int)100000); static float r3 = ((float)1e-40f) * ((int)100000);
      printf("185: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)100000); volatile float vy = ((float)1e-40f); float r1 = vx * vy; float r2 = ((int)100000) * ((float)1e-40f); static float r3 = ((int)100000) * ((float)1e-40f);
      printf("186: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1.0f); volatile int vy = ((int)-7); float r1 = vx * vy; float r2 = ((float)1.0f) * ((int)-7); static float r3 = ((float)1.0f) * ((int)-7);
      printf("187: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)-7); volatile float vy = ((float)1.0f); float r1 = vx * vy; float r2 = ((int)-7) * ((float)1.0f); static float r3 = ((int)-7) * ((float)1.0f);
      printf("188: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile int vy = ((int)100000); float r1 = vx * vy; float r2 = ((float)-0.1f) * ((int)100000); static float r3 = ((float)-0.1f) * ((int)100000);
      printf("189: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)100000); volatile float vy = ((float)-0.1f); float r1 = vx * vy; float r2 = ((int)100000) * ((float)-0.1f); static float r3 = ((int)100000) * ((float)-0.1f);
      printf("190: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile int vy = ((int)-7); float r1 = vx * vy; float r2 = ((float)-0.1f) * ((int)-7); static float r3 = ((float)-0.1f) * ((int)-7);
      printf("191: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)-7); volatile float vy = ((float)-0.1f); float r1 = vx * vy; float r2 = ((int)-7) * ((float)-0.1f); static float r3 = ((int)-7) * ((float)-0.1f);
      printf("192: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile int vy = ((int)-7); float r1 = vx / vy; float r2 = ((float)0.1f) / ((int)-7); static float r3 = ((float)0.1f) / ((int)-7);
      printf("193: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)-7); volatile float vy = ((float)0.1f); float r1 = vx / vy; float r2 = ((int)-7) / ((float)0.1f); static float r3 = ((int)-7) / ((float)0.1f);
      printf("194: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile int vy = ((int)100000); float r1 = vx / vy; float r2 = ((float)0.1f) / ((int)100000); static float r3 = ((float)0.1f) / ((int)100000);
      printf("195: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)100000); volatile float vy = ((float)0.1f); float r1 = vx / vy; float r2 = ((int)100000) / ((float)0.1f); static float r3 = ((int)100000) / ((float)0.1f);
      printf("196: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile int vy = ((int)-7); float r1 = vx / vy; float r2 = ((float)0.3333333333333333f) / ((int)-7); static float r3 = ((float)0.3333333333333333f) / ((int)-7);
      printf("197: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)-7); volatile float vy = ((float)0.3333333333333333f); float r1 = vx / vy; float r2 = ((int)-7) / ((float)0.3333333333333333f); static float r3 = ((int)-7) / ((float)0.3333333333333333f);
      printf("198: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile int vy = ((int)100000); float r1 = vx / vy; float r2 = ((float)0.1f) / ((int)100000); static float r3 = ((float)0.1f) / ((int)100000);
      printf("199: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)100000); volatile float vy = ((float)0.1f); float r1 = vx / vy; float r2 = ((int)100000) / ((float)0.1f); static float r3 = ((int)100000) / ((float)0.1f);
      printf("200: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile int vy = ((int)3); int r1 = vx < vy; int r2 = ((float)-0.1f) < ((int)3); static int r3 = ((float)-0.1f) < ((int)3);
      printf("201: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)3); volatile float vy = ((float)-0.1f); int r1 = vx < vy; int r2 = ((int)3) < ((float)-0.1f); static int r3 = ((int)3) < ((float)-0.1f);
      printf("202: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile int vy = ((int)-7); int r1 = vx < vy; int r2 = ((float)-0.1f) < ((int)-7); static int r3 = ((float)-0.1f) < ((int)-7);
      printf("203: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile float vy = ((float)-0.1f); int r1 = vx < vy; int r2 = ((int)-7) < ((float)-0.1f); static int r3 = ((int)-7) < ((float)-0.1f);
      printf("204: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-40f); volatile int vy = ((int)-7); int r1 = vx < vy; int r2 = ((float)1e-40f) < ((int)-7); static int r3 = ((float)1e-40f) < ((int)-7);
      printf("205: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile float vy = ((float)1e-40f); int r1 = vx < vy; int r2 = ((int)-7) < ((float)1e-40f); static int r3 = ((int)-7) < ((float)1e-40f);
      printf("206: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-40f); volatile int vy = ((int)100000); int r1 = vx < vy; int r2 = ((float)1e-40f) < ((int)100000); static int r3 = ((float)1e-40f) < ((int)100000);
      printf("207: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)100000); volatile float vy = ((float)1e-40f); int r1 = vx < vy; int r2 = ((int)100000) < ((float)1e-40f); static int r3 = ((int)100000) < ((float)1e-40f);
      printf("208: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-5f); volatile int vy = ((int)-7); int r1 = vx == vy; int r2 = ((float)1e-5f) == ((int)-7); static int r3 = ((float)1e-5f) == ((int)-7);
      printf("209: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile float vy = ((float)1e-5f); int r1 = vx == vy; int r2 = ((int)-7) == ((float)1e-5f); static int r3 = ((int)-7) == ((float)1e-5f);
      printf("210: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)16777217.0f); volatile int vy = ((int)3); int r1 = vx == vy; int r2 = ((float)16777217.0f) == ((int)3); static int r3 = ((float)16777217.0f) == ((int)3);
      printf("211: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)3); volatile float vy = ((float)16777217.0f); int r1 = vx == vy; int r2 = ((int)3) == ((float)16777217.0f); static int r3 = ((int)3) == ((float)16777217.0f);
      printf("212: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)123456.789f); volatile int vy = ((int)100000); int r1 = vx == vy; int r2 = ((float)123456.789f) == ((int)100000); static int r3 = ((float)123456.789f) == ((int)100000);
      printf("213: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)100000); volatile float vy = ((float)123456.789f); int r1 = vx == vy; int r2 = ((int)100000) == ((float)123456.789f); static int r3 = ((int)100000) == ((float)123456.789f);
      printf("214: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e10f); volatile int vy = ((int)100000); int r1 = vx == vy; int r2 = ((float)1e10f) == ((int)100000); static int r3 = ((float)1e10f) == ((int)100000);
      printf("215: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)100000); volatile float vy = ((float)1e10f); int r1 = vx == vy; int r2 = ((int)100000) == ((float)1e10f); static int r3 = ((int)100000) == ((float)1e10f);
      printf("216: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-5f); volatile int vy = ((int)3); int r1 = vx >= vy; int r2 = ((float)1e-5f) >= ((int)3); static int r3 = ((float)1e-5f) >= ((int)3);
      printf("217: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)3); volatile float vy = ((float)1e-5f); int r1 = vx >= vy; int r2 = ((int)3) >= ((float)1e-5f); static int r3 = ((int)3) >= ((float)1e-5f);
      printf("218: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-40f); volatile int vy = ((int)3); int r1 = vx >= vy; int r2 = ((float)1e-40f) >= ((int)3); static int r3 = ((float)1e-40f) >= ((int)3);
      printf("219: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)3); volatile float vy = ((float)1e-40f); int r1 = vx >= vy; int r2 = ((int)3) >= ((float)1e-40f); static int r3 = ((int)3) >= ((float)1e-40f);
      printf("220: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)123456.789f); volatile int vy = ((int)100000); int r1 = vx >= vy; int r2 = ((float)123456.789f) >= ((int)100000); static int r3 = ((float)123456.789f) >= ((int)100000);
      printf("221: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)100000); volatile float vy = ((float)123456.789f); int r1 = vx >= vy; int r2 = ((int)100000) >= ((float)123456.789f); static int r3 = ((int)100000) >= ((float)123456.789f);
      printf("222: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-5f); volatile int vy = ((int)-7); int r1 = vx >= vy; int r2 = ((float)1e-5f) >= ((int)-7); static int r3 = ((float)1e-5f) >= ((int)-7);
      printf("223: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile float vy = ((float)1e-5f); int r1 = vx >= vy; int r2 = ((int)-7) >= ((float)1e-5f); static int r3 = ((int)-7) >= ((float)1e-5f);
      printf("224: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-2.5f); volatile unsigned vy = ((unsigned)4000000000u); float r1 = vx + vy; float r2 = ((float)-2.5f) + ((unsigned)4000000000u); static float r3 = ((float)-2.5f) + ((unsigned)4000000000u);
      printf("225: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)-2.5f); float r1 = vx + vy; float r2 = ((unsigned)4000000000u) + ((float)-2.5f); static float r3 = ((unsigned)4000000000u) + ((float)-2.5f);
      printf("226: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)7.0f); volatile unsigned vy = ((unsigned)3u); float r1 = vx + vy; float r2 = ((float)7.0f) + ((unsigned)3u); static float r3 = ((float)7.0f) + ((unsigned)3u);
      printf("227: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)7.0f); float r1 = vx + vy; float r2 = ((unsigned)3u) + ((float)7.0f); static float r3 = ((unsigned)3u) + ((float)7.0f);
      printf("228: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1.0f); volatile unsigned vy = ((unsigned)4000000000u); float r1 = vx + vy; float r2 = ((float)1.0f) + ((unsigned)4000000000u); static float r3 = ((float)1.0f) + ((unsigned)4000000000u);
      printf("229: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)1.0f); float r1 = vx + vy; float r2 = ((unsigned)4000000000u) + ((float)1.0f); static float r3 = ((unsigned)4000000000u) + ((float)1.0f);
      printf("230: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile unsigned vy = ((unsigned)4000000000u); float r1 = vx + vy; float r2 = ((float)1e-5f) + ((unsigned)4000000000u); static float r3 = ((float)1e-5f) + ((unsigned)4000000000u);
      printf("231: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)1e-5f); float r1 = vx + vy; float r2 = ((unsigned)4000000000u) + ((float)1e-5f); static float r3 = ((unsigned)4000000000u) + ((float)1e-5f);
      printf("232: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-2.5f); volatile unsigned vy = ((unsigned)4000000000u); float r1 = vx - vy; float r2 = ((float)-2.5f) - ((unsigned)4000000000u); static float r3 = ((float)-2.5f) - ((unsigned)4000000000u);
      printf("233: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)-2.5f); float r1 = vx - vy; float r2 = ((unsigned)4000000000u) - ((float)-2.5f); static float r3 = ((unsigned)4000000000u) - ((float)-2.5f);
      printf("234: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e10f); volatile unsigned vy = ((unsigned)3u); float r1 = vx - vy; float r2 = ((float)1e10f) - ((unsigned)3u); static float r3 = ((float)1e10f) - ((unsigned)3u);
      printf("235: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)1e10f); float r1 = vx - vy; float r2 = ((unsigned)3u) - ((float)1e10f); static float r3 = ((unsigned)3u) - ((float)1e10f);
      printf("236: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile unsigned vy = ((unsigned)3u); float r1 = vx - vy; float r2 = ((float)0.3333333333333333f) - ((unsigned)3u); static float r3 = ((float)0.3333333333333333f) - ((unsigned)3u);
      printf("237: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)0.3333333333333333f); float r1 = vx - vy; float r2 = ((unsigned)3u) - ((float)0.3333333333333333f); static float r3 = ((unsigned)3u) - ((float)0.3333333333333333f);
      printf("238: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile unsigned vy = ((unsigned)3u); float r1 = vx - vy; float r2 = ((float)0.3333333333333333f) - ((unsigned)3u); static float r3 = ((float)0.3333333333333333f) - ((unsigned)3u);
      printf("239: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)0.3333333333333333f); float r1 = vx - vy; float r2 = ((unsigned)3u) - ((float)0.3333333333333333f); static float r3 = ((unsigned)3u) - ((float)0.3333333333333333f);
      printf("240: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-2.5f); volatile unsigned vy = ((unsigned)3u); float r1 = vx * vy; float r2 = ((float)-2.5f) * ((unsigned)3u); static float r3 = ((float)-2.5f) * ((unsigned)3u);
      printf("241: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)-2.5f); float r1 = vx * vy; float r2 = ((unsigned)3u) * ((float)-2.5f); static float r3 = ((unsigned)3u) * ((float)-2.5f);
      printf("242: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1.0f); volatile unsigned vy = ((unsigned)4000000000u); float r1 = vx * vy; float r2 = ((float)1.0f) * ((unsigned)4000000000u); static float r3 = ((float)1.0f) * ((unsigned)4000000000u);
      printf("243: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)1.0f); float r1 = vx * vy; float r2 = ((unsigned)4000000000u) * ((float)1.0f); static float r3 = ((unsigned)4000000000u) * ((float)1.0f);
      printf("244: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile unsigned vy = ((unsigned)3u); float r1 = vx * vy; float r2 = ((float)0.1f) * ((unsigned)3u); static float r3 = ((float)0.1f) * ((unsigned)3u);
      printf("245: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)0.1f); float r1 = vx * vy; float r2 = ((unsigned)3u) * ((float)0.1f); static float r3 = ((unsigned)3u) * ((float)0.1f);
      printf("246: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1.0f); volatile unsigned vy = ((unsigned)3u); float r1 = vx * vy; float r2 = ((float)1.0f) * ((unsigned)3u); static float r3 = ((float)1.0f) * ((unsigned)3u);
      printf("247: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)1.0f); float r1 = vx * vy; float r2 = ((unsigned)3u) * ((float)1.0f); static float r3 = ((unsigned)3u) * ((float)1.0f);
      printf("248: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile unsigned vy = ((unsigned)4000000000u); float r1 = vx / vy; float r2 = ((float)1e-5f) / ((unsigned)4000000000u); static float r3 = ((float)1e-5f) / ((unsigned)4000000000u);
      printf("249: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)1e-5f); float r1 = vx / vy; float r2 = ((unsigned)4000000000u) / ((float)1e-5f); static float r3 = ((unsigned)4000000000u) / ((float)1e-5f);
      printf("250: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile unsigned vy = ((unsigned)4000000000u); float r1 = vx / vy; float r2 = ((float)0.3333333333333333f) / ((unsigned)4000000000u); static float r3 = ((float)0.3333333333333333f) / ((unsigned)4000000000u);
      printf("251: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)0.3333333333333333f); float r1 = vx / vy; float r2 = ((unsigned)4000000000u) / ((float)0.3333333333333333f); static float r3 = ((unsigned)4000000000u) / ((float)0.3333333333333333f);
      printf("252: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile unsigned vy = ((unsigned)4000000000u); float r1 = vx / vy; float r2 = ((float)16777217.0f) / ((unsigned)4000000000u); static float r3 = ((float)16777217.0f) / ((unsigned)4000000000u);
      printf("253: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)16777217.0f); float r1 = vx / vy; float r2 = ((unsigned)4000000000u) / ((float)16777217.0f); static float r3 = ((unsigned)4000000000u) / ((float)16777217.0f);
      printf("254: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-2.5f); volatile unsigned vy = ((unsigned)4000000000u); float r1 = vx / vy; float r2 = ((float)-2.5f) / ((unsigned)4000000000u); static float r3 = ((float)-2.5f) / ((unsigned)4000000000u);
      printf("255: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)-2.5f); float r1 = vx / vy; float r2 = ((unsigned)4000000000u) / ((float)-2.5f); static float r3 = ((unsigned)4000000000u) / ((float)-2.5f);
      printf("256: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile unsigned vy = ((unsigned)3u); int r1 = vx < vy; int r2 = ((float)-0.1f) < ((unsigned)3u); static int r3 = ((float)-0.1f) < ((unsigned)3u);
      printf("257: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)-0.1f); int r1 = vx < vy; int r2 = ((unsigned)3u) < ((float)-0.1f); static int r3 = ((unsigned)3u) < ((float)-0.1f);
      printf("258: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile unsigned vy = ((unsigned)3u); int r1 = vx < vy; int r2 = ((float)1e30f) < ((unsigned)3u); static int r3 = ((float)1e30f) < ((unsigned)3u);
      printf("259: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)1e30f); int r1 = vx < vy; int r2 = ((unsigned)3u) < ((float)1e30f); static int r3 = ((unsigned)3u) < ((float)1e30f);
      printf("260: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-40f); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx < vy; int r2 = ((float)1e-40f) < ((unsigned)4000000000u); static int r3 = ((float)1e-40f) < ((unsigned)4000000000u);
      printf("261: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)1e-40f); int r1 = vx < vy; int r2 = ((unsigned)4000000000u) < ((float)1e-40f); static int r3 = ((unsigned)4000000000u) < ((float)1e-40f);
      printf("262: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-2.5f); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx < vy; int r2 = ((float)-2.5f) < ((unsigned)4000000000u); static int r3 = ((float)-2.5f) < ((unsigned)4000000000u);
      printf("263: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)-2.5f); int r1 = vx < vy; int r2 = ((unsigned)4000000000u) < ((float)-2.5f); static int r3 = ((unsigned)4000000000u) < ((float)-2.5f);
      printf("264: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)123456.789f); volatile unsigned vy = ((unsigned)3u); int r1 = vx == vy; int r2 = ((float)123456.789f) == ((unsigned)3u); static int r3 = ((float)123456.789f) == ((unsigned)3u);
      printf("265: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)123456.789f); int r1 = vx == vy; int r2 = ((unsigned)3u) == ((float)123456.789f); static int r3 = ((unsigned)3u) == ((float)123456.789f);
      printf("266: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1.0f); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx == vy; int r2 = ((float)1.0f) == ((unsigned)4000000000u); static int r3 = ((float)1.0f) == ((unsigned)4000000000u);
      printf("267: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)1.0f); int r1 = vx == vy; int r2 = ((unsigned)4000000000u) == ((float)1.0f); static int r3 = ((unsigned)4000000000u) == ((float)1.0f);
      printf("268: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile unsigned vy = ((unsigned)3u); int r1 = vx == vy; int r2 = ((float)3.0f) == ((unsigned)3u); static int r3 = ((float)3.0f) == ((unsigned)3u);
      printf("269: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)3.0f); int r1 = vx == vy; int r2 = ((unsigned)3u) == ((float)3.0f); static int r3 = ((unsigned)3u) == ((float)3.0f);
      printf("270: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e10f); volatile unsigned vy = ((unsigned)3u); int r1 = vx == vy; int r2 = ((float)1e10f) == ((unsigned)3u); static int r3 = ((float)1e10f) == ((unsigned)3u);
      printf("271: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)1e10f); int r1 = vx == vy; int r2 = ((unsigned)3u) == ((float)1e10f); static int r3 = ((unsigned)3u) == ((float)1e10f);
      printf("272: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1.0f); volatile unsigned vy = ((unsigned)3u); int r1 = vx >= vy; int r2 = ((float)1.0f) >= ((unsigned)3u); static int r3 = ((float)1.0f) >= ((unsigned)3u);
      printf("273: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)1.0f); int r1 = vx >= vy; int r2 = ((unsigned)3u) >= ((float)1.0f); static int r3 = ((unsigned)3u) >= ((float)1.0f);
      printf("274: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx >= vy; int r2 = ((float)-0.1f) >= ((unsigned)4000000000u); static int r3 = ((float)-0.1f) >= ((unsigned)4000000000u);
      printf("275: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)-0.1f); int r1 = vx >= vy; int r2 = ((unsigned)4000000000u) >= ((float)-0.1f); static int r3 = ((unsigned)4000000000u) >= ((float)-0.1f);
      printf("276: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)7.0f); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx >= vy; int r2 = ((float)7.0f) >= ((unsigned)4000000000u); static int r3 = ((float)7.0f) >= ((unsigned)4000000000u);
      printf("277: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile float vy = ((float)7.0f); int r1 = vx >= vy; int r2 = ((unsigned)4000000000u) >= ((float)7.0f); static int r3 = ((unsigned)4000000000u) >= ((float)7.0f);
      printf("278: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)7.0f); volatile unsigned vy = ((unsigned)3u); int r1 = vx >= vy; int r2 = ((float)7.0f) >= ((unsigned)3u); static int r3 = ((float)7.0f) >= ((unsigned)3u);
      printf("279: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile float vy = ((float)7.0f); int r1 = vx >= vy; int r2 = ((unsigned)3u) >= ((float)7.0f); static int r3 = ((unsigned)3u) >= ((float)7.0f);
      printf("280: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-5f); volatile long long vy = ((long long)9007199254740993LL); float r1 = vx + vy; float r2 = ((float)1e-5f) + ((long long)9007199254740993LL); static float r3 = ((float)1e-5f) + ((long long)9007199254740993LL);
      printf("281: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)1e-5f); float r1 = vx + vy; float r2 = ((long long)9007199254740993LL) + ((float)1e-5f); static float r3 = ((long long)9007199254740993LL) + ((float)1e-5f);
      printf("282: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-40f); volatile long long vy = ((long long)-5LL); float r1 = vx + vy; float r2 = ((float)1e-40f) + ((long long)-5LL); static float r3 = ((float)1e-40f) + ((long long)-5LL);
      printf("283: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)1e-40f); float r1 = vx + vy; float r2 = ((long long)-5LL) + ((float)1e-40f); static float r3 = ((long long)-5LL) + ((float)1e-40f);
      printf("284: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile long long vy = ((long long)-5LL); float r1 = vx + vy; float r2 = ((float)1e-5f) + ((long long)-5LL); static float r3 = ((float)1e-5f) + ((long long)-5LL);
      printf("285: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)1e-5f); float r1 = vx + vy; float r2 = ((long long)-5LL) + ((float)1e-5f); static float r3 = ((long long)-5LL) + ((float)1e-5f);
      printf("286: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile long long vy = ((long long)9007199254740993LL); float r1 = vx + vy; float r2 = ((float)3.0f) + ((long long)9007199254740993LL); static float r3 = ((float)3.0f) + ((long long)9007199254740993LL);
      printf("287: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)3.0f); float r1 = vx + vy; float r2 = ((long long)9007199254740993LL) + ((float)3.0f); static float r3 = ((long long)9007199254740993LL) + ((float)3.0f);
      printf("288: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-2.5f); volatile long long vy = ((long long)-5LL); float r1 = vx - vy; float r2 = ((float)-2.5f) - ((long long)-5LL); static float r3 = ((float)-2.5f) - ((long long)-5LL);
      printf("289: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)-2.5f); float r1 = vx - vy; float r2 = ((long long)-5LL) - ((float)-2.5f); static float r3 = ((long long)-5LL) - ((float)-2.5f);
      printf("290: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)123456.789f); volatile long long vy = ((long long)-5LL); float r1 = vx - vy; float r2 = ((float)123456.789f) - ((long long)-5LL); static float r3 = ((float)123456.789f) - ((long long)-5LL);
      printf("291: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)123456.789f); float r1 = vx - vy; float r2 = ((long long)-5LL) - ((float)123456.789f); static float r3 = ((long long)-5LL) - ((float)123456.789f);
      printf("292: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile long long vy = ((long long)9007199254740993LL); float r1 = vx - vy; float r2 = ((float)0.3333333333333333f) - ((long long)9007199254740993LL); static float r3 = ((float)0.3333333333333333f) - ((long long)9007199254740993LL);
      printf("293: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)0.3333333333333333f); float r1 = vx - vy; float r2 = ((long long)9007199254740993LL) - ((float)0.3333333333333333f); static float r3 = ((long long)9007199254740993LL) - ((float)0.3333333333333333f);
      printf("294: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile long long vy = ((long long)9007199254740993LL); float r1 = vx - vy; float r2 = ((float)1e-5f) - ((long long)9007199254740993LL); static float r3 = ((float)1e-5f) - ((long long)9007199254740993LL);
      printf("295: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)1e-5f); float r1 = vx - vy; float r2 = ((long long)9007199254740993LL) - ((float)1e-5f); static float r3 = ((long long)9007199254740993LL) - ((float)1e-5f);
      printf("296: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)7.0f); volatile long long vy = ((long long)-5LL); float r1 = vx * vy; float r2 = ((float)7.0f) * ((long long)-5LL); static float r3 = ((float)7.0f) * ((long long)-5LL);
      printf("297: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)7.0f); float r1 = vx * vy; float r2 = ((long long)-5LL) * ((float)7.0f); static float r3 = ((long long)-5LL) * ((float)7.0f);
      printf("298: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile long long vy = ((long long)9007199254740993LL); float r1 = vx * vy; float r2 = ((float)16777217.0f) * ((long long)9007199254740993LL); static float r3 = ((float)16777217.0f) * ((long long)9007199254740993LL);
      printf("299: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)16777217.0f); float r1 = vx * vy; float r2 = ((long long)9007199254740993LL) * ((float)16777217.0f); static float r3 = ((long long)9007199254740993LL) * ((float)16777217.0f);
      printf("300: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-2.5f); volatile long long vy = ((long long)-5LL); float r1 = vx * vy; float r2 = ((float)-2.5f) * ((long long)-5LL); static float r3 = ((float)-2.5f) * ((long long)-5LL);
      printf("301: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)-2.5f); float r1 = vx * vy; float r2 = ((long long)-5LL) * ((float)-2.5f); static float r3 = ((long long)-5LL) * ((float)-2.5f);
      printf("302: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-40f); volatile long long vy = ((long long)-5LL); float r1 = vx * vy; float r2 = ((float)1e-40f) * ((long long)-5LL); static float r3 = ((float)1e-40f) * ((long long)-5LL);
      printf("303: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)1e-40f); float r1 = vx * vy; float r2 = ((long long)-5LL) * ((float)1e-40f); static float r3 = ((long long)-5LL) * ((float)1e-40f);
      printf("304: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile long long vy = ((long long)9007199254740993LL); float r1 = vx / vy; float r2 = ((float)3.0f) / ((long long)9007199254740993LL); static float r3 = ((float)3.0f) / ((long long)9007199254740993LL);
      printf("305: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)3.0f); float r1 = vx / vy; float r2 = ((long long)9007199254740993LL) / ((float)3.0f); static float r3 = ((long long)9007199254740993LL) / ((float)3.0f);
      printf("306: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e10f); volatile long long vy = ((long long)9007199254740993LL); float r1 = vx / vy; float r2 = ((float)1e10f) / ((long long)9007199254740993LL); static float r3 = ((float)1e10f) / ((long long)9007199254740993LL);
      printf("307: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)1e10f); float r1 = vx / vy; float r2 = ((long long)9007199254740993LL) / ((float)1e10f); static float r3 = ((long long)9007199254740993LL) / ((float)1e10f);
      printf("308: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-40f); volatile long long vy = ((long long)-5LL); float r1 = vx / vy; float r2 = ((float)1e-40f) / ((long long)-5LL); static float r3 = ((float)1e-40f) / ((long long)-5LL);
      printf("309: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)1e-40f); float r1 = vx / vy; float r2 = ((long long)-5LL) / ((float)1e-40f); static float r3 = ((long long)-5LL) / ((float)1e-40f);
      printf("310: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile long long vy = ((long long)-5LL); float r1 = vx / vy; float r2 = ((float)16777217.0f) / ((long long)-5LL); static float r3 = ((float)16777217.0f) / ((long long)-5LL);
      printf("311: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)16777217.0f); float r1 = vx / vy; float r2 = ((long long)-5LL) / ((float)16777217.0f); static float r3 = ((long long)-5LL) / ((float)16777217.0f);
      printf("312: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx < vy; int r2 = ((float)1e-5f) < ((long long)9007199254740993LL); static int r3 = ((float)1e-5f) < ((long long)9007199254740993LL);
      printf("313: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)1e-5f); int r1 = vx < vy; int r2 = ((long long)9007199254740993LL) < ((float)1e-5f); static int r3 = ((long long)9007199254740993LL) < ((float)1e-5f);
      printf("314: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx < vy; int r2 = ((float)0.3333333333333333f) < ((long long)9007199254740993LL); static int r3 = ((float)0.3333333333333333f) < ((long long)9007199254740993LL);
      printf("315: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)0.3333333333333333f); int r1 = vx < vy; int r2 = ((long long)9007199254740993LL) < ((float)0.3333333333333333f); static int r3 = ((long long)9007199254740993LL) < ((float)0.3333333333333333f);
      printf("316: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.1f); volatile long long vy = ((long long)-5LL); int r1 = vx < vy; int r2 = ((float)0.1f) < ((long long)-5LL); static int r3 = ((float)0.1f) < ((long long)-5LL);
      printf("317: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)0.1f); int r1 = vx < vy; int r2 = ((long long)-5LL) < ((float)0.1f); static int r3 = ((long long)-5LL) < ((float)0.1f);
      printf("318: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx < vy; int r2 = ((float)-0.1f) < ((long long)9007199254740993LL); static int r3 = ((float)-0.1f) < ((long long)9007199254740993LL);
      printf("319: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)-0.1f); int r1 = vx < vy; int r2 = ((long long)9007199254740993LL) < ((float)-0.1f); static int r3 = ((long long)9007199254740993LL) < ((float)-0.1f);
      printf("320: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.1f); volatile long long vy = ((long long)-5LL); int r1 = vx == vy; int r2 = ((float)0.1f) == ((long long)-5LL); static int r3 = ((float)0.1f) == ((long long)-5LL);
      printf("321: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)0.1f); int r1 = vx == vy; int r2 = ((long long)-5LL) == ((float)0.1f); static int r3 = ((long long)-5LL) == ((float)0.1f);
      printf("322: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-2.5f); volatile long long vy = ((long long)-5LL); int r1 = vx == vy; int r2 = ((float)-2.5f) == ((long long)-5LL); static int r3 = ((float)-2.5f) == ((long long)-5LL);
      printf("323: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)-2.5f); int r1 = vx == vy; int r2 = ((long long)-5LL) == ((float)-2.5f); static int r3 = ((long long)-5LL) == ((float)-2.5f);
      printf("324: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1.0f); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx == vy; int r2 = ((float)1.0f) == ((long long)9007199254740993LL); static int r3 = ((float)1.0f) == ((long long)9007199254740993LL);
      printf("325: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)1.0f); int r1 = vx == vy; int r2 = ((long long)9007199254740993LL) == ((float)1.0f); static int r3 = ((long long)9007199254740993LL) == ((float)1.0f);
      printf("326: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile long long vy = ((long long)-5LL); int r1 = vx == vy; int r2 = ((float)1e30f) == ((long long)-5LL); static int r3 = ((float)1e30f) == ((long long)-5LL);
      printf("327: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)1e30f); int r1 = vx == vy; int r2 = ((long long)-5LL) == ((float)1e30f); static int r3 = ((long long)-5LL) == ((float)1e30f);
      printf("328: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx >= vy; int r2 = ((float)3.0f) >= ((long long)9007199254740993LL); static int r3 = ((float)3.0f) >= ((long long)9007199254740993LL);
      printf("329: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)3.0f); int r1 = vx >= vy; int r2 = ((long long)9007199254740993LL) >= ((float)3.0f); static int r3 = ((long long)9007199254740993LL) >= ((float)3.0f);
      printf("330: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)16777217.0f); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx >= vy; int r2 = ((float)16777217.0f) >= ((long long)9007199254740993LL); static int r3 = ((float)16777217.0f) >= ((long long)9007199254740993LL);
      printf("331: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)16777217.0f); int r1 = vx >= vy; int r2 = ((long long)9007199254740993LL) >= ((float)16777217.0f); static int r3 = ((long long)9007199254740993LL) >= ((float)16777217.0f);
      printf("332: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)16777217.0f); volatile long long vy = ((long long)-5LL); int r1 = vx >= vy; int r2 = ((float)16777217.0f) >= ((long long)-5LL); static int r3 = ((float)16777217.0f) >= ((long long)-5LL);
      printf("333: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile float vy = ((float)16777217.0f); int r1 = vx >= vy; int r2 = ((long long)-5LL) >= ((float)16777217.0f); static int r3 = ((long long)-5LL) >= ((float)16777217.0f);
      printf("334: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)16777217.0f); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx >= vy; int r2 = ((float)16777217.0f) >= ((long long)9007199254740993LL); static int r3 = ((float)16777217.0f) >= ((long long)9007199254740993LL);
      printf("335: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile float vy = ((float)16777217.0f); int r1 = vx >= vy; int r2 = ((long long)9007199254740993LL) >= ((float)16777217.0f); static int r3 = ((long long)9007199254740993LL) >= ((float)16777217.0f);
      printf("336: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)7.0f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); float r1 = vx + vy; float r2 = ((float)7.0f) + ((unsigned long long)12345678901234567ULL); static float r3 = ((float)7.0f) + ((unsigned long long)12345678901234567ULL);
      printf("337: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)7.0f); float r1 = vx + vy; float r2 = ((unsigned long long)12345678901234567ULL) + ((float)7.0f); static float r3 = ((unsigned long long)12345678901234567ULL) + ((float)7.0f);
      printf("338: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); float r1 = vx + vy; float r2 = ((float)3.0f) + ((unsigned long long)12345678901234567ULL); static float r3 = ((float)3.0f) + ((unsigned long long)12345678901234567ULL);
      printf("339: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)3.0f); float r1 = vx + vy; float r2 = ((unsigned long long)12345678901234567ULL) + ((float)3.0f); static float r3 = ((unsigned long long)12345678901234567ULL) + ((float)3.0f);
      printf("340: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); float r1 = vx + vy; float r2 = ((float)16777217.0f) + ((unsigned long long)12345678901234567ULL); static float r3 = ((float)16777217.0f) + ((unsigned long long)12345678901234567ULL);
      printf("341: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)16777217.0f); float r1 = vx + vy; float r2 = ((unsigned long long)12345678901234567ULL) + ((float)16777217.0f); static float r3 = ((unsigned long long)12345678901234567ULL) + ((float)16777217.0f);
      printf("342: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); float r1 = vx + vy; float r2 = ((float)1e-5f) + ((unsigned long long)18446744073709551615ULL); static float r3 = ((float)1e-5f) + ((unsigned long long)18446744073709551615ULL);
      printf("343: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile float vy = ((float)1e-5f); float r1 = vx + vy; float r2 = ((unsigned long long)18446744073709551615ULL) + ((float)1e-5f); static float r3 = ((unsigned long long)18446744073709551615ULL) + ((float)1e-5f);
      printf("344: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); float r1 = vx - vy; float r2 = ((float)16777217.0f) - ((unsigned long long)18446744073709551615ULL); static float r3 = ((float)16777217.0f) - ((unsigned long long)18446744073709551615ULL);
      printf("345: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile float vy = ((float)16777217.0f); float r1 = vx - vy; float r2 = ((unsigned long long)18446744073709551615ULL) - ((float)16777217.0f); static float r3 = ((unsigned long long)18446744073709551615ULL) - ((float)16777217.0f);
      printf("346: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); float r1 = vx - vy; float r2 = ((float)0.1f) - ((unsigned long long)12345678901234567ULL); static float r3 = ((float)0.1f) - ((unsigned long long)12345678901234567ULL);
      printf("347: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)0.1f); float r1 = vx - vy; float r2 = ((unsigned long long)12345678901234567ULL) - ((float)0.1f); static float r3 = ((unsigned long long)12345678901234567ULL) - ((float)0.1f);
      printf("348: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); float r1 = vx - vy; float r2 = ((float)16777217.0f) - ((unsigned long long)18446744073709551615ULL); static float r3 = ((float)16777217.0f) - ((unsigned long long)18446744073709551615ULL);
      printf("349: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile float vy = ((float)16777217.0f); float r1 = vx - vy; float r2 = ((unsigned long long)18446744073709551615ULL) - ((float)16777217.0f); static float r3 = ((unsigned long long)18446744073709551615ULL) - ((float)16777217.0f);
      printf("350: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); float r1 = vx - vy; float r2 = ((float)0.3333333333333333f) - ((unsigned long long)18446744073709551615ULL); static float r3 = ((float)0.3333333333333333f) - ((unsigned long long)18446744073709551615ULL);
      printf("351: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile float vy = ((float)0.3333333333333333f); float r1 = vx - vy; float r2 = ((unsigned long long)18446744073709551615ULL) - ((float)0.3333333333333333f); static float r3 = ((unsigned long long)18446744073709551615ULL) - ((float)0.3333333333333333f);
      printf("352: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); float r1 = vx * vy; float r2 = ((float)-0.1f) * ((unsigned long long)18446744073709551615ULL); static float r3 = ((float)-0.1f) * ((unsigned long long)18446744073709551615ULL);
      printf("353: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile float vy = ((float)-0.1f); float r1 = vx * vy; float r2 = ((unsigned long long)18446744073709551615ULL) * ((float)-0.1f); static float r3 = ((unsigned long long)18446744073709551615ULL) * ((float)-0.1f);
      printf("354: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); float r1 = vx * vy; float r2 = ((float)16777217.0f) * ((unsigned long long)12345678901234567ULL); static float r3 = ((float)16777217.0f) * ((unsigned long long)12345678901234567ULL);
      printf("355: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)16777217.0f); float r1 = vx * vy; float r2 = ((unsigned long long)12345678901234567ULL) * ((float)16777217.0f); static float r3 = ((unsigned long long)12345678901234567ULL) * ((float)16777217.0f);
      printf("356: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)123456.789f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); float r1 = vx * vy; float r2 = ((float)123456.789f) * ((unsigned long long)12345678901234567ULL); static float r3 = ((float)123456.789f) * ((unsigned long long)12345678901234567ULL);
      printf("357: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)123456.789f); float r1 = vx * vy; float r2 = ((unsigned long long)12345678901234567ULL) * ((float)123456.789f); static float r3 = ((unsigned long long)12345678901234567ULL) * ((float)123456.789f);
      printf("358: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); float r1 = vx * vy; float r2 = ((float)3.0f) * ((unsigned long long)12345678901234567ULL); static float r3 = ((float)3.0f) * ((unsigned long long)12345678901234567ULL);
      printf("359: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)3.0f); float r1 = vx * vy; float r2 = ((unsigned long long)12345678901234567ULL) * ((float)3.0f); static float r3 = ((unsigned long long)12345678901234567ULL) * ((float)3.0f);
      printf("360: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1.0f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); float r1 = vx / vy; float r2 = ((float)1.0f) / ((unsigned long long)12345678901234567ULL); static float r3 = ((float)1.0f) / ((unsigned long long)12345678901234567ULL);
      printf("361: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)1.0f); float r1 = vx / vy; float r2 = ((unsigned long long)12345678901234567ULL) / ((float)1.0f); static float r3 = ((unsigned long long)12345678901234567ULL) / ((float)1.0f);
      printf("362: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); float r1 = vx / vy; float r2 = ((float)1e-5f) / ((unsigned long long)12345678901234567ULL); static float r3 = ((float)1e-5f) / ((unsigned long long)12345678901234567ULL);
      printf("363: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)1e-5f); float r1 = vx / vy; float r2 = ((unsigned long long)12345678901234567ULL) / ((float)1e-5f); static float r3 = ((unsigned long long)12345678901234567ULL) / ((float)1e-5f);
      printf("364: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); float r1 = vx / vy; float r2 = ((float)3.0f) / ((unsigned long long)12345678901234567ULL); static float r3 = ((float)3.0f) / ((unsigned long long)12345678901234567ULL);
      printf("365: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)3.0f); float r1 = vx / vy; float r2 = ((unsigned long long)12345678901234567ULL) / ((float)3.0f); static float r3 = ((unsigned long long)12345678901234567ULL) / ((float)3.0f);
      printf("366: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); float r1 = vx / vy; float r2 = ((float)0.1f) / ((unsigned long long)12345678901234567ULL); static float r3 = ((float)0.1f) / ((unsigned long long)12345678901234567ULL);
      printf("367: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)0.1f); float r1 = vx / vy; float r2 = ((unsigned long long)12345678901234567ULL) / ((float)0.1f); static float r3 = ((unsigned long long)12345678901234567ULL) / ((float)0.1f);
      printf("368: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-40f); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx < vy; int r2 = ((float)1e-40f) < ((unsigned long long)18446744073709551615ULL); static int r3 = ((float)1e-40f) < ((unsigned long long)18446744073709551615ULL);
      printf("369: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile float vy = ((float)1e-40f); int r1 = vx < vy; int r2 = ((unsigned long long)18446744073709551615ULL) < ((float)1e-40f); static int r3 = ((unsigned long long)18446744073709551615ULL) < ((float)1e-40f);
      printf("370: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx < vy; int r2 = ((float)3.0f) < ((unsigned long long)18446744073709551615ULL); static int r3 = ((float)3.0f) < ((unsigned long long)18446744073709551615ULL);
      printf("371: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile float vy = ((float)3.0f); int r1 = vx < vy; int r2 = ((unsigned long long)18446744073709551615ULL) < ((float)3.0f); static int r3 = ((unsigned long long)18446744073709551615ULL) < ((float)3.0f);
      printf("372: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.1f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx < vy; int r2 = ((float)0.1f) < ((unsigned long long)12345678901234567ULL); static int r3 = ((float)0.1f) < ((unsigned long long)12345678901234567ULL);
      printf("373: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)0.1f); int r1 = vx < vy; int r2 = ((unsigned long long)12345678901234567ULL) < ((float)0.1f); static int r3 = ((unsigned long long)12345678901234567ULL) < ((float)0.1f);
      printf("374: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-40f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx < vy; int r2 = ((float)1e-40f) < ((unsigned long long)12345678901234567ULL); static int r3 = ((float)1e-40f) < ((unsigned long long)12345678901234567ULL);
      printf("375: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)1e-40f); int r1 = vx < vy; int r2 = ((unsigned long long)12345678901234567ULL) < ((float)1e-40f); static int r3 = ((unsigned long long)12345678901234567ULL) < ((float)1e-40f);
      printf("376: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx == vy; int r2 = ((float)1e30f) == ((unsigned long long)12345678901234567ULL); static int r3 = ((float)1e30f) == ((unsigned long long)12345678901234567ULL);
      printf("377: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)1e30f); int r1 = vx == vy; int r2 = ((unsigned long long)12345678901234567ULL) == ((float)1e30f); static int r3 = ((unsigned long long)12345678901234567ULL) == ((float)1e30f);
      printf("378: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-5f); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx == vy; int r2 = ((float)1e-5f) == ((unsigned long long)18446744073709551615ULL); static int r3 = ((float)1e-5f) == ((unsigned long long)18446744073709551615ULL);
      printf("379: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile float vy = ((float)1e-5f); int r1 = vx == vy; int r2 = ((unsigned long long)18446744073709551615ULL) == ((float)1e-5f); static int r3 = ((unsigned long long)18446744073709551615ULL) == ((float)1e-5f);
      printf("380: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx == vy; int r2 = ((float)1e30f) == ((unsigned long long)12345678901234567ULL); static int r3 = ((float)1e30f) == ((unsigned long long)12345678901234567ULL);
      printf("381: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)1e30f); int r1 = vx == vy; int r2 = ((unsigned long long)12345678901234567ULL) == ((float)1e30f); static int r3 = ((unsigned long long)12345678901234567ULL) == ((float)1e30f);
      printf("382: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx == vy; int r2 = ((float)3.0f) == ((unsigned long long)12345678901234567ULL); static int r3 = ((float)3.0f) == ((unsigned long long)12345678901234567ULL);
      printf("383: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)3.0f); int r1 = vx == vy; int r2 = ((unsigned long long)12345678901234567ULL) == ((float)3.0f); static int r3 = ((unsigned long long)12345678901234567ULL) == ((float)3.0f);
      printf("384: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx >= vy; int r2 = ((float)0.3333333333333333f) >= ((unsigned long long)12345678901234567ULL); static int r3 = ((float)0.3333333333333333f) >= ((unsigned long long)12345678901234567ULL);
      printf("385: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile float vy = ((float)0.3333333333333333f); int r1 = vx >= vy; int r2 = ((unsigned long long)12345678901234567ULL) >= ((float)0.3333333333333333f); static int r3 = ((unsigned long long)12345678901234567ULL) >= ((float)0.3333333333333333f);
      printf("386: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx >= vy; int r2 = ((float)1e30f) >= ((unsigned long long)18446744073709551615ULL); static int r3 = ((float)1e30f) >= ((unsigned long long)18446744073709551615ULL);
      printf("387: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile float vy = ((float)1e30f); int r1 = vx >= vy; int r2 = ((unsigned long long)18446744073709551615ULL) >= ((float)1e30f); static int r3 = ((unsigned long long)18446744073709551615ULL) >= ((float)1e30f);
      printf("388: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx >= vy; int r2 = ((float)3.0f) >= ((unsigned long long)18446744073709551615ULL); static int r3 = ((float)3.0f) >= ((unsigned long long)18446744073709551615ULL);
      printf("389: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile float vy = ((float)3.0f); int r1 = vx >= vy; int r2 = ((unsigned long long)18446744073709551615ULL) >= ((float)3.0f); static int r3 = ((unsigned long long)18446744073709551615ULL) >= ((float)3.0f);
      printf("390: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx >= vy; int r2 = ((float)3.0f) >= ((unsigned long long)18446744073709551615ULL); static int r3 = ((float)3.0f) >= ((unsigned long long)18446744073709551615ULL);
      printf("391: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile float vy = ((float)3.0f); int r1 = vx >= vy; int r2 = ((unsigned long long)18446744073709551615ULL) >= ((float)3.0f); static int r3 = ((unsigned long long)18446744073709551615ULL) >= ((float)3.0f);
      printf("392: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-40f); volatile short vy = ((short)(short)-3); float r1 = vx + vy; float r2 = ((float)1e-40f) + ((short)(short)-3); static float r3 = ((float)1e-40f) + ((short)(short)-3);
      printf("393: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)1e-40f); float r1 = vx + vy; float r2 = ((short)(short)-3) + ((float)1e-40f); static float r3 = ((short)(short)-3) + ((float)1e-40f);
      printf("394: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-2.5f); volatile short vy = ((short)(short)-3); float r1 = vx + vy; float r2 = ((float)-2.5f) + ((short)(short)-3); static float r3 = ((float)-2.5f) + ((short)(short)-3);
      printf("395: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)-2.5f); float r1 = vx + vy; float r2 = ((short)(short)-3) + ((float)-2.5f); static float r3 = ((short)(short)-3) + ((float)-2.5f);
      printf("396: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile short vy = ((short)(short)-3); float r1 = vx + vy; float r2 = ((float)3.0f) + ((short)(short)-3); static float r3 = ((float)3.0f) + ((short)(short)-3);
      printf("397: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)3.0f); float r1 = vx + vy; float r2 = ((short)(short)-3) + ((float)3.0f); static float r3 = ((short)(short)-3) + ((float)3.0f);
      printf("398: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1.0f); volatile short vy = ((short)(short)-3); float r1 = vx + vy; float r2 = ((float)1.0f) + ((short)(short)-3); static float r3 = ((float)1.0f) + ((short)(short)-3);
      printf("399: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)1.0f); float r1 = vx + vy; float r2 = ((short)(short)-3) + ((float)1.0f); static float r3 = ((short)(short)-3) + ((float)1.0f);
      printf("400: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-40f); volatile short vy = ((short)(short)-3); float r1 = vx - vy; float r2 = ((float)1e-40f) - ((short)(short)-3); static float r3 = ((float)1e-40f) - ((short)(short)-3);
      printf("401: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)1e-40f); float r1 = vx - vy; float r2 = ((short)(short)-3) - ((float)1e-40f); static float r3 = ((short)(short)-3) - ((float)1e-40f);
      printf("402: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile short vy = ((short)(short)-3); float r1 = vx - vy; float r2 = ((float)0.3333333333333333f) - ((short)(short)-3); static float r3 = ((float)0.3333333333333333f) - ((short)(short)-3);
      printf("403: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)0.3333333333333333f); float r1 = vx - vy; float r2 = ((short)(short)-3) - ((float)0.3333333333333333f); static float r3 = ((short)(short)-3) - ((float)0.3333333333333333f);
      printf("404: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile short vy = ((short)(short)-3); float r1 = vx - vy; float r2 = ((float)0.3333333333333333f) - ((short)(short)-3); static float r3 = ((float)0.3333333333333333f) - ((short)(short)-3);
      printf("405: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)0.3333333333333333f); float r1 = vx - vy; float r2 = ((short)(short)-3) - ((float)0.3333333333333333f); static float r3 = ((short)(short)-3) - ((float)0.3333333333333333f);
      printf("406: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile short vy = ((short)(short)-3); float r1 = vx - vy; float r2 = ((float)0.3333333333333333f) - ((short)(short)-3); static float r3 = ((float)0.3333333333333333f) - ((short)(short)-3);
      printf("407: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)0.3333333333333333f); float r1 = vx - vy; float r2 = ((short)(short)-3) - ((float)0.3333333333333333f); static float r3 = ((short)(short)-3) - ((float)0.3333333333333333f);
      printf("408: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile short vy = ((short)(short)-3); float r1 = vx * vy; float r2 = ((float)3.0f) * ((short)(short)-3); static float r3 = ((float)3.0f) * ((short)(short)-3);
      printf("409: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)3.0f); float r1 = vx * vy; float r2 = ((short)(short)-3) * ((float)3.0f); static float r3 = ((short)(short)-3) * ((float)3.0f);
      printf("410: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile short vy = ((short)(short)-3); float r1 = vx * vy; float r2 = ((float)-0.1f) * ((short)(short)-3); static float r3 = ((float)-0.1f) * ((short)(short)-3);
      printf("411: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)-0.1f); float r1 = vx * vy; float r2 = ((short)(short)-3) * ((float)-0.1f); static float r3 = ((short)(short)-3) * ((float)-0.1f);
      printf("412: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)7.0f); volatile short vy = ((short)(short)-3); float r1 = vx * vy; float r2 = ((float)7.0f) * ((short)(short)-3); static float r3 = ((float)7.0f) * ((short)(short)-3);
      printf("413: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)7.0f); float r1 = vx * vy; float r2 = ((short)(short)-3) * ((float)7.0f); static float r3 = ((short)(short)-3) * ((float)7.0f);
      printf("414: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile short vy = ((short)(short)-3); float r1 = vx * vy; float r2 = ((float)-0.1f) * ((short)(short)-3); static float r3 = ((float)-0.1f) * ((short)(short)-3);
      printf("415: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)-0.1f); float r1 = vx * vy; float r2 = ((short)(short)-3) * ((float)-0.1f); static float r3 = ((short)(short)-3) * ((float)-0.1f);
      printf("416: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile short vy = ((short)(short)-3); float r1 = vx / vy; float r2 = ((float)0.1f) / ((short)(short)-3); static float r3 = ((float)0.1f) / ((short)(short)-3);
      printf("417: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)0.1f); float r1 = vx / vy; float r2 = ((short)(short)-3) / ((float)0.1f); static float r3 = ((short)(short)-3) / ((float)0.1f);
      printf("418: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile short vy = ((short)(short)-3); float r1 = vx / vy; float r2 = ((float)16777217.0f) / ((short)(short)-3); static float r3 = ((float)16777217.0f) / ((short)(short)-3);
      printf("419: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)16777217.0f); float r1 = vx / vy; float r2 = ((short)(short)-3) / ((float)16777217.0f); static float r3 = ((short)(short)-3) / ((float)16777217.0f);
      printf("420: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile short vy = ((short)(short)-3); float r1 = vx / vy; float r2 = ((float)3.0f) / ((short)(short)-3); static float r3 = ((float)3.0f) / ((short)(short)-3);
      printf("421: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)3.0f); float r1 = vx / vy; float r2 = ((short)(short)-3) / ((float)3.0f); static float r3 = ((short)(short)-3) / ((float)3.0f);
      printf("422: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile short vy = ((short)(short)-3); float r1 = vx / vy; float r2 = ((float)3.0f) / ((short)(short)-3); static float r3 = ((float)3.0f) / ((short)(short)-3);
      printf("423: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)3.0f); float r1 = vx / vy; float r2 = ((short)(short)-3) / ((float)3.0f); static float r3 = ((short)(short)-3) / ((float)3.0f);
      printf("424: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e10f); volatile short vy = ((short)(short)-3); int r1 = vx < vy; int r2 = ((float)1e10f) < ((short)(short)-3); static int r3 = ((float)1e10f) < ((short)(short)-3);
      printf("425: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)1e10f); int r1 = vx < vy; int r2 = ((short)(short)-3) < ((float)1e10f); static int r3 = ((short)(short)-3) < ((float)1e10f);
      printf("426: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.1f); volatile short vy = ((short)(short)-3); int r1 = vx < vy; int r2 = ((float)0.1f) < ((short)(short)-3); static int r3 = ((float)0.1f) < ((short)(short)-3);
      printf("427: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)0.1f); int r1 = vx < vy; int r2 = ((short)(short)-3) < ((float)0.1f); static int r3 = ((short)(short)-3) < ((float)0.1f);
      printf("428: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-5f); volatile short vy = ((short)(short)-3); int r1 = vx < vy; int r2 = ((float)1e-5f) < ((short)(short)-3); static int r3 = ((float)1e-5f) < ((short)(short)-3);
      printf("429: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)1e-5f); int r1 = vx < vy; int r2 = ((short)(short)-3) < ((float)1e-5f); static int r3 = ((short)(short)-3) < ((float)1e-5f);
      printf("430: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile short vy = ((short)(short)-3); int r1 = vx < vy; int r2 = ((float)-0.1f) < ((short)(short)-3); static int r3 = ((float)-0.1f) < ((short)(short)-3);
      printf("431: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)-0.1f); int r1 = vx < vy; int r2 = ((short)(short)-3) < ((float)-0.1f); static int r3 = ((short)(short)-3) < ((float)-0.1f);
      printf("432: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-5f); volatile short vy = ((short)(short)-3); int r1 = vx == vy; int r2 = ((float)1e-5f) == ((short)(short)-3); static int r3 = ((float)1e-5f) == ((short)(short)-3);
      printf("433: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)1e-5f); int r1 = vx == vy; int r2 = ((short)(short)-3) == ((float)1e-5f); static int r3 = ((short)(short)-3) == ((float)1e-5f);
      printf("434: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-2.5f); volatile short vy = ((short)(short)-3); int r1 = vx == vy; int r2 = ((float)-2.5f) == ((short)(short)-3); static int r3 = ((float)-2.5f) == ((short)(short)-3);
      printf("435: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)-2.5f); int r1 = vx == vy; int r2 = ((short)(short)-3) == ((float)-2.5f); static int r3 = ((short)(short)-3) == ((float)-2.5f);
      printf("436: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile short vy = ((short)(short)-3); int r1 = vx == vy; int r2 = ((float)1e30f) == ((short)(short)-3); static int r3 = ((float)1e30f) == ((short)(short)-3);
      printf("437: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)1e30f); int r1 = vx == vy; int r2 = ((short)(short)-3) == ((float)1e30f); static int r3 = ((short)(short)-3) == ((float)1e30f);
      printf("438: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)7.0f); volatile short vy = ((short)(short)-3); int r1 = vx == vy; int r2 = ((float)7.0f) == ((short)(short)-3); static int r3 = ((float)7.0f) == ((short)(short)-3);
      printf("439: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)7.0f); int r1 = vx == vy; int r2 = ((short)(short)-3) == ((float)7.0f); static int r3 = ((short)(short)-3) == ((float)7.0f);
      printf("440: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1.0f); volatile short vy = ((short)(short)-3); int r1 = vx >= vy; int r2 = ((float)1.0f) >= ((short)(short)-3); static int r3 = ((float)1.0f) >= ((short)(short)-3);
      printf("441: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)1.0f); int r1 = vx >= vy; int r2 = ((short)(short)-3) >= ((float)1.0f); static int r3 = ((short)(short)-3) >= ((float)1.0f);
      printf("442: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-2.5f); volatile short vy = ((short)(short)-3); int r1 = vx >= vy; int r2 = ((float)-2.5f) >= ((short)(short)-3); static int r3 = ((float)-2.5f) >= ((short)(short)-3);
      printf("443: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)-2.5f); int r1 = vx >= vy; int r2 = ((short)(short)-3) >= ((float)-2.5f); static int r3 = ((short)(short)-3) >= ((float)-2.5f);
      printf("444: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile short vy = ((short)(short)-3); int r1 = vx >= vy; int r2 = ((float)3.0f) >= ((short)(short)-3); static int r3 = ((float)3.0f) >= ((short)(short)-3);
      printf("445: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)3.0f); int r1 = vx >= vy; int r2 = ((short)(short)-3) >= ((float)3.0f); static int r3 = ((short)(short)-3) >= ((float)3.0f);
      printf("446: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)16777217.0f); volatile short vy = ((short)(short)-3); int r1 = vx >= vy; int r2 = ((float)16777217.0f) >= ((short)(short)-3); static int r3 = ((float)16777217.0f) >= ((short)(short)-3);
      printf("447: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile float vy = ((float)16777217.0f); int r1 = vx >= vy; int r2 = ((short)(short)-3) >= ((float)16777217.0f); static int r3 = ((short)(short)-3) >= ((float)16777217.0f);
      printf("448: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx + vy; float r2 = ((float)-0.1f) + ((unsigned char)(unsigned char)200); static float r3 = ((float)-0.1f) + ((unsigned char)(unsigned char)200);
      printf("449: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)-0.1f); float r1 = vx + vy; float r2 = ((unsigned char)(unsigned char)200) + ((float)-0.1f); static float r3 = ((unsigned char)(unsigned char)200) + ((float)-0.1f);
      printf("450: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e30f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx + vy; float r2 = ((float)1e30f) + ((unsigned char)(unsigned char)200); static float r3 = ((float)1e30f) + ((unsigned char)(unsigned char)200);
      printf("451: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)1e30f); float r1 = vx + vy; float r2 = ((unsigned char)(unsigned char)200) + ((float)1e30f); static float r3 = ((unsigned char)(unsigned char)200) + ((float)1e30f);
      printf("452: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e30f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx + vy; float r2 = ((float)1e30f) + ((unsigned char)(unsigned char)200); static float r3 = ((float)1e30f) + ((unsigned char)(unsigned char)200);
      printf("453: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)1e30f); float r1 = vx + vy; float r2 = ((unsigned char)(unsigned char)200) + ((float)1e30f); static float r3 = ((unsigned char)(unsigned char)200) + ((float)1e30f);
      printf("454: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)123456.789f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx + vy; float r2 = ((float)123456.789f) + ((unsigned char)(unsigned char)200); static float r3 = ((float)123456.789f) + ((unsigned char)(unsigned char)200);
      printf("455: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)123456.789f); float r1 = vx + vy; float r2 = ((unsigned char)(unsigned char)200) + ((float)123456.789f); static float r3 = ((unsigned char)(unsigned char)200) + ((float)123456.789f);
      printf("456: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-0.1f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx - vy; float r2 = ((float)-0.1f) - ((unsigned char)(unsigned char)200); static float r3 = ((float)-0.1f) - ((unsigned char)(unsigned char)200);
      printf("457: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)-0.1f); float r1 = vx - vy; float r2 = ((unsigned char)(unsigned char)200) - ((float)-0.1f); static float r3 = ((unsigned char)(unsigned char)200) - ((float)-0.1f);
      printf("458: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1.0f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx - vy; float r2 = ((float)1.0f) - ((unsigned char)(unsigned char)200); static float r3 = ((float)1.0f) - ((unsigned char)(unsigned char)200);
      printf("459: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)1.0f); float r1 = vx - vy; float r2 = ((unsigned char)(unsigned char)200) - ((float)1.0f); static float r3 = ((unsigned char)(unsigned char)200) - ((float)1.0f);
      printf("460: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-40f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx - vy; float r2 = ((float)1e-40f) - ((unsigned char)(unsigned char)200); static float r3 = ((float)1e-40f) - ((unsigned char)(unsigned char)200);
      printf("461: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)1e-40f); float r1 = vx - vy; float r2 = ((unsigned char)(unsigned char)200) - ((float)1e-40f); static float r3 = ((unsigned char)(unsigned char)200) - ((float)1e-40f);
      printf("462: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx - vy; float r2 = ((float)0.3333333333333333f) - ((unsigned char)(unsigned char)200); static float r3 = ((float)0.3333333333333333f) - ((unsigned char)(unsigned char)200);
      printf("463: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)0.3333333333333333f); float r1 = vx - vy; float r2 = ((unsigned char)(unsigned char)200) - ((float)0.3333333333333333f); static float r3 = ((unsigned char)(unsigned char)200) - ((float)0.3333333333333333f);
      printf("464: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx * vy; float r2 = ((float)3.0f) * ((unsigned char)(unsigned char)200); static float r3 = ((float)3.0f) * ((unsigned char)(unsigned char)200);
      printf("465: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)3.0f); float r1 = vx * vy; float r2 = ((unsigned char)(unsigned char)200) * ((float)3.0f); static float r3 = ((unsigned char)(unsigned char)200) * ((float)3.0f);
      printf("466: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)123456.789f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx * vy; float r2 = ((float)123456.789f) * ((unsigned char)(unsigned char)200); static float r3 = ((float)123456.789f) * ((unsigned char)(unsigned char)200);
      printf("467: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)123456.789f); float r1 = vx * vy; float r2 = ((unsigned char)(unsigned char)200) * ((float)123456.789f); static float r3 = ((unsigned char)(unsigned char)200) * ((float)123456.789f);
      printf("468: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx * vy; float r2 = ((float)1e-5f) * ((unsigned char)(unsigned char)200); static float r3 = ((float)1e-5f) * ((unsigned char)(unsigned char)200);
      printf("469: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)1e-5f); float r1 = vx * vy; float r2 = ((unsigned char)(unsigned char)200) * ((float)1e-5f); static float r3 = ((unsigned char)(unsigned char)200) * ((float)1e-5f);
      printf("470: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)-2.5f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx * vy; float r2 = ((float)-2.5f) * ((unsigned char)(unsigned char)200); static float r3 = ((float)-2.5f) * ((unsigned char)(unsigned char)200);
      printf("471: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)-2.5f); float r1 = vx * vy; float r2 = ((unsigned char)(unsigned char)200) * ((float)-2.5f); static float r3 = ((unsigned char)(unsigned char)200) * ((float)-2.5f);
      printf("472: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e10f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx / vy; float r2 = ((float)1e10f) / ((unsigned char)(unsigned char)200); static float r3 = ((float)1e10f) / ((unsigned char)(unsigned char)200);
      printf("473: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)1e10f); float r1 = vx / vy; float r2 = ((unsigned char)(unsigned char)200) / ((float)1e10f); static float r3 = ((unsigned char)(unsigned char)200) / ((float)1e10f);
      printf("474: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx / vy; float r2 = ((float)3.0f) / ((unsigned char)(unsigned char)200); static float r3 = ((float)3.0f) / ((unsigned char)(unsigned char)200);
      printf("475: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)3.0f); float r1 = vx / vy; float r2 = ((unsigned char)(unsigned char)200) / ((float)3.0f); static float r3 = ((unsigned char)(unsigned char)200) / ((float)3.0f);
      printf("476: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1.0f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx / vy; float r2 = ((float)1.0f) / ((unsigned char)(unsigned char)200); static float r3 = ((float)1.0f) / ((unsigned char)(unsigned char)200);
      printf("477: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)1.0f); float r1 = vx / vy; float r2 = ((unsigned char)(unsigned char)200) / ((float)1.0f); static float r3 = ((unsigned char)(unsigned char)200) / ((float)1.0f);
      printf("478: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); float r1 = vx / vy; float r2 = ((float)16777217.0f) / ((unsigned char)(unsigned char)200); static float r3 = ((float)16777217.0f) / ((unsigned char)(unsigned char)200);
      printf("479: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)16777217.0f); float r1 = vx / vy; float r2 = ((unsigned char)(unsigned char)200) / ((float)16777217.0f); static float r3 = ((unsigned char)(unsigned char)200) / ((float)16777217.0f);
      printf("480: %.9g %.9g %.9g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)3.0f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx < vy; int r2 = ((float)3.0f) < ((unsigned char)(unsigned char)200); static int r3 = ((float)3.0f) < ((unsigned char)(unsigned char)200);
      printf("481: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)3.0f); int r1 = vx < vy; int r2 = ((unsigned char)(unsigned char)200) < ((float)3.0f); static int r3 = ((unsigned char)(unsigned char)200) < ((float)3.0f);
      printf("482: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx < vy; int r2 = ((float)0.3333333333333333f) < ((unsigned char)(unsigned char)200); static int r3 = ((float)0.3333333333333333f) < ((unsigned char)(unsigned char)200);
      printf("483: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)0.3333333333333333f); int r1 = vx < vy; int r2 = ((unsigned char)(unsigned char)200) < ((float)0.3333333333333333f); static int r3 = ((unsigned char)(unsigned char)200) < ((float)0.3333333333333333f);
      printf("484: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx < vy; int r2 = ((float)1e30f) < ((unsigned char)(unsigned char)200); static int r3 = ((float)1e30f) < ((unsigned char)(unsigned char)200);
      printf("485: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)1e30f); int r1 = vx < vy; int r2 = ((unsigned char)(unsigned char)200) < ((float)1e30f); static int r3 = ((unsigned char)(unsigned char)200) < ((float)1e30f);
      printf("486: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx < vy; int r2 = ((float)3.0f) < ((unsigned char)(unsigned char)200); static int r3 = ((float)3.0f) < ((unsigned char)(unsigned char)200);
      printf("487: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)3.0f); int r1 = vx < vy; int r2 = ((unsigned char)(unsigned char)200) < ((float)3.0f); static int r3 = ((unsigned char)(unsigned char)200) < ((float)3.0f);
      printf("488: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx == vy; int r2 = ((float)0.3333333333333333f) == ((unsigned char)(unsigned char)200); static int r3 = ((float)0.3333333333333333f) == ((unsigned char)(unsigned char)200);
      printf("489: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)0.3333333333333333f); int r1 = vx == vy; int r2 = ((unsigned char)(unsigned char)200) == ((float)0.3333333333333333f); static int r3 = ((unsigned char)(unsigned char)200) == ((float)0.3333333333333333f);
      printf("490: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx == vy; int r2 = ((float)3.0f) == ((unsigned char)(unsigned char)200); static int r3 = ((float)3.0f) == ((unsigned char)(unsigned char)200);
      printf("491: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)3.0f); int r1 = vx == vy; int r2 = ((unsigned char)(unsigned char)200) == ((float)3.0f); static int r3 = ((unsigned char)(unsigned char)200) == ((float)3.0f);
      printf("492: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx == vy; int r2 = ((float)-0.1f) == ((unsigned char)(unsigned char)200); static int r3 = ((float)-0.1f) == ((unsigned char)(unsigned char)200);
      printf("493: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)-0.1f); int r1 = vx == vy; int r2 = ((unsigned char)(unsigned char)200) == ((float)-0.1f); static int r3 = ((unsigned char)(unsigned char)200) == ((float)-0.1f);
      printf("494: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e10f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx == vy; int r2 = ((float)1e10f) == ((unsigned char)(unsigned char)200); static int r3 = ((float)1e10f) == ((unsigned char)(unsigned char)200);
      printf("495: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)1e10f); int r1 = vx == vy; int r2 = ((unsigned char)(unsigned char)200) == ((float)1e10f); static int r3 = ((unsigned char)(unsigned char)200) == ((float)1e10f);
      printf("496: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)16777217.0f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx >= vy; int r2 = ((float)16777217.0f) >= ((unsigned char)(unsigned char)200); static int r3 = ((float)16777217.0f) >= ((unsigned char)(unsigned char)200);
      printf("497: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)16777217.0f); int r1 = vx >= vy; int r2 = ((unsigned char)(unsigned char)200) >= ((float)16777217.0f); static int r3 = ((unsigned char)(unsigned char)200) >= ((float)16777217.0f);
      printf("498: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx >= vy; int r2 = ((float)1e30f) >= ((unsigned char)(unsigned char)200); static int r3 = ((float)1e30f) >= ((unsigned char)(unsigned char)200);
      printf("499: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)1e30f); int r1 = vx >= vy; int r2 = ((unsigned char)(unsigned char)200) >= ((float)1e30f); static int r3 = ((unsigned char)(unsigned char)200) >= ((float)1e30f);
      printf("500: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-5f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx >= vy; int r2 = ((float)1e-5f) >= ((unsigned char)(unsigned char)200); static int r3 = ((float)1e-5f) >= ((unsigned char)(unsigned char)200);
      printf("501: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)1e-5f); int r1 = vx >= vy; int r2 = ((unsigned char)(unsigned char)200) >= ((float)1e-5f); static int r3 = ((unsigned char)(unsigned char)200) >= ((float)1e-5f);
      printf("502: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx >= vy; int r2 = ((float)1e30f) >= ((unsigned char)(unsigned char)200); static int r3 = ((float)1e30f) >= ((unsigned char)(unsigned char)200);
      printf("503: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile float vy = ((float)1e30f); int r1 = vx >= vy; int r2 = ((unsigned char)(unsigned char)200) >= ((float)1e30f); static int r3 = ((unsigned char)(unsigned char)200) >= ((float)1e30f);
      printf("504: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1.0); volatile float vy = ((float)0.3333333333333333f); double r1 = vx + vy; double r2 = ((double)1.0) + ((float)0.3333333333333333f); static double r3 = ((double)1.0) + ((float)0.3333333333333333f);
      printf("505: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile double vy = ((double)1.0); double r1 = vx + vy; double r2 = ((float)0.3333333333333333f) + ((double)1.0); static double r3 = ((float)0.3333333333333333f) + ((double)1.0);
      printf("506: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile float vy = ((float)1e-5f); double r1 = vx + vy; double r2 = ((double)1.0) + ((float)1e-5f); static double r3 = ((double)1.0) + ((float)1e-5f);
      printf("507: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile double vy = ((double)1.0); double r1 = vx + vy; double r2 = ((float)1e-5f) + ((double)1.0); static double r3 = ((float)1e-5f) + ((double)1.0);
      printf("508: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e30); volatile float vy = ((float)1e30f); double r1 = vx + vy; double r2 = ((double)1e30) + ((float)1e30f); static double r3 = ((double)1e30) + ((float)1e30f);
      printf("509: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e30f); volatile double vy = ((double)1e30); double r1 = vx + vy; double r2 = ((float)1e30f) + ((double)1e30); static double r3 = ((float)1e30f) + ((double)1e30);
      printf("510: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile float vy = ((float)1e-5f); double r1 = vx + vy; double r2 = ((double)0.3333333333333333) + ((float)1e-5f); static double r3 = ((double)0.3333333333333333) + ((float)1e-5f);
      printf("511: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-5f); volatile double vy = ((double)0.3333333333333333); double r1 = vx + vy; double r2 = ((float)1e-5f) + ((double)0.3333333333333333); static double r3 = ((float)1e-5f) + ((double)0.3333333333333333);
      printf("512: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile float vy = ((float)16777217.0f); double r1 = vx - vy; double r2 = ((double)1e10) - ((float)16777217.0f); static double r3 = ((double)1e10) - ((float)16777217.0f);
      printf("513: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile double vy = ((double)1e10); double r1 = vx - vy; double r2 = ((float)16777217.0f) - ((double)1e10); static double r3 = ((float)16777217.0f) - ((double)1e10);
      printf("514: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e30); volatile float vy = ((float)1e10f); double r1 = vx - vy; double r2 = ((double)1e30) - ((float)1e10f); static double r3 = ((double)1e30) - ((float)1e10f);
      printf("515: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e10f); volatile double vy = ((double)1e30); double r1 = vx - vy; double r2 = ((float)1e10f) - ((double)1e30); static double r3 = ((float)1e10f) - ((double)1e30);
      printf("516: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)16777217.0); volatile float vy = ((float)1e10f); double r1 = vx - vy; double r2 = ((double)16777217.0) - ((float)1e10f); static double r3 = ((double)16777217.0) - ((float)1e10f);
      printf("517: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e10f); volatile double vy = ((double)16777217.0); double r1 = vx - vy; double r2 = ((float)1e10f) - ((double)16777217.0); static double r3 = ((float)1e10f) - ((double)16777217.0);
      printf("518: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)7.0); volatile float vy = ((float)1e10f); double r1 = vx - vy; double r2 = ((double)7.0) - ((float)1e10f); static double r3 = ((double)7.0) - ((float)1e10f);
      printf("519: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e10f); volatile double vy = ((double)7.0); double r1 = vx - vy; double r2 = ((float)1e10f) - ((double)7.0); static double r3 = ((float)1e10f) - ((double)7.0);
      printf("520: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile float vy = ((float)7.0f); double r1 = vx * vy; double r2 = ((double)1e10) * ((float)7.0f); static double r3 = ((double)1e10) * ((float)7.0f);
      printf("521: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)7.0f); volatile double vy = ((double)1e10); double r1 = vx * vy; double r2 = ((float)7.0f) * ((double)1e10); static double r3 = ((float)7.0f) * ((double)1e10);
      printf("522: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile float vy = ((float)1e10f); double r1 = vx * vy; double r2 = ((double)1e-40) * ((float)1e10f); static double r3 = ((double)1e-40) * ((float)1e10f);
      printf("523: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e10f); volatile double vy = ((double)1e-40); double r1 = vx * vy; double r2 = ((float)1e10f) * ((double)1e-40); static double r3 = ((float)1e10f) * ((double)1e-40);
      printf("524: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.1); volatile float vy = ((float)123456.789f); double r1 = vx * vy; double r2 = ((double)0.1) * ((float)123456.789f); static double r3 = ((double)0.1) * ((float)123456.789f);
      printf("525: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)123456.789f); volatile double vy = ((double)0.1); double r1 = vx * vy; double r2 = ((float)123456.789f) * ((double)0.1); static double r3 = ((float)123456.789f) * ((double)0.1);
      printf("526: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e30); volatile float vy = ((float)0.1f); double r1 = vx * vy; double r2 = ((double)1e30) * ((float)0.1f); static double r3 = ((double)1e30) * ((float)0.1f);
      printf("527: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.1f); volatile double vy = ((double)1e30); double r1 = vx * vy; double r2 = ((float)0.1f) * ((double)1e30); static double r3 = ((float)0.1f) * ((double)1e30);
      printf("528: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile float vy = ((float)0.3333333333333333f); double r1 = vx / vy; double r2 = ((double)1.0) / ((float)0.3333333333333333f); static double r3 = ((double)1.0) / ((float)0.3333333333333333f);
      printf("529: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile double vy = ((double)1.0); double r1 = vx / vy; double r2 = ((float)0.3333333333333333f) / ((double)1.0); static double r3 = ((float)0.3333333333333333f) / ((double)1.0);
      printf("530: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)7.0); volatile float vy = ((float)0.3333333333333333f); double r1 = vx / vy; double r2 = ((double)7.0) / ((float)0.3333333333333333f); static double r3 = ((double)7.0) / ((float)0.3333333333333333f);
      printf("531: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile double vy = ((double)7.0); double r1 = vx / vy; double r2 = ((float)0.3333333333333333f) / ((double)7.0); static double r3 = ((float)0.3333333333333333f) / ((double)7.0);
      printf("532: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-2.5); volatile float vy = ((float)1e-40f); double r1 = vx / vy; double r2 = ((double)-2.5) / ((float)1e-40f); static double r3 = ((double)-2.5) / ((float)1e-40f);
      printf("533: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e-40f); volatile double vy = ((double)-2.5); double r1 = vx / vy; double r2 = ((float)1e-40f) / ((double)-2.5); static double r3 = ((float)1e-40f) / ((double)-2.5);
      printf("534: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.1); volatile float vy = ((float)1e30f); double r1 = vx / vy; double r2 = ((double)0.1) / ((float)1e30f); static double r3 = ((double)0.1) / ((float)1e30f);
      printf("535: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile float vx = ((float)1e30f); volatile double vy = ((double)0.1); double r1 = vx / vy; double r2 = ((float)1e30f) / ((double)0.1); static double r3 = ((float)1e30f) / ((double)0.1);
      printf("536: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-2.5); volatile float vy = ((float)-0.1f); int r1 = vx < vy; int r2 = ((double)-2.5) < ((float)-0.1f); static int r3 = ((double)-2.5) < ((float)-0.1f);
      printf("537: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile double vy = ((double)-2.5); int r1 = vx < vy; int r2 = ((float)-0.1f) < ((double)-2.5); static int r3 = ((float)-0.1f) < ((double)-2.5);
      printf("538: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)3.0); volatile float vy = ((float)0.3333333333333333f); int r1 = vx < vy; int r2 = ((double)3.0) < ((float)0.3333333333333333f); static int r3 = ((double)3.0) < ((float)0.3333333333333333f);
      printf("539: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile double vy = ((double)3.0); int r1 = vx < vy; int r2 = ((float)0.3333333333333333f) < ((double)3.0); static int r3 = ((float)0.3333333333333333f) < ((double)3.0);
      printf("540: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile float vy = ((float)1e10f); int r1 = vx < vy; int r2 = ((double)1e-5) < ((float)1e10f); static int r3 = ((double)1e-5) < ((float)1e10f);
      printf("541: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e10f); volatile double vy = ((double)1e-5); int r1 = vx < vy; int r2 = ((float)1e10f) < ((double)1e-5); static int r3 = ((float)1e10f) < ((double)1e-5);
      printf("542: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)3.0); volatile float vy = ((float)0.1f); int r1 = vx < vy; int r2 = ((double)3.0) < ((float)0.1f); static int r3 = ((double)3.0) < ((float)0.1f);
      printf("543: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.1f); volatile double vy = ((double)3.0); int r1 = vx < vy; int r2 = ((float)0.1f) < ((double)3.0); static int r3 = ((float)0.1f) < ((double)3.0);
      printf("544: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-0.1); volatile float vy = ((float)123456.789f); int r1 = vx == vy; int r2 = ((double)-0.1) == ((float)123456.789f); static int r3 = ((double)-0.1) == ((float)123456.789f);
      printf("545: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)123456.789f); volatile double vy = ((double)-0.1); int r1 = vx == vy; int r2 = ((float)123456.789f) == ((double)-0.1); static int r3 = ((float)123456.789f) == ((double)-0.1);
      printf("546: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile float vy = ((float)1.0f); int r1 = vx == vy; int r2 = ((double)1e-5) == ((float)1.0f); static int r3 = ((double)1e-5) == ((float)1.0f);
      printf("547: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1.0f); volatile double vy = ((double)1e-5); int r1 = vx == vy; int r2 = ((float)1.0f) == ((double)1e-5); static int r3 = ((float)1.0f) == ((double)1e-5);
      printf("548: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e30); volatile float vy = ((float)1e-40f); int r1 = vx == vy; int r2 = ((double)1e30) == ((float)1e-40f); static int r3 = ((double)1e30) == ((float)1e-40f);
      printf("549: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-40f); volatile double vy = ((double)1e30); int r1 = vx == vy; int r2 = ((float)1e-40f) == ((double)1e30); static int r3 = ((float)1e-40f) == ((double)1e30);
      printf("550: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.1); volatile float vy = ((float)1e30f); int r1 = vx == vy; int r2 = ((double)0.1) == ((float)1e30f); static int r3 = ((double)0.1) == ((float)1e30f);
      printf("551: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e30f); volatile double vy = ((double)0.1); int r1 = vx == vy; int r2 = ((float)1e30f) == ((double)0.1); static int r3 = ((float)1e30f) == ((double)0.1);
      printf("552: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile float vy = ((float)-0.1f); int r1 = vx >= vy; int r2 = ((double)1e-5) >= ((float)-0.1f); static int r3 = ((double)1e-5) >= ((float)-0.1f);
      printf("553: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile double vy = ((double)1e-5); int r1 = vx >= vy; int r2 = ((float)-0.1f) >= ((double)1e-5); static int r3 = ((float)-0.1f) >= ((double)1e-5);
      printf("554: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.1); volatile float vy = ((float)-2.5f); int r1 = vx >= vy; int r2 = ((double)0.1) >= ((float)-2.5f); static int r3 = ((double)0.1) >= ((float)-2.5f);
      printf("555: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-2.5f); volatile double vy = ((double)0.1); int r1 = vx >= vy; int r2 = ((float)-2.5f) >= ((double)0.1); static int r3 = ((float)-2.5f) >= ((double)0.1);
      printf("556: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1.0); volatile float vy = ((float)-0.1f); int r1 = vx >= vy; int r2 = ((double)1.0) >= ((float)-0.1f); static int r3 = ((double)1.0) >= ((float)-0.1f);
      printf("557: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile double vy = ((double)1.0); int r1 = vx >= vy; int r2 = ((float)-0.1f) >= ((double)1.0); static int r3 = ((float)-0.1f) >= ((double)1.0);
      printf("558: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile float vy = ((float)1.0f); int r1 = vx >= vy; int r2 = ((double)1e-5) >= ((float)1.0f); static int r3 = ((double)1e-5) >= ((float)1.0f);
      printf("559: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1.0f); volatile double vy = ((double)1e-5); int r1 = vx >= vy; int r2 = ((float)1.0f) >= ((double)1e-5); static int r3 = ((float)1.0f) >= ((double)1e-5);
      printf("560: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile double vy = ((double)1e-5); double r1 = vx + vy; double r2 = ((double)0.3333333333333333) + ((double)1e-5); static double r3 = ((double)0.3333333333333333) + ((double)1e-5);
      printf("561: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-5); volatile double vy = ((double)0.3333333333333333); double r1 = vx + vy; double r2 = ((double)1e-5) + ((double)0.3333333333333333); static double r3 = ((double)1e-5) + ((double)0.3333333333333333);
      printf("562: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile double vy = ((double)7.0); double r1 = vx + vy; double r2 = ((double)1e10) + ((double)7.0); static double r3 = ((double)1e10) + ((double)7.0);
      printf("563: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)7.0); volatile double vy = ((double)1e10); double r1 = vx + vy; double r2 = ((double)7.0) + ((double)1e10); static double r3 = ((double)7.0) + ((double)1e10);
      printf("564: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)7.0); volatile double vy = ((double)0.3333333333333333); double r1 = vx + vy; double r2 = ((double)7.0) + ((double)0.3333333333333333); static double r3 = ((double)7.0) + ((double)0.3333333333333333);
      printf("565: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile double vy = ((double)7.0); double r1 = vx + vy; double r2 = ((double)0.3333333333333333) + ((double)7.0); static double r3 = ((double)0.3333333333333333) + ((double)7.0);
      printf("566: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)123456.789); volatile double vy = ((double)1e-40); double r1 = vx + vy; double r2 = ((double)123456.789) + ((double)1e-40); static double r3 = ((double)123456.789) + ((double)1e-40);
      printf("567: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile double vy = ((double)123456.789); double r1 = vx + vy; double r2 = ((double)1e-40) + ((double)123456.789); static double r3 = ((double)1e-40) + ((double)123456.789);
      printf("568: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile double vy = ((double)1e10); double r1 = vx - vy; double r2 = ((double)1.0) - ((double)1e10); static double r3 = ((double)1.0) - ((double)1e10);
      printf("569: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile double vy = ((double)1.0); double r1 = vx - vy; double r2 = ((double)1e10) - ((double)1.0); static double r3 = ((double)1e10) - ((double)1.0);
      printf("570: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile double vy = ((double)3.0); double r1 = vx - vy; double r2 = ((double)1.0) - ((double)3.0); static double r3 = ((double)1.0) - ((double)3.0);
      printf("571: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile double vy = ((double)1.0); double r1 = vx - vy; double r2 = ((double)3.0) - ((double)1.0); static double r3 = ((double)3.0) - ((double)1.0);
      printf("572: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-0.1); volatile double vy = ((double)16777217.0); double r1 = vx - vy; double r2 = ((double)-0.1) - ((double)16777217.0); static double r3 = ((double)-0.1) - ((double)16777217.0);
      printf("573: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)16777217.0); volatile double vy = ((double)-0.1); double r1 = vx - vy; double r2 = ((double)16777217.0) - ((double)-0.1); static double r3 = ((double)16777217.0) - ((double)-0.1);
      printf("574: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)123456.789); volatile double vy = ((double)16777217.0); double r1 = vx - vy; double r2 = ((double)123456.789) - ((double)16777217.0); static double r3 = ((double)123456.789) - ((double)16777217.0);
      printf("575: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)16777217.0); volatile double vy = ((double)123456.789); double r1 = vx - vy; double r2 = ((double)16777217.0) - ((double)123456.789); static double r3 = ((double)16777217.0) - ((double)123456.789);
      printf("576: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-2.5); volatile double vy = ((double)1e10); double r1 = vx * vy; double r2 = ((double)-2.5) * ((double)1e10); static double r3 = ((double)-2.5) * ((double)1e10);
      printf("577: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile double vy = ((double)-2.5); double r1 = vx * vy; double r2 = ((double)1e10) * ((double)-2.5); static double r3 = ((double)1e10) * ((double)-2.5);
      printf("578: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e30); volatile double vy = ((double)7.0); double r1 = vx * vy; double r2 = ((double)1e30) * ((double)7.0); static double r3 = ((double)1e30) * ((double)7.0);
      printf("579: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)7.0); volatile double vy = ((double)1e30); double r1 = vx * vy; double r2 = ((double)7.0) * ((double)1e30); static double r3 = ((double)7.0) * ((double)1e30);
      printf("580: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e30); volatile double vy = ((double)1e-40); double r1 = vx * vy; double r2 = ((double)1e30) * ((double)1e-40); static double r3 = ((double)1e30) * ((double)1e-40);
      printf("581: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile double vy = ((double)1e30); double r1 = vx * vy; double r2 = ((double)1e-40) * ((double)1e30); static double r3 = ((double)1e-40) * ((double)1e30);
      printf("582: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile double vy = ((double)7.0); double r1 = vx * vy; double r2 = ((double)3.0) * ((double)7.0); static double r3 = ((double)3.0) * ((double)7.0);
      printf("583: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)7.0); volatile double vy = ((double)3.0); double r1 = vx * vy; double r2 = ((double)7.0) * ((double)3.0); static double r3 = ((double)7.0) * ((double)3.0);
      printf("584: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)123456.789); volatile double vy = ((double)3.0); double r1 = vx / vy; double r2 = ((double)123456.789) / ((double)3.0); static double r3 = ((double)123456.789) / ((double)3.0);
      printf("585: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile double vy = ((double)123456.789); double r1 = vx / vy; double r2 = ((double)3.0) / ((double)123456.789); static double r3 = ((double)3.0) / ((double)123456.789);
      printf("586: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile double vy = ((double)16777217.0); double r1 = vx / vy; double r2 = ((double)3.0) / ((double)16777217.0); static double r3 = ((double)3.0) / ((double)16777217.0);
      printf("587: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)16777217.0); volatile double vy = ((double)3.0); double r1 = vx / vy; double r2 = ((double)16777217.0) / ((double)3.0); static double r3 = ((double)16777217.0) / ((double)3.0);
      printf("588: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile double vy = ((double)7.0); double r1 = vx / vy; double r2 = ((double)3.0) / ((double)7.0); static double r3 = ((double)3.0) / ((double)7.0);
      printf("589: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)7.0); volatile double vy = ((double)3.0); double r1 = vx / vy; double r2 = ((double)7.0) / ((double)3.0); static double r3 = ((double)7.0) / ((double)3.0);
      printf("590: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)7.0); volatile double vy = ((double)0.1); double r1 = vx / vy; double r2 = ((double)7.0) / ((double)0.1); static double r3 = ((double)7.0) / ((double)0.1);
      printf("591: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.1); volatile double vy = ((double)7.0); double r1 = vx / vy; double r2 = ((double)0.1) / ((double)7.0); static double r3 = ((double)0.1) / ((double)7.0);
      printf("592: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile double vy = ((double)1e10); int r1 = vx < vy; int r2 = ((double)1e10) < ((double)1e10); static int r3 = ((double)1e10) < ((double)1e10);
      printf("593: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e10); volatile double vy = ((double)1e10); int r1 = vx < vy; int r2 = ((double)1e10) < ((double)1e10); static int r3 = ((double)1e10) < ((double)1e10);
      printf("594: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.1); volatile double vy = ((double)1e30); int r1 = vx < vy; int r2 = ((double)0.1) < ((double)1e30); static int r3 = ((double)0.1) < ((double)1e30);
      printf("595: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e30); volatile double vy = ((double)0.1); int r1 = vx < vy; int r2 = ((double)1e30) < ((double)0.1); static int r3 = ((double)1e30) < ((double)0.1);
      printf("596: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-0.1); volatile double vy = ((double)3.0); int r1 = vx < vy; int r2 = ((double)-0.1) < ((double)3.0); static int r3 = ((double)-0.1) < ((double)3.0);
      printf("597: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)3.0); volatile double vy = ((double)-0.1); int r1 = vx < vy; int r2 = ((double)3.0) < ((double)-0.1); static int r3 = ((double)3.0) < ((double)-0.1);
      printf("598: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e10); volatile double vy = ((double)-0.1); int r1 = vx < vy; int r2 = ((double)1e10) < ((double)-0.1); static int r3 = ((double)1e10) < ((double)-0.1);
      printf("599: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-0.1); volatile double vy = ((double)1e10); int r1 = vx < vy; int r2 = ((double)-0.1) < ((double)1e10); static int r3 = ((double)-0.1) < ((double)1e10);
      printf("600: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-0.1); volatile double vy = ((double)7.0); int r1 = vx == vy; int r2 = ((double)-0.1) == ((double)7.0); static int r3 = ((double)-0.1) == ((double)7.0);
      printf("601: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)7.0); volatile double vy = ((double)-0.1); int r1 = vx == vy; int r2 = ((double)7.0) == ((double)-0.1); static int r3 = ((double)7.0) == ((double)-0.1);
      printf("602: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)16777217.0); volatile double vy = ((double)3.0); int r1 = vx == vy; int r2 = ((double)16777217.0) == ((double)3.0); static int r3 = ((double)16777217.0) == ((double)3.0);
      printf("603: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)3.0); volatile double vy = ((double)16777217.0); int r1 = vx == vy; int r2 = ((double)3.0) == ((double)16777217.0); static int r3 = ((double)3.0) == ((double)16777217.0);
      printf("604: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-2.5); volatile double vy = ((double)1e-5); int r1 = vx == vy; int r2 = ((double)-2.5) == ((double)1e-5); static int r3 = ((double)-2.5) == ((double)1e-5);
      printf("605: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile double vy = ((double)-2.5); int r1 = vx == vy; int r2 = ((double)1e-5) == ((double)-2.5); static int r3 = ((double)1e-5) == ((double)-2.5);
      printf("606: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-2.5); volatile double vy = ((double)3.0); int r1 = vx == vy; int r2 = ((double)-2.5) == ((double)3.0); static int r3 = ((double)-2.5) == ((double)3.0);
      printf("607: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)3.0); volatile double vy = ((double)-2.5); int r1 = vx == vy; int r2 = ((double)3.0) == ((double)-2.5); static int r3 = ((double)3.0) == ((double)-2.5);
      printf("608: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-2.5); volatile double vy = ((double)1e-5); int r1 = vx >= vy; int r2 = ((double)-2.5) >= ((double)1e-5); static int r3 = ((double)-2.5) >= ((double)1e-5);
      printf("609: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile double vy = ((double)-2.5); int r1 = vx >= vy; int r2 = ((double)1e-5) >= ((double)-2.5); static int r3 = ((double)1e-5) >= ((double)-2.5);
      printf("610: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1.0); volatile double vy = ((double)-0.1); int r1 = vx >= vy; int r2 = ((double)1.0) >= ((double)-0.1); static int r3 = ((double)1.0) >= ((double)-0.1);
      printf("611: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-0.1); volatile double vy = ((double)1.0); int r1 = vx >= vy; int r2 = ((double)-0.1) >= ((double)1.0); static int r3 = ((double)-0.1) >= ((double)1.0);
      printf("612: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-0.1); volatile double vy = ((double)1e-5); int r1 = vx >= vy; int r2 = ((double)-0.1) >= ((double)1e-5); static int r3 = ((double)-0.1) >= ((double)1e-5);
      printf("613: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile double vy = ((double)-0.1); int r1 = vx >= vy; int r2 = ((double)1e-5) >= ((double)-0.1); static int r3 = ((double)1e-5) >= ((double)-0.1);
      printf("614: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e30); volatile double vy = ((double)123456.789); int r1 = vx >= vy; int r2 = ((double)1e30) >= ((double)123456.789); static int r3 = ((double)1e30) >= ((double)123456.789);
      printf("615: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile double vy = ((double)1e30); int r1 = vx >= vy; int r2 = ((double)123456.789) >= ((double)1e30); static int r3 = ((double)123456.789) >= ((double)1e30);
      printf("616: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)3.0); volatile long double vy = ((long double)1e-5L); long double r1 = vx + vy; long double r2 = ((double)3.0) + ((long double)1e-5L); static long double r3 = ((double)3.0) + ((long double)1e-5L);
      printf("617: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile double vy = ((double)3.0); long double r1 = vx + vy; long double r2 = ((long double)1e-5L) + ((double)3.0); static long double r3 = ((long double)1e-5L) + ((double)3.0);
      printf("618: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e-40); volatile long double vy = ((long double)1e10L); long double r1 = vx + vy; long double r2 = ((double)1e-40) + ((long double)1e10L); static long double r3 = ((double)1e-40) + ((long double)1e10L);
      printf("619: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile double vy = ((double)1e-40); long double r1 = vx + vy; long double r2 = ((long double)1e10L) + ((double)1e-40); static long double r3 = ((long double)1e10L) + ((double)1e-40);
      printf("620: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)123456.789); volatile long double vy = ((long double)1.0L); long double r1 = vx + vy; long double r2 = ((double)123456.789) + ((long double)1.0L); static long double r3 = ((double)123456.789) + ((long double)1.0L);
      printf("621: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile double vy = ((double)123456.789); long double r1 = vx + vy; long double r2 = ((long double)1.0L) + ((double)123456.789); static long double r3 = ((long double)1.0L) + ((double)123456.789);
      printf("622: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1.0); volatile long double vy = ((long double)7.0L); long double r1 = vx + vy; long double r2 = ((double)1.0) + ((long double)7.0L); static long double r3 = ((double)1.0) + ((long double)7.0L);
      printf("623: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)7.0L); volatile double vy = ((double)1.0); long double r1 = vx + vy; long double r2 = ((long double)7.0L) + ((double)1.0); static long double r3 = ((long double)7.0L) + ((double)1.0);
      printf("624: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)-2.5); volatile long double vy = ((long double)1e10L); long double r1 = vx - vy; long double r2 = ((double)-2.5) - ((long double)1e10L); static long double r3 = ((double)-2.5) - ((long double)1e10L);
      printf("625: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile double vy = ((double)-2.5); long double r1 = vx - vy; long double r2 = ((long double)1e10L) - ((double)-2.5); static long double r3 = ((long double)1e10L) - ((double)-2.5);
      printf("626: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)123456.789); volatile long double vy = ((long double)1e-40L); long double r1 = vx - vy; long double r2 = ((double)123456.789) - ((long double)1e-40L); static long double r3 = ((double)123456.789) - ((long double)1e-40L);
      printf("627: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile double vy = ((double)123456.789); long double r1 = vx - vy; long double r2 = ((long double)1e-40L) - ((double)123456.789); static long double r3 = ((long double)1e-40L) - ((double)123456.789);
      printf("628: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile long double vy = ((long double)7.0L); long double r1 = vx - vy; long double r2 = ((double)0.3333333333333333) - ((long double)7.0L); static long double r3 = ((double)0.3333333333333333) - ((long double)7.0L);
      printf("629: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)7.0L); volatile double vy = ((double)0.3333333333333333); long double r1 = vx - vy; long double r2 = ((long double)7.0L) - ((double)0.3333333333333333); static long double r3 = ((long double)7.0L) - ((double)0.3333333333333333);
      printf("630: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e30); volatile long double vy = ((long double)-0.1L); long double r1 = vx - vy; long double r2 = ((double)1e30) - ((long double)-0.1L); static long double r3 = ((double)1e30) - ((long double)-0.1L);
      printf("631: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile double vy = ((double)1e30); long double r1 = vx - vy; long double r2 = ((long double)-0.1L) - ((double)1e30); static long double r3 = ((long double)-0.1L) - ((double)1e30);
      printf("632: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)-2.5); volatile long double vy = ((long double)-0.1L); long double r1 = vx * vy; long double r2 = ((double)-2.5) * ((long double)-0.1L); static long double r3 = ((double)-2.5) * ((long double)-0.1L);
      printf("633: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile double vy = ((double)-2.5); long double r1 = vx * vy; long double r2 = ((long double)-0.1L) * ((double)-2.5); static long double r3 = ((long double)-0.1L) * ((double)-2.5);
      printf("634: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)-0.1); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx * vy; long double r2 = ((double)-0.1) * ((long double)0.3333333333333333L); static long double r3 = ((double)-0.1) * ((long double)0.3333333333333333L);
      printf("635: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile double vy = ((double)-0.1); long double r1 = vx * vy; long double r2 = ((long double)0.3333333333333333L) * ((double)-0.1); static long double r3 = ((long double)0.3333333333333333L) * ((double)-0.1);
      printf("636: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e30); volatile long double vy = ((long double)1e-5L); long double r1 = vx * vy; long double r2 = ((double)1e30) * ((long double)1e-5L); static long double r3 = ((double)1e30) * ((long double)1e-5L);
      printf("637: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile double vy = ((double)1e30); long double r1 = vx * vy; long double r2 = ((long double)1e-5L) * ((double)1e30); static long double r3 = ((long double)1e-5L) * ((double)1e30);
      printf("638: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e-5); volatile long double vy = ((long double)1e10L); long double r1 = vx * vy; long double r2 = ((double)1e-5) * ((long double)1e10L); static long double r3 = ((double)1e-5) * ((long double)1e10L);
      printf("639: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile double vy = ((double)1e-5); long double r1 = vx * vy; long double r2 = ((long double)1e10L) * ((double)1e-5); static long double r3 = ((long double)1e10L) * ((double)1e-5);
      printf("640: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)0.1); volatile long double vy = ((long double)-0.1L); long double r1 = vx / vy; long double r2 = ((double)0.1) / ((long double)-0.1L); static long double r3 = ((double)0.1) / ((long double)-0.1L);
      printf("641: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile double vy = ((double)0.1); long double r1 = vx / vy; long double r2 = ((long double)-0.1L) / ((double)0.1); static long double r3 = ((long double)-0.1L) / ((double)0.1);
      printf("642: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)3.0); volatile long double vy = ((long double)7.0L); long double r1 = vx / vy; long double r2 = ((double)3.0) / ((long double)7.0L); static long double r3 = ((double)3.0) / ((long double)7.0L);
      printf("643: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)7.0L); volatile double vy = ((double)3.0); long double r1 = vx / vy; long double r2 = ((long double)7.0L) / ((double)3.0); static long double r3 = ((long double)7.0L) / ((double)3.0);
      printf("644: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)-0.1); volatile long double vy = ((long double)1e-40L); long double r1 = vx / vy; long double r2 = ((double)-0.1) / ((long double)1e-40L); static long double r3 = ((double)-0.1) / ((long double)1e-40L);
      printf("645: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile double vy = ((double)-0.1); long double r1 = vx / vy; long double r2 = ((long double)1e-40L) / ((double)-0.1); static long double r3 = ((long double)1e-40L) / ((double)-0.1);
      printf("646: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e-5); volatile long double vy = ((long double)1e-40L); long double r1 = vx / vy; long double r2 = ((double)1e-5) / ((long double)1e-40L); static long double r3 = ((double)1e-5) / ((long double)1e-40L);
      printf("647: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile double vy = ((double)1e-5); long double r1 = vx / vy; long double r2 = ((long double)1e-40L) / ((double)1e-5); static long double r3 = ((long double)1e-40L) / ((double)1e-5);
      printf("648: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e-40); volatile long double vy = ((long double)-2.5L); int r1 = vx < vy; int r2 = ((double)1e-40) < ((long double)-2.5L); static int r3 = ((double)1e-40) < ((long double)-2.5L);
      printf("649: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile double vy = ((double)1e-40); int r1 = vx < vy; int r2 = ((long double)-2.5L) < ((double)1e-40); static int r3 = ((long double)-2.5L) < ((double)1e-40);
      printf("650: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1.0); volatile long double vy = ((long double)3.0L); int r1 = vx < vy; int r2 = ((double)1.0) < ((long double)3.0L); static int r3 = ((double)1.0) < ((long double)3.0L);
      printf("651: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)3.0L); volatile double vy = ((double)1.0); int r1 = vx < vy; int r2 = ((long double)3.0L) < ((double)1.0); static int r3 = ((long double)3.0L) < ((double)1.0);
      printf("652: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)7.0); volatile long double vy = ((long double)1e-40L); int r1 = vx < vy; int r2 = ((double)7.0) < ((long double)1e-40L); static int r3 = ((double)7.0) < ((long double)1e-40L);
      printf("653: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile double vy = ((double)7.0); int r1 = vx < vy; int r2 = ((long double)1e-40L) < ((double)7.0); static int r3 = ((long double)1e-40L) < ((double)7.0);
      printf("654: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-2.5); volatile long double vy = ((long double)-0.1L); int r1 = vx < vy; int r2 = ((double)-2.5) < ((long double)-0.1L); static int r3 = ((double)-2.5) < ((long double)-0.1L);
      printf("655: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile double vy = ((double)-2.5); int r1 = vx < vy; int r2 = ((long double)-0.1L) < ((double)-2.5); static int r3 = ((long double)-0.1L) < ((double)-2.5);
      printf("656: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-40); volatile long double vy = ((long double)1e-5L); int r1 = vx == vy; int r2 = ((double)1e-40) == ((long double)1e-5L); static int r3 = ((double)1e-40) == ((long double)1e-5L);
      printf("657: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile double vy = ((double)1e-40); int r1 = vx == vy; int r2 = ((long double)1e-5L) == ((double)1e-40); static int r3 = ((long double)1e-5L) == ((double)1e-40);
      printf("658: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e30); volatile long double vy = ((long double)7.0L); int r1 = vx == vy; int r2 = ((double)1e30) == ((long double)7.0L); static int r3 = ((double)1e30) == ((long double)7.0L);
      printf("659: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)7.0L); volatile double vy = ((double)1e30); int r1 = vx == vy; int r2 = ((long double)7.0L) == ((double)1e30); static int r3 = ((long double)7.0L) == ((double)1e30);
      printf("660: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile long double vy = ((long double)16777217.0L); int r1 = vx == vy; int r2 = ((double)1e-5) == ((long double)16777217.0L); static int r3 = ((double)1e-5) == ((long double)16777217.0L);
      printf("661: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile double vy = ((double)1e-5); int r1 = vx == vy; int r2 = ((long double)16777217.0L) == ((double)1e-5); static int r3 = ((long double)16777217.0L) == ((double)1e-5);
      printf("662: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e10); volatile long double vy = ((long double)1e-5L); int r1 = vx == vy; int r2 = ((double)1e10) == ((long double)1e-5L); static int r3 = ((double)1e10) == ((long double)1e-5L);
      printf("663: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile double vy = ((double)1e10); int r1 = vx == vy; int r2 = ((long double)1e-5L) == ((double)1e10); static int r3 = ((long double)1e-5L) == ((double)1e10);
      printf("664: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)3.0); volatile long double vy = ((long double)0.3333333333333333L); int r1 = vx >= vy; int r2 = ((double)3.0) >= ((long double)0.3333333333333333L); static int r3 = ((double)3.0) >= ((long double)0.3333333333333333L);
      printf("665: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile double vy = ((double)3.0); int r1 = vx >= vy; int r2 = ((long double)0.3333333333333333L) >= ((double)3.0); static int r3 = ((long double)0.3333333333333333L) >= ((double)3.0);
      printf("666: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)16777217.0); volatile long double vy = ((long double)3.0L); int r1 = vx >= vy; int r2 = ((double)16777217.0) >= ((long double)3.0L); static int r3 = ((double)16777217.0) >= ((long double)3.0L);
      printf("667: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)3.0L); volatile double vy = ((double)16777217.0); int r1 = vx >= vy; int r2 = ((long double)3.0L) >= ((double)16777217.0); static int r3 = ((long double)3.0L) >= ((double)16777217.0);
      printf("668: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-2.5); volatile long double vy = ((long double)-2.5L); int r1 = vx >= vy; int r2 = ((double)-2.5) >= ((long double)-2.5L); static int r3 = ((double)-2.5) >= ((long double)-2.5L);
      printf("669: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile double vy = ((double)-2.5); int r1 = vx >= vy; int r2 = ((long double)-2.5L) >= ((double)-2.5); static int r3 = ((long double)-2.5L) >= ((double)-2.5);
      printf("670: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-2.5); volatile long double vy = ((long double)7.0L); int r1 = vx >= vy; int r2 = ((double)-2.5) >= ((long double)7.0L); static int r3 = ((double)-2.5) >= ((long double)7.0L);
      printf("671: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)7.0L); volatile double vy = ((double)-2.5); int r1 = vx >= vy; int r2 = ((long double)7.0L) >= ((double)-2.5); static int r3 = ((long double)7.0L) >= ((double)-2.5);
      printf("672: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e30); volatile int vy = ((int)-7); double r1 = vx + vy; double r2 = ((double)1e30) + ((int)-7); static double r3 = ((double)1e30) + ((int)-7);
      printf("673: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)-7); volatile double vy = ((double)1e30); double r1 = vx + vy; double r2 = ((int)-7) + ((double)1e30); static double r3 = ((int)-7) + ((double)1e30);
      printf("674: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile int vy = ((int)100000); double r1 = vx + vy; double r2 = ((double)3.0) + ((int)100000); static double r3 = ((double)3.0) + ((int)100000);
      printf("675: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)100000); volatile double vy = ((double)3.0); double r1 = vx + vy; double r2 = ((int)100000) + ((double)3.0); static double r3 = ((int)100000) + ((double)3.0);
      printf("676: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-2.5); volatile int vy = ((int)-7); double r1 = vx + vy; double r2 = ((double)-2.5) + ((int)-7); static double r3 = ((double)-2.5) + ((int)-7);
      printf("677: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)-7); volatile double vy = ((double)-2.5); double r1 = vx + vy; double r2 = ((int)-7) + ((double)-2.5); static double r3 = ((int)-7) + ((double)-2.5);
      printf("678: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile int vy = ((int)100000); double r1 = vx + vy; double r2 = ((double)1.0) + ((int)100000); static double r3 = ((double)1.0) + ((int)100000);
      printf("679: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)100000); volatile double vy = ((double)1.0); double r1 = vx + vy; double r2 = ((int)100000) + ((double)1.0); static double r3 = ((int)100000) + ((double)1.0);
      printf("680: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile int vy = ((int)3); double r1 = vx - vy; double r2 = ((double)1.0) - ((int)3); static double r3 = ((double)1.0) - ((int)3);
      printf("681: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)3); volatile double vy = ((double)1.0); double r1 = vx - vy; double r2 = ((int)3) - ((double)1.0); static double r3 = ((int)3) - ((double)1.0);
      printf("682: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)123456.789); volatile int vy = ((int)-7); double r1 = vx - vy; double r2 = ((double)123456.789) - ((int)-7); static double r3 = ((double)123456.789) - ((int)-7);
      printf("683: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)-7); volatile double vy = ((double)123456.789); double r1 = vx - vy; double r2 = ((int)-7) - ((double)123456.789); static double r3 = ((int)-7) - ((double)123456.789);
      printf("684: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)123456.789); volatile int vy = ((int)3); double r1 = vx - vy; double r2 = ((double)123456.789) - ((int)3); static double r3 = ((double)123456.789) - ((int)3);
      printf("685: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)3); volatile double vy = ((double)123456.789); double r1 = vx - vy; double r2 = ((int)3) - ((double)123456.789); static double r3 = ((int)3) - ((double)123456.789);
      printf("686: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)16777217.0); volatile int vy = ((int)3); double r1 = vx - vy; double r2 = ((double)16777217.0) - ((int)3); static double r3 = ((double)16777217.0) - ((int)3);
      printf("687: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)3); volatile double vy = ((double)16777217.0); double r1 = vx - vy; double r2 = ((int)3) - ((double)16777217.0); static double r3 = ((int)3) - ((double)16777217.0);
      printf("688: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile int vy = ((int)100000); double r1 = vx * vy; double r2 = ((double)1.0) * ((int)100000); static double r3 = ((double)1.0) * ((int)100000);
      printf("689: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)100000); volatile double vy = ((double)1.0); double r1 = vx * vy; double r2 = ((int)100000) * ((double)1.0); static double r3 = ((int)100000) * ((double)1.0);
      printf("690: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.1); volatile int vy = ((int)100000); double r1 = vx * vy; double r2 = ((double)0.1) * ((int)100000); static double r3 = ((double)0.1) * ((int)100000);
      printf("691: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)100000); volatile double vy = ((double)0.1); double r1 = vx * vy; double r2 = ((int)100000) * ((double)0.1); static double r3 = ((int)100000) * ((double)0.1);
      printf("692: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile int vy = ((int)3); double r1 = vx * vy; double r2 = ((double)0.3333333333333333) * ((int)3); static double r3 = ((double)0.3333333333333333) * ((int)3);
      printf("693: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)3); volatile double vy = ((double)0.3333333333333333); double r1 = vx * vy; double r2 = ((int)3) * ((double)0.3333333333333333); static double r3 = ((int)3) * ((double)0.3333333333333333);
      printf("694: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)16777217.0); volatile int vy = ((int)-7); double r1 = vx * vy; double r2 = ((double)16777217.0) * ((int)-7); static double r3 = ((double)16777217.0) * ((int)-7);
      printf("695: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)-7); volatile double vy = ((double)16777217.0); double r1 = vx * vy; double r2 = ((int)-7) * ((double)16777217.0); static double r3 = ((int)-7) * ((double)16777217.0);
      printf("696: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)16777217.0); volatile int vy = ((int)3); double r1 = vx / vy; double r2 = ((double)16777217.0) / ((int)3); static double r3 = ((double)16777217.0) / ((int)3);
      printf("697: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)3); volatile double vy = ((double)16777217.0); double r1 = vx / vy; double r2 = ((int)3) / ((double)16777217.0); static double r3 = ((int)3) / ((double)16777217.0);
      printf("698: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-0.1); volatile int vy = ((int)-7); double r1 = vx / vy; double r2 = ((double)-0.1) / ((int)-7); static double r3 = ((double)-0.1) / ((int)-7);
      printf("699: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)-7); volatile double vy = ((double)-0.1); double r1 = vx / vy; double r2 = ((int)-7) / ((double)-0.1); static double r3 = ((int)-7) / ((double)-0.1);
      printf("700: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-0.1); volatile int vy = ((int)-7); double r1 = vx / vy; double r2 = ((double)-0.1) / ((int)-7); static double r3 = ((double)-0.1) / ((int)-7);
      printf("701: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)-7); volatile double vy = ((double)-0.1); double r1 = vx / vy; double r2 = ((int)-7) / ((double)-0.1); static double r3 = ((int)-7) / ((double)-0.1);
      printf("702: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.1); volatile int vy = ((int)100000); double r1 = vx / vy; double r2 = ((double)0.1) / ((int)100000); static double r3 = ((double)0.1) / ((int)100000);
      printf("703: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile int vx = ((int)100000); volatile double vy = ((double)0.1); double r1 = vx / vy; double r2 = ((int)100000) / ((double)0.1); static double r3 = ((int)100000) / ((double)0.1);
      printf("704: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile int vy = ((int)3); int r1 = vx < vy; int r2 = ((double)1.0) < ((int)3); static int r3 = ((double)1.0) < ((int)3);
      printf("705: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)3); volatile double vy = ((double)1.0); int r1 = vx < vy; int r2 = ((int)3) < ((double)1.0); static int r3 = ((int)3) < ((double)1.0);
      printf("706: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)7.0); volatile int vy = ((int)-7); int r1 = vx < vy; int r2 = ((double)7.0) < ((int)-7); static int r3 = ((double)7.0) < ((int)-7);
      printf("707: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile double vy = ((double)7.0); int r1 = vx < vy; int r2 = ((int)-7) < ((double)7.0); static int r3 = ((int)-7) < ((double)7.0);
      printf("708: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-40); volatile int vy = ((int)3); int r1 = vx < vy; int r2 = ((double)1e-40) < ((int)3); static int r3 = ((double)1e-40) < ((int)3);
      printf("709: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)3); volatile double vy = ((double)1e-40); int r1 = vx < vy; int r2 = ((int)3) < ((double)1e-40); static int r3 = ((int)3) < ((double)1e-40);
      printf("710: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile int vy = ((int)100000); int r1 = vx < vy; int r2 = ((double)123456.789) < ((int)100000); static int r3 = ((double)123456.789) < ((int)100000);
      printf("711: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)100000); volatile double vy = ((double)123456.789); int r1 = vx < vy; int r2 = ((int)100000) < ((double)123456.789); static int r3 = ((int)100000) < ((double)123456.789);
      printf("712: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile int vy = ((int)-7); int r1 = vx == vy; int r2 = ((double)123456.789) == ((int)-7); static int r3 = ((double)123456.789) == ((int)-7);
      printf("713: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile double vy = ((double)123456.789); int r1 = vx == vy; int r2 = ((int)-7) == ((double)123456.789); static int r3 = ((int)-7) == ((double)123456.789);
      printf("714: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-2.5); volatile int vy = ((int)-7); int r1 = vx == vy; int r2 = ((double)-2.5) == ((int)-7); static int r3 = ((double)-2.5) == ((int)-7);
      printf("715: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile double vy = ((double)-2.5); int r1 = vx == vy; int r2 = ((int)-7) == ((double)-2.5); static int r3 = ((int)-7) == ((double)-2.5);
      printf("716: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-2.5); volatile int vy = ((int)100000); int r1 = vx == vy; int r2 = ((double)-2.5) == ((int)100000); static int r3 = ((double)-2.5) == ((int)100000);
      printf("717: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)100000); volatile double vy = ((double)-2.5); int r1 = vx == vy; int r2 = ((int)100000) == ((double)-2.5); static int r3 = ((int)100000) == ((double)-2.5);
      printf("718: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile int vy = ((int)-7); int r1 = vx == vy; int r2 = ((double)1e-5) == ((int)-7); static int r3 = ((double)1e-5) == ((int)-7);
      printf("719: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile double vy = ((double)1e-5); int r1 = vx == vy; int r2 = ((int)-7) == ((double)1e-5); static int r3 = ((int)-7) == ((double)1e-5);
      printf("720: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile int vy = ((int)100000); int r1 = vx >= vy; int r2 = ((double)123456.789) >= ((int)100000); static int r3 = ((double)123456.789) >= ((int)100000);
      printf("721: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)100000); volatile double vy = ((double)123456.789); int r1 = vx >= vy; int r2 = ((int)100000) >= ((double)123456.789); static int r3 = ((int)100000) >= ((double)123456.789);
      printf("722: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-40); volatile int vy = ((int)100000); int r1 = vx >= vy; int r2 = ((double)1e-40) >= ((int)100000); static int r3 = ((double)1e-40) >= ((int)100000);
      printf("723: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)100000); volatile double vy = ((double)1e-40); int r1 = vx >= vy; int r2 = ((int)100000) >= ((double)1e-40); static int r3 = ((int)100000) >= ((double)1e-40);
      printf("724: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile int vy = ((int)100000); int r1 = vx >= vy; int r2 = ((double)0.3333333333333333) >= ((int)100000); static int r3 = ((double)0.3333333333333333) >= ((int)100000);
      printf("725: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)100000); volatile double vy = ((double)0.3333333333333333); int r1 = vx >= vy; int r2 = ((int)100000) >= ((double)0.3333333333333333); static int r3 = ((int)100000) >= ((double)0.3333333333333333);
      printf("726: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.1); volatile int vy = ((int)-7); int r1 = vx >= vy; int r2 = ((double)0.1) >= ((int)-7); static int r3 = ((double)0.1) >= ((int)-7);
      printf("727: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile double vy = ((double)0.1); int r1 = vx >= vy; int r2 = ((int)-7) >= ((double)0.1); static int r3 = ((int)-7) >= ((double)0.1);
      printf("728: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile unsigned vy = ((unsigned)3u); double r1 = vx + vy; double r2 = ((double)1e-5) + ((unsigned)3u); static double r3 = ((double)1e-5) + ((unsigned)3u);
      printf("729: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile double vy = ((double)1e-5); double r1 = vx + vy; double r2 = ((unsigned)3u) + ((double)1e-5); static double r3 = ((unsigned)3u) + ((double)1e-5);
      printf("730: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-2.5); volatile unsigned vy = ((unsigned)3u); double r1 = vx + vy; double r2 = ((double)-2.5) + ((unsigned)3u); static double r3 = ((double)-2.5) + ((unsigned)3u);
      printf("731: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile double vy = ((double)-2.5); double r1 = vx + vy; double r2 = ((unsigned)3u) + ((double)-2.5); static double r3 = ((unsigned)3u) + ((double)-2.5);
      printf("732: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-0.1); volatile unsigned vy = ((unsigned)4000000000u); double r1 = vx + vy; double r2 = ((double)-0.1) + ((unsigned)4000000000u); static double r3 = ((double)-0.1) + ((unsigned)4000000000u);
      printf("733: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)-0.1); double r1 = vx + vy; double r2 = ((unsigned)4000000000u) + ((double)-0.1); static double r3 = ((unsigned)4000000000u) + ((double)-0.1);
      printf("734: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile unsigned vy = ((unsigned)3u); double r1 = vx + vy; double r2 = ((double)1.0) + ((unsigned)3u); static double r3 = ((double)1.0) + ((unsigned)3u);
      printf("735: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile double vy = ((double)1.0); double r1 = vx + vy; double r2 = ((unsigned)3u) + ((double)1.0); static double r3 = ((unsigned)3u) + ((double)1.0);
      printf("736: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile unsigned vy = ((unsigned)4000000000u); double r1 = vx - vy; double r2 = ((double)1.0) - ((unsigned)4000000000u); static double r3 = ((double)1.0) - ((unsigned)4000000000u);
      printf("737: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)1.0); double r1 = vx - vy; double r2 = ((unsigned)4000000000u) - ((double)1.0); static double r3 = ((unsigned)4000000000u) - ((double)1.0);
      printf("738: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile unsigned vy = ((unsigned)4000000000u); double r1 = vx - vy; double r2 = ((double)1.0) - ((unsigned)4000000000u); static double r3 = ((double)1.0) - ((unsigned)4000000000u);
      printf("739: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)1.0); double r1 = vx - vy; double r2 = ((unsigned)4000000000u) - ((double)1.0); static double r3 = ((unsigned)4000000000u) - ((double)1.0);
      printf("740: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile unsigned vy = ((unsigned)4000000000u); double r1 = vx - vy; double r2 = ((double)0.3333333333333333) - ((unsigned)4000000000u); static double r3 = ((double)0.3333333333333333) - ((unsigned)4000000000u);
      printf("741: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)0.3333333333333333); double r1 = vx - vy; double r2 = ((unsigned)4000000000u) - ((double)0.3333333333333333); static double r3 = ((unsigned)4000000000u) - ((double)0.3333333333333333);
      printf("742: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile unsigned vy = ((unsigned)4000000000u); double r1 = vx - vy; double r2 = ((double)1e10) - ((unsigned)4000000000u); static double r3 = ((double)1e10) - ((unsigned)4000000000u);
      printf("743: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)1e10); double r1 = vx - vy; double r2 = ((unsigned)4000000000u) - ((double)1e10); static double r3 = ((unsigned)4000000000u) - ((double)1e10);
      printf("744: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-5); volatile unsigned vy = ((unsigned)4000000000u); double r1 = vx * vy; double r2 = ((double)1e-5) * ((unsigned)4000000000u); static double r3 = ((double)1e-5) * ((unsigned)4000000000u);
      printf("745: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)1e-5); double r1 = vx * vy; double r2 = ((unsigned)4000000000u) * ((double)1e-5); static double r3 = ((unsigned)4000000000u) * ((double)1e-5);
      printf("746: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-0.1); volatile unsigned vy = ((unsigned)4000000000u); double r1 = vx * vy; double r2 = ((double)-0.1) * ((unsigned)4000000000u); static double r3 = ((double)-0.1) * ((unsigned)4000000000u);
      printf("747: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)-0.1); double r1 = vx * vy; double r2 = ((unsigned)4000000000u) * ((double)-0.1); static double r3 = ((unsigned)4000000000u) * ((double)-0.1);
      printf("748: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile unsigned vy = ((unsigned)3u); double r1 = vx * vy; double r2 = ((double)1.0) * ((unsigned)3u); static double r3 = ((double)1.0) * ((unsigned)3u);
      printf("749: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile double vy = ((double)1.0); double r1 = vx * vy; double r2 = ((unsigned)3u) * ((double)1.0); static double r3 = ((unsigned)3u) * ((double)1.0);
      printf("750: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)123456.789); volatile unsigned vy = ((unsigned)4000000000u); double r1 = vx * vy; double r2 = ((double)123456.789) * ((unsigned)4000000000u); static double r3 = ((double)123456.789) * ((unsigned)4000000000u);
      printf("751: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)123456.789); double r1 = vx * vy; double r2 = ((unsigned)4000000000u) * ((double)123456.789); static double r3 = ((unsigned)4000000000u) * ((double)123456.789);
      printf("752: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e30); volatile unsigned vy = ((unsigned)4000000000u); double r1 = vx / vy; double r2 = ((double)1e30) / ((unsigned)4000000000u); static double r3 = ((double)1e30) / ((unsigned)4000000000u);
      printf("753: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)1e30); double r1 = vx / vy; double r2 = ((unsigned)4000000000u) / ((double)1e30); static double r3 = ((unsigned)4000000000u) / ((double)1e30);
      printf("754: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.1); volatile unsigned vy = ((unsigned)4000000000u); double r1 = vx / vy; double r2 = ((double)0.1) / ((unsigned)4000000000u); static double r3 = ((double)0.1) / ((unsigned)4000000000u);
      printf("755: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)0.1); double r1 = vx / vy; double r2 = ((unsigned)4000000000u) / ((double)0.1); static double r3 = ((unsigned)4000000000u) / ((double)0.1);
      printf("756: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile unsigned vy = ((unsigned)3u); double r1 = vx / vy; double r2 = ((double)3.0) / ((unsigned)3u); static double r3 = ((double)3.0) / ((unsigned)3u);
      printf("757: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile double vy = ((double)3.0); double r1 = vx / vy; double r2 = ((unsigned)3u) / ((double)3.0); static double r3 = ((unsigned)3u) / ((double)3.0);
      printf("758: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile unsigned vy = ((unsigned)3u); double r1 = vx / vy; double r2 = ((double)3.0) / ((unsigned)3u); static double r3 = ((double)3.0) / ((unsigned)3u);
      printf("759: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile double vy = ((double)3.0); double r1 = vx / vy; double r2 = ((unsigned)3u) / ((double)3.0); static double r3 = ((unsigned)3u) / ((double)3.0);
      printf("760: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx < vy; int r2 = ((double)1e-40) < ((unsigned)4000000000u); static int r3 = ((double)1e-40) < ((unsigned)4000000000u);
      printf("761: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)1e-40); int r1 = vx < vy; int r2 = ((unsigned)4000000000u) < ((double)1e-40); static int r3 = ((unsigned)4000000000u) < ((double)1e-40);
      printf("762: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)16777217.0); volatile unsigned vy = ((unsigned)3u); int r1 = vx < vy; int r2 = ((double)16777217.0) < ((unsigned)3u); static int r3 = ((double)16777217.0) < ((unsigned)3u);
      printf("763: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile double vy = ((double)16777217.0); int r1 = vx < vy; int r2 = ((unsigned)3u) < ((double)16777217.0); static int r3 = ((unsigned)3u) < ((double)16777217.0);
      printf("764: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1.0); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx < vy; int r2 = ((double)1.0) < ((unsigned)4000000000u); static int r3 = ((double)1.0) < ((unsigned)4000000000u);
      printf("765: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)1.0); int r1 = vx < vy; int r2 = ((unsigned)4000000000u) < ((double)1.0); static int r3 = ((unsigned)4000000000u) < ((double)1.0);
      printf("766: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1.0); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx < vy; int r2 = ((double)1.0) < ((unsigned)4000000000u); static int r3 = ((double)1.0) < ((unsigned)4000000000u);
      printf("767: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)1.0); int r1 = vx < vy; int r2 = ((unsigned)4000000000u) < ((double)1.0); static int r3 = ((unsigned)4000000000u) < ((double)1.0);
      printf("768: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)16777217.0); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx == vy; int r2 = ((double)16777217.0) == ((unsigned)4000000000u); static int r3 = ((double)16777217.0) == ((unsigned)4000000000u);
      printf("769: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)16777217.0); int r1 = vx == vy; int r2 = ((unsigned)4000000000u) == ((double)16777217.0); static int r3 = ((unsigned)4000000000u) == ((double)16777217.0);
      printf("770: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx == vy; int r2 = ((double)1e-5) == ((unsigned)4000000000u); static int r3 = ((double)1e-5) == ((unsigned)4000000000u);
      printf("771: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)1e-5); int r1 = vx == vy; int r2 = ((unsigned)4000000000u) == ((double)1e-5); static int r3 = ((unsigned)4000000000u) == ((double)1e-5);
      printf("772: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)7.0); volatile unsigned vy = ((unsigned)3u); int r1 = vx == vy; int r2 = ((double)7.0) == ((unsigned)3u); static int r3 = ((double)7.0) == ((unsigned)3u);
      printf("773: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile double vy = ((double)7.0); int r1 = vx == vy; int r2 = ((unsigned)3u) == ((double)7.0); static int r3 = ((unsigned)3u) == ((double)7.0);
      printf("774: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)3.0); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx == vy; int r2 = ((double)3.0) == ((unsigned)4000000000u); static int r3 = ((double)3.0) == ((unsigned)4000000000u);
      printf("775: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)3.0); int r1 = vx == vy; int r2 = ((unsigned)4000000000u) == ((double)3.0); static int r3 = ((unsigned)4000000000u) == ((double)3.0);
      printf("776: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)3.0); volatile unsigned vy = ((unsigned)3u); int r1 = vx >= vy; int r2 = ((double)3.0) >= ((unsigned)3u); static int r3 = ((double)3.0) >= ((unsigned)3u);
      printf("777: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile double vy = ((double)3.0); int r1 = vx >= vy; int r2 = ((unsigned)3u) >= ((double)3.0); static int r3 = ((unsigned)3u) >= ((double)3.0);
      printf("778: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx >= vy; int r2 = ((double)123456.789) >= ((unsigned)4000000000u); static int r3 = ((double)123456.789) >= ((unsigned)4000000000u);
      printf("779: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)123456.789); int r1 = vx >= vy; int r2 = ((unsigned)4000000000u) >= ((double)123456.789); static int r3 = ((unsigned)4000000000u) >= ((double)123456.789);
      printf("780: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx >= vy; int r2 = ((double)123456.789) >= ((unsigned)4000000000u); static int r3 = ((double)123456.789) >= ((unsigned)4000000000u);
      printf("781: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile double vy = ((double)123456.789); int r1 = vx >= vy; int r2 = ((unsigned)4000000000u) >= ((double)123456.789); static int r3 = ((unsigned)4000000000u) >= ((double)123456.789);
      printf("782: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.1); volatile unsigned vy = ((unsigned)3u); int r1 = vx >= vy; int r2 = ((double)0.1) >= ((unsigned)3u); static int r3 = ((double)0.1) >= ((unsigned)3u);
      printf("783: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile double vy = ((double)0.1); int r1 = vx >= vy; int r2 = ((unsigned)3u) >= ((double)0.1); static int r3 = ((unsigned)3u) >= ((double)0.1);
      printf("784: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile long long vy = ((long long)9007199254740993LL); double r1 = vx + vy; double r2 = ((double)1e-5) + ((long long)9007199254740993LL); static double r3 = ((double)1e-5) + ((long long)9007199254740993LL);
      printf("785: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile double vy = ((double)1e-5); double r1 = vx + vy; double r2 = ((long long)9007199254740993LL) + ((double)1e-5); static double r3 = ((long long)9007199254740993LL) + ((double)1e-5);
      printf("786: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)123456.789); volatile long long vy = ((long long)-5LL); double r1 = vx + vy; double r2 = ((double)123456.789) + ((long long)-5LL); static double r3 = ((double)123456.789) + ((long long)-5LL);
      printf("787: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)123456.789); double r1 = vx + vy; double r2 = ((long long)-5LL) + ((double)123456.789); static double r3 = ((long long)-5LL) + ((double)123456.789);
      printf("788: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile long long vy = ((long long)-5LL); double r1 = vx + vy; double r2 = ((double)0.3333333333333333) + ((long long)-5LL); static double r3 = ((double)0.3333333333333333) + ((long long)-5LL);
      printf("789: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)0.3333333333333333); double r1 = vx + vy; double r2 = ((long long)-5LL) + ((double)0.3333333333333333); static double r3 = ((long long)-5LL) + ((double)0.3333333333333333);
      printf("790: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile long long vy = ((long long)9007199254740993LL); double r1 = vx + vy; double r2 = ((double)1.0) + ((long long)9007199254740993LL); static double r3 = ((double)1.0) + ((long long)9007199254740993LL);
      printf("791: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile double vy = ((double)1.0); double r1 = vx + vy; double r2 = ((long long)9007199254740993LL) + ((double)1.0); static double r3 = ((long long)9007199254740993LL) + ((double)1.0);
      printf("792: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile long long vy = ((long long)-5LL); double r1 = vx - vy; double r2 = ((double)1e10) - ((long long)-5LL); static double r3 = ((double)1e10) - ((long long)-5LL);
      printf("793: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)1e10); double r1 = vx - vy; double r2 = ((long long)-5LL) - ((double)1e10); static double r3 = ((long long)-5LL) - ((double)1e10);
      printf("794: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile long long vy = ((long long)9007199254740993LL); double r1 = vx - vy; double r2 = ((double)1e-40) - ((long long)9007199254740993LL); static double r3 = ((double)1e-40) - ((long long)9007199254740993LL);
      printf("795: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile double vy = ((double)1e-40); double r1 = vx - vy; double r2 = ((long long)9007199254740993LL) - ((double)1e-40); static double r3 = ((long long)9007199254740993LL) - ((double)1e-40);
      printf("796: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)16777217.0); volatile long long vy = ((long long)-5LL); double r1 = vx - vy; double r2 = ((double)16777217.0) - ((long long)-5LL); static double r3 = ((double)16777217.0) - ((long long)-5LL);
      printf("797: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)16777217.0); double r1 = vx - vy; double r2 = ((long long)-5LL) - ((double)16777217.0); static double r3 = ((long long)-5LL) - ((double)16777217.0);
      printf("798: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e30); volatile long long vy = ((long long)-5LL); double r1 = vx - vy; double r2 = ((double)1e30) - ((long long)-5LL); static double r3 = ((double)1e30) - ((long long)-5LL);
      printf("799: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)1e30); double r1 = vx - vy; double r2 = ((long long)-5LL) - ((double)1e30); static double r3 = ((long long)-5LL) - ((double)1e30);
      printf("800: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile long long vy = ((long long)-5LL); double r1 = vx * vy; double r2 = ((double)3.0) * ((long long)-5LL); static double r3 = ((double)3.0) * ((long long)-5LL);
      printf("801: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)3.0); double r1 = vx * vy; double r2 = ((long long)-5LL) * ((double)3.0); static double r3 = ((long long)-5LL) * ((double)3.0);
      printf("802: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-0.1); volatile long long vy = ((long long)9007199254740993LL); double r1 = vx * vy; double r2 = ((double)-0.1) * ((long long)9007199254740993LL); static double r3 = ((double)-0.1) * ((long long)9007199254740993LL);
      printf("803: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile double vy = ((double)-0.1); double r1 = vx * vy; double r2 = ((long long)9007199254740993LL) * ((double)-0.1); static double r3 = ((long long)9007199254740993LL) * ((double)-0.1);
      printf("804: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e30); volatile long long vy = ((long long)-5LL); double r1 = vx * vy; double r2 = ((double)1e30) * ((long long)-5LL); static double r3 = ((double)1e30) * ((long long)-5LL);
      printf("805: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)1e30); double r1 = vx * vy; double r2 = ((long long)-5LL) * ((double)1e30); static double r3 = ((long long)-5LL) * ((double)1e30);
      printf("806: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile long long vy = ((long long)9007199254740993LL); double r1 = vx * vy; double r2 = ((double)1e-40) * ((long long)9007199254740993LL); static double r3 = ((double)1e-40) * ((long long)9007199254740993LL);
      printf("807: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile double vy = ((double)1e-40); double r1 = vx * vy; double r2 = ((long long)9007199254740993LL) * ((double)1e-40); static double r3 = ((long long)9007199254740993LL) * ((double)1e-40);
      printf("808: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile long long vy = ((long long)9007199254740993LL); double r1 = vx / vy; double r2 = ((double)1e-40) / ((long long)9007199254740993LL); static double r3 = ((double)1e-40) / ((long long)9007199254740993LL);
      printf("809: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile double vy = ((double)1e-40); double r1 = vx / vy; double r2 = ((long long)9007199254740993LL) / ((double)1e-40); static double r3 = ((long long)9007199254740993LL) / ((double)1e-40);
      printf("810: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)123456.789); volatile long long vy = ((long long)-5LL); double r1 = vx / vy; double r2 = ((double)123456.789) / ((long long)-5LL); static double r3 = ((double)123456.789) / ((long long)-5LL);
      printf("811: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)123456.789); double r1 = vx / vy; double r2 = ((long long)-5LL) / ((double)123456.789); static double r3 = ((long long)-5LL) / ((double)123456.789);
      printf("812: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile long long vy = ((long long)-5LL); double r1 = vx / vy; double r2 = ((double)3.0) / ((long long)-5LL); static double r3 = ((double)3.0) / ((long long)-5LL);
      printf("813: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)3.0); double r1 = vx / vy; double r2 = ((long long)-5LL) / ((double)3.0); static double r3 = ((long long)-5LL) / ((double)3.0);
      printf("814: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-5); volatile long long vy = ((long long)-5LL); double r1 = vx / vy; double r2 = ((double)1e-5) / ((long long)-5LL); static double r3 = ((double)1e-5) / ((long long)-5LL);
      printf("815: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)1e-5); double r1 = vx / vy; double r2 = ((long long)-5LL) / ((double)1e-5); static double r3 = ((long long)-5LL) / ((double)1e-5);
      printf("816: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)16777217.0); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx < vy; int r2 = ((double)16777217.0) < ((long long)9007199254740993LL); static int r3 = ((double)16777217.0) < ((long long)9007199254740993LL);
      printf("817: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile double vy = ((double)16777217.0); int r1 = vx < vy; int r2 = ((long long)9007199254740993LL) < ((double)16777217.0); static int r3 = ((long long)9007199254740993LL) < ((double)16777217.0);
      printf("818: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile long long vy = ((long long)-5LL); int r1 = vx < vy; int r2 = ((double)1e-5) < ((long long)-5LL); static int r3 = ((double)1e-5) < ((long long)-5LL);
      printf("819: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)1e-5); int r1 = vx < vy; int r2 = ((long long)-5LL) < ((double)1e-5); static int r3 = ((long long)-5LL) < ((double)1e-5);
      printf("820: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-2.5); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx < vy; int r2 = ((double)-2.5) < ((long long)9007199254740993LL); static int r3 = ((double)-2.5) < ((long long)9007199254740993LL);
      printf("821: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile double vy = ((double)-2.5); int r1 = vx < vy; int r2 = ((long long)9007199254740993LL) < ((double)-2.5); static int r3 = ((long long)9007199254740993LL) < ((double)-2.5);
      printf("822: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-0.1); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx < vy; int r2 = ((double)-0.1) < ((long long)9007199254740993LL); static int r3 = ((double)-0.1) < ((long long)9007199254740993LL);
      printf("823: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile double vy = ((double)-0.1); int r1 = vx < vy; int r2 = ((long long)9007199254740993LL) < ((double)-0.1); static int r3 = ((long long)9007199254740993LL) < ((double)-0.1);
      printf("824: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)7.0); volatile long long vy = ((long long)-5LL); int r1 = vx == vy; int r2 = ((double)7.0) == ((long long)-5LL); static int r3 = ((double)7.0) == ((long long)-5LL);
      printf("825: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)7.0); int r1 = vx == vy; int r2 = ((long long)-5LL) == ((double)7.0); static int r3 = ((long long)-5LL) == ((double)7.0);
      printf("826: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-0.1); volatile long long vy = ((long long)-5LL); int r1 = vx == vy; int r2 = ((double)-0.1) == ((long long)-5LL); static int r3 = ((double)-0.1) == ((long long)-5LL);
      printf("827: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)-0.1); int r1 = vx == vy; int r2 = ((long long)-5LL) == ((double)-0.1); static int r3 = ((long long)-5LL) == ((double)-0.1);
      printf("828: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)7.0); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx == vy; int r2 = ((double)7.0) == ((long long)9007199254740993LL); static int r3 = ((double)7.0) == ((long long)9007199254740993LL);
      printf("829: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile double vy = ((double)7.0); int r1 = vx == vy; int r2 = ((long long)9007199254740993LL) == ((double)7.0); static int r3 = ((long long)9007199254740993LL) == ((double)7.0);
      printf("830: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile long long vy = ((long long)-5LL); int r1 = vx == vy; int r2 = ((double)1e-5) == ((long long)-5LL); static int r3 = ((double)1e-5) == ((long long)-5LL);
      printf("831: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)1e-5); int r1 = vx == vy; int r2 = ((long long)-5LL) == ((double)1e-5); static int r3 = ((long long)-5LL) == ((double)1e-5);
      printf("832: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile long long vy = ((long long)-5LL); int r1 = vx >= vy; int r2 = ((double)1e-5) >= ((long long)-5LL); static int r3 = ((double)1e-5) >= ((long long)-5LL);
      printf("833: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)1e-5); int r1 = vx >= vy; int r2 = ((long long)-5LL) >= ((double)1e-5); static int r3 = ((long long)-5LL) >= ((double)1e-5);
      printf("834: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-40); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx >= vy; int r2 = ((double)1e-40) >= ((long long)9007199254740993LL); static int r3 = ((double)1e-40) >= ((long long)9007199254740993LL);
      printf("835: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile double vy = ((double)1e-40); int r1 = vx >= vy; int r2 = ((long long)9007199254740993LL) >= ((double)1e-40); static int r3 = ((long long)9007199254740993LL) >= ((double)1e-40);
      printf("836: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e10); volatile long long vy = ((long long)-5LL); int r1 = vx >= vy; int r2 = ((double)1e10) >= ((long long)-5LL); static int r3 = ((double)1e10) >= ((long long)-5LL);
      printf("837: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)1e10); int r1 = vx >= vy; int r2 = ((long long)-5LL) >= ((double)1e10); static int r3 = ((long long)-5LL) >= ((double)1e10);
      printf("838: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile long long vy = ((long long)-5LL); int r1 = vx >= vy; int r2 = ((double)123456.789) >= ((long long)-5LL); static int r3 = ((double)123456.789) >= ((long long)-5LL);
      printf("839: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile double vy = ((double)123456.789); int r1 = vx >= vy; int r2 = ((long long)-5LL) >= ((double)123456.789); static int r3 = ((long long)-5LL) >= ((double)123456.789);
      printf("840: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-40); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); double r1 = vx + vy; double r2 = ((double)1e-40) + ((unsigned long long)18446744073709551615ULL); static double r3 = ((double)1e-40) + ((unsigned long long)18446744073709551615ULL);
      printf("841: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)1e-40); double r1 = vx + vy; double r2 = ((unsigned long long)18446744073709551615ULL) + ((double)1e-40); static double r3 = ((unsigned long long)18446744073709551615ULL) + ((double)1e-40);
      printf("842: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); double r1 = vx + vy; double r2 = ((double)1.0) + ((unsigned long long)12345678901234567ULL); static double r3 = ((double)1.0) + ((unsigned long long)12345678901234567ULL);
      printf("843: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile double vy = ((double)1.0); double r1 = vx + vy; double r2 = ((unsigned long long)12345678901234567ULL) + ((double)1.0); static double r3 = ((unsigned long long)12345678901234567ULL) + ((double)1.0);
      printf("844: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); double r1 = vx + vy; double r2 = ((double)0.3333333333333333) + ((unsigned long long)12345678901234567ULL); static double r3 = ((double)0.3333333333333333) + ((unsigned long long)12345678901234567ULL);
      printf("845: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile double vy = ((double)0.3333333333333333); double r1 = vx + vy; double r2 = ((unsigned long long)12345678901234567ULL) + ((double)0.3333333333333333); static double r3 = ((unsigned long long)12345678901234567ULL) + ((double)0.3333333333333333);
      printf("846: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)123456.789); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); double r1 = vx + vy; double r2 = ((double)123456.789) + ((unsigned long long)12345678901234567ULL); static double r3 = ((double)123456.789) + ((unsigned long long)12345678901234567ULL);
      printf("847: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile double vy = ((double)123456.789); double r1 = vx + vy; double r2 = ((unsigned long long)12345678901234567ULL) + ((double)123456.789); static double r3 = ((unsigned long long)12345678901234567ULL) + ((double)123456.789);
      printf("848: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)7.0); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); double r1 = vx - vy; double r2 = ((double)7.0) - ((unsigned long long)18446744073709551615ULL); static double r3 = ((double)7.0) - ((unsigned long long)18446744073709551615ULL);
      printf("849: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)7.0); double r1 = vx - vy; double r2 = ((unsigned long long)18446744073709551615ULL) - ((double)7.0); static double r3 = ((unsigned long long)18446744073709551615ULL) - ((double)7.0);
      printf("850: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); double r1 = vx - vy; double r2 = ((double)3.0) - ((unsigned long long)18446744073709551615ULL); static double r3 = ((double)3.0) - ((unsigned long long)18446744073709551615ULL);
      printf("851: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)3.0); double r1 = vx - vy; double r2 = ((unsigned long long)18446744073709551615ULL) - ((double)3.0); static double r3 = ((unsigned long long)18446744073709551615ULL) - ((double)3.0);
      printf("852: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); double r1 = vx - vy; double r2 = ((double)1e-40) - ((unsigned long long)18446744073709551615ULL); static double r3 = ((double)1e-40) - ((unsigned long long)18446744073709551615ULL);
      printf("853: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)1e-40); double r1 = vx - vy; double r2 = ((unsigned long long)18446744073709551615ULL) - ((double)1e-40); static double r3 = ((unsigned long long)18446744073709551615ULL) - ((double)1e-40);
      printf("854: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); double r1 = vx - vy; double r2 = ((double)1e10) - ((unsigned long long)12345678901234567ULL); static double r3 = ((double)1e10) - ((unsigned long long)12345678901234567ULL);
      printf("855: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile double vy = ((double)1e10); double r1 = vx - vy; double r2 = ((unsigned long long)12345678901234567ULL) - ((double)1e10); static double r3 = ((unsigned long long)12345678901234567ULL) - ((double)1e10);
      printf("856: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-5); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); double r1 = vx * vy; double r2 = ((double)1e-5) * ((unsigned long long)12345678901234567ULL); static double r3 = ((double)1e-5) * ((unsigned long long)12345678901234567ULL);
      printf("857: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile double vy = ((double)1e-5); double r1 = vx * vy; double r2 = ((unsigned long long)12345678901234567ULL) * ((double)1e-5); static double r3 = ((unsigned long long)12345678901234567ULL) * ((double)1e-5);
      printf("858: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); double r1 = vx * vy; double r2 = ((double)0.3333333333333333) * ((unsigned long long)18446744073709551615ULL); static double r3 = ((double)0.3333333333333333) * ((unsigned long long)18446744073709551615ULL);
      printf("859: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)0.3333333333333333); double r1 = vx * vy; double r2 = ((unsigned long long)18446744073709551615ULL) * ((double)0.3333333333333333); static double r3 = ((unsigned long long)18446744073709551615ULL) * ((double)0.3333333333333333);
      printf("860: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); double r1 = vx * vy; double r2 = ((double)1.0) * ((unsigned long long)18446744073709551615ULL); static double r3 = ((double)1.0) * ((unsigned long long)18446744073709551615ULL);
      printf("861: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)1.0); double r1 = vx * vy; double r2 = ((unsigned long long)18446744073709551615ULL) * ((double)1.0); static double r3 = ((unsigned long long)18446744073709551615ULL) * ((double)1.0);
      printf("862: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)7.0); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); double r1 = vx * vy; double r2 = ((double)7.0) * ((unsigned long long)18446744073709551615ULL); static double r3 = ((double)7.0) * ((unsigned long long)18446744073709551615ULL);
      printf("863: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)7.0); double r1 = vx * vy; double r2 = ((unsigned long long)18446744073709551615ULL) * ((double)7.0); static double r3 = ((unsigned long long)18446744073709551615ULL) * ((double)7.0);
      printf("864: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); double r1 = vx / vy; double r2 = ((double)3.0) / ((unsigned long long)12345678901234567ULL); static double r3 = ((double)3.0) / ((unsigned long long)12345678901234567ULL);
      printf("865: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile double vy = ((double)3.0); double r1 = vx / vy; double r2 = ((unsigned long long)12345678901234567ULL) / ((double)3.0); static double r3 = ((unsigned long long)12345678901234567ULL) / ((double)3.0);
      printf("866: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-5); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); double r1 = vx / vy; double r2 = ((double)1e-5) / ((unsigned long long)18446744073709551615ULL); static double r3 = ((double)1e-5) / ((unsigned long long)18446744073709551615ULL);
      printf("867: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)1e-5); double r1 = vx / vy; double r2 = ((unsigned long long)18446744073709551615ULL) / ((double)1e-5); static double r3 = ((unsigned long long)18446744073709551615ULL) / ((double)1e-5);
      printf("868: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); double r1 = vx / vy; double r2 = ((double)3.0) / ((unsigned long long)18446744073709551615ULL); static double r3 = ((double)3.0) / ((unsigned long long)18446744073709551615ULL);
      printf("869: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)3.0); double r1 = vx / vy; double r2 = ((unsigned long long)18446744073709551615ULL) / ((double)3.0); static double r3 = ((unsigned long long)18446744073709551615ULL) / ((double)3.0);
      printf("870: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-0.1); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); double r1 = vx / vy; double r2 = ((double)-0.1) / ((unsigned long long)12345678901234567ULL); static double r3 = ((double)-0.1) / ((unsigned long long)12345678901234567ULL);
      printf("871: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile double vy = ((double)-0.1); double r1 = vx / vy; double r2 = ((unsigned long long)12345678901234567ULL) / ((double)-0.1); static double r3 = ((unsigned long long)12345678901234567ULL) / ((double)-0.1);
      printf("872: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)123456.789); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx < vy; int r2 = ((double)123456.789) < ((unsigned long long)18446744073709551615ULL); static int r3 = ((double)123456.789) < ((unsigned long long)18446744073709551615ULL);
      printf("873: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)123456.789); int r1 = vx < vy; int r2 = ((unsigned long long)18446744073709551615ULL) < ((double)123456.789); static int r3 = ((unsigned long long)18446744073709551615ULL) < ((double)123456.789);
      printf("874: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx < vy; int r2 = ((double)0.3333333333333333) < ((unsigned long long)18446744073709551615ULL); static int r3 = ((double)0.3333333333333333) < ((unsigned long long)18446744073709551615ULL);
      printf("875: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)0.3333333333333333); int r1 = vx < vy; int r2 = ((unsigned long long)18446744073709551615ULL) < ((double)0.3333333333333333); static int r3 = ((unsigned long long)18446744073709551615ULL) < ((double)0.3333333333333333);
      printf("876: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)3.0); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx < vy; int r2 = ((double)3.0) < ((unsigned long long)18446744073709551615ULL); static int r3 = ((double)3.0) < ((unsigned long long)18446744073709551615ULL);
      printf("877: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)3.0); int r1 = vx < vy; int r2 = ((unsigned long long)18446744073709551615ULL) < ((double)3.0); static int r3 = ((unsigned long long)18446744073709551615ULL) < ((double)3.0);
      printf("878: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx < vy; int r2 = ((double)0.3333333333333333) < ((unsigned long long)12345678901234567ULL); static int r3 = ((double)0.3333333333333333) < ((unsigned long long)12345678901234567ULL);
      printf("879: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile double vy = ((double)0.3333333333333333); int r1 = vx < vy; int r2 = ((unsigned long long)12345678901234567ULL) < ((double)0.3333333333333333); static int r3 = ((unsigned long long)12345678901234567ULL) < ((double)0.3333333333333333);
      printf("880: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx == vy; int r2 = ((double)0.3333333333333333) == ((unsigned long long)18446744073709551615ULL); static int r3 = ((double)0.3333333333333333) == ((unsigned long long)18446744073709551615ULL);
      printf("881: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)0.3333333333333333); int r1 = vx == vy; int r2 = ((unsigned long long)18446744073709551615ULL) == ((double)0.3333333333333333); static int r3 = ((unsigned long long)18446744073709551615ULL) == ((double)0.3333333333333333);
      printf("882: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx == vy; int r2 = ((double)0.3333333333333333) == ((unsigned long long)12345678901234567ULL); static int r3 = ((double)0.3333333333333333) == ((unsigned long long)12345678901234567ULL);
      printf("883: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile double vy = ((double)0.3333333333333333); int r1 = vx == vy; int r2 = ((unsigned long long)12345678901234567ULL) == ((double)0.3333333333333333); static int r3 = ((unsigned long long)12345678901234567ULL) == ((double)0.3333333333333333);
      printf("884: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx == vy; int r2 = ((double)123456.789) == ((unsigned long long)18446744073709551615ULL); static int r3 = ((double)123456.789) == ((unsigned long long)18446744073709551615ULL);
      printf("885: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)123456.789); int r1 = vx == vy; int r2 = ((unsigned long long)18446744073709551615ULL) == ((double)123456.789); static int r3 = ((unsigned long long)18446744073709551615ULL) == ((double)123456.789);
      printf("886: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e30); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx == vy; int r2 = ((double)1e30) == ((unsigned long long)18446744073709551615ULL); static int r3 = ((double)1e30) == ((unsigned long long)18446744073709551615ULL);
      printf("887: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)1e30); int r1 = vx == vy; int r2 = ((unsigned long long)18446744073709551615ULL) == ((double)1e30); static int r3 = ((unsigned long long)18446744073709551615ULL) == ((double)1e30);
      printf("888: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-2.5); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx >= vy; int r2 = ((double)-2.5) >= ((unsigned long long)18446744073709551615ULL); static int r3 = ((double)-2.5) >= ((unsigned long long)18446744073709551615ULL);
      printf("889: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile double vy = ((double)-2.5); int r1 = vx >= vy; int r2 = ((unsigned long long)18446744073709551615ULL) >= ((double)-2.5); static int r3 = ((unsigned long long)18446744073709551615ULL) >= ((double)-2.5);
      printf("890: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e10); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx >= vy; int r2 = ((double)1e10) >= ((unsigned long long)12345678901234567ULL); static int r3 = ((double)1e10) >= ((unsigned long long)12345678901234567ULL);
      printf("891: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile double vy = ((double)1e10); int r1 = vx >= vy; int r2 = ((unsigned long long)12345678901234567ULL) >= ((double)1e10); static int r3 = ((unsigned long long)12345678901234567ULL) >= ((double)1e10);
      printf("892: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)16777217.0); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx >= vy; int r2 = ((double)16777217.0) >= ((unsigned long long)12345678901234567ULL); static int r3 = ((double)16777217.0) >= ((unsigned long long)12345678901234567ULL);
      printf("893: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile double vy = ((double)16777217.0); int r1 = vx >= vy; int r2 = ((unsigned long long)12345678901234567ULL) >= ((double)16777217.0); static int r3 = ((unsigned long long)12345678901234567ULL) >= ((double)16777217.0);
      printf("894: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)16777217.0); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx >= vy; int r2 = ((double)16777217.0) >= ((unsigned long long)12345678901234567ULL); static int r3 = ((double)16777217.0) >= ((unsigned long long)12345678901234567ULL);
      printf("895: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile double vy = ((double)16777217.0); int r1 = vx >= vy; int r2 = ((unsigned long long)12345678901234567ULL) >= ((double)16777217.0); static int r3 = ((unsigned long long)12345678901234567ULL) >= ((double)16777217.0);
      printf("896: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e10); volatile short vy = ((short)(short)-3); double r1 = vx + vy; double r2 = ((double)1e10) + ((short)(short)-3); static double r3 = ((double)1e10) + ((short)(short)-3);
      printf("897: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)1e10); double r1 = vx + vy; double r2 = ((short)(short)-3) + ((double)1e10); static double r3 = ((short)(short)-3) + ((double)1e10);
      printf("898: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile short vy = ((short)(short)-3); double r1 = vx + vy; double r2 = ((double)1e-40) + ((short)(short)-3); static double r3 = ((double)1e-40) + ((short)(short)-3);
      printf("899: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)1e-40); double r1 = vx + vy; double r2 = ((short)(short)-3) + ((double)1e-40); static double r3 = ((short)(short)-3) + ((double)1e-40);
      printf("900: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)123456.789); volatile short vy = ((short)(short)-3); double r1 = vx + vy; double r2 = ((double)123456.789) + ((short)(short)-3); static double r3 = ((double)123456.789) + ((short)(short)-3);
      printf("901: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)123456.789); double r1 = vx + vy; double r2 = ((short)(short)-3) + ((double)123456.789); static double r3 = ((short)(short)-3) + ((double)123456.789);
      printf("902: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile short vy = ((short)(short)-3); double r1 = vx + vy; double r2 = ((double)3.0) + ((short)(short)-3); static double r3 = ((double)3.0) + ((short)(short)-3);
      printf("903: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)3.0); double r1 = vx + vy; double r2 = ((short)(short)-3) + ((double)3.0); static double r3 = ((short)(short)-3) + ((double)3.0);
      printf("904: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)16777217.0); volatile short vy = ((short)(short)-3); double r1 = vx - vy; double r2 = ((double)16777217.0) - ((short)(short)-3); static double r3 = ((double)16777217.0) - ((short)(short)-3);
      printf("905: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)16777217.0); double r1 = vx - vy; double r2 = ((short)(short)-3) - ((double)16777217.0); static double r3 = ((short)(short)-3) - ((double)16777217.0);
      printf("906: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile short vy = ((short)(short)-3); double r1 = vx - vy; double r2 = ((double)1e-40) - ((short)(short)-3); static double r3 = ((double)1e-40) - ((short)(short)-3);
      printf("907: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)1e-40); double r1 = vx - vy; double r2 = ((short)(short)-3) - ((double)1e-40); static double r3 = ((short)(short)-3) - ((double)1e-40);
      printf("908: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile short vy = ((short)(short)-3); double r1 = vx - vy; double r2 = ((double)1e-40) - ((short)(short)-3); static double r3 = ((double)1e-40) - ((short)(short)-3);
      printf("909: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)1e-40); double r1 = vx - vy; double r2 = ((short)(short)-3) - ((double)1e-40); static double r3 = ((short)(short)-3) - ((double)1e-40);
      printf("910: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.1); volatile short vy = ((short)(short)-3); double r1 = vx - vy; double r2 = ((double)0.1) - ((short)(short)-3); static double r3 = ((double)0.1) - ((short)(short)-3);
      printf("911: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)0.1); double r1 = vx - vy; double r2 = ((short)(short)-3) - ((double)0.1); static double r3 = ((short)(short)-3) - ((double)0.1);
      printf("912: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-0.1); volatile short vy = ((short)(short)-3); double r1 = vx * vy; double r2 = ((double)-0.1) * ((short)(short)-3); static double r3 = ((double)-0.1) * ((short)(short)-3);
      printf("913: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)-0.1); double r1 = vx * vy; double r2 = ((short)(short)-3) * ((double)-0.1); static double r3 = ((short)(short)-3) * ((double)-0.1);
      printf("914: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-5); volatile short vy = ((short)(short)-3); double r1 = vx * vy; double r2 = ((double)1e-5) * ((short)(short)-3); static double r3 = ((double)1e-5) * ((short)(short)-3);
      printf("915: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)1e-5); double r1 = vx * vy; double r2 = ((short)(short)-3) * ((double)1e-5); static double r3 = ((short)(short)-3) * ((double)1e-5);
      printf("916: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-2.5); volatile short vy = ((short)(short)-3); double r1 = vx * vy; double r2 = ((double)-2.5) * ((short)(short)-3); static double r3 = ((double)-2.5) * ((short)(short)-3);
      printf("917: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)-2.5); double r1 = vx * vy; double r2 = ((short)(short)-3) * ((double)-2.5); static double r3 = ((short)(short)-3) * ((double)-2.5);
      printf("918: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-2.5); volatile short vy = ((short)(short)-3); double r1 = vx * vy; double r2 = ((double)-2.5) * ((short)(short)-3); static double r3 = ((double)-2.5) * ((short)(short)-3);
      printf("919: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)-2.5); double r1 = vx * vy; double r2 = ((short)(short)-3) * ((double)-2.5); static double r3 = ((short)(short)-3) * ((double)-2.5);
      printf("920: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-0.1); volatile short vy = ((short)(short)-3); double r1 = vx / vy; double r2 = ((double)-0.1) / ((short)(short)-3); static double r3 = ((double)-0.1) / ((short)(short)-3);
      printf("921: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)-0.1); double r1 = vx / vy; double r2 = ((short)(short)-3) / ((double)-0.1); static double r3 = ((short)(short)-3) / ((double)-0.1);
      printf("922: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1.0); volatile short vy = ((short)(short)-3); double r1 = vx / vy; double r2 = ((double)1.0) / ((short)(short)-3); static double r3 = ((double)1.0) / ((short)(short)-3);
      printf("923: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)1.0); double r1 = vx / vy; double r2 = ((short)(short)-3) / ((double)1.0); static double r3 = ((short)(short)-3) / ((double)1.0);
      printf("924: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)16777217.0); volatile short vy = ((short)(short)-3); double r1 = vx / vy; double r2 = ((double)16777217.0) / ((short)(short)-3); static double r3 = ((double)16777217.0) / ((short)(short)-3);
      printf("925: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)16777217.0); double r1 = vx / vy; double r2 = ((short)(short)-3) / ((double)16777217.0); static double r3 = ((short)(short)-3) / ((double)16777217.0);
      printf("926: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile short vy = ((short)(short)-3); double r1 = vx / vy; double r2 = ((double)1e10) / ((short)(short)-3); static double r3 = ((double)1e10) / ((short)(short)-3);
      printf("927: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)1e10); double r1 = vx / vy; double r2 = ((short)(short)-3) / ((double)1e10); static double r3 = ((short)(short)-3) / ((double)1e10);
      printf("928: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-2.5); volatile short vy = ((short)(short)-3); int r1 = vx < vy; int r2 = ((double)-2.5) < ((short)(short)-3); static int r3 = ((double)-2.5) < ((short)(short)-3);
      printf("929: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)-2.5); int r1 = vx < vy; int r2 = ((short)(short)-3) < ((double)-2.5); static int r3 = ((short)(short)-3) < ((double)-2.5);
      printf("930: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.1); volatile short vy = ((short)(short)-3); int r1 = vx < vy; int r2 = ((double)0.1) < ((short)(short)-3); static int r3 = ((double)0.1) < ((short)(short)-3);
      printf("931: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)0.1); int r1 = vx < vy; int r2 = ((short)(short)-3) < ((double)0.1); static int r3 = ((short)(short)-3) < ((double)0.1);
      printf("932: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-2.5); volatile short vy = ((short)(short)-3); int r1 = vx < vy; int r2 = ((double)-2.5) < ((short)(short)-3); static int r3 = ((double)-2.5) < ((short)(short)-3);
      printf("933: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)-2.5); int r1 = vx < vy; int r2 = ((short)(short)-3) < ((double)-2.5); static int r3 = ((short)(short)-3) < ((double)-2.5);
      printf("934: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e30); volatile short vy = ((short)(short)-3); int r1 = vx < vy; int r2 = ((double)1e30) < ((short)(short)-3); static int r3 = ((double)1e30) < ((short)(short)-3);
      printf("935: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)1e30); int r1 = vx < vy; int r2 = ((short)(short)-3) < ((double)1e30); static int r3 = ((short)(short)-3) < ((double)1e30);
      printf("936: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1.0); volatile short vy = ((short)(short)-3); int r1 = vx == vy; int r2 = ((double)1.0) == ((short)(short)-3); static int r3 = ((double)1.0) == ((short)(short)-3);
      printf("937: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)1.0); int r1 = vx == vy; int r2 = ((short)(short)-3) == ((double)1.0); static int r3 = ((short)(short)-3) == ((double)1.0);
      printf("938: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.1); volatile short vy = ((short)(short)-3); int r1 = vx == vy; int r2 = ((double)0.1) == ((short)(short)-3); static int r3 = ((double)0.1) == ((short)(short)-3);
      printf("939: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)0.1); int r1 = vx == vy; int r2 = ((short)(short)-3) == ((double)0.1); static int r3 = ((short)(short)-3) == ((double)0.1);
      printf("940: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile short vy = ((short)(short)-3); int r1 = vx == vy; int r2 = ((double)123456.789) == ((short)(short)-3); static int r3 = ((double)123456.789) == ((short)(short)-3);
      printf("941: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)123456.789); int r1 = vx == vy; int r2 = ((short)(short)-3) == ((double)123456.789); static int r3 = ((short)(short)-3) == ((double)123456.789);
      printf("942: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile short vy = ((short)(short)-3); int r1 = vx == vy; int r2 = ((double)123456.789) == ((short)(short)-3); static int r3 = ((double)123456.789) == ((short)(short)-3);
      printf("943: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)123456.789); int r1 = vx == vy; int r2 = ((short)(short)-3) == ((double)123456.789); static int r3 = ((short)(short)-3) == ((double)123456.789);
      printf("944: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e30); volatile short vy = ((short)(short)-3); int r1 = vx >= vy; int r2 = ((double)1e30) >= ((short)(short)-3); static int r3 = ((double)1e30) >= ((short)(short)-3);
      printf("945: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)1e30); int r1 = vx >= vy; int r2 = ((short)(short)-3) >= ((double)1e30); static int r3 = ((short)(short)-3) >= ((double)1e30);
      printf("946: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)3.0); volatile short vy = ((short)(short)-3); int r1 = vx >= vy; int r2 = ((double)3.0) >= ((short)(short)-3); static int r3 = ((double)3.0) >= ((short)(short)-3);
      printf("947: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)3.0); int r1 = vx >= vy; int r2 = ((short)(short)-3) >= ((double)3.0); static int r3 = ((short)(short)-3) >= ((double)3.0);
      printf("948: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.1); volatile short vy = ((short)(short)-3); int r1 = vx >= vy; int r2 = ((double)0.1) >= ((short)(short)-3); static int r3 = ((double)0.1) >= ((short)(short)-3);
      printf("949: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)0.1); int r1 = vx >= vy; int r2 = ((short)(short)-3) >= ((double)0.1); static int r3 = ((short)(short)-3) >= ((double)0.1);
      printf("950: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile short vy = ((short)(short)-3); int r1 = vx >= vy; int r2 = ((double)0.3333333333333333) >= ((short)(short)-3); static int r3 = ((double)0.3333333333333333) >= ((short)(short)-3);
      printf("951: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile double vy = ((double)0.3333333333333333); int r1 = vx >= vy; int r2 = ((short)(short)-3) >= ((double)0.3333333333333333); static int r3 = ((short)(short)-3) >= ((double)0.3333333333333333);
      printf("952: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx + vy; double r2 = ((double)0.3333333333333333) + ((unsigned char)(unsigned char)200); static double r3 = ((double)0.3333333333333333) + ((unsigned char)(unsigned char)200);
      printf("953: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)0.3333333333333333); double r1 = vx + vy; double r2 = ((unsigned char)(unsigned char)200) + ((double)0.3333333333333333); static double r3 = ((unsigned char)(unsigned char)200) + ((double)0.3333333333333333);
      printf("954: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)16777217.0); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx + vy; double r2 = ((double)16777217.0) + ((unsigned char)(unsigned char)200); static double r3 = ((double)16777217.0) + ((unsigned char)(unsigned char)200);
      printf("955: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)16777217.0); double r1 = vx + vy; double r2 = ((unsigned char)(unsigned char)200) + ((double)16777217.0); static double r3 = ((unsigned char)(unsigned char)200) + ((double)16777217.0);
      printf("956: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)3.0); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx + vy; double r2 = ((double)3.0) + ((unsigned char)(unsigned char)200); static double r3 = ((double)3.0) + ((unsigned char)(unsigned char)200);
      printf("957: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)3.0); double r1 = vx + vy; double r2 = ((unsigned char)(unsigned char)200) + ((double)3.0); static double r3 = ((unsigned char)(unsigned char)200) + ((double)3.0);
      printf("958: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)7.0); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx + vy; double r2 = ((double)7.0) + ((unsigned char)(unsigned char)200); static double r3 = ((double)7.0) + ((unsigned char)(unsigned char)200);
      printf("959: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)7.0); double r1 = vx + vy; double r2 = ((unsigned char)(unsigned char)200) + ((double)7.0); static double r3 = ((unsigned char)(unsigned char)200) + ((double)7.0);
      printf("960: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)7.0); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx - vy; double r2 = ((double)7.0) - ((unsigned char)(unsigned char)200); static double r3 = ((double)7.0) - ((unsigned char)(unsigned char)200);
      printf("961: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)7.0); double r1 = vx - vy; double r2 = ((unsigned char)(unsigned char)200) - ((double)7.0); static double r3 = ((unsigned char)(unsigned char)200) - ((double)7.0);
      printf("962: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-5); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx - vy; double r2 = ((double)1e-5) - ((unsigned char)(unsigned char)200); static double r3 = ((double)1e-5) - ((unsigned char)(unsigned char)200);
      printf("963: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)1e-5); double r1 = vx - vy; double r2 = ((unsigned char)(unsigned char)200) - ((double)1e-5); static double r3 = ((unsigned char)(unsigned char)200) - ((double)1e-5);
      printf("964: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-2.5); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx - vy; double r2 = ((double)-2.5) - ((unsigned char)(unsigned char)200); static double r3 = ((double)-2.5) - ((unsigned char)(unsigned char)200);
      printf("965: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)-2.5); double r1 = vx - vy; double r2 = ((unsigned char)(unsigned char)200) - ((double)-2.5); static double r3 = ((unsigned char)(unsigned char)200) - ((double)-2.5);
      printf("966: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx - vy; double r2 = ((double)1e-40) - ((unsigned char)(unsigned char)200); static double r3 = ((double)1e-40) - ((unsigned char)(unsigned char)200);
      printf("967: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)1e-40); double r1 = vx - vy; double r2 = ((unsigned char)(unsigned char)200) - ((double)1e-40); static double r3 = ((unsigned char)(unsigned char)200) - ((double)1e-40);
      printf("968: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx * vy; double r2 = ((double)1e10) * ((unsigned char)(unsigned char)200); static double r3 = ((double)1e10) * ((unsigned char)(unsigned char)200);
      printf("969: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)1e10); double r1 = vx * vy; double r2 = ((unsigned char)(unsigned char)200) * ((double)1e10); static double r3 = ((unsigned char)(unsigned char)200) * ((double)1e10);
      printf("970: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e-40); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx * vy; double r2 = ((double)1e-40) * ((unsigned char)(unsigned char)200); static double r3 = ((double)1e-40) * ((unsigned char)(unsigned char)200);
      printf("971: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)1e-40); double r1 = vx * vy; double r2 = ((unsigned char)(unsigned char)200) * ((double)1e-40); static double r3 = ((unsigned char)(unsigned char)200) * ((double)1e-40);
      printf("972: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-0.1); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx * vy; double r2 = ((double)-0.1) * ((unsigned char)(unsigned char)200); static double r3 = ((double)-0.1) * ((unsigned char)(unsigned char)200);
      printf("973: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)-0.1); double r1 = vx * vy; double r2 = ((unsigned char)(unsigned char)200) * ((double)-0.1); static double r3 = ((unsigned char)(unsigned char)200) * ((double)-0.1);
      printf("974: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-2.5); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx * vy; double r2 = ((double)-2.5) * ((unsigned char)(unsigned char)200); static double r3 = ((double)-2.5) * ((unsigned char)(unsigned char)200);
      printf("975: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)-2.5); double r1 = vx * vy; double r2 = ((unsigned char)(unsigned char)200) * ((double)-2.5); static double r3 = ((unsigned char)(unsigned char)200) * ((double)-2.5);
      printf("976: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.1); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx / vy; double r2 = ((double)0.1) / ((unsigned char)(unsigned char)200); static double r3 = ((double)0.1) / ((unsigned char)(unsigned char)200);
      printf("977: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)0.1); double r1 = vx / vy; double r2 = ((unsigned char)(unsigned char)200) / ((double)0.1); static double r3 = ((unsigned char)(unsigned char)200) / ((double)0.1);
      printf("978: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)1e10); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx / vy; double r2 = ((double)1e10) / ((unsigned char)(unsigned char)200); static double r3 = ((double)1e10) / ((unsigned char)(unsigned char)200);
      printf("979: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)1e10); double r1 = vx / vy; double r2 = ((unsigned char)(unsigned char)200) / ((double)1e10); static double r3 = ((unsigned char)(unsigned char)200) / ((double)1e10);
      printf("980: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx / vy; double r2 = ((double)0.3333333333333333) / ((unsigned char)(unsigned char)200); static double r3 = ((double)0.3333333333333333) / ((unsigned char)(unsigned char)200);
      printf("981: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)0.3333333333333333); double r1 = vx / vy; double r2 = ((unsigned char)(unsigned char)200) / ((double)0.3333333333333333); static double r3 = ((unsigned char)(unsigned char)200) / ((double)0.3333333333333333);
      printf("982: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)123456.789); volatile unsigned char vy = ((unsigned char)(unsigned char)200); double r1 = vx / vy; double r2 = ((double)123456.789) / ((unsigned char)(unsigned char)200); static double r3 = ((double)123456.789) / ((unsigned char)(unsigned char)200);
      printf("983: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)123456.789); double r1 = vx / vy; double r2 = ((unsigned char)(unsigned char)200) / ((double)123456.789); static double r3 = ((unsigned char)(unsigned char)200) / ((double)123456.789);
      printf("984: %.17g %.17g %.17g\n", (double)r1, (double)r2, (double)r3); }
    { volatile double vx = ((double)-0.1); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx < vy; int r2 = ((double)-0.1) < ((unsigned char)(unsigned char)200); static int r3 = ((double)-0.1) < ((unsigned char)(unsigned char)200);
      printf("985: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)-0.1); int r1 = vx < vy; int r2 = ((unsigned char)(unsigned char)200) < ((double)-0.1); static int r3 = ((unsigned char)(unsigned char)200) < ((double)-0.1);
      printf("986: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-5); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx < vy; int r2 = ((double)1e-5) < ((unsigned char)(unsigned char)200); static int r3 = ((double)1e-5) < ((unsigned char)(unsigned char)200);
      printf("987: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)1e-5); int r1 = vx < vy; int r2 = ((unsigned char)(unsigned char)200) < ((double)1e-5); static int r3 = ((unsigned char)(unsigned char)200) < ((double)1e-5);
      printf("988: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx < vy; int r2 = ((double)0.3333333333333333) < ((unsigned char)(unsigned char)200); static int r3 = ((double)0.3333333333333333) < ((unsigned char)(unsigned char)200);
      printf("989: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)0.3333333333333333); int r1 = vx < vy; int r2 = ((unsigned char)(unsigned char)200) < ((double)0.3333333333333333); static int r3 = ((unsigned char)(unsigned char)200) < ((double)0.3333333333333333);
      printf("990: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-40); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx < vy; int r2 = ((double)1e-40) < ((unsigned char)(unsigned char)200); static int r3 = ((double)1e-40) < ((unsigned char)(unsigned char)200);
      printf("991: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)1e-40); int r1 = vx < vy; int r2 = ((unsigned char)(unsigned char)200) < ((double)1e-40); static int r3 = ((unsigned char)(unsigned char)200) < ((double)1e-40);
      printf("992: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)3.0); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx == vy; int r2 = ((double)3.0) == ((unsigned char)(unsigned char)200); static int r3 = ((double)3.0) == ((unsigned char)(unsigned char)200);
      printf("993: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)3.0); int r1 = vx == vy; int r2 = ((unsigned char)(unsigned char)200) == ((double)3.0); static int r3 = ((unsigned char)(unsigned char)200) == ((double)3.0);
      printf("994: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx == vy; int r2 = ((double)0.3333333333333333) == ((unsigned char)(unsigned char)200); static int r3 = ((double)0.3333333333333333) == ((unsigned char)(unsigned char)200);
      printf("995: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)0.3333333333333333); int r1 = vx == vy; int r2 = ((unsigned char)(unsigned char)200) == ((double)0.3333333333333333); static int r3 = ((unsigned char)(unsigned char)200) == ((double)0.3333333333333333);
      printf("996: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e-40); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx == vy; int r2 = ((double)1e-40) == ((unsigned char)(unsigned char)200); static int r3 = ((double)1e-40) == ((unsigned char)(unsigned char)200);
      printf("997: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)1e-40); int r1 = vx == vy; int r2 = ((unsigned char)(unsigned char)200) == ((double)1e-40); static int r3 = ((unsigned char)(unsigned char)200) == ((double)1e-40);
      printf("998: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.3333333333333333); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx == vy; int r2 = ((double)0.3333333333333333) == ((unsigned char)(unsigned char)200); static int r3 = ((double)0.3333333333333333) == ((unsigned char)(unsigned char)200);
      printf("999: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)0.3333333333333333); int r1 = vx == vy; int r2 = ((unsigned char)(unsigned char)200) == ((double)0.3333333333333333); static int r3 = ((unsigned char)(unsigned char)200) == ((double)0.3333333333333333);
      printf("1000: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)7.0); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx >= vy; int r2 = ((double)7.0) >= ((unsigned char)(unsigned char)200); static int r3 = ((double)7.0) >= ((unsigned char)(unsigned char)200);
      printf("1001: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)7.0); int r1 = vx >= vy; int r2 = ((unsigned char)(unsigned char)200) >= ((double)7.0); static int r3 = ((unsigned char)(unsigned char)200) >= ((double)7.0);
      printf("1002: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-0.1); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx >= vy; int r2 = ((double)-0.1) >= ((unsigned char)(unsigned char)200); static int r3 = ((double)-0.1) >= ((unsigned char)(unsigned char)200);
      printf("1003: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)-0.1); int r1 = vx >= vy; int r2 = ((unsigned char)(unsigned char)200) >= ((double)-0.1); static int r3 = ((unsigned char)(unsigned char)200) >= ((double)-0.1);
      printf("1004: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)16777217.0); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx >= vy; int r2 = ((double)16777217.0) >= ((unsigned char)(unsigned char)200); static int r3 = ((double)16777217.0) >= ((unsigned char)(unsigned char)200);
      printf("1005: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)16777217.0); int r1 = vx >= vy; int r2 = ((unsigned char)(unsigned char)200) >= ((double)16777217.0); static int r3 = ((unsigned char)(unsigned char)200) >= ((double)16777217.0);
      printf("1006: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)16777217.0); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx >= vy; int r2 = ((double)16777217.0) >= ((unsigned char)(unsigned char)200); static int r3 = ((double)16777217.0) >= ((unsigned char)(unsigned char)200);
      printf("1007: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile double vy = ((double)16777217.0); int r1 = vx >= vy; int r2 = ((unsigned char)(unsigned char)200) >= ((double)16777217.0); static int r3 = ((unsigned char)(unsigned char)200) >= ((double)16777217.0);
      printf("1008: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e10L); volatile float vy = ((float)3.0f); long double r1 = vx + vy; long double r2 = ((long double)1e10L) + ((float)3.0f); static long double r3 = ((long double)1e10L) + ((float)3.0f);
      printf("1009: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)3.0f); volatile long double vy = ((long double)1e10L); long double r1 = vx + vy; long double r2 = ((float)3.0f) + ((long double)1e10L); static long double r3 = ((float)3.0f) + ((long double)1e10L);
      printf("1010: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile float vy = ((float)0.1f); long double r1 = vx + vy; long double r2 = ((long double)1e10L) + ((float)0.1f); static long double r3 = ((long double)1e10L) + ((float)0.1f);
      printf("1011: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)0.1f); volatile long double vy = ((long double)1e10L); long double r1 = vx + vy; long double r2 = ((float)0.1f) + ((long double)1e10L); static long double r3 = ((float)0.1f) + ((long double)1e10L);
      printf("1012: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile float vy = ((float)-0.1f); long double r1 = vx + vy; long double r2 = ((long double)1.0L) + ((float)-0.1f); static long double r3 = ((long double)1.0L) + ((float)-0.1f);
      printf("1013: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)-0.1f); volatile long double vy = ((long double)1.0L); long double r1 = vx + vy; long double r2 = ((float)-0.1f) + ((long double)1.0L); static long double r3 = ((float)-0.1f) + ((long double)1.0L);
      printf("1014: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile float vy = ((float)16777217.0f); long double r1 = vx + vy; long double r2 = ((long double)1e-5L) + ((float)16777217.0f); static long double r3 = ((long double)1e-5L) + ((float)16777217.0f);
      printf("1015: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)16777217.0f); volatile long double vy = ((long double)1e-5L); long double r1 = vx + vy; long double r2 = ((float)16777217.0f) + ((long double)1e-5L); static long double r3 = ((float)16777217.0f) + ((long double)1e-5L);
      printf("1016: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile float vy = ((float)-2.5f); long double r1 = vx - vy; long double r2 = ((long double)1.0L) - ((float)-2.5f); static long double r3 = ((long double)1.0L) - ((float)-2.5f);
      printf("1017: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)-2.5f); volatile long double vy = ((long double)1.0L); long double r1 = vx - vy; long double r2 = ((float)-2.5f) - ((long double)1.0L); static long double r3 = ((float)-2.5f) - ((long double)1.0L);
      printf("1018: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile float vy = ((float)3.0f); long double r1 = vx - vy; long double r2 = ((long double)1e30L) - ((float)3.0f); static long double r3 = ((long double)1e30L) - ((float)3.0f);
      printf("1019: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)3.0f); volatile long double vy = ((long double)1e30L); long double r1 = vx - vy; long double r2 = ((float)3.0f) - ((long double)1e30L); static long double r3 = ((float)3.0f) - ((long double)1e30L);
      printf("1020: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile float vy = ((float)0.1f); long double r1 = vx - vy; long double r2 = ((long double)1e10L) - ((float)0.1f); static long double r3 = ((long double)1e10L) - ((float)0.1f);
      printf("1021: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)0.1f); volatile long double vy = ((long double)1e10L); long double r1 = vx - vy; long double r2 = ((float)0.1f) - ((long double)1e10L); static long double r3 = ((float)0.1f) - ((long double)1e10L);
      printf("1022: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile float vy = ((float)1e10f); long double r1 = vx - vy; long double r2 = ((long double)-2.5L) - ((float)1e10f); static long double r3 = ((long double)-2.5L) - ((float)1e10f);
      printf("1023: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)1e10f); volatile long double vy = ((long double)-2.5L); long double r1 = vx - vy; long double r2 = ((float)1e10f) - ((long double)-2.5L); static long double r3 = ((float)1e10f) - ((long double)-2.5L);
      printf("1024: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile float vy = ((float)1.0f); long double r1 = vx * vy; long double r2 = ((long double)1e-5L) * ((float)1.0f); static long double r3 = ((long double)1e-5L) * ((float)1.0f);
      printf("1025: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)1.0f); volatile long double vy = ((long double)1e-5L); long double r1 = vx * vy; long double r2 = ((float)1.0f) * ((long double)1e-5L); static long double r3 = ((float)1.0f) * ((long double)1e-5L);
      printf("1026: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile float vy = ((float)1e-5f); long double r1 = vx * vy; long double r2 = ((long double)123456.789L) * ((float)1e-5f); static long double r3 = ((long double)123456.789L) * ((float)1e-5f);
      printf("1027: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)1e-5f); volatile long double vy = ((long double)123456.789L); long double r1 = vx * vy; long double r2 = ((float)1e-5f) * ((long double)123456.789L); static long double r3 = ((float)1e-5f) * ((long double)123456.789L);
      printf("1028: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)3.0L); volatile float vy = ((float)1e10f); long double r1 = vx * vy; long double r2 = ((long double)3.0L) * ((float)1e10f); static long double r3 = ((long double)3.0L) * ((float)1e10f);
      printf("1029: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)1e10f); volatile long double vy = ((long double)3.0L); long double r1 = vx * vy; long double r2 = ((float)1e10f) * ((long double)3.0L); static long double r3 = ((float)1e10f) * ((long double)3.0L);
      printf("1030: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile float vy = ((float)3.0f); long double r1 = vx * vy; long double r2 = ((long double)0.1L) * ((float)3.0f); static long double r3 = ((long double)0.1L) * ((float)3.0f);
      printf("1031: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)3.0f); volatile long double vy = ((long double)0.1L); long double r1 = vx * vy; long double r2 = ((float)3.0f) * ((long double)0.1L); static long double r3 = ((float)3.0f) * ((long double)0.1L);
      printf("1032: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile float vy = ((float)1e-5f); long double r1 = vx / vy; long double r2 = ((long double)-2.5L) / ((float)1e-5f); static long double r3 = ((long double)-2.5L) / ((float)1e-5f);
      printf("1033: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)1e-5f); volatile long double vy = ((long double)-2.5L); long double r1 = vx / vy; long double r2 = ((float)1e-5f) / ((long double)-2.5L); static long double r3 = ((float)1e-5f) / ((long double)-2.5L);
      printf("1034: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile float vy = ((float)7.0f); long double r1 = vx / vy; long double r2 = ((long double)1.0L) / ((float)7.0f); static long double r3 = ((long double)1.0L) / ((float)7.0f);
      printf("1035: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)7.0f); volatile long double vy = ((long double)1.0L); long double r1 = vx / vy; long double r2 = ((float)7.0f) / ((long double)1.0L); static long double r3 = ((float)7.0f) / ((long double)1.0L);
      printf("1036: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile float vy = ((float)3.0f); long double r1 = vx / vy; long double r2 = ((long double)16777217.0L) / ((float)3.0f); static long double r3 = ((long double)16777217.0L) / ((float)3.0f);
      printf("1037: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)3.0f); volatile long double vy = ((long double)16777217.0L); long double r1 = vx / vy; long double r2 = ((float)3.0f) / ((long double)16777217.0L); static long double r3 = ((float)3.0f) / ((long double)16777217.0L);
      printf("1038: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)7.0L); volatile float vy = ((float)3.0f); long double r1 = vx / vy; long double r2 = ((long double)7.0L) / ((float)3.0f); static long double r3 = ((long double)7.0L) / ((float)3.0f);
      printf("1039: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile float vx = ((float)3.0f); volatile long double vy = ((long double)7.0L); long double r1 = vx / vy; long double r2 = ((float)3.0f) / ((long double)7.0L); static long double r3 = ((float)3.0f) / ((long double)7.0L);
      printf("1040: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile float vy = ((float)7.0f); int r1 = vx < vy; int r2 = ((long double)0.1L) < ((float)7.0f); static int r3 = ((long double)0.1L) < ((float)7.0f);
      printf("1041: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)7.0f); volatile long double vy = ((long double)0.1L); int r1 = vx < vy; int r2 = ((float)7.0f) < ((long double)0.1L); static int r3 = ((float)7.0f) < ((long double)0.1L);
      printf("1042: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e10L); volatile float vy = ((float)1e-40f); int r1 = vx < vy; int r2 = ((long double)1e10L) < ((float)1e-40f); static int r3 = ((long double)1e10L) < ((float)1e-40f);
      printf("1043: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-40f); volatile long double vy = ((long double)1e10L); int r1 = vx < vy; int r2 = ((float)1e-40f) < ((long double)1e10L); static int r3 = ((float)1e-40f) < ((long double)1e10L);
      printf("1044: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e10L); volatile float vy = ((float)0.3333333333333333f); int r1 = vx < vy; int r2 = ((long double)1e10L) < ((float)0.3333333333333333f); static int r3 = ((long double)1e10L) < ((float)0.3333333333333333f);
      printf("1045: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)0.3333333333333333f); volatile long double vy = ((long double)1e10L); int r1 = vx < vy; int r2 = ((float)0.3333333333333333f) < ((long double)1e10L); static int r3 = ((float)0.3333333333333333f) < ((long double)1e10L);
      printf("1046: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1.0L); volatile float vy = ((float)-2.5f); int r1 = vx < vy; int r2 = ((long double)1.0L) < ((float)-2.5f); static int r3 = ((long double)1.0L) < ((float)-2.5f);
      printf("1047: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-2.5f); volatile long double vy = ((long double)1.0L); int r1 = vx < vy; int r2 = ((float)-2.5f) < ((long double)1.0L); static int r3 = ((float)-2.5f) < ((long double)1.0L);
      printf("1048: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile float vy = ((float)1e-5f); int r1 = vx == vy; int r2 = ((long double)123456.789L) == ((float)1e-5f); static int r3 = ((long double)123456.789L) == ((float)1e-5f);
      printf("1049: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-5f); volatile long double vy = ((long double)123456.789L); int r1 = vx == vy; int r2 = ((float)1e-5f) == ((long double)123456.789L); static int r3 = ((float)1e-5f) == ((long double)123456.789L);
      printf("1050: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile float vy = ((float)1.0f); int r1 = vx == vy; int r2 = ((long double)-0.1L) == ((float)1.0f); static int r3 = ((long double)-0.1L) == ((float)1.0f);
      printf("1051: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1.0f); volatile long double vy = ((long double)-0.1L); int r1 = vx == vy; int r2 = ((float)1.0f) == ((long double)-0.1L); static int r3 = ((float)1.0f) == ((long double)-0.1L);
      printf("1052: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e10L); volatile float vy = ((float)-0.1f); int r1 = vx == vy; int r2 = ((long double)1e10L) == ((float)-0.1f); static int r3 = ((long double)1e10L) == ((float)-0.1f);
      printf("1053: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)-0.1f); volatile long double vy = ((long double)1e10L); int r1 = vx == vy; int r2 = ((float)-0.1f) == ((long double)1e10L); static int r3 = ((float)-0.1f) == ((long double)1e10L);
      printf("1054: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile float vy = ((float)3.0f); int r1 = vx == vy; int r2 = ((long double)1e-40L) == ((float)3.0f); static int r3 = ((long double)1e-40L) == ((float)3.0f);
      printf("1055: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)3.0f); volatile long double vy = ((long double)1e-40L); int r1 = vx == vy; int r2 = ((float)3.0f) == ((long double)1e-40L); static int r3 = ((float)3.0f) == ((long double)1e-40L);
      printf("1056: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1.0L); volatile float vy = ((float)1e-40f); int r1 = vx >= vy; int r2 = ((long double)1.0L) >= ((float)1e-40f); static int r3 = ((long double)1.0L) >= ((float)1e-40f);
      printf("1057: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e-40f); volatile long double vy = ((long double)1.0L); int r1 = vx >= vy; int r2 = ((float)1e-40f) >= ((long double)1.0L); static int r3 = ((float)1e-40f) >= ((long double)1.0L);
      printf("1058: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile float vy = ((float)7.0f); int r1 = vx >= vy; int r2 = ((long double)16777217.0L) >= ((float)7.0f); static int r3 = ((long double)16777217.0L) >= ((float)7.0f);
      printf("1059: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)7.0f); volatile long double vy = ((long double)16777217.0L); int r1 = vx >= vy; int r2 = ((float)7.0f) >= ((long double)16777217.0L); static int r3 = ((float)7.0f) >= ((long double)16777217.0L);
      printf("1060: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile float vy = ((float)1e10f); int r1 = vx >= vy; int r2 = ((long double)123456.789L) >= ((float)1e10f); static int r3 = ((long double)123456.789L) >= ((float)1e10f);
      printf("1061: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)1e10f); volatile long double vy = ((long double)123456.789L); int r1 = vx >= vy; int r2 = ((float)1e10f) >= ((long double)123456.789L); static int r3 = ((float)1e10f) >= ((long double)123456.789L);
      printf("1062: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile float vy = ((float)7.0f); int r1 = vx >= vy; int r2 = ((long double)123456.789L) >= ((float)7.0f); static int r3 = ((long double)123456.789L) >= ((float)7.0f);
      printf("1063: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile float vx = ((float)7.0f); volatile long double vy = ((long double)123456.789L); int r1 = vx >= vy; int r2 = ((float)7.0f) >= ((long double)123456.789L); static int r3 = ((float)7.0f) >= ((long double)123456.789L);
      printf("1064: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile double vy = ((double)1e30); long double r1 = vx + vy; long double r2 = ((long double)-0.1L) + ((double)1e30); static long double r3 = ((long double)-0.1L) + ((double)1e30);
      printf("1065: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e30); volatile long double vy = ((long double)-0.1L); long double r1 = vx + vy; long double r2 = ((double)1e30) + ((long double)-0.1L); static long double r3 = ((double)1e30) + ((long double)-0.1L);
      printf("1066: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)3.0L); volatile double vy = ((double)1e10); long double r1 = vx + vy; long double r2 = ((long double)3.0L) + ((double)1e10); static long double r3 = ((long double)3.0L) + ((double)1e10);
      printf("1067: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e10); volatile long double vy = ((long double)3.0L); long double r1 = vx + vy; long double r2 = ((double)1e10) + ((long double)3.0L); static long double r3 = ((double)1e10) + ((long double)3.0L);
      printf("1068: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)7.0L); volatile double vy = ((double)3.0); long double r1 = vx + vy; long double r2 = ((long double)7.0L) + ((double)3.0); static long double r3 = ((long double)7.0L) + ((double)3.0);
      printf("1069: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)3.0); volatile long double vy = ((long double)7.0L); long double r1 = vx + vy; long double r2 = ((double)3.0) + ((long double)7.0L); static long double r3 = ((double)3.0) + ((long double)7.0L);
      printf("1070: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile double vy = ((double)7.0); long double r1 = vx + vy; long double r2 = ((long double)1.0L) + ((double)7.0); static long double r3 = ((long double)1.0L) + ((double)7.0);
      printf("1071: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)7.0); volatile long double vy = ((long double)1.0L); long double r1 = vx + vy; long double r2 = ((double)7.0) + ((long double)1.0L); static long double r3 = ((double)7.0) + ((long double)1.0L);
      printf("1072: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile double vy = ((double)1e-5); long double r1 = vx - vy; long double r2 = ((long double)-0.1L) - ((double)1e-5); static long double r3 = ((long double)-0.1L) - ((double)1e-5);
      printf("1073: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e-5); volatile long double vy = ((long double)-0.1L); long double r1 = vx - vy; long double r2 = ((double)1e-5) - ((long double)-0.1L); static long double r3 = ((double)1e-5) - ((long double)-0.1L);
      printf("1074: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile double vy = ((double)16777217.0); long double r1 = vx - vy; long double r2 = ((long double)1e-5L) - ((double)16777217.0); static long double r3 = ((long double)1e-5L) - ((double)16777217.0);
      printf("1075: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)16777217.0); volatile long double vy = ((long double)1e-5L); long double r1 = vx - vy; long double r2 = ((double)16777217.0) - ((long double)1e-5L); static long double r3 = ((double)16777217.0) - ((long double)1e-5L);
      printf("1076: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile double vy = ((double)0.1); long double r1 = vx - vy; long double r2 = ((long double)16777217.0L) - ((double)0.1); static long double r3 = ((long double)16777217.0L) - ((double)0.1);
      printf("1077: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)0.1); volatile long double vy = ((long double)16777217.0L); long double r1 = vx - vy; long double r2 = ((double)0.1) - ((long double)16777217.0L); static long double r3 = ((double)0.1) - ((long double)16777217.0L);
      printf("1078: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile double vy = ((double)1e30); long double r1 = vx - vy; long double r2 = ((long double)1e30L) - ((double)1e30); static long double r3 = ((long double)1e30L) - ((double)1e30);
      printf("1079: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e30); volatile long double vy = ((long double)1e30L); long double r1 = vx - vy; long double r2 = ((double)1e30) - ((long double)1e30L); static long double r3 = ((double)1e30) - ((long double)1e30L);
      printf("1080: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile double vy = ((double)1e10); long double r1 = vx * vy; long double r2 = ((long double)16777217.0L) * ((double)1e10); static long double r3 = ((long double)16777217.0L) * ((double)1e10);
      printf("1081: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e10); volatile long double vy = ((long double)16777217.0L); long double r1 = vx * vy; long double r2 = ((double)1e10) * ((long double)16777217.0L); static long double r3 = ((double)1e10) * ((long double)16777217.0L);
      printf("1082: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile double vy = ((double)1e30); long double r1 = vx * vy; long double r2 = ((long double)123456.789L) * ((double)1e30); static long double r3 = ((long double)123456.789L) * ((double)1e30);
      printf("1083: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e30); volatile long double vy = ((long double)123456.789L); long double r1 = vx * vy; long double r2 = ((double)1e30) * ((long double)123456.789L); static long double r3 = ((double)1e30) * ((long double)123456.789L);
      printf("1084: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile double vy = ((double)1e10); long double r1 = vx * vy; long double r2 = ((long double)1.0L) * ((double)1e10); static long double r3 = ((long double)1.0L) * ((double)1e10);
      printf("1085: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e10); volatile long double vy = ((long double)1.0L); long double r1 = vx * vy; long double r2 = ((double)1e10) * ((long double)1.0L); static long double r3 = ((double)1e10) * ((long double)1.0L);
      printf("1086: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)3.0L); volatile double vy = ((double)3.0); long double r1 = vx * vy; long double r2 = ((long double)3.0L) * ((double)3.0); static long double r3 = ((long double)3.0L) * ((double)3.0);
      printf("1087: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)3.0); volatile long double vy = ((long double)3.0L); long double r1 = vx * vy; long double r2 = ((double)3.0) * ((long double)3.0L); static long double r3 = ((double)3.0) * ((long double)3.0L);
      printf("1088: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile double vy = ((double)1e-40); long double r1 = vx / vy; long double r2 = ((long double)1e10L) / ((double)1e-40); static long double r3 = ((long double)1e10L) / ((double)1e-40);
      printf("1089: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1e-40); volatile long double vy = ((long double)1e10L); long double r1 = vx / vy; long double r2 = ((double)1e-40) / ((long double)1e10L); static long double r3 = ((double)1e-40) / ((long double)1e10L);
      printf("1090: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile double vy = ((double)1.0); long double r1 = vx / vy; long double r2 = ((long double)0.1L) / ((double)1.0); static long double r3 = ((long double)0.1L) / ((double)1.0);
      printf("1091: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)1.0); volatile long double vy = ((long double)0.1L); long double r1 = vx / vy; long double r2 = ((double)1.0) / ((long double)0.1L); static long double r3 = ((double)1.0) / ((long double)0.1L);
      printf("1092: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile double vy = ((double)0.1); long double r1 = vx / vy; long double r2 = ((long double)1.0L) / ((double)0.1); static long double r3 = ((long double)1.0L) / ((double)0.1);
      printf("1093: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)0.1); volatile long double vy = ((long double)1.0L); long double r1 = vx / vy; long double r2 = ((double)0.1) / ((long double)1.0L); static long double r3 = ((double)0.1) / ((long double)1.0L);
      printf("1094: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile double vy = ((double)123456.789); long double r1 = vx / vy; long double r2 = ((long double)1e-5L) / ((double)123456.789); static long double r3 = ((long double)1e-5L) / ((double)123456.789);
      printf("1095: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile double vx = ((double)123456.789); volatile long double vy = ((long double)1e-5L); long double r1 = vx / vy; long double r2 = ((double)123456.789) / ((long double)1e-5L); static long double r3 = ((double)123456.789) / ((long double)1e-5L);
      printf("1096: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile double vy = ((double)0.1); int r1 = vx < vy; int r2 = ((long double)0.3333333333333333L) < ((double)0.1); static int r3 = ((long double)0.3333333333333333L) < ((double)0.1);
      printf("1097: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)0.1); volatile long double vy = ((long double)0.3333333333333333L); int r1 = vx < vy; int r2 = ((double)0.1) < ((long double)0.3333333333333333L); static int r3 = ((double)0.1) < ((long double)0.3333333333333333L);
      printf("1098: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.1L); volatile double vy = ((double)1.0); int r1 = vx < vy; int r2 = ((long double)0.1L) < ((double)1.0); static int r3 = ((long double)0.1L) < ((double)1.0);
      printf("1099: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1.0); volatile long double vy = ((long double)0.1L); int r1 = vx < vy; int r2 = ((double)1.0) < ((long double)0.1L); static int r3 = ((double)1.0) < ((long double)0.1L);
      printf("1100: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile double vy = ((double)123456.789); int r1 = vx < vy; int r2 = ((long double)16777217.0L) < ((double)123456.789); static int r3 = ((long double)16777217.0L) < ((double)123456.789);
      printf("1101: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile long double vy = ((long double)16777217.0L); int r1 = vx < vy; int r2 = ((double)123456.789) < ((long double)16777217.0L); static int r3 = ((double)123456.789) < ((long double)16777217.0L);
      printf("1102: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile double vy = ((double)-2.5); int r1 = vx < vy; int r2 = ((long double)-2.5L) < ((double)-2.5); static int r3 = ((long double)-2.5L) < ((double)-2.5);
      printf("1103: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-2.5); volatile long double vy = ((long double)-2.5L); int r1 = vx < vy; int r2 = ((double)-2.5) < ((long double)-2.5L); static int r3 = ((double)-2.5) < ((long double)-2.5L);
      printf("1104: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile double vy = ((double)123456.789); int r1 = vx == vy; int r2 = ((long double)1e-5L) == ((double)123456.789); static int r3 = ((long double)1e-5L) == ((double)123456.789);
      printf("1105: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile long double vy = ((long double)1e-5L); int r1 = vx == vy; int r2 = ((double)123456.789) == ((long double)1e-5L); static int r3 = ((double)123456.789) == ((long double)1e-5L);
      printf("1106: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile double vy = ((double)1e10); int r1 = vx == vy; int r2 = ((long double)16777217.0L) == ((double)1e10); static int r3 = ((long double)16777217.0L) == ((double)1e10);
      printf("1107: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)1e10); volatile long double vy = ((long double)16777217.0L); int r1 = vx == vy; int r2 = ((double)1e10) == ((long double)16777217.0L); static int r3 = ((double)1e10) == ((long double)16777217.0L);
      printf("1108: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)7.0L); volatile double vy = ((double)7.0); int r1 = vx == vy; int r2 = ((long double)7.0L) == ((double)7.0); static int r3 = ((long double)7.0L) == ((double)7.0);
      printf("1109: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)7.0); volatile long double vy = ((long double)7.0L); int r1 = vx == vy; int r2 = ((double)7.0) == ((long double)7.0L); static int r3 = ((double)7.0) == ((long double)7.0L);
      printf("1110: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile double vy = ((double)16777217.0); int r1 = vx == vy; int r2 = ((long double)1e-40L) == ((double)16777217.0); static int r3 = ((long double)1e-40L) == ((double)16777217.0);
      printf("1111: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)16777217.0); volatile long double vy = ((long double)1e-40L); int r1 = vx == vy; int r2 = ((double)16777217.0) == ((long double)1e-40L); static int r3 = ((double)16777217.0) == ((long double)1e-40L);
      printf("1112: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e30L); volatile double vy = ((double)123456.789); int r1 = vx >= vy; int r2 = ((long double)1e30L) >= ((double)123456.789); static int r3 = ((long double)1e30L) >= ((double)123456.789);
      printf("1113: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)123456.789); volatile long double vy = ((long double)1e30L); int r1 = vx >= vy; int r2 = ((double)123456.789) >= ((long double)1e30L); static int r3 = ((double)123456.789) >= ((long double)1e30L);
      printf("1114: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1.0L); volatile double vy = ((double)-0.1); int r1 = vx >= vy; int r2 = ((long double)1.0L) >= ((double)-0.1); static int r3 = ((long double)1.0L) >= ((double)-0.1);
      printf("1115: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)-0.1); volatile long double vy = ((long double)1.0L); int r1 = vx >= vy; int r2 = ((double)-0.1) >= ((long double)1.0L); static int r3 = ((double)-0.1) >= ((long double)1.0L);
      printf("1116: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile double vy = ((double)3.0); int r1 = vx >= vy; int r2 = ((long double)0.3333333333333333L) >= ((double)3.0); static int r3 = ((long double)0.3333333333333333L) >= ((double)3.0);
      printf("1117: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)3.0); volatile long double vy = ((long double)0.3333333333333333L); int r1 = vx >= vy; int r2 = ((double)3.0) >= ((long double)0.3333333333333333L); static int r3 = ((double)3.0) >= ((long double)0.3333333333333333L);
      printf("1118: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile double vy = ((double)16777217.0); int r1 = vx >= vy; int r2 = ((long double)1e-5L) >= ((double)16777217.0); static int r3 = ((long double)1e-5L) >= ((double)16777217.0);
      printf("1119: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile double vx = ((double)16777217.0); volatile long double vy = ((long double)1e-5L); int r1 = vx >= vy; int r2 = ((double)16777217.0) >= ((long double)1e-5L); static int r3 = ((double)16777217.0) >= ((long double)1e-5L);
      printf("1120: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e10L); volatile long double vy = ((long double)123456.789L); long double r1 = vx + vy; long double r2 = ((long double)1e10L) + ((long double)123456.789L); static long double r3 = ((long double)1e10L) + ((long double)123456.789L);
      printf("1121: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile long double vy = ((long double)1e10L); long double r1 = vx + vy; long double r2 = ((long double)123456.789L) + ((long double)1e10L); static long double r3 = ((long double)123456.789L) + ((long double)1e10L);
      printf("1122: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile long double vy = ((long double)1e-40L); long double r1 = vx + vy; long double r2 = ((long double)123456.789L) + ((long double)1e-40L); static long double r3 = ((long double)123456.789L) + ((long double)1e-40L);
      printf("1123: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile long double vy = ((long double)123456.789L); long double r1 = vx + vy; long double r2 = ((long double)1e-40L) + ((long double)123456.789L); static long double r3 = ((long double)1e-40L) + ((long double)123456.789L);
      printf("1124: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile long double vy = ((long double)1e-5L); long double r1 = vx + vy; long double r2 = ((long double)1e10L) + ((long double)1e-5L); static long double r3 = ((long double)1e10L) + ((long double)1e-5L);
      printf("1125: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile long double vy = ((long double)1e10L); long double r1 = vx + vy; long double r2 = ((long double)1e-5L) + ((long double)1e10L); static long double r3 = ((long double)1e-5L) + ((long double)1e10L);
      printf("1126: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)3.0L); volatile long double vy = ((long double)1e30L); long double r1 = vx + vy; long double r2 = ((long double)3.0L) + ((long double)1e30L); static long double r3 = ((long double)3.0L) + ((long double)1e30L);
      printf("1127: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile long double vy = ((long double)3.0L); long double r1 = vx + vy; long double r2 = ((long double)1e30L) + ((long double)3.0L); static long double r3 = ((long double)1e30L) + ((long double)3.0L);
      printf("1128: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile long double vy = ((long double)1e10L); long double r1 = vx - vy; long double r2 = ((long double)1e-5L) - ((long double)1e10L); static long double r3 = ((long double)1e-5L) - ((long double)1e10L);
      printf("1129: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile long double vy = ((long double)1e-5L); long double r1 = vx - vy; long double r2 = ((long double)1e10L) - ((long double)1e-5L); static long double r3 = ((long double)1e10L) - ((long double)1e-5L);
      printf("1130: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)3.0L); volatile long double vy = ((long double)1e30L); long double r1 = vx - vy; long double r2 = ((long double)3.0L) - ((long double)1e30L); static long double r3 = ((long double)3.0L) - ((long double)1e30L);
      printf("1131: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile long double vy = ((long double)3.0L); long double r1 = vx - vy; long double r2 = ((long double)1e30L) - ((long double)3.0L); static long double r3 = ((long double)1e30L) - ((long double)3.0L);
      printf("1132: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile long double vy = ((long double)1e30L); long double r1 = vx - vy; long double r2 = ((long double)0.1L) - ((long double)1e30L); static long double r3 = ((long double)0.1L) - ((long double)1e30L);
      printf("1133: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile long double vy = ((long double)0.1L); long double r1 = vx - vy; long double r2 = ((long double)1e30L) - ((long double)0.1L); static long double r3 = ((long double)1e30L) - ((long double)0.1L);
      printf("1134: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile long double vy = ((long double)-2.5L); long double r1 = vx - vy; long double r2 = ((long double)1.0L) - ((long double)-2.5L); static long double r3 = ((long double)1.0L) - ((long double)-2.5L);
      printf("1135: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile long double vy = ((long double)1.0L); long double r1 = vx - vy; long double r2 = ((long double)-2.5L) - ((long double)1.0L); static long double r3 = ((long double)-2.5L) - ((long double)1.0L);
      printf("1136: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile long double vy = ((long double)0.1L); long double r1 = vx * vy; long double r2 = ((long double)1e30L) * ((long double)0.1L); static long double r3 = ((long double)1e30L) * ((long double)0.1L);
      printf("1137: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile long double vy = ((long double)1e30L); long double r1 = vx * vy; long double r2 = ((long double)0.1L) * ((long double)1e30L); static long double r3 = ((long double)0.1L) * ((long double)1e30L);
      printf("1138: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile long double vy = ((long double)123456.789L); long double r1 = vx * vy; long double r2 = ((long double)1e-5L) * ((long double)123456.789L); static long double r3 = ((long double)1e-5L) * ((long double)123456.789L);
      printf("1139: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile long double vy = ((long double)1e-5L); long double r1 = vx * vy; long double r2 = ((long double)123456.789L) * ((long double)1e-5L); static long double r3 = ((long double)123456.789L) * ((long double)1e-5L);
      printf("1140: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile long double vy = ((long double)1e-5L); long double r1 = vx * vy; long double r2 = ((long double)16777217.0L) * ((long double)1e-5L); static long double r3 = ((long double)16777217.0L) * ((long double)1e-5L);
      printf("1141: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile long double vy = ((long double)16777217.0L); long double r1 = vx * vy; long double r2 = ((long double)1e-5L) * ((long double)16777217.0L); static long double r3 = ((long double)1e-5L) * ((long double)16777217.0L);
      printf("1142: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile long double vy = ((long double)123456.789L); long double r1 = vx * vy; long double r2 = ((long double)1e10L) * ((long double)123456.789L); static long double r3 = ((long double)1e10L) * ((long double)123456.789L);
      printf("1143: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile long double vy = ((long double)1e10L); long double r1 = vx * vy; long double r2 = ((long double)123456.789L) * ((long double)1e10L); static long double r3 = ((long double)123456.789L) * ((long double)1e10L);
      printf("1144: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile long double vy = ((long double)-0.1L); long double r1 = vx / vy; long double r2 = ((long double)-0.1L) / ((long double)-0.1L); static long double r3 = ((long double)-0.1L) / ((long double)-0.1L);
      printf("1145: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile long double vy = ((long double)-0.1L); long double r1 = vx / vy; long double r2 = ((long double)-0.1L) / ((long double)-0.1L); static long double r3 = ((long double)-0.1L) / ((long double)-0.1L);
      printf("1146: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile long double vy = ((long double)7.0L); long double r1 = vx / vy; long double r2 = ((long double)1e30L) / ((long double)7.0L); static long double r3 = ((long double)1e30L) / ((long double)7.0L);
      printf("1147: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)7.0L); volatile long double vy = ((long double)1e30L); long double r1 = vx / vy; long double r2 = ((long double)7.0L) / ((long double)1e30L); static long double r3 = ((long double)7.0L) / ((long double)1e30L);
      printf("1148: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile long double vy = ((long double)123456.789L); long double r1 = vx / vy; long double r2 = ((long double)1e30L) / ((long double)123456.789L); static long double r3 = ((long double)1e30L) / ((long double)123456.789L);
      printf("1149: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile long double vy = ((long double)1e30L); long double r1 = vx / vy; long double r2 = ((long double)123456.789L) / ((long double)1e30L); static long double r3 = ((long double)123456.789L) / ((long double)1e30L);
      printf("1150: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)3.0L); volatile long double vy = ((long double)1e30L); long double r1 = vx / vy; long double r2 = ((long double)3.0L) / ((long double)1e30L); static long double r3 = ((long double)3.0L) / ((long double)1e30L);
      printf("1151: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile long double vy = ((long double)3.0L); long double r1 = vx / vy; long double r2 = ((long double)1e30L) / ((long double)3.0L); static long double r3 = ((long double)1e30L) / ((long double)3.0L);
      printf("1152: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile long double vy = ((long double)1e10L); int r1 = vx < vy; int r2 = ((long double)0.1L) < ((long double)1e10L); static int r3 = ((long double)0.1L) < ((long double)1e10L);
      printf("1153: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e10L); volatile long double vy = ((long double)0.1L); int r1 = vx < vy; int r2 = ((long double)1e10L) < ((long double)0.1L); static int r3 = ((long double)1e10L) < ((long double)0.1L);
      printf("1154: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.1L); volatile long double vy = ((long double)1e-40L); int r1 = vx < vy; int r2 = ((long double)0.1L) < ((long double)1e-40L); static int r3 = ((long double)0.1L) < ((long double)1e-40L);
      printf("1155: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile long double vy = ((long double)0.1L); int r1 = vx < vy; int r2 = ((long double)1e-40L) < ((long double)0.1L); static int r3 = ((long double)1e-40L) < ((long double)0.1L);
      printf("1156: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e30L); volatile long double vy = ((long double)1e-40L); int r1 = vx < vy; int r2 = ((long double)1e30L) < ((long double)1e-40L); static int r3 = ((long double)1e30L) < ((long double)1e-40L);
      printf("1157: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile long double vy = ((long double)1e30L); int r1 = vx < vy; int r2 = ((long double)1e-40L) < ((long double)1e30L); static int r3 = ((long double)1e-40L) < ((long double)1e30L);
      printf("1158: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile long double vy = ((long double)1e10L); int r1 = vx < vy; int r2 = ((long double)123456.789L) < ((long double)1e10L); static int r3 = ((long double)123456.789L) < ((long double)1e10L);
      printf("1159: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e10L); volatile long double vy = ((long double)123456.789L); int r1 = vx < vy; int r2 = ((long double)1e10L) < ((long double)123456.789L); static int r3 = ((long double)1e10L) < ((long double)123456.789L);
      printf("1160: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e30L); volatile long double vy = ((long double)1e-5L); int r1 = vx == vy; int r2 = ((long double)1e30L) == ((long double)1e-5L); static int r3 = ((long double)1e30L) == ((long double)1e-5L);
      printf("1161: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile long double vy = ((long double)1e30L); int r1 = vx == vy; int r2 = ((long double)1e-5L) == ((long double)1e30L); static int r3 = ((long double)1e-5L) == ((long double)1e30L);
      printf("1162: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)7.0L); volatile long double vy = ((long double)16777217.0L); int r1 = vx == vy; int r2 = ((long double)7.0L) == ((long double)16777217.0L); static int r3 = ((long double)7.0L) == ((long double)16777217.0L);
      printf("1163: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile long double vy = ((long double)7.0L); int r1 = vx == vy; int r2 = ((long double)16777217.0L) == ((long double)7.0L); static int r3 = ((long double)16777217.0L) == ((long double)7.0L);
      printf("1164: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile long double vy = ((long double)1e10L); int r1 = vx == vy; int r2 = ((long double)0.3333333333333333L) == ((long double)1e10L); static int r3 = ((long double)0.3333333333333333L) == ((long double)1e10L);
      printf("1165: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e10L); volatile long double vy = ((long double)0.3333333333333333L); int r1 = vx == vy; int r2 = ((long double)1e10L) == ((long double)0.3333333333333333L); static int r3 = ((long double)1e10L) == ((long double)0.3333333333333333L);
      printf("1166: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1.0L); volatile long double vy = ((long double)1e-5L); int r1 = vx == vy; int r2 = ((long double)1.0L) == ((long double)1e-5L); static int r3 = ((long double)1.0L) == ((long double)1e-5L);
      printf("1167: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile long double vy = ((long double)1.0L); int r1 = vx == vy; int r2 = ((long double)1e-5L) == ((long double)1.0L); static int r3 = ((long double)1e-5L) == ((long double)1.0L);
      printf("1168: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)7.0L); volatile long double vy = ((long double)1e30L); int r1 = vx >= vy; int r2 = ((long double)7.0L) >= ((long double)1e30L); static int r3 = ((long double)7.0L) >= ((long double)1e30L);
      printf("1169: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e30L); volatile long double vy = ((long double)7.0L); int r1 = vx >= vy; int r2 = ((long double)1e30L) >= ((long double)7.0L); static int r3 = ((long double)1e30L) >= ((long double)7.0L);
      printf("1170: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile long double vy = ((long double)0.1L); int r1 = vx >= vy; int r2 = ((long double)1e-5L) >= ((long double)0.1L); static int r3 = ((long double)1e-5L) >= ((long double)0.1L);
      printf("1171: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.1L); volatile long double vy = ((long double)1e-5L); int r1 = vx >= vy; int r2 = ((long double)0.1L) >= ((long double)1e-5L); static int r3 = ((long double)0.1L) >= ((long double)1e-5L);
      printf("1172: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile long double vy = ((long double)1e-5L); int r1 = vx >= vy; int r2 = ((long double)0.3333333333333333L) >= ((long double)1e-5L); static int r3 = ((long double)0.3333333333333333L) >= ((long double)1e-5L);
      printf("1173: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile long double vy = ((long double)0.3333333333333333L); int r1 = vx >= vy; int r2 = ((long double)1e-5L) >= ((long double)0.3333333333333333L); static int r3 = ((long double)1e-5L) >= ((long double)0.3333333333333333L);
      printf("1174: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile long double vy = ((long double)3.0L); int r1 = vx >= vy; int r2 = ((long double)1e-40L) >= ((long double)3.0L); static int r3 = ((long double)1e-40L) >= ((long double)3.0L);
      printf("1175: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)3.0L); volatile long double vy = ((long double)1e-40L); int r1 = vx >= vy; int r2 = ((long double)3.0L) >= ((long double)1e-40L); static int r3 = ((long double)3.0L) >= ((long double)1e-40L);
      printf("1176: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.1L); volatile int vy = ((int)3); long double r1 = vx + vy; long double r2 = ((long double)0.1L) + ((int)3); static long double r3 = ((long double)0.1L) + ((int)3);
      printf("1177: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)3); volatile long double vy = ((long double)0.1L); long double r1 = vx + vy; long double r2 = ((int)3) + ((long double)0.1L); static long double r3 = ((int)3) + ((long double)0.1L);
      printf("1178: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile int vy = ((int)100000); long double r1 = vx + vy; long double r2 = ((long double)123456.789L) + ((int)100000); static long double r3 = ((long double)123456.789L) + ((int)100000);
      printf("1179: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)100000); volatile long double vy = ((long double)123456.789L); long double r1 = vx + vy; long double r2 = ((int)100000) + ((long double)123456.789L); static long double r3 = ((int)100000) + ((long double)123456.789L);
      printf("1180: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile int vy = ((int)100000); long double r1 = vx + vy; long double r2 = ((long double)1.0L) + ((int)100000); static long double r3 = ((long double)1.0L) + ((int)100000);
      printf("1181: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)100000); volatile long double vy = ((long double)1.0L); long double r1 = vx + vy; long double r2 = ((int)100000) + ((long double)1.0L); static long double r3 = ((int)100000) + ((long double)1.0L);
      printf("1182: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile int vy = ((int)100000); long double r1 = vx + vy; long double r2 = ((long double)16777217.0L) + ((int)100000); static long double r3 = ((long double)16777217.0L) + ((int)100000);
      printf("1183: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)100000); volatile long double vy = ((long double)16777217.0L); long double r1 = vx + vy; long double r2 = ((int)100000) + ((long double)16777217.0L); static long double r3 = ((int)100000) + ((long double)16777217.0L);
      printf("1184: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile int vy = ((int)3); long double r1 = vx - vy; long double r2 = ((long double)0.1L) - ((int)3); static long double r3 = ((long double)0.1L) - ((int)3);
      printf("1185: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)3); volatile long double vy = ((long double)0.1L); long double r1 = vx - vy; long double r2 = ((int)3) - ((long double)0.1L); static long double r3 = ((int)3) - ((long double)0.1L);
      printf("1186: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile int vy = ((int)3); long double r1 = vx - vy; long double r2 = ((long double)16777217.0L) - ((int)3); static long double r3 = ((long double)16777217.0L) - ((int)3);
      printf("1187: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)3); volatile long double vy = ((long double)16777217.0L); long double r1 = vx - vy; long double r2 = ((int)3) - ((long double)16777217.0L); static long double r3 = ((int)3) - ((long double)16777217.0L);
      printf("1188: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile int vy = ((int)100000); long double r1 = vx - vy; long double r2 = ((long double)0.3333333333333333L) - ((int)100000); static long double r3 = ((long double)0.3333333333333333L) - ((int)100000);
      printf("1189: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)100000); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx - vy; long double r2 = ((int)100000) - ((long double)0.3333333333333333L); static long double r3 = ((int)100000) - ((long double)0.3333333333333333L);
      printf("1190: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile int vy = ((int)-7); long double r1 = vx - vy; long double r2 = ((long double)1e-5L) - ((int)-7); static long double r3 = ((long double)1e-5L) - ((int)-7);
      printf("1191: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)-7); volatile long double vy = ((long double)1e-5L); long double r1 = vx - vy; long double r2 = ((int)-7) - ((long double)1e-5L); static long double r3 = ((int)-7) - ((long double)1e-5L);
      printf("1192: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile int vy = ((int)100000); long double r1 = vx * vy; long double r2 = ((long double)0.1L) * ((int)100000); static long double r3 = ((long double)0.1L) * ((int)100000);
      printf("1193: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)100000); volatile long double vy = ((long double)0.1L); long double r1 = vx * vy; long double r2 = ((int)100000) * ((long double)0.1L); static long double r3 = ((int)100000) * ((long double)0.1L);
      printf("1194: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile int vy = ((int)100000); long double r1 = vx * vy; long double r2 = ((long double)123456.789L) * ((int)100000); static long double r3 = ((long double)123456.789L) * ((int)100000);
      printf("1195: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)100000); volatile long double vy = ((long double)123456.789L); long double r1 = vx * vy; long double r2 = ((int)100000) * ((long double)123456.789L); static long double r3 = ((int)100000) * ((long double)123456.789L);
      printf("1196: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)3.0L); volatile int vy = ((int)-7); long double r1 = vx * vy; long double r2 = ((long double)3.0L) * ((int)-7); static long double r3 = ((long double)3.0L) * ((int)-7);
      printf("1197: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)-7); volatile long double vy = ((long double)3.0L); long double r1 = vx * vy; long double r2 = ((int)-7) * ((long double)3.0L); static long double r3 = ((int)-7) * ((long double)3.0L);
      printf("1198: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile int vy = ((int)100000); long double r1 = vx * vy; long double r2 = ((long double)-0.1L) * ((int)100000); static long double r3 = ((long double)-0.1L) * ((int)100000);
      printf("1199: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)100000); volatile long double vy = ((long double)-0.1L); long double r1 = vx * vy; long double r2 = ((int)100000) * ((long double)-0.1L); static long double r3 = ((int)100000) * ((long double)-0.1L);
      printf("1200: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile int vy = ((int)-7); long double r1 = vx / vy; long double r2 = ((long double)0.3333333333333333L) / ((int)-7); static long double r3 = ((long double)0.3333333333333333L) / ((int)-7);
      printf("1201: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)-7); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx / vy; long double r2 = ((int)-7) / ((long double)0.3333333333333333L); static long double r3 = ((int)-7) / ((long double)0.3333333333333333L);
      printf("1202: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)3.0L); volatile int vy = ((int)-7); long double r1 = vx / vy; long double r2 = ((long double)3.0L) / ((int)-7); static long double r3 = ((long double)3.0L) / ((int)-7);
      printf("1203: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)-7); volatile long double vy = ((long double)3.0L); long double r1 = vx / vy; long double r2 = ((int)-7) / ((long double)3.0L); static long double r3 = ((int)-7) / ((long double)3.0L);
      printf("1204: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile int vy = ((int)100000); long double r1 = vx / vy; long double r2 = ((long double)-2.5L) / ((int)100000); static long double r3 = ((long double)-2.5L) / ((int)100000);
      printf("1205: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)100000); volatile long double vy = ((long double)-2.5L); long double r1 = vx / vy; long double r2 = ((int)100000) / ((long double)-2.5L); static long double r3 = ((int)100000) / ((long double)-2.5L);
      printf("1206: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile int vy = ((int)3); long double r1 = vx / vy; long double r2 = ((long double)0.1L) / ((int)3); static long double r3 = ((long double)0.1L) / ((int)3);
      printf("1207: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile int vx = ((int)3); volatile long double vy = ((long double)0.1L); long double r1 = vx / vy; long double r2 = ((int)3) / ((long double)0.1L); static long double r3 = ((int)3) / ((long double)0.1L);
      printf("1208: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile int vy = ((int)100000); int r1 = vx < vy; int r2 = ((long double)1e30L) < ((int)100000); static int r3 = ((long double)1e30L) < ((int)100000);
      printf("1209: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)100000); volatile long double vy = ((long double)1e30L); int r1 = vx < vy; int r2 = ((int)100000) < ((long double)1e30L); static int r3 = ((int)100000) < ((long double)1e30L);
      printf("1210: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile int vy = ((int)-7); int r1 = vx < vy; int r2 = ((long double)1e-40L) < ((int)-7); static int r3 = ((long double)1e-40L) < ((int)-7);
      printf("1211: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile long double vy = ((long double)1e-40L); int r1 = vx < vy; int r2 = ((int)-7) < ((long double)1e-40L); static int r3 = ((int)-7) < ((long double)1e-40L);
      printf("1212: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile int vy = ((int)100000); int r1 = vx < vy; int r2 = ((long double)1e-40L) < ((int)100000); static int r3 = ((long double)1e-40L) < ((int)100000);
      printf("1213: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)100000); volatile long double vy = ((long double)1e-40L); int r1 = vx < vy; int r2 = ((int)100000) < ((long double)1e-40L); static int r3 = ((int)100000) < ((long double)1e-40L);
      printf("1214: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile int vy = ((int)-7); int r1 = vx < vy; int r2 = ((long double)1e-5L) < ((int)-7); static int r3 = ((long double)1e-5L) < ((int)-7);
      printf("1215: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile long double vy = ((long double)1e-5L); int r1 = vx < vy; int r2 = ((int)-7) < ((long double)1e-5L); static int r3 = ((int)-7) < ((long double)1e-5L);
      printf("1216: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile int vy = ((int)-7); int r1 = vx == vy; int r2 = ((long double)0.3333333333333333L) == ((int)-7); static int r3 = ((long double)0.3333333333333333L) == ((int)-7);
      printf("1217: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile long double vy = ((long double)0.3333333333333333L); int r1 = vx == vy; int r2 = ((int)-7) == ((long double)0.3333333333333333L); static int r3 = ((int)-7) == ((long double)0.3333333333333333L);
      printf("1218: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.1L); volatile int vy = ((int)3); int r1 = vx == vy; int r2 = ((long double)0.1L) == ((int)3); static int r3 = ((long double)0.1L) == ((int)3);
      printf("1219: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)3); volatile long double vy = ((long double)0.1L); int r1 = vx == vy; int r2 = ((int)3) == ((long double)0.1L); static int r3 = ((int)3) == ((long double)0.1L);
      printf("1220: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile int vy = ((int)3); int r1 = vx == vy; int r2 = ((long double)-2.5L) == ((int)3); static int r3 = ((long double)-2.5L) == ((int)3);
      printf("1221: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)3); volatile long double vy = ((long double)-2.5L); int r1 = vx == vy; int r2 = ((int)3) == ((long double)-2.5L); static int r3 = ((int)3) == ((long double)-2.5L);
      printf("1222: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile int vy = ((int)100000); int r1 = vx == vy; int r2 = ((long double)123456.789L) == ((int)100000); static int r3 = ((long double)123456.789L) == ((int)100000);
      printf("1223: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)100000); volatile long double vy = ((long double)123456.789L); int r1 = vx == vy; int r2 = ((int)100000) == ((long double)123456.789L); static int r3 = ((int)100000) == ((long double)123456.789L);
      printf("1224: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1.0L); volatile int vy = ((int)-7); int r1 = vx >= vy; int r2 = ((long double)1.0L) >= ((int)-7); static int r3 = ((long double)1.0L) >= ((int)-7);
      printf("1225: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile long double vy = ((long double)1.0L); int r1 = vx >= vy; int r2 = ((int)-7) >= ((long double)1.0L); static int r3 = ((int)-7) >= ((long double)1.0L);
      printf("1226: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile int vy = ((int)3); int r1 = vx >= vy; int r2 = ((long double)0.3333333333333333L) >= ((int)3); static int r3 = ((long double)0.3333333333333333L) >= ((int)3);
      printf("1227: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)3); volatile long double vy = ((long double)0.3333333333333333L); int r1 = vx >= vy; int r2 = ((int)3) >= ((long double)0.3333333333333333L); static int r3 = ((int)3) >= ((long double)0.3333333333333333L);
      printf("1228: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile int vy = ((int)-7); int r1 = vx >= vy; int r2 = ((long double)1e-5L) >= ((int)-7); static int r3 = ((long double)1e-5L) >= ((int)-7);
      printf("1229: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile long double vy = ((long double)1e-5L); int r1 = vx >= vy; int r2 = ((int)-7) >= ((long double)1e-5L); static int r3 = ((int)-7) >= ((long double)1e-5L);
      printf("1230: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile int vy = ((int)-7); int r1 = vx >= vy; int r2 = ((long double)1e-5L) >= ((int)-7); static int r3 = ((long double)1e-5L) >= ((int)-7);
      printf("1231: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile int vx = ((int)-7); volatile long double vy = ((long double)1e-5L); int r1 = vx >= vy; int r2 = ((int)-7) >= ((long double)1e-5L); static int r3 = ((int)-7) >= ((long double)1e-5L);
      printf("1232: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)7.0L); volatile unsigned vy = ((unsigned)3u); long double r1 = vx + vy; long double r2 = ((long double)7.0L) + ((unsigned)3u); static long double r3 = ((long double)7.0L) + ((unsigned)3u);
      printf("1233: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)7.0L); long double r1 = vx + vy; long double r2 = ((unsigned)3u) + ((long double)7.0L); static long double r3 = ((unsigned)3u) + ((long double)7.0L);
      printf("1234: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)3.0L); volatile unsigned vy = ((unsigned)4000000000u); long double r1 = vx + vy; long double r2 = ((long double)3.0L) + ((unsigned)4000000000u); static long double r3 = ((long double)3.0L) + ((unsigned)4000000000u);
      printf("1235: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)3.0L); long double r1 = vx + vy; long double r2 = ((unsigned)4000000000u) + ((long double)3.0L); static long double r3 = ((unsigned)4000000000u) + ((long double)3.0L);
      printf("1236: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile unsigned vy = ((unsigned)3u); long double r1 = vx + vy; long double r2 = ((long double)-0.1L) + ((unsigned)3u); static long double r3 = ((long double)-0.1L) + ((unsigned)3u);
      printf("1237: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)-0.1L); long double r1 = vx + vy; long double r2 = ((unsigned)3u) + ((long double)-0.1L); static long double r3 = ((unsigned)3u) + ((long double)-0.1L);
      printf("1238: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile unsigned vy = ((unsigned)4000000000u); long double r1 = vx + vy; long double r2 = ((long double)1e10L) + ((unsigned)4000000000u); static long double r3 = ((long double)1e10L) + ((unsigned)4000000000u);
      printf("1239: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)1e10L); long double r1 = vx + vy; long double r2 = ((unsigned)4000000000u) + ((long double)1e10L); static long double r3 = ((unsigned)4000000000u) + ((long double)1e10L);
      printf("1240: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile unsigned vy = ((unsigned)3u); long double r1 = vx - vy; long double r2 = ((long double)1e30L) - ((unsigned)3u); static long double r3 = ((long double)1e30L) - ((unsigned)3u);
      printf("1241: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)1e30L); long double r1 = vx - vy; long double r2 = ((unsigned)3u) - ((long double)1e30L); static long double r3 = ((unsigned)3u) - ((long double)1e30L);
      printf("1242: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile unsigned vy = ((unsigned)4000000000u); long double r1 = vx - vy; long double r2 = ((long double)1.0L) - ((unsigned)4000000000u); static long double r3 = ((long double)1.0L) - ((unsigned)4000000000u);
      printf("1243: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)1.0L); long double r1 = vx - vy; long double r2 = ((unsigned)4000000000u) - ((long double)1.0L); static long double r3 = ((unsigned)4000000000u) - ((long double)1.0L);
      printf("1244: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile unsigned vy = ((unsigned)3u); long double r1 = vx - vy; long double r2 = ((long double)0.3333333333333333L) - ((unsigned)3u); static long double r3 = ((long double)0.3333333333333333L) - ((unsigned)3u);
      printf("1245: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx - vy; long double r2 = ((unsigned)3u) - ((long double)0.3333333333333333L); static long double r3 = ((unsigned)3u) - ((long double)0.3333333333333333L);
      printf("1246: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile unsigned vy = ((unsigned)4000000000u); long double r1 = vx - vy; long double r2 = ((long double)-0.1L) - ((unsigned)4000000000u); static long double r3 = ((long double)-0.1L) - ((unsigned)4000000000u);
      printf("1247: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)-0.1L); long double r1 = vx - vy; long double r2 = ((unsigned)4000000000u) - ((long double)-0.1L); static long double r3 = ((unsigned)4000000000u) - ((long double)-0.1L);
      printf("1248: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile unsigned vy = ((unsigned)4000000000u); long double r1 = vx * vy; long double r2 = ((long double)1.0L) * ((unsigned)4000000000u); static long double r3 = ((long double)1.0L) * ((unsigned)4000000000u);
      printf("1249: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)1.0L); long double r1 = vx * vy; long double r2 = ((unsigned)4000000000u) * ((long double)1.0L); static long double r3 = ((unsigned)4000000000u) * ((long double)1.0L);
      printf("1250: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile unsigned vy = ((unsigned)3u); long double r1 = vx * vy; long double r2 = ((long double)123456.789L) * ((unsigned)3u); static long double r3 = ((long double)123456.789L) * ((unsigned)3u);
      printf("1251: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)123456.789L); long double r1 = vx * vy; long double r2 = ((unsigned)3u) * ((long double)123456.789L); static long double r3 = ((unsigned)3u) * ((long double)123456.789L);
      printf("1252: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile unsigned vy = ((unsigned)3u); long double r1 = vx * vy; long double r2 = ((long double)-0.1L) * ((unsigned)3u); static long double r3 = ((long double)-0.1L) * ((unsigned)3u);
      printf("1253: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)-0.1L); long double r1 = vx * vy; long double r2 = ((unsigned)3u) * ((long double)-0.1L); static long double r3 = ((unsigned)3u) * ((long double)-0.1L);
      printf("1254: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile unsigned vy = ((unsigned)4000000000u); long double r1 = vx * vy; long double r2 = ((long double)0.3333333333333333L) * ((unsigned)4000000000u); static long double r3 = ((long double)0.3333333333333333L) * ((unsigned)4000000000u);
      printf("1255: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx * vy; long double r2 = ((unsigned)4000000000u) * ((long double)0.3333333333333333L); static long double r3 = ((unsigned)4000000000u) * ((long double)0.3333333333333333L);
      printf("1256: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile unsigned vy = ((unsigned)4000000000u); long double r1 = vx / vy; long double r2 = ((long double)0.3333333333333333L) / ((unsigned)4000000000u); static long double r3 = ((long double)0.3333333333333333L) / ((unsigned)4000000000u);
      printf("1257: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx / vy; long double r2 = ((unsigned)4000000000u) / ((long double)0.3333333333333333L); static long double r3 = ((unsigned)4000000000u) / ((long double)0.3333333333333333L);
      printf("1258: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile unsigned vy = ((unsigned)3u); long double r1 = vx / vy; long double r2 = ((long double)123456.789L) / ((unsigned)3u); static long double r3 = ((long double)123456.789L) / ((unsigned)3u);
      printf("1259: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)123456.789L); long double r1 = vx / vy; long double r2 = ((unsigned)3u) / ((long double)123456.789L); static long double r3 = ((unsigned)3u) / ((long double)123456.789L);
      printf("1260: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile unsigned vy = ((unsigned)3u); long double r1 = vx / vy; long double r2 = ((long double)0.3333333333333333L) / ((unsigned)3u); static long double r3 = ((long double)0.3333333333333333L) / ((unsigned)3u);
      printf("1261: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx / vy; long double r2 = ((unsigned)3u) / ((long double)0.3333333333333333L); static long double r3 = ((unsigned)3u) / ((long double)0.3333333333333333L);
      printf("1262: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile unsigned vy = ((unsigned)3u); long double r1 = vx / vy; long double r2 = ((long double)0.1L) / ((unsigned)3u); static long double r3 = ((long double)0.1L) / ((unsigned)3u);
      printf("1263: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)0.1L); long double r1 = vx / vy; long double r2 = ((unsigned)3u) / ((long double)0.1L); static long double r3 = ((unsigned)3u) / ((long double)0.1L);
      printf("1264: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile unsigned vy = ((unsigned)3u); int r1 = vx < vy; int r2 = ((long double)16777217.0L) < ((unsigned)3u); static int r3 = ((long double)16777217.0L) < ((unsigned)3u);
      printf("1265: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)16777217.0L); int r1 = vx < vy; int r2 = ((unsigned)3u) < ((long double)16777217.0L); static int r3 = ((unsigned)3u) < ((long double)16777217.0L);
      printf("1266: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)3.0L); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx < vy; int r2 = ((long double)3.0L) < ((unsigned)4000000000u); static int r3 = ((long double)3.0L) < ((unsigned)4000000000u);
      printf("1267: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)3.0L); int r1 = vx < vy; int r2 = ((unsigned)4000000000u) < ((long double)3.0L); static int r3 = ((unsigned)4000000000u) < ((long double)3.0L);
      printf("1268: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx < vy; int r2 = ((long double)0.3333333333333333L) < ((unsigned)4000000000u); static int r3 = ((long double)0.3333333333333333L) < ((unsigned)4000000000u);
      printf("1269: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)0.3333333333333333L); int r1 = vx < vy; int r2 = ((unsigned)4000000000u) < ((long double)0.3333333333333333L); static int r3 = ((unsigned)4000000000u) < ((long double)0.3333333333333333L);
      printf("1270: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile unsigned vy = ((unsigned)3u); int r1 = vx < vy; int r2 = ((long double)1e-40L) < ((unsigned)3u); static int r3 = ((long double)1e-40L) < ((unsigned)3u);
      printf("1271: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)1e-40L); int r1 = vx < vy; int r2 = ((unsigned)3u) < ((long double)1e-40L); static int r3 = ((unsigned)3u) < ((long double)1e-40L);
      printf("1272: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx == vy; int r2 = ((long double)1e-40L) == ((unsigned)4000000000u); static int r3 = ((long double)1e-40L) == ((unsigned)4000000000u);
      printf("1273: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)1e-40L); int r1 = vx == vy; int r2 = ((unsigned)4000000000u) == ((long double)1e-40L); static int r3 = ((unsigned)4000000000u) == ((long double)1e-40L);
      printf("1274: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx == vy; int r2 = ((long double)1e-40L) == ((unsigned)4000000000u); static int r3 = ((long double)1e-40L) == ((unsigned)4000000000u);
      printf("1275: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)1e-40L); int r1 = vx == vy; int r2 = ((unsigned)4000000000u) == ((long double)1e-40L); static int r3 = ((unsigned)4000000000u) == ((long double)1e-40L);
      printf("1276: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile unsigned vy = ((unsigned)3u); int r1 = vx == vy; int r2 = ((long double)0.3333333333333333L) == ((unsigned)3u); static int r3 = ((long double)0.3333333333333333L) == ((unsigned)3u);
      printf("1277: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)0.3333333333333333L); int r1 = vx == vy; int r2 = ((unsigned)3u) == ((long double)0.3333333333333333L); static int r3 = ((unsigned)3u) == ((long double)0.3333333333333333L);
      printf("1278: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e10L); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx == vy; int r2 = ((long double)1e10L) == ((unsigned)4000000000u); static int r3 = ((long double)1e10L) == ((unsigned)4000000000u);
      printf("1279: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)1e10L); int r1 = vx == vy; int r2 = ((unsigned)4000000000u) == ((long double)1e10L); static int r3 = ((unsigned)4000000000u) == ((long double)1e10L);
      printf("1280: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1.0L); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx >= vy; int r2 = ((long double)1.0L) >= ((unsigned)4000000000u); static int r3 = ((long double)1.0L) >= ((unsigned)4000000000u);
      printf("1281: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)1.0L); int r1 = vx >= vy; int r2 = ((unsigned)4000000000u) >= ((long double)1.0L); static int r3 = ((unsigned)4000000000u) >= ((long double)1.0L);
      printf("1282: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile unsigned vy = ((unsigned)3u); int r1 = vx >= vy; int r2 = ((long double)-2.5L) >= ((unsigned)3u); static int r3 = ((long double)-2.5L) >= ((unsigned)3u);
      printf("1283: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)-2.5L); int r1 = vx >= vy; int r2 = ((unsigned)3u) >= ((long double)-2.5L); static int r3 = ((unsigned)3u) >= ((long double)-2.5L);
      printf("1284: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e10L); volatile unsigned vy = ((unsigned)4000000000u); int r1 = vx >= vy; int r2 = ((long double)1e10L) >= ((unsigned)4000000000u); static int r3 = ((long double)1e10L) >= ((unsigned)4000000000u);
      printf("1285: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)4000000000u); volatile long double vy = ((long double)1e10L); int r1 = vx >= vy; int r2 = ((unsigned)4000000000u) >= ((long double)1e10L); static int r3 = ((unsigned)4000000000u) >= ((long double)1e10L);
      printf("1286: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e30L); volatile unsigned vy = ((unsigned)3u); int r1 = vx >= vy; int r2 = ((long double)1e30L) >= ((unsigned)3u); static int r3 = ((long double)1e30L) >= ((unsigned)3u);
      printf("1287: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned vx = ((unsigned)3u); volatile long double vy = ((long double)1e30L); int r1 = vx >= vy; int r2 = ((unsigned)3u) >= ((long double)1e30L); static int r3 = ((unsigned)3u) >= ((long double)1e30L);
      printf("1288: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.1L); volatile long long vy = ((long long)-5LL); long double r1 = vx + vy; long double r2 = ((long double)0.1L) + ((long long)-5LL); static long double r3 = ((long double)0.1L) + ((long long)-5LL);
      printf("1289: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile long double vy = ((long double)0.1L); long double r1 = vx + vy; long double r2 = ((long long)-5LL) + ((long double)0.1L); static long double r3 = ((long long)-5LL) + ((long double)0.1L);
      printf("1290: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)7.0L); volatile long long vy = ((long long)9007199254740993LL); long double r1 = vx + vy; long double r2 = ((long double)7.0L) + ((long long)9007199254740993LL); static long double r3 = ((long double)7.0L) + ((long long)9007199254740993LL);
      printf("1291: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)7.0L); long double r1 = vx + vy; long double r2 = ((long long)9007199254740993LL) + ((long double)7.0L); static long double r3 = ((long long)9007199254740993LL) + ((long double)7.0L);
      printf("1292: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile long long vy = ((long long)9007199254740993LL); long double r1 = vx + vy; long double r2 = ((long double)0.1L) + ((long long)9007199254740993LL); static long double r3 = ((long double)0.1L) + ((long long)9007199254740993LL);
      printf("1293: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)0.1L); long double r1 = vx + vy; long double r2 = ((long long)9007199254740993LL) + ((long double)0.1L); static long double r3 = ((long long)9007199254740993LL) + ((long double)0.1L);
      printf("1294: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)7.0L); volatile long long vy = ((long long)-5LL); long double r1 = vx + vy; long double r2 = ((long double)7.0L) + ((long long)-5LL); static long double r3 = ((long double)7.0L) + ((long long)-5LL);
      printf("1295: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile long double vy = ((long double)7.0L); long double r1 = vx + vy; long double r2 = ((long long)-5LL) + ((long double)7.0L); static long double r3 = ((long long)-5LL) + ((long double)7.0L);
      printf("1296: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile long long vy = ((long long)9007199254740993LL); long double r1 = vx - vy; long double r2 = ((long double)16777217.0L) - ((long long)9007199254740993LL); static long double r3 = ((long double)16777217.0L) - ((long long)9007199254740993LL);
      printf("1297: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)16777217.0L); long double r1 = vx - vy; long double r2 = ((long long)9007199254740993LL) - ((long double)16777217.0L); static long double r3 = ((long long)9007199254740993LL) - ((long double)16777217.0L);
      printf("1298: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile long long vy = ((long long)9007199254740993LL); long double r1 = vx - vy; long double r2 = ((long double)-0.1L) - ((long long)9007199254740993LL); static long double r3 = ((long double)-0.1L) - ((long long)9007199254740993LL);
      printf("1299: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)-0.1L); long double r1 = vx - vy; long double r2 = ((long long)9007199254740993LL) - ((long double)-0.1L); static long double r3 = ((long long)9007199254740993LL) - ((long double)-0.1L);
      printf("1300: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile long long vy = ((long long)9007199254740993LL); long double r1 = vx - vy; long double r2 = ((long double)123456.789L) - ((long long)9007199254740993LL); static long double r3 = ((long double)123456.789L) - ((long long)9007199254740993LL);
      printf("1301: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)123456.789L); long double r1 = vx - vy; long double r2 = ((long long)9007199254740993LL) - ((long double)123456.789L); static long double r3 = ((long long)9007199254740993LL) - ((long double)123456.789L);
      printf("1302: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile long long vy = ((long long)9007199254740993LL); long double r1 = vx - vy; long double r2 = ((long double)123456.789L) - ((long long)9007199254740993LL); static long double r3 = ((long double)123456.789L) - ((long long)9007199254740993LL);
      printf("1303: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)123456.789L); long double r1 = vx - vy; long double r2 = ((long long)9007199254740993LL) - ((long double)123456.789L); static long double r3 = ((long long)9007199254740993LL) - ((long double)123456.789L);
      printf("1304: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile long long vy = ((long long)9007199254740993LL); long double r1 = vx * vy; long double r2 = ((long double)0.3333333333333333L) * ((long long)9007199254740993LL); static long double r3 = ((long double)0.3333333333333333L) * ((long long)9007199254740993LL);
      printf("1305: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx * vy; long double r2 = ((long long)9007199254740993LL) * ((long double)0.3333333333333333L); static long double r3 = ((long long)9007199254740993LL) * ((long double)0.3333333333333333L);
      printf("1306: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile long long vy = ((long long)9007199254740993LL); long double r1 = vx * vy; long double r2 = ((long double)1e-5L) * ((long long)9007199254740993LL); static long double r3 = ((long double)1e-5L) * ((long long)9007199254740993LL);
      printf("1307: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)1e-5L); long double r1 = vx * vy; long double r2 = ((long long)9007199254740993LL) * ((long double)1e-5L); static long double r3 = ((long long)9007199254740993LL) * ((long double)1e-5L);
      printf("1308: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile long long vy = ((long long)9007199254740993LL); long double r1 = vx * vy; long double r2 = ((long double)1e10L) * ((long long)9007199254740993LL); static long double r3 = ((long double)1e10L) * ((long long)9007199254740993LL);
      printf("1309: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)1e10L); long double r1 = vx * vy; long double r2 = ((long long)9007199254740993LL) * ((long double)1e10L); static long double r3 = ((long long)9007199254740993LL) * ((long double)1e10L);
      printf("1310: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile long long vy = ((long long)-5LL); long double r1 = vx * vy; long double r2 = ((long double)1e10L) * ((long long)-5LL); static long double r3 = ((long double)1e10L) * ((long long)-5LL);
      printf("1311: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile long double vy = ((long double)1e10L); long double r1 = vx * vy; long double r2 = ((long long)-5LL) * ((long double)1e10L); static long double r3 = ((long long)-5LL) * ((long double)1e10L);
      printf("1312: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile long long vy = ((long long)-5LL); long double r1 = vx / vy; long double r2 = ((long double)0.3333333333333333L) / ((long long)-5LL); static long double r3 = ((long double)0.3333333333333333L) / ((long long)-5LL);
      printf("1313: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx / vy; long double r2 = ((long long)-5LL) / ((long double)0.3333333333333333L); static long double r3 = ((long long)-5LL) / ((long double)0.3333333333333333L);
      printf("1314: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile long long vy = ((long long)-5LL); long double r1 = vx / vy; long double r2 = ((long double)123456.789L) / ((long long)-5LL); static long double r3 = ((long double)123456.789L) / ((long long)-5LL);
      printf("1315: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile long double vy = ((long double)123456.789L); long double r1 = vx / vy; long double r2 = ((long long)-5LL) / ((long double)123456.789L); static long double r3 = ((long long)-5LL) / ((long double)123456.789L);
      printf("1316: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)7.0L); volatile long long vy = ((long long)-5LL); long double r1 = vx / vy; long double r2 = ((long double)7.0L) / ((long long)-5LL); static long double r3 = ((long double)7.0L) / ((long long)-5LL);
      printf("1317: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)-5LL); volatile long double vy = ((long double)7.0L); long double r1 = vx / vy; long double r2 = ((long long)-5LL) / ((long double)7.0L); static long double r3 = ((long long)-5LL) / ((long double)7.0L);
      printf("1318: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile long long vy = ((long long)9007199254740993LL); long double r1 = vx / vy; long double r2 = ((long double)-0.1L) / ((long long)9007199254740993LL); static long double r3 = ((long double)-0.1L) / ((long long)9007199254740993LL);
      printf("1319: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)-0.1L); long double r1 = vx / vy; long double r2 = ((long long)9007199254740993LL) / ((long double)-0.1L); static long double r3 = ((long long)9007199254740993LL) / ((long double)-0.1L);
      printf("1320: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile long long vy = ((long long)-5LL); int r1 = vx < vy; int r2 = ((long double)123456.789L) < ((long long)-5LL); static int r3 = ((long double)123456.789L) < ((long long)-5LL);
      printf("1321: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile long double vy = ((long double)123456.789L); int r1 = vx < vy; int r2 = ((long long)-5LL) < ((long double)123456.789L); static int r3 = ((long long)-5LL) < ((long double)123456.789L);
      printf("1322: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e30L); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx < vy; int r2 = ((long double)1e30L) < ((long long)9007199254740993LL); static int r3 = ((long double)1e30L) < ((long long)9007199254740993LL);
      printf("1323: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)1e30L); int r1 = vx < vy; int r2 = ((long long)9007199254740993LL) < ((long double)1e30L); static int r3 = ((long long)9007199254740993LL) < ((long double)1e30L);
      printf("1324: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx < vy; int r2 = ((long double)16777217.0L) < ((long long)9007199254740993LL); static int r3 = ((long double)16777217.0L) < ((long long)9007199254740993LL);
      printf("1325: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)16777217.0L); int r1 = vx < vy; int r2 = ((long long)9007199254740993LL) < ((long double)16777217.0L); static int r3 = ((long long)9007199254740993LL) < ((long double)16777217.0L);
      printf("1326: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.1L); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx < vy; int r2 = ((long double)0.1L) < ((long long)9007199254740993LL); static int r3 = ((long double)0.1L) < ((long long)9007199254740993LL);
      printf("1327: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)0.1L); int r1 = vx < vy; int r2 = ((long long)9007199254740993LL) < ((long double)0.1L); static int r3 = ((long long)9007199254740993LL) < ((long double)0.1L);
      printf("1328: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile long long vy = ((long long)-5LL); int r1 = vx == vy; int r2 = ((long double)-2.5L) == ((long long)-5LL); static int r3 = ((long double)-2.5L) == ((long long)-5LL);
      printf("1329: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile long double vy = ((long double)-2.5L); int r1 = vx == vy; int r2 = ((long long)-5LL) == ((long double)-2.5L); static int r3 = ((long long)-5LL) == ((long double)-2.5L);
      printf("1330: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e10L); volatile long long vy = ((long long)-5LL); int r1 = vx == vy; int r2 = ((long double)1e10L) == ((long long)-5LL); static int r3 = ((long double)1e10L) == ((long long)-5LL);
      printf("1331: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile long double vy = ((long double)1e10L); int r1 = vx == vy; int r2 = ((long long)-5LL) == ((long double)1e10L); static int r3 = ((long long)-5LL) == ((long double)1e10L);
      printf("1332: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx == vy; int r2 = ((long double)1e-5L) == ((long long)9007199254740993LL); static int r3 = ((long double)1e-5L) == ((long long)9007199254740993LL);
      printf("1333: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)1e-5L); int r1 = vx == vy; int r2 = ((long long)9007199254740993LL) == ((long double)1e-5L); static int r3 = ((long long)9007199254740993LL) == ((long double)1e-5L);
      printf("1334: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx == vy; int r2 = ((long double)1e-40L) == ((long long)9007199254740993LL); static int r3 = ((long double)1e-40L) == ((long long)9007199254740993LL);
      printf("1335: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)1e-40L); int r1 = vx == vy; int r2 = ((long long)9007199254740993LL) == ((long double)1e-40L); static int r3 = ((long long)9007199254740993LL) == ((long double)1e-40L);
      printf("1336: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile long long vy = ((long long)-5LL); int r1 = vx >= vy; int r2 = ((long double)-0.1L) >= ((long long)-5LL); static int r3 = ((long double)-0.1L) >= ((long long)-5LL);
      printf("1337: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile long double vy = ((long double)-0.1L); int r1 = vx >= vy; int r2 = ((long long)-5LL) >= ((long double)-0.1L); static int r3 = ((long long)-5LL) >= ((long double)-0.1L);
      printf("1338: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx >= vy; int r2 = ((long double)-2.5L) >= ((long long)9007199254740993LL); static int r3 = ((long double)-2.5L) >= ((long long)9007199254740993LL);
      printf("1339: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)-2.5L); int r1 = vx >= vy; int r2 = ((long long)9007199254740993LL) >= ((long double)-2.5L); static int r3 = ((long long)9007199254740993LL) >= ((long double)-2.5L);
      printf("1340: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e30L); volatile long long vy = ((long long)9007199254740993LL); int r1 = vx >= vy; int r2 = ((long double)1e30L) >= ((long long)9007199254740993LL); static int r3 = ((long double)1e30L) >= ((long long)9007199254740993LL);
      printf("1341: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)9007199254740993LL); volatile long double vy = ((long double)1e30L); int r1 = vx >= vy; int r2 = ((long long)9007199254740993LL) >= ((long double)1e30L); static int r3 = ((long long)9007199254740993LL) >= ((long double)1e30L);
      printf("1342: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)7.0L); volatile long long vy = ((long long)-5LL); int r1 = vx >= vy; int r2 = ((long double)7.0L) >= ((long long)-5LL); static int r3 = ((long double)7.0L) >= ((long long)-5LL);
      printf("1343: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long long vx = ((long long)-5LL); volatile long double vy = ((long double)7.0L); int r1 = vx >= vy; int r2 = ((long long)-5LL) >= ((long double)7.0L); static int r3 = ((long long)-5LL) >= ((long double)7.0L);
      printf("1344: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); long double r1 = vx + vy; long double r2 = ((long double)1e-5L) + ((unsigned long long)12345678901234567ULL); static long double r3 = ((long double)1e-5L) + ((unsigned long long)12345678901234567ULL);
      printf("1345: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)1e-5L); long double r1 = vx + vy; long double r2 = ((unsigned long long)12345678901234567ULL) + ((long double)1e-5L); static long double r3 = ((unsigned long long)12345678901234567ULL) + ((long double)1e-5L);
      printf("1346: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); long double r1 = vx + vy; long double r2 = ((long double)-2.5L) + ((unsigned long long)12345678901234567ULL); static long double r3 = ((long double)-2.5L) + ((unsigned long long)12345678901234567ULL);
      printf("1347: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)-2.5L); long double r1 = vx + vy; long double r2 = ((unsigned long long)12345678901234567ULL) + ((long double)-2.5L); static long double r3 = ((unsigned long long)12345678901234567ULL) + ((long double)-2.5L);
      printf("1348: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); long double r1 = vx + vy; long double r2 = ((long double)1e30L) + ((unsigned long long)18446744073709551615ULL); static long double r3 = ((long double)1e30L) + ((unsigned long long)18446744073709551615ULL);
      printf("1349: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)1e30L); long double r1 = vx + vy; long double r2 = ((unsigned long long)18446744073709551615ULL) + ((long double)1e30L); static long double r3 = ((unsigned long long)18446744073709551615ULL) + ((long double)1e30L);
      printf("1350: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); long double r1 = vx + vy; long double r2 = ((long double)-0.1L) + ((unsigned long long)18446744073709551615ULL); static long double r3 = ((long double)-0.1L) + ((unsigned long long)18446744073709551615ULL);
      printf("1351: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)-0.1L); long double r1 = vx + vy; long double r2 = ((unsigned long long)18446744073709551615ULL) + ((long double)-0.1L); static long double r3 = ((unsigned long long)18446744073709551615ULL) + ((long double)-0.1L);
      printf("1352: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); long double r1 = vx - vy; long double r2 = ((long double)16777217.0L) - ((unsigned long long)12345678901234567ULL); static long double r3 = ((long double)16777217.0L) - ((unsigned long long)12345678901234567ULL);
      printf("1353: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)16777217.0L); long double r1 = vx - vy; long double r2 = ((unsigned long long)12345678901234567ULL) - ((long double)16777217.0L); static long double r3 = ((unsigned long long)12345678901234567ULL) - ((long double)16777217.0L);
      printf("1354: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); long double r1 = vx - vy; long double r2 = ((long double)0.3333333333333333L) - ((unsigned long long)18446744073709551615ULL); static long double r3 = ((long double)0.3333333333333333L) - ((unsigned long long)18446744073709551615ULL);
      printf("1355: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx - vy; long double r2 = ((unsigned long long)18446744073709551615ULL) - ((long double)0.3333333333333333L); static long double r3 = ((unsigned long long)18446744073709551615ULL) - ((long double)0.3333333333333333L);
      printf("1356: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); long double r1 = vx - vy; long double r2 = ((long double)-0.1L) - ((unsigned long long)12345678901234567ULL); static long double r3 = ((long double)-0.1L) - ((unsigned long long)12345678901234567ULL);
      printf("1357: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)-0.1L); long double r1 = vx - vy; long double r2 = ((unsigned long long)12345678901234567ULL) - ((long double)-0.1L); static long double r3 = ((unsigned long long)12345678901234567ULL) - ((long double)-0.1L);
      printf("1358: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); long double r1 = vx - vy; long double r2 = ((long double)0.3333333333333333L) - ((unsigned long long)12345678901234567ULL); static long double r3 = ((long double)0.3333333333333333L) - ((unsigned long long)12345678901234567ULL);
      printf("1359: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx - vy; long double r2 = ((unsigned long long)12345678901234567ULL) - ((long double)0.3333333333333333L); static long double r3 = ((unsigned long long)12345678901234567ULL) - ((long double)0.3333333333333333L);
      printf("1360: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); long double r1 = vx * vy; long double r2 = ((long double)123456.789L) * ((unsigned long long)18446744073709551615ULL); static long double r3 = ((long double)123456.789L) * ((unsigned long long)18446744073709551615ULL);
      printf("1361: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)123456.789L); long double r1 = vx * vy; long double r2 = ((unsigned long long)18446744073709551615ULL) * ((long double)123456.789L); static long double r3 = ((unsigned long long)18446744073709551615ULL) * ((long double)123456.789L);
      printf("1362: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); long double r1 = vx * vy; long double r2 = ((long double)16777217.0L) * ((unsigned long long)12345678901234567ULL); static long double r3 = ((long double)16777217.0L) * ((unsigned long long)12345678901234567ULL);
      printf("1363: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)16777217.0L); long double r1 = vx * vy; long double r2 = ((unsigned long long)12345678901234567ULL) * ((long double)16777217.0L); static long double r3 = ((unsigned long long)12345678901234567ULL) * ((long double)16777217.0L);
      printf("1364: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); long double r1 = vx * vy; long double r2 = ((long double)1e-5L) * ((unsigned long long)12345678901234567ULL); static long double r3 = ((long double)1e-5L) * ((unsigned long long)12345678901234567ULL);
      printf("1365: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)1e-5L); long double r1 = vx * vy; long double r2 = ((unsigned long long)12345678901234567ULL) * ((long double)1e-5L); static long double r3 = ((unsigned long long)12345678901234567ULL) * ((long double)1e-5L);
      printf("1366: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); long double r1 = vx * vy; long double r2 = ((long double)0.1L) * ((unsigned long long)18446744073709551615ULL); static long double r3 = ((long double)0.1L) * ((unsigned long long)18446744073709551615ULL);
      printf("1367: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)0.1L); long double r1 = vx * vy; long double r2 = ((unsigned long long)18446744073709551615ULL) * ((long double)0.1L); static long double r3 = ((unsigned long long)18446744073709551615ULL) * ((long double)0.1L);
      printf("1368: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)3.0L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); long double r1 = vx / vy; long double r2 = ((long double)3.0L) / ((unsigned long long)12345678901234567ULL); static long double r3 = ((long double)3.0L) / ((unsigned long long)12345678901234567ULL);
      printf("1369: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)3.0L); long double r1 = vx / vy; long double r2 = ((unsigned long long)12345678901234567ULL) / ((long double)3.0L); static long double r3 = ((unsigned long long)12345678901234567ULL) / ((long double)3.0L);
      printf("1370: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); long double r1 = vx / vy; long double r2 = ((long double)1e10L) / ((unsigned long long)18446744073709551615ULL); static long double r3 = ((long double)1e10L) / ((unsigned long long)18446744073709551615ULL);
      printf("1371: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)1e10L); long double r1 = vx / vy; long double r2 = ((unsigned long long)18446744073709551615ULL) / ((long double)1e10L); static long double r3 = ((unsigned long long)18446744073709551615ULL) / ((long double)1e10L);
      printf("1372: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); long double r1 = vx / vy; long double r2 = ((long double)16777217.0L) / ((unsigned long long)18446744073709551615ULL); static long double r3 = ((long double)16777217.0L) / ((unsigned long long)18446744073709551615ULL);
      printf("1373: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)16777217.0L); long double r1 = vx / vy; long double r2 = ((unsigned long long)18446744073709551615ULL) / ((long double)16777217.0L); static long double r3 = ((unsigned long long)18446744073709551615ULL) / ((long double)16777217.0L);
      printf("1374: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); long double r1 = vx / vy; long double r2 = ((long double)0.3333333333333333L) / ((unsigned long long)18446744073709551615ULL); static long double r3 = ((long double)0.3333333333333333L) / ((unsigned long long)18446744073709551615ULL);
      printf("1375: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx / vy; long double r2 = ((unsigned long long)18446744073709551615ULL) / ((long double)0.3333333333333333L); static long double r3 = ((unsigned long long)18446744073709551615ULL) / ((long double)0.3333333333333333L);
      printf("1376: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx < vy; int r2 = ((long double)16777217.0L) < ((unsigned long long)12345678901234567ULL); static int r3 = ((long double)16777217.0L) < ((unsigned long long)12345678901234567ULL);
      printf("1377: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)16777217.0L); int r1 = vx < vy; int r2 = ((unsigned long long)12345678901234567ULL) < ((long double)16777217.0L); static int r3 = ((unsigned long long)12345678901234567ULL) < ((long double)16777217.0L);
      printf("1378: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx < vy; int r2 = ((long double)123456.789L) < ((unsigned long long)18446744073709551615ULL); static int r3 = ((long double)123456.789L) < ((unsigned long long)18446744073709551615ULL);
      printf("1379: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)123456.789L); int r1 = vx < vy; int r2 = ((unsigned long long)18446744073709551615ULL) < ((long double)123456.789L); static int r3 = ((unsigned long long)18446744073709551615ULL) < ((long double)123456.789L);
      printf("1380: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx < vy; int r2 = ((long double)16777217.0L) < ((unsigned long long)12345678901234567ULL); static int r3 = ((long double)16777217.0L) < ((unsigned long long)12345678901234567ULL);
      printf("1381: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)16777217.0L); int r1 = vx < vy; int r2 = ((unsigned long long)12345678901234567ULL) < ((long double)16777217.0L); static int r3 = ((unsigned long long)12345678901234567ULL) < ((long double)16777217.0L);
      printf("1382: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx < vy; int r2 = ((long double)-2.5L) < ((unsigned long long)18446744073709551615ULL); static int r3 = ((long double)-2.5L) < ((unsigned long long)18446744073709551615ULL);
      printf("1383: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)-2.5L); int r1 = vx < vy; int r2 = ((unsigned long long)18446744073709551615ULL) < ((long double)-2.5L); static int r3 = ((unsigned long long)18446744073709551615ULL) < ((long double)-2.5L);
      printf("1384: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx == vy; int r2 = ((long double)-2.5L) == ((unsigned long long)18446744073709551615ULL); static int r3 = ((long double)-2.5L) == ((unsigned long long)18446744073709551615ULL);
      printf("1385: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)-2.5L); int r1 = vx == vy; int r2 = ((unsigned long long)18446744073709551615ULL) == ((long double)-2.5L); static int r3 = ((unsigned long long)18446744073709551615ULL) == ((long double)-2.5L);
      printf("1386: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx == vy; int r2 = ((long double)0.3333333333333333L) == ((unsigned long long)18446744073709551615ULL); static int r3 = ((long double)0.3333333333333333L) == ((unsigned long long)18446744073709551615ULL);
      printf("1387: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)0.3333333333333333L); int r1 = vx == vy; int r2 = ((unsigned long long)18446744073709551615ULL) == ((long double)0.3333333333333333L); static int r3 = ((unsigned long long)18446744073709551615ULL) == ((long double)0.3333333333333333L);
      printf("1388: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1.0L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx == vy; int r2 = ((long double)1.0L) == ((unsigned long long)18446744073709551615ULL); static int r3 = ((long double)1.0L) == ((unsigned long long)18446744073709551615ULL);
      printf("1389: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)1.0L); int r1 = vx == vy; int r2 = ((unsigned long long)18446744073709551615ULL) == ((long double)1.0L); static int r3 = ((unsigned long long)18446744073709551615ULL) == ((long double)1.0L);
      printf("1390: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e30L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx == vy; int r2 = ((long double)1e30L) == ((unsigned long long)12345678901234567ULL); static int r3 = ((long double)1e30L) == ((unsigned long long)12345678901234567ULL);
      printf("1391: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)1e30L); int r1 = vx == vy; int r2 = ((unsigned long long)12345678901234567ULL) == ((long double)1e30L); static int r3 = ((unsigned long long)12345678901234567ULL) == ((long double)1e30L);
      printf("1392: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e30L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx >= vy; int r2 = ((long double)1e30L) >= ((unsigned long long)12345678901234567ULL); static int r3 = ((long double)1e30L) >= ((unsigned long long)12345678901234567ULL);
      printf("1393: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)1e30L); int r1 = vx >= vy; int r2 = ((unsigned long long)12345678901234567ULL) >= ((long double)1e30L); static int r3 = ((unsigned long long)12345678901234567ULL) >= ((long double)1e30L);
      printf("1394: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx >= vy; int r2 = ((long double)0.3333333333333333L) >= ((unsigned long long)12345678901234567ULL); static int r3 = ((long double)0.3333333333333333L) >= ((unsigned long long)12345678901234567ULL);
      printf("1395: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)0.3333333333333333L); int r1 = vx >= vy; int r2 = ((unsigned long long)12345678901234567ULL) >= ((long double)0.3333333333333333L); static int r3 = ((unsigned long long)12345678901234567ULL) >= ((long double)0.3333333333333333L);
      printf("1396: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile unsigned long long vy = ((unsigned long long)12345678901234567ULL); int r1 = vx >= vy; int r2 = ((long double)1e-40L) >= ((unsigned long long)12345678901234567ULL); static int r3 = ((long double)1e-40L) >= ((unsigned long long)12345678901234567ULL);
      printf("1397: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)12345678901234567ULL); volatile long double vy = ((long double)1e-40L); int r1 = vx >= vy; int r2 = ((unsigned long long)12345678901234567ULL) >= ((long double)1e-40L); static int r3 = ((unsigned long long)12345678901234567ULL) >= ((long double)1e-40L);
      printf("1398: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)7.0L); volatile unsigned long long vy = ((unsigned long long)18446744073709551615ULL); int r1 = vx >= vy; int r2 = ((long double)7.0L) >= ((unsigned long long)18446744073709551615ULL); static int r3 = ((long double)7.0L) >= ((unsigned long long)18446744073709551615ULL);
      printf("1399: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned long long vx = ((unsigned long long)18446744073709551615ULL); volatile long double vy = ((long double)7.0L); int r1 = vx >= vy; int r2 = ((unsigned long long)18446744073709551615ULL) >= ((long double)7.0L); static int r3 = ((unsigned long long)18446744073709551615ULL) >= ((long double)7.0L);
      printf("1400: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile short vy = ((short)(short)-3); long double r1 = vx + vy; long double r2 = ((long double)-2.5L) + ((short)(short)-3); static long double r3 = ((long double)-2.5L) + ((short)(short)-3);
      printf("1401: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)-2.5L); long double r1 = vx + vy; long double r2 = ((short)(short)-3) + ((long double)-2.5L); static long double r3 = ((short)(short)-3) + ((long double)-2.5L);
      printf("1402: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile short vy = ((short)(short)-3); long double r1 = vx + vy; long double r2 = ((long double)1e-40L) + ((short)(short)-3); static long double r3 = ((long double)1e-40L) + ((short)(short)-3);
      printf("1403: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1e-40L); long double r1 = vx + vy; long double r2 = ((short)(short)-3) + ((long double)1e-40L); static long double r3 = ((short)(short)-3) + ((long double)1e-40L);
      printf("1404: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile short vy = ((short)(short)-3); long double r1 = vx + vy; long double r2 = ((long double)-0.1L) + ((short)(short)-3); static long double r3 = ((long double)-0.1L) + ((short)(short)-3);
      printf("1405: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)-0.1L); long double r1 = vx + vy; long double r2 = ((short)(short)-3) + ((long double)-0.1L); static long double r3 = ((short)(short)-3) + ((long double)-0.1L);
      printf("1406: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile short vy = ((short)(short)-3); long double r1 = vx + vy; long double r2 = ((long double)1e10L) + ((short)(short)-3); static long double r3 = ((long double)1e10L) + ((short)(short)-3);
      printf("1407: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1e10L); long double r1 = vx + vy; long double r2 = ((short)(short)-3) + ((long double)1e10L); static long double r3 = ((short)(short)-3) + ((long double)1e10L);
      printf("1408: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile short vy = ((short)(short)-3); long double r1 = vx - vy; long double r2 = ((long double)16777217.0L) - ((short)(short)-3); static long double r3 = ((long double)16777217.0L) - ((short)(short)-3);
      printf("1409: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)16777217.0L); long double r1 = vx - vy; long double r2 = ((short)(short)-3) - ((long double)16777217.0L); static long double r3 = ((short)(short)-3) - ((long double)16777217.0L);
      printf("1410: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile short vy = ((short)(short)-3); long double r1 = vx - vy; long double r2 = ((long double)1.0L) - ((short)(short)-3); static long double r3 = ((long double)1.0L) - ((short)(short)-3);
      printf("1411: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1.0L); long double r1 = vx - vy; long double r2 = ((short)(short)-3) - ((long double)1.0L); static long double r3 = ((short)(short)-3) - ((long double)1.0L);
      printf("1412: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile short vy = ((short)(short)-3); long double r1 = vx - vy; long double r2 = ((long double)0.1L) - ((short)(short)-3); static long double r3 = ((long double)0.1L) - ((short)(short)-3);
      printf("1413: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)0.1L); long double r1 = vx - vy; long double r2 = ((short)(short)-3) - ((long double)0.1L); static long double r3 = ((short)(short)-3) - ((long double)0.1L);
      printf("1414: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile short vy = ((short)(short)-3); long double r1 = vx - vy; long double r2 = ((long double)0.3333333333333333L) - ((short)(short)-3); static long double r3 = ((long double)0.3333333333333333L) - ((short)(short)-3);
      printf("1415: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx - vy; long double r2 = ((short)(short)-3) - ((long double)0.3333333333333333L); static long double r3 = ((short)(short)-3) - ((long double)0.3333333333333333L);
      printf("1416: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile short vy = ((short)(short)-3); long double r1 = vx * vy; long double r2 = ((long double)1e-40L) * ((short)(short)-3); static long double r3 = ((long double)1e-40L) * ((short)(short)-3);
      printf("1417: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1e-40L); long double r1 = vx * vy; long double r2 = ((short)(short)-3) * ((long double)1e-40L); static long double r3 = ((short)(short)-3) * ((long double)1e-40L);
      printf("1418: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile short vy = ((short)(short)-3); long double r1 = vx * vy; long double r2 = ((long double)1e-40L) * ((short)(short)-3); static long double r3 = ((long double)1e-40L) * ((short)(short)-3);
      printf("1419: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1e-40L); long double r1 = vx * vy; long double r2 = ((short)(short)-3) * ((long double)1e-40L); static long double r3 = ((short)(short)-3) * ((long double)1e-40L);
      printf("1420: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile short vy = ((short)(short)-3); long double r1 = vx * vy; long double r2 = ((long double)-2.5L) * ((short)(short)-3); static long double r3 = ((long double)-2.5L) * ((short)(short)-3);
      printf("1421: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)-2.5L); long double r1 = vx * vy; long double r2 = ((short)(short)-3) * ((long double)-2.5L); static long double r3 = ((short)(short)-3) * ((long double)-2.5L);
      printf("1422: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile short vy = ((short)(short)-3); long double r1 = vx * vy; long double r2 = ((long double)1.0L) * ((short)(short)-3); static long double r3 = ((long double)1.0L) * ((short)(short)-3);
      printf("1423: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1.0L); long double r1 = vx * vy; long double r2 = ((short)(short)-3) * ((long double)1.0L); static long double r3 = ((short)(short)-3) * ((long double)1.0L);
      printf("1424: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile short vy = ((short)(short)-3); long double r1 = vx / vy; long double r2 = ((long double)1e-40L) / ((short)(short)-3); static long double r3 = ((long double)1e-40L) / ((short)(short)-3);
      printf("1425: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1e-40L); long double r1 = vx / vy; long double r2 = ((short)(short)-3) / ((long double)1e-40L); static long double r3 = ((short)(short)-3) / ((long double)1e-40L);
      printf("1426: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile short vy = ((short)(short)-3); long double r1 = vx / vy; long double r2 = ((long double)1e-40L) / ((short)(short)-3); static long double r3 = ((long double)1e-40L) / ((short)(short)-3);
      printf("1427: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1e-40L); long double r1 = vx / vy; long double r2 = ((short)(short)-3) / ((long double)1e-40L); static long double r3 = ((short)(short)-3) / ((long double)1e-40L);
      printf("1428: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile short vy = ((short)(short)-3); long double r1 = vx / vy; long double r2 = ((long double)1.0L) / ((short)(short)-3); static long double r3 = ((long double)1.0L) / ((short)(short)-3);
      printf("1429: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1.0L); long double r1 = vx / vy; long double r2 = ((short)(short)-3) / ((long double)1.0L); static long double r3 = ((short)(short)-3) / ((long double)1.0L);
      printf("1430: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile short vy = ((short)(short)-3); long double r1 = vx / vy; long double r2 = ((long double)0.3333333333333333L) / ((short)(short)-3); static long double r3 = ((long double)0.3333333333333333L) / ((short)(short)-3);
      printf("1431: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx / vy; long double r2 = ((short)(short)-3) / ((long double)0.3333333333333333L); static long double r3 = ((short)(short)-3) / ((long double)0.3333333333333333L);
      printf("1432: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)7.0L); volatile short vy = ((short)(short)-3); int r1 = vx < vy; int r2 = ((long double)7.0L) < ((short)(short)-3); static int r3 = ((long double)7.0L) < ((short)(short)-3);
      printf("1433: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)7.0L); int r1 = vx < vy; int r2 = ((short)(short)-3) < ((long double)7.0L); static int r3 = ((short)(short)-3) < ((long double)7.0L);
      printf("1434: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile short vy = ((short)(short)-3); int r1 = vx < vy; int r2 = ((long double)1e-5L) < ((short)(short)-3); static int r3 = ((long double)1e-5L) < ((short)(short)-3);
      printf("1435: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1e-5L); int r1 = vx < vy; int r2 = ((short)(short)-3) < ((long double)1e-5L); static int r3 = ((short)(short)-3) < ((long double)1e-5L);
      printf("1436: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile short vy = ((short)(short)-3); int r1 = vx < vy; int r2 = ((long double)1e-40L) < ((short)(short)-3); static int r3 = ((long double)1e-40L) < ((short)(short)-3);
      printf("1437: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1e-40L); int r1 = vx < vy; int r2 = ((short)(short)-3) < ((long double)1e-40L); static int r3 = ((short)(short)-3) < ((long double)1e-40L);
      printf("1438: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e10L); volatile short vy = ((short)(short)-3); int r1 = vx < vy; int r2 = ((long double)1e10L) < ((short)(short)-3); static int r3 = ((long double)1e10L) < ((short)(short)-3);
      printf("1439: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1e10L); int r1 = vx < vy; int r2 = ((short)(short)-3) < ((long double)1e10L); static int r3 = ((short)(short)-3) < ((long double)1e10L);
      printf("1440: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)7.0L); volatile short vy = ((short)(short)-3); int r1 = vx == vy; int r2 = ((long double)7.0L) == ((short)(short)-3); static int r3 = ((long double)7.0L) == ((short)(short)-3);
      printf("1441: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)7.0L); int r1 = vx == vy; int r2 = ((short)(short)-3) == ((long double)7.0L); static int r3 = ((short)(short)-3) == ((long double)7.0L);
      printf("1442: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1.0L); volatile short vy = ((short)(short)-3); int r1 = vx == vy; int r2 = ((long double)1.0L) == ((short)(short)-3); static int r3 = ((long double)1.0L) == ((short)(short)-3);
      printf("1443: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1.0L); int r1 = vx == vy; int r2 = ((short)(short)-3) == ((long double)1.0L); static int r3 = ((short)(short)-3) == ((long double)1.0L);
      printf("1444: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)3.0L); volatile short vy = ((short)(short)-3); int r1 = vx == vy; int r2 = ((long double)3.0L) == ((short)(short)-3); static int r3 = ((long double)3.0L) == ((short)(short)-3);
      printf("1445: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)3.0L); int r1 = vx == vy; int r2 = ((short)(short)-3) == ((long double)3.0L); static int r3 = ((short)(short)-3) == ((long double)3.0L);
      printf("1446: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile short vy = ((short)(short)-3); int r1 = vx == vy; int r2 = ((long double)1e-40L) == ((short)(short)-3); static int r3 = ((long double)1e-40L) == ((short)(short)-3);
      printf("1447: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1e-40L); int r1 = vx == vy; int r2 = ((short)(short)-3) == ((long double)1e-40L); static int r3 = ((short)(short)-3) == ((long double)1e-40L);
      printf("1448: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile short vy = ((short)(short)-3); int r1 = vx >= vy; int r2 = ((long double)-2.5L) >= ((short)(short)-3); static int r3 = ((long double)-2.5L) >= ((short)(short)-3);
      printf("1449: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)-2.5L); int r1 = vx >= vy; int r2 = ((short)(short)-3) >= ((long double)-2.5L); static int r3 = ((short)(short)-3) >= ((long double)-2.5L);
      printf("1450: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile short vy = ((short)(short)-3); int r1 = vx >= vy; int r2 = ((long double)1e-5L) >= ((short)(short)-3); static int r3 = ((long double)1e-5L) >= ((short)(short)-3);
      printf("1451: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)1e-5L); int r1 = vx >= vy; int r2 = ((short)(short)-3) >= ((long double)1e-5L); static int r3 = ((short)(short)-3) >= ((long double)1e-5L);
      printf("1452: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile short vy = ((short)(short)-3); int r1 = vx >= vy; int r2 = ((long double)-0.1L) >= ((short)(short)-3); static int r3 = ((long double)-0.1L) >= ((short)(short)-3);
      printf("1453: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)-0.1L); int r1 = vx >= vy; int r2 = ((short)(short)-3) >= ((long double)-0.1L); static int r3 = ((short)(short)-3) >= ((long double)-0.1L);
      printf("1454: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile short vy = ((short)(short)-3); int r1 = vx >= vy; int r2 = ((long double)123456.789L) >= ((short)(short)-3); static int r3 = ((long double)123456.789L) >= ((short)(short)-3);
      printf("1455: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile short vx = ((short)(short)-3); volatile long double vy = ((long double)123456.789L); int r1 = vx >= vy; int r2 = ((short)(short)-3) >= ((long double)123456.789L); static int r3 = ((short)(short)-3) >= ((long double)123456.789L);
      printf("1456: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.3333333333333333L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx + vy; long double r2 = ((long double)0.3333333333333333L) + ((unsigned char)(unsigned char)200); static long double r3 = ((long double)0.3333333333333333L) + ((unsigned char)(unsigned char)200);
      printf("1457: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)0.3333333333333333L); long double r1 = vx + vy; long double r2 = ((unsigned char)(unsigned char)200) + ((long double)0.3333333333333333L); static long double r3 = ((unsigned char)(unsigned char)200) + ((long double)0.3333333333333333L);
      printf("1458: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx + vy; long double r2 = ((long double)1.0L) + ((unsigned char)(unsigned char)200); static long double r3 = ((long double)1.0L) + ((unsigned char)(unsigned char)200);
      printf("1459: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)1.0L); long double r1 = vx + vy; long double r2 = ((unsigned char)(unsigned char)200) + ((long double)1.0L); static long double r3 = ((unsigned char)(unsigned char)200) + ((long double)1.0L);
      printf("1460: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx + vy; long double r2 = ((long double)1e-40L) + ((unsigned char)(unsigned char)200); static long double r3 = ((long double)1e-40L) + ((unsigned char)(unsigned char)200);
      printf("1461: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)1e-40L); long double r1 = vx + vy; long double r2 = ((unsigned char)(unsigned char)200) + ((long double)1e-40L); static long double r3 = ((unsigned char)(unsigned char)200) + ((long double)1e-40L);
      printf("1462: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx + vy; long double r2 = ((long double)1e10L) + ((unsigned char)(unsigned char)200); static long double r3 = ((long double)1e10L) + ((unsigned char)(unsigned char)200);
      printf("1463: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)1e10L); long double r1 = vx + vy; long double r2 = ((unsigned char)(unsigned char)200) + ((long double)1e10L); static long double r3 = ((unsigned char)(unsigned char)200) + ((long double)1e10L);
      printf("1464: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1.0L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx - vy; long double r2 = ((long double)1.0L) - ((unsigned char)(unsigned char)200); static long double r3 = ((long double)1.0L) - ((unsigned char)(unsigned char)200);
      printf("1465: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)1.0L); long double r1 = vx - vy; long double r2 = ((unsigned char)(unsigned char)200) - ((long double)1.0L); static long double r3 = ((unsigned char)(unsigned char)200) - ((long double)1.0L);
      printf("1466: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e10L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx - vy; long double r2 = ((long double)1e10L) - ((unsigned char)(unsigned char)200); static long double r3 = ((long double)1e10L) - ((unsigned char)(unsigned char)200);
      printf("1467: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)1e10L); long double r1 = vx - vy; long double r2 = ((unsigned char)(unsigned char)200) - ((long double)1e10L); static long double r3 = ((unsigned char)(unsigned char)200) - ((long double)1e10L);
      printf("1468: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx - vy; long double r2 = ((long double)-0.1L) - ((unsigned char)(unsigned char)200); static long double r3 = ((long double)-0.1L) - ((unsigned char)(unsigned char)200);
      printf("1469: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)-0.1L); long double r1 = vx - vy; long double r2 = ((unsigned char)(unsigned char)200) - ((long double)-0.1L); static long double r3 = ((unsigned char)(unsigned char)200) - ((long double)-0.1L);
      printf("1470: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx - vy; long double r2 = ((long double)0.1L) - ((unsigned char)(unsigned char)200); static long double r3 = ((long double)0.1L) - ((unsigned char)(unsigned char)200);
      printf("1471: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)0.1L); long double r1 = vx - vy; long double r2 = ((unsigned char)(unsigned char)200) - ((long double)0.1L); static long double r3 = ((unsigned char)(unsigned char)200) - ((long double)0.1L);
      printf("1472: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e30L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx * vy; long double r2 = ((long double)1e30L) * ((unsigned char)(unsigned char)200); static long double r3 = ((long double)1e30L) * ((unsigned char)(unsigned char)200);
      printf("1473: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)1e30L); long double r1 = vx * vy; long double r2 = ((unsigned char)(unsigned char)200) * ((long double)1e30L); static long double r3 = ((unsigned char)(unsigned char)200) * ((long double)1e30L);
      printf("1474: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)7.0L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx * vy; long double r2 = ((long double)7.0L) * ((unsigned char)(unsigned char)200); static long double r3 = ((long double)7.0L) * ((unsigned char)(unsigned char)200);
      printf("1475: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)7.0L); long double r1 = vx * vy; long double r2 = ((unsigned char)(unsigned char)200) * ((long double)7.0L); static long double r3 = ((unsigned char)(unsigned char)200) * ((long double)7.0L);
      printf("1476: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx * vy; long double r2 = ((long double)-2.5L) * ((unsigned char)(unsigned char)200); static long double r3 = ((long double)-2.5L) * ((unsigned char)(unsigned char)200);
      printf("1477: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)-2.5L); long double r1 = vx * vy; long double r2 = ((unsigned char)(unsigned char)200) * ((long double)-2.5L); static long double r3 = ((unsigned char)(unsigned char)200) * ((long double)-2.5L);
      printf("1478: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx * vy; long double r2 = ((long double)1e-5L) * ((unsigned char)(unsigned char)200); static long double r3 = ((long double)1e-5L) * ((unsigned char)(unsigned char)200);
      printf("1479: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)1e-5L); long double r1 = vx * vy; long double r2 = ((unsigned char)(unsigned char)200) * ((long double)1e-5L); static long double r3 = ((unsigned char)(unsigned char)200) * ((long double)1e-5L);
      printf("1480: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)16777217.0L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx / vy; long double r2 = ((long double)16777217.0L) / ((unsigned char)(unsigned char)200); static long double r3 = ((long double)16777217.0L) / ((unsigned char)(unsigned char)200);
      printf("1481: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)16777217.0L); long double r1 = vx / vy; long double r2 = ((unsigned char)(unsigned char)200) / ((long double)16777217.0L); static long double r3 = ((unsigned char)(unsigned char)200) / ((long double)16777217.0L);
      printf("1482: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx / vy; long double r2 = ((long double)0.1L) / ((unsigned char)(unsigned char)200); static long double r3 = ((long double)0.1L) / ((unsigned char)(unsigned char)200);
      printf("1483: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)0.1L); long double r1 = vx / vy; long double r2 = ((unsigned char)(unsigned char)200) / ((long double)0.1L); static long double r3 = ((unsigned char)(unsigned char)200) / ((long double)0.1L);
      printf("1484: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)0.1L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx / vy; long double r2 = ((long double)0.1L) / ((unsigned char)(unsigned char)200); static long double r3 = ((long double)0.1L) / ((unsigned char)(unsigned char)200);
      printf("1485: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)0.1L); long double r1 = vx / vy; long double r2 = ((unsigned char)(unsigned char)200) / ((long double)0.1L); static long double r3 = ((unsigned char)(unsigned char)200) / ((long double)0.1L);
      printf("1486: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); long double r1 = vx / vy; long double r2 = ((long double)-0.1L) / ((unsigned char)(unsigned char)200); static long double r3 = ((long double)-0.1L) / ((unsigned char)(unsigned char)200);
      printf("1487: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)-0.1L); long double r1 = vx / vy; long double r2 = ((unsigned char)(unsigned char)200) / ((long double)-0.1L); static long double r3 = ((unsigned char)(unsigned char)200) / ((long double)-0.1L);
      printf("1488: %.21Lg %.21Lg %.21Lg\n", (long double)r1, (long double)r2, (long double)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx < vy; int r2 = ((long double)-2.5L) < ((unsigned char)(unsigned char)200); static int r3 = ((long double)-2.5L) < ((unsigned char)(unsigned char)200);
      printf("1489: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)-2.5L); int r1 = vx < vy; int r2 = ((unsigned char)(unsigned char)200) < ((long double)-2.5L); static int r3 = ((unsigned char)(unsigned char)200) < ((long double)-2.5L);
      printf("1490: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx < vy; int r2 = ((long double)123456.789L) < ((unsigned char)(unsigned char)200); static int r3 = ((long double)123456.789L) < ((unsigned char)(unsigned char)200);
      printf("1491: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)123456.789L); int r1 = vx < vy; int r2 = ((unsigned char)(unsigned char)200) < ((long double)123456.789L); static int r3 = ((unsigned char)(unsigned char)200) < ((long double)123456.789L);
      printf("1492: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-5L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx < vy; int r2 = ((long double)1e-5L) < ((unsigned char)(unsigned char)200); static int r3 = ((long double)1e-5L) < ((unsigned char)(unsigned char)200);
      printf("1493: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)1e-5L); int r1 = vx < vy; int r2 = ((unsigned char)(unsigned char)200) < ((long double)1e-5L); static int r3 = ((unsigned char)(unsigned char)200) < ((long double)1e-5L);
      printf("1494: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx < vy; int r2 = ((long double)-2.5L) < ((unsigned char)(unsigned char)200); static int r3 = ((long double)-2.5L) < ((unsigned char)(unsigned char)200);
      printf("1495: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)-2.5L); int r1 = vx < vy; int r2 = ((unsigned char)(unsigned char)200) < ((long double)-2.5L); static int r3 = ((unsigned char)(unsigned char)200) < ((long double)-2.5L);
      printf("1496: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.1L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx == vy; int r2 = ((long double)0.1L) == ((unsigned char)(unsigned char)200); static int r3 = ((long double)0.1L) == ((unsigned char)(unsigned char)200);
      printf("1497: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)0.1L); int r1 = vx == vy; int r2 = ((unsigned char)(unsigned char)200) == ((long double)0.1L); static int r3 = ((unsigned char)(unsigned char)200) == ((long double)0.1L);
      printf("1498: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e30L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx == vy; int r2 = ((long double)1e30L) == ((unsigned char)(unsigned char)200); static int r3 = ((long double)1e30L) == ((unsigned char)(unsigned char)200);
      printf("1499: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)1e30L); int r1 = vx == vy; int r2 = ((unsigned char)(unsigned char)200) == ((long double)1e30L); static int r3 = ((unsigned char)(unsigned char)200) == ((long double)1e30L);
      printf("1500: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)3.0L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx == vy; int r2 = ((long double)3.0L) == ((unsigned char)(unsigned char)200); static int r3 = ((long double)3.0L) == ((unsigned char)(unsigned char)200);
      printf("1501: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)3.0L); int r1 = vx == vy; int r2 = ((unsigned char)(unsigned char)200) == ((long double)3.0L); static int r3 = ((unsigned char)(unsigned char)200) == ((long double)3.0L);
      printf("1502: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)123456.789L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx == vy; int r2 = ((long double)123456.789L) == ((unsigned char)(unsigned char)200); static int r3 = ((long double)123456.789L) == ((unsigned char)(unsigned char)200);
      printf("1503: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)123456.789L); int r1 = vx == vy; int r2 = ((unsigned char)(unsigned char)200) == ((long double)123456.789L); static int r3 = ((unsigned char)(unsigned char)200) == ((long double)123456.789L);
      printf("1504: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)1e-40L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx >= vy; int r2 = ((long double)1e-40L) >= ((unsigned char)(unsigned char)200); static int r3 = ((long double)1e-40L) >= ((unsigned char)(unsigned char)200);
      printf("1505: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)1e-40L); int r1 = vx >= vy; int r2 = ((unsigned char)(unsigned char)200) >= ((long double)1e-40L); static int r3 = ((unsigned char)(unsigned char)200) >= ((long double)1e-40L);
      printf("1506: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)0.1L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx >= vy; int r2 = ((long double)0.1L) >= ((unsigned char)(unsigned char)200); static int r3 = ((long double)0.1L) >= ((unsigned char)(unsigned char)200);
      printf("1507: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)0.1L); int r1 = vx >= vy; int r2 = ((unsigned char)(unsigned char)200) >= ((long double)0.1L); static int r3 = ((unsigned char)(unsigned char)200) >= ((long double)0.1L);
      printf("1508: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-0.1L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx >= vy; int r2 = ((long double)-0.1L) >= ((unsigned char)(unsigned char)200); static int r3 = ((long double)-0.1L) >= ((unsigned char)(unsigned char)200);
      printf("1509: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)-0.1L); int r1 = vx >= vy; int r2 = ((unsigned char)(unsigned char)200) >= ((long double)-0.1L); static int r3 = ((unsigned char)(unsigned char)200) >= ((long double)-0.1L);
      printf("1510: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile long double vx = ((long double)-2.5L); volatile unsigned char vy = ((unsigned char)(unsigned char)200); int r1 = vx >= vy; int r2 = ((long double)-2.5L) >= ((unsigned char)(unsigned char)200); static int r3 = ((long double)-2.5L) >= ((unsigned char)(unsigned char)200);
      printf("1511: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { volatile unsigned char vx = ((unsigned char)(unsigned char)200); volatile long double vy = ((long double)-2.5L); int r1 = vx >= vy; int r2 = ((unsigned char)(unsigned char)200) >= ((long double)-2.5L); static int r3 = ((unsigned char)(unsigned char)200) >= ((long double)-2.5L);
      printf("1512: %d %d %d\n", (int)r1, (int)r2, (int)r3); }
    { int x = (int)3; volatile float y = (float)1.75f; x += y; float z = (float)2.5f; z += (int)3; printf("1513: %d %.9g\n", x, (double)z); }
    { int x = (int)3; volatile float y = (float)1.75f; x -= y; float z = (float)2.5f; z -= (int)3; printf("1514: %d %.9g\n", x, (double)z); }
    { int x = (int)3; volatile float y = (float)1.75f; x *= y; float z = (float)2.5f; z *= (int)3; printf("1515: %d %.9g\n", x, (double)z); }
    { int x = (int)3; volatile float y = (float)1.75f; x /= y; float z = (float)2.5f; z /= (int)3; printf("1516: %d %.9g\n", x, (double)z); }
    { int x = (int)3; volatile double y = (double)1.75; x += y; double z = (double)2.5; z += (int)3; printf("1517: %d %.17g\n", x, (double)z); }
    { int x = (int)3; volatile double y = (double)1.75; x -= y; double z = (double)2.5; z -= (int)3; printf("1518: %d %.17g\n", x, (double)z); }
    { int x = (int)3; volatile double y = (double)1.75; x *= y; double z = (double)2.5; z *= (int)3; printf("1519: %d %.17g\n", x, (double)z); }
    { int x = (int)3; volatile double y = (double)1.75; x /= y; double z = (double)2.5; z /= (int)3; printf("1520: %d %.17g\n", x, (double)z); }
    { int x = (int)3; volatile long double y = (long double)1.75L; x += y; long double z = (long double)2.5L; z += (int)3; printf("1521: %d %.21Lg\n", x, (long double)z); }
    { int x = (int)3; volatile long double y = (long double)1.75L; x -= y; long double z = (long double)2.5L; z -= (int)3; printf("1522: %d %.21Lg\n", x, (long double)z); }
    { int x = (int)3; volatile long double y = (long double)1.75L; x *= y; long double z = (long double)2.5L; z *= (int)3; printf("1523: %d %.21Lg\n", x, (long double)z); }
    { int x = (int)3; volatile long double y = (long double)1.75L; x /= y; long double z = (long double)2.5L; z /= (int)3; printf("1524: %d %.21Lg\n", x, (long double)z); }
    { unsigned x = (unsigned)3u; volatile float y = (float)1.75f; x += y; float z = (float)2.5f; z += (unsigned)3u; printf("1525: %u %.9g\n", x, (double)z); }
    { unsigned x = (unsigned)3u; volatile float y = (float)1.75f; x -= y; float z = (float)2.5f; z -= (unsigned)3u; printf("1526: %u %.9g\n", x, (double)z); }
    { unsigned x = (unsigned)3u; volatile float y = (float)1.75f; x *= y; float z = (float)2.5f; z *= (unsigned)3u; printf("1527: %u %.9g\n", x, (double)z); }
    { unsigned x = (unsigned)3u; volatile float y = (float)1.75f; x /= y; float z = (float)2.5f; z /= (unsigned)3u; printf("1528: %u %.9g\n", x, (double)z); }
    { unsigned x = (unsigned)3u; volatile double y = (double)1.75; x += y; double z = (double)2.5; z += (unsigned)3u; printf("1529: %u %.17g\n", x, (double)z); }
    { unsigned x = (unsigned)3u; volatile double y = (double)1.75; x -= y; double z = (double)2.5; z -= (unsigned)3u; printf("1530: %u %.17g\n", x, (double)z); }
    { unsigned x = (unsigned)3u; volatile double y = (double)1.75; x *= y; double z = (double)2.5; z *= (unsigned)3u; printf("1531: %u %.17g\n", x, (double)z); }
    { unsigned x = (unsigned)3u; volatile double y = (double)1.75; x /= y; double z = (double)2.5; z /= (unsigned)3u; printf("1532: %u %.17g\n", x, (double)z); }
    { unsigned x = (unsigned)3u; volatile long double y = (long double)1.75L; x += y; long double z = (long double)2.5L; z += (unsigned)3u; printf("1533: %u %.21Lg\n", x, (long double)z); }
    { unsigned x = (unsigned)3u; volatile long double y = (long double)1.75L; x -= y; long double z = (long double)2.5L; z -= (unsigned)3u; printf("1534: %u %.21Lg\n", x, (long double)z); }
    { unsigned x = (unsigned)3u; volatile long double y = (long double)1.75L; x *= y; long double z = (long double)2.5L; z *= (unsigned)3u; printf("1535: %u %.21Lg\n", x, (long double)z); }
    { unsigned x = (unsigned)3u; volatile long double y = (long double)1.75L; x /= y; long double z = (long double)2.5L; z /= (unsigned)3u; printf("1536: %u %.21Lg\n", x, (long double)z); }
    { long long x = (long long)-5LL; volatile float y = (float)1.75f; x += y; float z = (float)2.5f; z += (long long)-5LL; printf("1537: %lld %.9g\n", x, (double)z); }
    { long long x = (long long)-5LL; volatile float y = (float)1.75f; x -= y; float z = (float)2.5f; z -= (long long)-5LL; printf("1538: %lld %.9g\n", x, (double)z); }
    { long long x = (long long)-5LL; volatile float y = (float)1.75f; x *= y; float z = (float)2.5f; z *= (long long)-5LL; printf("1539: %lld %.9g\n", x, (double)z); }
    { long long x = (long long)-5LL; volatile float y = (float)1.75f; x /= y; float z = (float)2.5f; z /= (long long)-5LL; printf("1540: %lld %.9g\n", x, (double)z); }
    { long long x = (long long)-5LL; volatile double y = (double)1.75; x += y; double z = (double)2.5; z += (long long)-5LL; printf("1541: %lld %.17g\n", x, (double)z); }
    { long long x = (long long)-5LL; volatile double y = (double)1.75; x -= y; double z = (double)2.5; z -= (long long)-5LL; printf("1542: %lld %.17g\n", x, (double)z); }
    { long long x = (long long)-5LL; volatile double y = (double)1.75; x *= y; double z = (double)2.5; z *= (long long)-5LL; printf("1543: %lld %.17g\n", x, (double)z); }
    { long long x = (long long)-5LL; volatile double y = (double)1.75; x /= y; double z = (double)2.5; z /= (long long)-5LL; printf("1544: %lld %.17g\n", x, (double)z); }
    { long long x = (long long)-5LL; volatile long double y = (long double)1.75L; x += y; long double z = (long double)2.5L; z += (long long)-5LL; printf("1545: %lld %.21Lg\n", x, (long double)z); }
    { long long x = (long long)-5LL; volatile long double y = (long double)1.75L; x -= y; long double z = (long double)2.5L; z -= (long long)-5LL; printf("1546: %lld %.21Lg\n", x, (long double)z); }
    { long long x = (long long)-5LL; volatile long double y = (long double)1.75L; x *= y; long double z = (long double)2.5L; z *= (long long)-5LL; printf("1547: %lld %.21Lg\n", x, (long double)z); }
    { long long x = (long long)-5LL; volatile long double y = (long double)1.75L; x /= y; long double z = (long double)2.5L; z /= (long long)-5LL; printf("1548: %lld %.21Lg\n", x, (long double)z); }
    { unsigned long long x = (unsigned long long)18446744073709551615ULL; volatile float y = (float)1.75f; x += y; float z = (float)2.5f; z += (unsigned long long)18446744073709551615ULL; printf("1549: %llu %.9g\n", x, (double)z); }
    { unsigned long long x = (unsigned long long)18446744073709551615ULL; volatile float y = (float)1.75f; x -= y; float z = (float)2.5f; z -= (unsigned long long)18446744073709551615ULL; printf("1550: %llu %.9g\n", x, (double)z); }
    { unsigned long long x = (unsigned long long)18446744073709551615ULL; volatile float y = (float)1.75f; x *= y; float z = (float)2.5f; z *= (unsigned long long)18446744073709551615ULL; printf("1551: %llu %.9g\n", x, (double)z); }
    { unsigned long long x = (unsigned long long)18446744073709551615ULL; volatile float y = (float)1.75f; x /= y; float z = (float)2.5f; z /= (unsigned long long)18446744073709551615ULL; printf("1552: %llu %.9g\n", x, (double)z); }
    { unsigned long long x = (unsigned long long)18446744073709551615ULL; volatile double y = (double)1.75; x += y; double z = (double)2.5; z += (unsigned long long)18446744073709551615ULL; printf("1553: %llu %.17g\n", x, (double)z); }
    { unsigned long long x = (unsigned long long)18446744073709551615ULL; volatile double y = (double)1.75; x -= y; double z = (double)2.5; z -= (unsigned long long)18446744073709551615ULL; printf("1554: %llu %.17g\n", x, (double)z); }
    { unsigned long long x = (unsigned long long)18446744073709551615ULL; volatile double y = (double)1.75; x *= y; double z = (double)2.5; z *= (unsigned long long)18446744073709551615ULL; printf("1555: %llu %.17g\n", x, (double)z); }
    { unsigned long long x = (unsigned long long)18446744073709551615ULL; volatile double y = (double)1.75; x /= y; double z = (double)2.5; z /= (unsigned long long)18446744073709551615ULL; printf("1556: %llu %.17g\n", x, (double)z); }
    { unsigned long long x = (unsigned long long)18446744073709551615ULL; volatile long double y = (long double)1.75L; x += y; long double z = (long double)2.5L; z += (unsigned long long)18446744073709551615ULL; printf("1557: %llu %.21Lg\n", x, (long double)z); }
    { unsigned long long x = (unsigned long long)18446744073709551615ULL; volatile long double y = (long double)1.75L; x -= y; long double z = (long double)2.5L; z -= (unsigned long long)18446744073709551615ULL; printf("1558: %llu %.21Lg\n", x, (long double)z); }
    { unsigned long long x = (unsigned long long)18446744073709551615ULL; volatile long double y = (long double)1.75L; x *= y; long double z = (long double)2.5L; z *= (unsigned long long)18446744073709551615ULL; printf("1559: %llu %.21Lg\n", x, (long double)z); }
    { unsigned long long x = (unsigned long long)18446744073709551615ULL; volatile long double y = (long double)1.75L; x /= y; long double z = (long double)2.5L; z /= (unsigned long long)18446744073709551615ULL; printf("1560: %llu %.21Lg\n", x, (long double)z); }
    { short x = (short)(short)-3; volatile float y = (float)1.75f; x += y; float z = (float)2.5f; z += (short)(short)-3; printf("1561: %d %.9g\n", x, (double)z); }
    { short x = (short)(short)-3; volatile float y = (float)1.75f; x -= y; float z = (float)2.5f; z -= (short)(short)-3; printf("1562: %d %.9g\n", x, (double)z); }
    { short x = (short)(short)-3; volatile float y = (float)1.75f; x *= y; float z = (float)2.5f; z *= (short)(short)-3; printf("1563: %d %.9g\n", x, (double)z); }
    { short x = (short)(short)-3; volatile float y = (float)1.75f; x /= y; float z = (float)2.5f; z /= (short)(short)-3; printf("1564: %d %.9g\n", x, (double)z); }
    { short x = (short)(short)-3; volatile double y = (double)1.75; x += y; double z = (double)2.5; z += (short)(short)-3; printf("1565: %d %.17g\n", x, (double)z); }
    { short x = (short)(short)-3; volatile double y = (double)1.75; x -= y; double z = (double)2.5; z -= (short)(short)-3; printf("1566: %d %.17g\n", x, (double)z); }
    { short x = (short)(short)-3; volatile double y = (double)1.75; x *= y; double z = (double)2.5; z *= (short)(short)-3; printf("1567: %d %.17g\n", x, (double)z); }
    { short x = (short)(short)-3; volatile double y = (double)1.75; x /= y; double z = (double)2.5; z /= (short)(short)-3; printf("1568: %d %.17g\n", x, (double)z); }
    { short x = (short)(short)-3; volatile long double y = (long double)1.75L; x += y; long double z = (long double)2.5L; z += (short)(short)-3; printf("1569: %d %.21Lg\n", x, (long double)z); }
    { short x = (short)(short)-3; volatile long double y = (long double)1.75L; x -= y; long double z = (long double)2.5L; z -= (short)(short)-3; printf("1570: %d %.21Lg\n", x, (long double)z); }
    { short x = (short)(short)-3; volatile long double y = (long double)1.75L; x *= y; long double z = (long double)2.5L; z *= (short)(short)-3; printf("1571: %d %.21Lg\n", x, (long double)z); }
    { short x = (short)(short)-3; volatile long double y = (long double)1.75L; x /= y; long double z = (long double)2.5L; z /= (short)(short)-3; printf("1572: %d %.21Lg\n", x, (long double)z); }
    { unsigned char x = (unsigned char)(unsigned char)200; volatile float y = (float)1.75f; x += y; float z = (float)2.5f; z += (unsigned char)(unsigned char)200; printf("1573: %d %.9g\n", x, (double)z); }
    { unsigned char x = (unsigned char)(unsigned char)200; volatile float y = (float)1.75f; x -= y; float z = (float)2.5f; z -= (unsigned char)(unsigned char)200; printf("1574: %d %.9g\n", x, (double)z); }
    { unsigned char x = (unsigned char)(unsigned char)200; volatile float y = (float)1.75f; x /= y; float z = (float)2.5f; z /= (unsigned char)(unsigned char)200; printf("1575: %d %.9g\n", x, (double)z); }
    { unsigned char x = (unsigned char)(unsigned char)200; volatile double y = (double)1.75; x += y; double z = (double)2.5; z += (unsigned char)(unsigned char)200; printf("1576: %d %.17g\n", x, (double)z); }
    { unsigned char x = (unsigned char)(unsigned char)200; volatile double y = (double)1.75; x -= y; double z = (double)2.5; z -= (unsigned char)(unsigned char)200; printf("1577: %d %.17g\n", x, (double)z); }
    { unsigned char x = (unsigned char)(unsigned char)200; volatile double y = (double)1.75; x /= y; double z = (double)2.5; z /= (unsigned char)(unsigned char)200; printf("1578: %d %.17g\n", x, (double)z); }
    { unsigned char x = (unsigned char)(unsigned char)200; volatile long double y = (long double)1.75L; x += y; long double z = (long double)2.5L; z += (unsigned char)(unsigned char)200; printf("1579: %d %.21Lg\n", x, (long double)z); }
    { unsigned char x = (unsigned char)(unsigned char)200; volatile long double y = (long double)1.75L; x -= y; long double z = (long double)2.5L; z -= (unsigned char)(unsigned char)200; printf("1580: %d %.21Lg\n", x, (long double)z); }
    { unsigned char x = (unsigned char)(unsigned char)200; volatile long double y = (long double)1.75L; x /= y; long double z = (long double)2.5L; z /= (unsigned char)(unsigned char)200; printf("1581: %d %.21Lg\n", x, (long double)z); }
    return 0;
}
