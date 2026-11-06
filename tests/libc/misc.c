// The C library: sorting and searching, time, regular expressions,
// patterns, options, iconv, allocation, threads, signals, non-local jumps
#define _GNU_SOURCE
#include <errno.h>
#include <fnmatch.h>
#include <getopt.h>
#include <iconv.h>
#include <pthread.h>
#include <regex.h>
#include <search.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static unsigned long long rng = 88172645463325252ULL;
static unsigned next(void) {
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return (unsigned)rng;
}

static int cmpint(const void *a, const void *b) {
	int x = *(const int *)a, y = *(const int *)b;
	return (x > y) - (x < y);
}
static int cmpstr(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }
static int cmpmod(const void *a, const void *b, void *m) {
	int x = *(const int *)a % *(int *)m, y = *(const int *)b % *(int *)m;
	return (x > y) - (x < y);
}

static void sorting(void) {
	static int a[5000];
	unsigned h = 0;
	for (int n = 0; n < 5000; n = n * 3 + 1) {
		for (int i = 0; i < n; i++)
			a[i] = (int)(next() % 1000) - 500;
		qsort(a, n, sizeof *a, cmpint);
		int ok = 1;
		for (int i = 1; i < n; i++)
			ok &= a[i - 1] <= a[i];
		for (int i = 0; i < n; i++)
			h = h * 31 + (unsigned)a[i];
		int key = 17;
		int *f = bsearch(&key, a, n, sizeof *a, cmpint);
		printf("qsort %d %d %d\n", n, ok, f ? *f : -1);
	}
	int m = 7;
	qsort_r(a, 100, sizeof *a, cmpmod, &m);
	for (int i = 0; i < 100; i++)
		h = h * 31 + (unsigned)(a[i] % 7);
	const char *w[] = {"pear", "apple", "fig", "banana", "kiwi", "apricot", "date"};
	char *ws[7];
	memcpy(ws, w, sizeof ws);
	qsort(ws, 7, sizeof *ws, cmpstr);
	for (int i = 0; i < 7; i++)
		printf("%s ", ws[i]);
	printf("%08x\n", h);
	// search.h: hash table and binary tree
	hcreate(64);
	for (int i = 0; i < 7; i++)
		hsearch((ENTRY){(char *)w[i], (void *)(intptr_t)i}, ENTER);
	ENTRY *e = hsearch((ENTRY){"fig", 0}, FIND);
	printf("hsearch %d %p\n", e ? (int)(intptr_t)e->data : -1, (void *)hsearch((ENTRY){"none", 0}, FIND));
	hdestroy();
	void *root = 0;
	for (int i = 0; i < 7; i++)
		tsearch(w[i], &root, (int (*)(const void *, const void *))strcmp);
	printf("tfind %s\n", *(char **)tfind("kiwi", &root, (int (*)(const void *, const void *))strcmp));
}

static void times(void) {
	setenv("TZ", "UTC0", 1);
	tzset();
	time_t ts[] = {0, 951782400, 1234567890, 2147483647, 4102444800LL, -86400, 1700000000};
	for (size_t i = 0; i < sizeof ts / sizeof *ts; i++) {
		struct tm tm;
		gmtime_r(&ts[i], &tm);
		char buf[200];
		strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S %a %A %b %B %j %U %W %V %G %u %w %y %C %e %p %I %Z %z %s %%", &tm);
		time_t back = timegm(&tm);
		struct tm lt;
		localtime_r(&ts[i], &lt);
		time_t mk = mktime(&lt);
		printf("%lld: %s | %lld %lld %s", (long long)ts[i], buf, (long long)back, (long long)mk, asctime(&tm));
	}
	struct tm tm = {0};
	char *r = strptime("2024-02-29 13:45:01", "%Y-%m-%d %H:%M:%S", &tm);
	printf("strptime %d %d-%d-%d %d:%d:%d wday %d\n", r != 0, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min,
		tm.tm_sec, tm.tm_wday);
	tm = (struct tm){.tm_year = 124, .tm_mon = 13, .tm_mday = 35, .tm_hour = 25};
	time_t t = timegm(&tm);
	printf("normalized %lld %d-%d-%d %d\n", (long long)t, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour);
	setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
	tzset();
	time_t summer = 1720000000;
	struct tm l;
	localtime_r(&summer, &l);
	char b[64];
	strftime(b, sizeof b, "%F %T %Z %z", &l);
	printf("tz %s %d %ld\n", b, l.tm_isdst, (long)timezone);
	struct timespec mono;
	printf("clock %d\n", clock_gettime(CLOCK_MONOTONIC, &mono) == 0 && (mono.tv_sec > 0 || mono.tv_nsec > 0));
	printf("difftime %g\n", difftime(100, 40));
}

