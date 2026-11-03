/* Smoke test: <stdio.h>, <stdlib.h>, <stdarg.h>, <errno.h>, <limits.h>.  */
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int
cmp_int (const void *a, const void *b)
{
  int x = *(const int *) a, y = *(const int *) b;
  return (x > y) - (x < y);
}

static int
fmt (char *buf, size_t n, const char *f, ...)
{
  va_list ap, ap2;
  int r;
  va_start (ap, f);
  va_copy (ap2, ap);
  r = vsnprintf (buf, n, f, ap);
  if (r != vsnprintf (NULL, 0, f, ap2))
    r = -1;
  va_end (ap2);
  va_end (ap);
  return r;
}

static int exit_seen;
static void
on_exit_fn (void)
{
  if (!exit_seen)
    puts ("atexit handler ran");
}

int
main (void)
{
  char buf[256], *line = NULL, *mem = NULL;
  int arr[8] = { 5, 3, 9, 1, 7, 2, 8, 6 }, key = 7, *found, i, n;
  size_t cap = 0, memsize = 0;
  long l;
  double d;
  char *end;
  FILE *f;
  fpos_t pos;
  div_t dv;
  ldiv_t ldv;
  char tmpl[] = "/tmp/altc_smokeXXXXXX";
  int fd;

  atexit (on_exit_fn);
  printf ("%d %5.2f %-5s| %x %o %c %e %g %p\n", 42, 3.14159, "ab", 255, 8,
          'z', 12345.678, 0.0001, (void *) 0);
  printf ("%ld %lu %zu %td %lld %llu\n", LONG_MAX, ULONG_MAX,
          sizeof (long double), (ptrdiff_t) -3, (long long) LLONG_MIN,
          (unsigned long long) ULLONG_MAX);
  n = fmt (buf, sizeof buf, "%s-%d-%.3f", "x", 7, 2.5);
  printf ("fmt: %s (%d)\n", buf, n);
  n = snprintf (NULL, 0, "%d", 123456);
  printf ("snprintf length: %d\n", n);

  qsort (arr, 8, sizeof (int), cmp_int);
  for (i = 0; i < 8; i++)
    printf ("%d ", arr[i]);
  found = (int *) bsearch (&key, arr, 8, sizeof (int), cmp_int);
  printf ("found %d at %d\n", *found, (int) (found - arr));

  l = strtol ("  -123xyz", &end, 10);
  printf ("strtol %ld rest '%s'\n", l, end);
  errno = 0;
  l = strtol ("99999999999999999999999", NULL, 10);
  printf ("overflow %ld errno ERANGE %d\n", l, errno == ERANGE);
  d = strtod ("1.5e3", NULL);
  printf ("strtod %g atoi %d atol %ld\n", d, atoi ("77"), atol ("-5"));
  printf ("strtoul %lu strtoull %llu\n", strtoul ("ff", NULL, 16),
          strtoull ("777", NULL, 8));
  dv = div (17, 5);
  ldv = ldiv (-17L, 5L);
  printf ("div %d %d ldiv %ld %ld abs %d labs %ld\n", dv.quot, dv.rem,
          ldv.quot, ldv.rem, abs (-4), labs (-9L));

  n = sscanf ("12 abc 3.5 0x1f", "%d %s %lf %i", &i, buf, &d, &key);
  printf ("sscanf %d: %d %s %g %d\n", n, i, buf, d, key);

  setenv ("ALTC_TEST_VAR", "hello", 1);
  printf ("getenv %s\n", getenv ("ALTC_TEST_VAR"));
  unsetenv ("ALTC_TEST_VAR");
  printf ("unset %d\n", getenv ("ALTC_TEST_VAR") == NULL);

  {
    unsigned int seed = 1;
    int a = rand_r (&seed);
    seed = 1;
    printf ("rand_r repeatable %d RAND_MAX %d\n", a == rand_r (&seed),
            RAND_MAX);
  }

  fd = mkstemp (tmpl);
  f = fdopen (fd, "w+");
  fprintf (f, "line one\nline two\nthird\n");
  fflush (f);
  rewind (f);
  fgetpos (f, &pos);
  while (getline (&line, &cap, f) != -1)
    printf ("read: %s", line);
  printf ("ftell %ld feof %d\n", ftell (f), feof (f) != 0);
  fsetpos (f, &pos);
  printf ("fgetc %c\n", fgetc (f));
  fseek (f, -6, SEEK_END);
  if (fgets (buf, sizeof buf, f))
    printf ("tail %s", buf);
  printf ("fileno ok %d\n", fileno (f) == fd);
  fclose (f);
  remove (tmpl);
  free (line);

  f = open_memstream (&mem, &memsize);
  fprintf (f, "memstream %d", 5);
  fclose (f);
  printf ("%s (%zu)\n", mem, memsize);
  free (mem);

  f = fmemopen (buf, sizeof buf, "w");
  fputs ("fmem", f);
  fputc ('!', f);
  fclose (f);
  printf ("%s\n", buf);

  f = popen ("echo from-popen", "r");
  if (f && fgets (buf, sizeof buf, f))
    printf ("%s", buf);
  printf ("pclose %d\n", f ? pclose (f) : -1);

  {
    void *p = malloc (10), *q;
    q = realloc (p, 1000);
    memset (q, 0, 1000);
    p = calloc (4, 8);
    printf ("alloc ok %d\n", p != NULL && q != NULL);
    free (p);
    free (q);
    if (posix_memalign (&p, 64, 128) == 0)
      printf ("aligned %d\n", ((unsigned long) p & 63) == 0);
    free (p);
  }
  fprintf (stderr, "%s", "");
  printf ("stdin %d stdout %d stderr %d\n", fileno (stdin), fileno (stdout),
          fileno (stderr));
  printf ("EOF %d BUFSIZ %d FILENAME_MAX %d\n", EOF, BUFSIZ, FILENAME_MAX);
  return EXIT_SUCCESS;
}
