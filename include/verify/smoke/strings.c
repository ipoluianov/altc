/* Smoke test: <string.h>, <strings.h>, <ctype.h>, <wchar.h>, <wctype.h>,
   <locale.h>, <uchar.h>, <inttypes.h>.  */
#include <ctype.h>
#include <inttypes.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <wchar.h>
#include <wctype.h>

int
main (void)
{
  char buf[64], *p, *save, *dup;
  char s[] = "alpha,beta,,gamma";
  wchar_t wbuf[32];
  const char *mb = "h\xc3\xa9llo";
  struct lconv *lc;
  int i, cnt;
  intmax_t im;
  int64_t i64 = INT64_C (-1234567890123);
  uint32_t u32 = UINT32_MAX;
  mbstate_t st;

  strcpy (buf, "Hello");
  strcat (buf, ", world");
  printf ("%s len %zu cmp %d ncmp %d\n", buf, strlen (buf),
          strcmp ("a", "b") < 0, strncmp ("abc", "abd", 2));
  printf ("chr %s rchr %s str %s spn %zu cspn %zu pbrk %s\n",
          strchr (buf, 'o'), strrchr (buf, 'o'), strstr (buf, "wor"),
          strspn ("aaab", "a"), strcspn ("abc", "c"), strpbrk (buf, ",!"));
  for (p = strtok_r (s, ",", &save); p; p = strtok_r (NULL, ",", &save))
    printf ("[%s]", p);
  printf ("\n");
  dup = strdup ("duplicate");
  printf ("%s %s\n", dup, strndup ("abcdef", 3));
  free (dup);
  printf ("nlen %zu stpcpy %s\n", strnlen ("abcdef", 3),
          stpcpy (buf, "xy") - 2);
  printf ("casecmp %d %d memchr %s\n", strcasecmp ("HeLLo", "hello"),
          strncasecmp ("ABx", "aby", 2), (char *) memchr ("find", 'n', 4));
  memset (buf, 'x', 5);
  buf[5] = 0;
  memmove (buf + 1, buf, 3);
  printf ("%s memcmp %d\n", buf, memcmp ("ab", "ac", 2) < 0);
  printf ("strerror %s\n", strerror (2));
  if (strerror_r (2, buf, sizeof buf) == 0)
    printf ("strerror_r %s\n", buf);
  printf ("strsignal %s\n", strsignal (2));
  printf ("ffs %d index %s\n", ffs (8), index ("abc", 'b'));
  p = (char *) memmem ("haystack", 8, "st", 2);
  printf ("memmem %s strcasestr %s\n", p, strcasestr ("HayStack", "STA"));

  cnt = 0;
  for (i = 0; i < 128; i++)
    cnt += isalpha (i) ? 1 : 0;
  printf ("alpha %d digit %d space %d upper %c lower %c punct %d\n", cnt,
          isdigit ('7') != 0, isspace ('\t') != 0, toupper ('q'),
          tolower ('Q'), ispunct ('!') != 0);
  printf ("xdigit %d cntrl %d print %d graph %d blank %d\n",
          isxdigit ('f') != 0, iscntrl ('\n') != 0, isprint (' ') != 0,
          isgraph (' ') != 0, isblank (' ') != 0);

  printf ("setlocale %s\n", setlocale (LC_ALL, "C.UTF-8") ? "ok" : "fail");
  lc = localeconv ();
  printf ("decimal_point '%s'\n", lc->decimal_point);
  memset (&st, 0, sizeof st);
  printf ("mbsinit %d mblen %d\n", mbsinit (&st), mblen (mb + 1, 4));
  printf ("mbstowcs %zu\n", mbstowcs (wbuf, mb, 32));
  printf ("wcslen %zu wc[1] %x iswalpha %d towupper %lc\n", wcslen (wbuf),
          (unsigned) wbuf[1], iswalpha (wbuf[1]) != 0,
          (wint_t) towupper (L'a'));
  swprintf (wbuf, 32, L"%ls-%d", L"wide", 5);
  printf ("swprintf %ls cmp %d\n", wbuf, wcscmp (wbuf, L"wide-5"));
  printf ("wcstol %ld\n", wcstol (L"  42", NULL, 10));

  im = strtoimax ("-9000000000", NULL, 10);
  printf ("imax %" PRIdMAX " i64 %" PRId64 " u32 %" PRIu32 " x %" PRIx64
          "\n", im, i64, u32, (uint64_t) 255);
  printf ("imaxabs %" PRIdMAX "\n", imaxabs (im));
  {
    int64_t v;
    if (sscanf ("77", "%" SCNd64, &v) == 1)
      printf ("scn %" PRId64 "\n", v);
  }
  return 0;
}