static void regexes(void) {
	const char *pats[] = {"^a.*b$", "(foo|bar)+", "[[:digit:]]{2,3}", "x*", "\\(ab\\)*c", "[^a-z]", "a{,2}b", "(a)(b)?(c)"};
	const int flags[] = {REG_EXTENDED, REG_EXTENDED, REG_EXTENDED, 0, 0, REG_EXTENDED | REG_ICASE, REG_EXTENDED, REG_EXTENDED};
	const char *subj[] = {"axxb", "foobarfoo!", "abc12345", "", "ababc", "abcD", "aab", "ac"};
	for (size_t i = 0; i < sizeof pats / sizeof *pats; i++) {
		regex_t re;
		int rc = regcomp(&re, pats[i], flags[i]);
		printf("re %s %d", pats[i], rc);
		for (size_t j = 0; j < sizeof subj / sizeof *subj && rc == 0; j++) {
			regmatch_t m[4];
			int e = regexec(&re, subj[j], 4, m, 0);
			printf(" %d", e);
			if (e == 0)
				for (int k = 0; k < 4; k++)
					printf(":%d-%d", (int)m[k].rm_so, (int)m[k].rm_eo);
		}
		if (rc == 0)
			regfree(&re);
		printf("\n");
	}
	regex_t re;
	char err[64];
	int rc = regcomp(&re, "a(b", REG_EXTENDED);
	regerror(rc, &re, err, sizeof err);
	printf("regerror %d %s\n", rc, err);
	const char *fp[] = {"*.c", "a?c", "[a-c]*", "\\*", "*/x", "[!a]b"};
	const char *fs[] = {"main.c", "abc", "cat", "*", "dir/x", "bb", ".hidden.c"};
	for (size_t i = 0; i < sizeof fp / sizeof *fp; i++) {
		printf("fnmatch %s", fp[i]);
		for (size_t j = 0; j < sizeof fs / sizeof *fs; j++)
			printf(" %d", fnmatch(fp[i], fs[j], FNM_PATHNAME | FNM_PERIOD));
		printf("\n");
	}
}

static void options(void) {
	char *argv[] = {"prog", "-a", "-b", "val", "--long=x", "-cfile", "--flag", "rest", "-z", 0};
	int argc = 9, c, idx;
	struct option lo[] = {{"long", required_argument, 0, 'L'}, {"flag", no_argument, 0, 'F'}, {0}};
	opterr = 0;
	optind = 1;
	while ((c = getopt_long(argc, argv, "ab:c:", lo, &idx)) != -1)
		printf("opt %c %s\n", c, optarg ? optarg : "-");
	printf("optind %d %s\n", optind, argv[optind]);
}

static void conversions(void) {
	iconv_t cd = iconv_open("UTF-16LE", "UTF-8");
	char in[] = "Grü\xc3\x9f \xe2\x82\xac", out[64], *ip = in, *op = out;
	size_t il = strlen(in), ol = sizeof out;
	size_t r = iconv(cd, &ip, &il, &op, &ol);
	printf("iconv %zu %zu:", r, sizeof out - ol);
	for (char *p = out; p < op; p++)
		printf(" %02x", (unsigned char)*p);
	printf("\n");
	iconv_close(cd);
	cd = iconv_open("ISO-8859-1", "UTF-8");
	char in2[] = "caf\xc3\xa9 \xe2\x82\xac", *ip2 = in2;
	il = strlen(in2);
	op = out;
	ol = sizeof out;
	r = iconv(cd, &ip2, &il, &op, &ol);
	printf("latin1 %d %d %zu\n", (int)r, errno == EILSEQ, sizeof out - ol);
	iconv_close(cd);
	// EUC-KR: a valid syllable, an extended (UHC) one, and lead bytes
	// above the extended range (CVE-2025-26519)
	const char *kr[] = {"\xb0\xa1", "\x81\x41", "\xc6\x52", "\xc6\x53", "\xc7\x41", "\xfe\x41"};
	for (size_t i = 0; i < sizeof kr / sizeof *kr; i++) {
		cd = iconv_open("UTF-8", "EUC-KR");
		char *kp = (char *)kr[i];
		il = 2;
		op = out;
		ol = sizeof out;
		errno = 0;
		r = iconv(cd, &kp, &il, &op, &ol);
		int err = errno == EILSEQ;
		printf("euckr %zu %d %d:", i, (int)r, err);
		for (char *p = out; p < op; p++)
			printf(" %02x", (unsigned char)*p);
		printf("\n");
		iconv_close(cd);
	}
}

