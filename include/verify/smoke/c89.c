/* Strict C89 smoke test using only C89 features in the program itself.  */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct pt { char c; double d; };

static double
sum (int n, ...)
{
  va_list ap;
  double s = 0;
  int i;
  va_start (ap, n);
  for (i = 0; i < n; i++)
    s += va_arg (ap, double);
  va_end (ap);
  return s;
}

int
main (void)
{
  jmp_buf jb;
  volatile int k = 0;
  char buf[32];
  printf ("sum %.1f offsetof %d\n", sum (3, 1.0, 2.0, 3.5),
          (int) offsetof (struct pt, d));
  printf ("INT_MAX %d CHAR_MIN %d DBL_EPSILON %g\n", INT_MAX, CHAR_MIN,
          DBL_EPSILON);
  printf ("sqrt %.3f isnan %d\n", sqrt (2.0), isnan (sqrt (-1.0)) != 0);
  if (setjmp (jb) == 0)
    {
      k = 1;
      longjmp (jb, 1);
    }
  printf ("k %d\n", k);
  sprintf (buf, "%5.1f|%-3d|", 2.25, 7);
  printf ("%s %d\n", buf, toupper ('a'));
  assert (k == 1);
  errno = 0;
  return 0;
}