static void allocation(void) {
	enum { SLOTS = 512 };
	static void *p[SLOTS];
	static size_t sz[SLOTS];
	unsigned h = 0;
	for (int round = 0; round < 20000; round++) {
		int i = next() % SLOTS;
		if (p[i]) {
			unsigned char *q = p[i];
			for (size_t k = 0; k < sz[i]; k++)
				if (q[k] != (unsigned char)(i + k)) {
					printf("corrupt %d\n", i);
					return;
				}
			if (next() % 3 == 0) {
				size_t n = next() % 5000;
				q = realloc(q, n);
				for (size_t k = sz[i]; k < n; k++)
					q[k] = (unsigned char)(i + k);
				p[i] = q;
				sz[i] = n;
				continue;
			}
			free(q);
			p[i] = 0;
			continue;
		}
		size_t n = next() % 3 == 0 ? next() % 200000 : next() % 100;
		unsigned char *q = next() % 4 ? malloc(n) : calloc(n, 1);
		if (!q)
			continue;
		for (size_t k = 0; k < n; k++)
			q[k] = (unsigned char)(i + k);
		p[i] = q;
		sz[i] = n;
		h += (uintptr_t)q % 16 == 0; /* not the address: it changes from run to run */
	}
	for (int i = 0; i < SLOTS; i++)
		free(p[i]);
	void *al;
	printf("alloc ok aligned %u %d %d\n", h, posix_memalign(&al, 4096, 100) == 0 && (uintptr_t)al % 4096 == 0,
		(uintptr_t)aligned_alloc(64, 128) % 64 == 0);
	free(al);
}

static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static long counter, turn;
static __thread int tls = 100;
static pthread_once_t once = PTHREAD_ONCE_INIT;
static int onces;
static void init_once(void) { onces++; }

static void *worker(void *arg) {
	int id = (int)(intptr_t)arg;
	pthread_once(&once, init_once);
	tls += id;
	for (int i = 0; i < 10000; i++) {
		pthread_mutex_lock(&mu);
		counter += id;
		pthread_mutex_unlock(&mu);
	}
	pthread_mutex_lock(&mu);
	while (turn != id)
		pthread_cond_wait(&cv, &mu);
	turn++;
	pthread_cond_broadcast(&cv);
	pthread_mutex_unlock(&mu);
	return (void *)(intptr_t)tls;
}

static void threads(void) {
	pthread_t t[8];
	for (int i = 0; i < 8; i++)
		pthread_create(&t[i], 0, worker, (void *)(intptr_t)i);
	long sum = 0;
	for (int i = 0; i < 8; i++) {
		void *r;
		pthread_join(t[i], &r);
		sum += (intptr_t)r;
	}
	printf("threads %ld %ld %d %d\n", counter, sum, onces, tls);
	pthread_attr_t at;
	pthread_attr_init(&at);
	size_t ss;
	pthread_attr_getstacksize(&at, &ss);
	printf("stack %d\n", ss > 0);
}

static volatile sig_atomic_t got;
static void handler(int s) { got = s; }
static jmp_buf jb;
static sigjmp_buf sjb;
static void jumper(int n) {
	if (n == 0)
		longjmp(jb, 42);
	jumper(n - 1);
}
static void on_segv(int s) {
	(void)s;
	siglongjmp(sjb, 1);
}

static void control(void) {
	struct sigaction sa = {0};
	sa.sa_handler = handler;
	sigaction(SIGUSR1, &sa, 0);
	raise(SIGUSR1);
	printf("signal %d\n", got == SIGUSR1);
	sigset_t set, old;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR2);
	signal(SIGUSR2, handler);
	sigprocmask(SIG_BLOCK, &set, &old);
	raise(SIGUSR2);
	printf("blocked %d", got == SIGUSR2);
	sigprocmask(SIG_SETMASK, &old, 0);
	printf(" delivered %d\n", got == SIGUSR2);
	int v = setjmp(jb);
	if (v == 0)
		jumper(10);
	printf("longjmp %d\n", v);
	signal(SIGSEGV, on_segv);
	if (sigsetjmp(sjb, 1) == 0)
		*(volatile int *)0 = 1;
	printf("recovered from SIGSEGV\n");
	char host[256];
	printf("gethostname %d\n", gethostname(host, sizeof host) == 0);
	srand(7);
	int r1 = rand(), r2 = rand();
	printf("rand %d %d\n", r1, r2);
	srandom(9);
	long q1 = random(), q2 = random();
	printf("random %ld %ld\n", q1, q2);
	unsigned short xs[3] = {1, 2, 3};
	double e48 = erand48(xs);
	printf("erand48 %a %ld\n", e48, nrand48(xs));
}

int main(void) {
	sorting();
	times();
	regexes();
	options();
	conversions();
	allocation();
	threads();
	control();
	return 0;
}
