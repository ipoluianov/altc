/* Tests and benchmarks after Go's testing package (BSD-3-Clause, see
   LICENSE-Go): the same runner (tRunner) with parallel tests, the same
   name matching (-run/-skip split at / and |, unique subtest names) and
   the same output, buffered per test and indented under its parent. A
   test runs on a thread of its own; Fatalf leaves its function by
   longjmp, as Go's Goexit does. */
#define _GNU_SOURCE
#include <alt/regexp.h>
#include <alt/strconv.h>
#include <alt/testing.h>
#include <alt/time.h>
#include <alt/unicode.h>
#include <alt/unicode/utf8.h>
#include <errno.h>
#include <ftw.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/* ---- flags ---- */

static struct {
	bool verbose, short_, failfast;
	const char *run, *skip, *bench, *list;
	alt_dur benchtime;
	size_t benchn; /* -benchtime Nx */
	unsigned count;
	alt_dur timeout;
	int parallel;
} flags = {.count = 1, .benchtime = ALT_SEC, .timeout = 10 * ALT_MIN};

bool alt_testing_short(void) { return flags.short_; }
bool alt_testing_verbose(void) { return flags.verbose; }

/* one lock for the state shared between tests (output, names, lists) */
static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static int nfailed; /* for -failfast */

/* ---- the matcher ---- */

typedef struct {
	alt_regexp **re; /* the elements of one alternative */
	size_t n;
} alternative;

typedef struct {
	alternative *alt;
	size_t n;
	bool empty; /* no pattern: everything matches */
} filter;

typedef struct {
	char *name;
	int32_t n;
} subname;

static struct {
	filter run, skip;
	subname *names;
	size_t n, cap;
} matcher;

/* Go's rewrite: spaces become _, non-printable runes their escapes */
static void rewrite(alt_buf *b, const char *s) {
	size_t n = strlen(s);
	for (size_t i = 0; i < n;) {
		int32_t r;
		size_t k = alt_utf8_decode(s + i, n - i, &r);
		if (k == 0)
			k = 1, r = 0xfffd;
		bool space = r < 0x2000 ? (r == '\t' || r == '\n' || r == '\v' || r == '\f' || r == '\r' || r == ' ' || r == 0x85 ||
						  r == 0xa0 || r == 0x1680)
					: (r <= 0x200a || r == 0x2028 || r == 0x2029 || r == 0x202f || r == 0x205f || r == 0x3000);
		if (space)
			alt_buf_append(b, "_", 1);
		else if (r == 0xfffd && k == 1)
			alt_buf_append(b, "\xef\xbf\xbd", 3);
		else if (!alt_unicode_is_print(r)) {
			alt_buf q = ALT_BUF_INIT;
			alt_quote(&q, alt_str_make(s + i, k));
			alt_buf_append(b, q.p + 1, q.n - 2);
			alt_buf_free(&q);
		} else
			alt_buf_append(b, s + i, k);
		i += k;
	}
}

static bool match_string(alt_regexp *re, const char *s) { return alt_regexp_match(re, alt_str_make(s, strlen(s))); }

/* splitRegexp and verify: false (with the message printed) for a bad pattern */
static bool filter_init(filter *f, const char *s, const char *flag) {
	if (!s || !*s) {
		f->empty = true;
		return true;
	}
	size_t len = strlen(s);
	char **elems = calloc(len + 2, sizeof *elems);
	size_t ne = 0;
	f->alt = calloc(len + 2, sizeof *f->alt);
	int cs = 0, cp = 0;
	size_t start = 0;
	for (size_t i = 0; i <= len; i++) {
		char c = i < len ? s[i] : 0;
		switch (c) {
		case '[':
			cs++;
			continue;
		case ']':
			if (--cs < 0)
				cs = 0;
			continue;
		case '(':
			if (cs == 0)
				cp++;
			continue;
		case ')':
			if (cs == 0)
				cp--;
			continue;
		case '\\':
			i++;
			continue;
		case '/':
		case '|':
		case 0:
			if (c && (cs || cp))
				continue;
			break;
		default:
			continue;
		}
		elems[ne++] = strndup(s + start, i - start);
		start = i + 1;
		if (c == '|' || c == 0) {
			alternative *a = &f->alt[f->n++];
			a->re = calloc(ne, sizeof *a->re);
			a->n = ne;
			for (size_t k = 0; k < ne; k++) {
				alt_buf rw = ALT_BUF_INIT, err = ALT_BUF_INIT;
				rewrite(&rw, elems[k]);
				if (alt_regexp_compile(&a->re[k], alt_buf_str(&rw), &err) < 0) {
					/* Go's text: element i of -test.run ("pat"): error */
					alt_buf q = ALT_BUF_INIT;
					alt_quote(&q, alt_buf_str(&rw));
					const char *m = alt_buf_cstr(&err);
					if (f->n > 1 || c == '|')
						fprintf(stderr, "testing: invalid regexp for alternation %zu of element %zu of %s (%s): %s\n", f->n - 1,
							k, flag, alt_buf_cstr(&q), m);
					else
						fprintf(stderr, "testing: invalid regexp for element %zu of %s (%s): %s\n", k, flag, alt_buf_cstr(&q),
							m);
					return false;
				}
				alt_buf_free(&rw);
				alt_buf_free(&err);
				free(elems[k]);
			}
			ne = 0;
		}
	}
	free(elems);
	return true;
}

static void filter_free(filter *f) {
	for (size_t a = 0; a < f->n; a++) {
		for (size_t k = 0; k < f->alt[a].n; k++)
			alt_regexp_free(f->alt[a].re[k]);
		free(f->alt[a].re);
	}
	free(f->alt);
	*f = (filter){0};
}

/* matches the elements of a name: ok, and partial when the pattern has
   more elements than the name */
static bool filter_matches(const filter *f, char **elem, size_t n, bool *partial) {
	*partial = false;
	if (f->empty) {
		return true; /* simpleMatch{}: partial is len(name) < 0 */
	}
	for (size_t a = 0; a < f->n; a++) {
		const alternative *al = &f->alt[a];
		bool ok = true;
		for (size_t i = 0; i < n && i < al->n; i++)
			if (!match_string(al->re[i], elem[i])) {
				ok = false;
				break;
			}
		if (ok) {
			*partial = n < al->n;
			return true;
		}
	}
	return false;
}

static int32_t *subname_slot(const char *name) {
	for (size_t i = 0; i < matcher.n; i++)
		if (!strcmp(matcher.names[i].name, name))
			return &matcher.names[i].n;
	if (matcher.n == matcher.cap) {
		matcher.cap = matcher.cap ? 2 * matcher.cap : 16;
		matcher.names = realloc(matcher.names, matcher.cap * sizeof *matcher.names);
	}
	matcher.names[matcher.n] = (subname){strdup(name), 0};
	return &matcher.names[matcher.n++].n;
}

static int32_t subname_get(const char *name) {
	for (size_t i = 0; i < matcher.n; i++)
		if (!strcmp(matcher.names[i].name, name))
			return matcher.names[i].n;
	return 0;
}

static void subnames_clear(void) {
	for (size_t i = 0; i < matcher.n; i++)
		free(matcher.names[i].name);
	matcher.n = 0;
}

/* parseSubtestNumber */
static size_t parse_subtest_number(const char *s, int32_t *nn) {
	*nn = 0;
	size_t len = strlen(s);
	const char *h = strrchr(s, '#');
	if (!h)
		return len;
	const char *suf = h + 1;
	size_t sl = strlen(suf);
	if (sl < 2 || (sl > 2 && suf[0] == '0'))
		return len;
	if (!strcmp(suf, "00") && (h == s || h[-1] != '/'))
		return len;
	int64_t v = 0;
	for (const char *p = suf; *p; p++) {
		if (*p < '0' || *p > '9' || (v = v * 10 + (*p - '0')) > INT32_MAX)
			return len;
	}
	*nn = (int32_t)v;
	return (size_t)(h - s);
}

/* m.unique: parent/subname, with #NN for repeats */
static char *unique(const char *parent, const char *sub) {
	alt_buf base = ALT_BUF_INIT;
	alt_buf_printf(&base, "%s/%s", parent, sub);
	for (;;) {
		int32_t *slot = subname_slot(alt_buf_cstr(&base));
		int32_t n = (*slot)++;
		if (n == 0 && *sub) {
			int32_t nn;
			size_t plen = parse_subtest_number(base.p, &nn);
			if (plen < base.n) {
				char *prefix = strndup(base.p, plen);
				bool again = nn < subname_get(prefix);
				free(prefix);
				if (again)
					continue;
			}
			return base.p;
		}
		alt_buf name = ALT_BUF_INIT;
		alt_buf_printf(&name, "%s#%02d", base.p, n);
		if (subname_get(name.p) != 0) {
			alt_buf_free(&name);
			continue;
		}
		alt_buf_free(&base);
		return name.p;
	}
}

/* m.fullName: the test's name and whether it runs */
static char *full_name(int parent_level, const char *parent, const char *sub, bool *ok) {
	char *name;
	if (parent_level > 0) {
		alt_buf rw = ALT_BUF_INIT;
		rewrite(&rw, sub);
		name = unique(parent, alt_buf_cstr(&rw));
		alt_buf_free(&rw);
	} else
		name = strdup(sub);
	/* the elements as strings.Split (empty ones kept) */
	char *copy = strdup(name), *elem[256];
	size_t n = 0;
	char *p = copy;
	elem[n++] = p;
	for (; *p; p++)
		if (*p == '/' && n < 256) {
			*p = 0;
			elem[n++] = p + 1;
		}
	bool partial, skp, pskip;
	*ok = filter_matches(&matcher.run, elem, n, &partial);
	if (*ok && !matcher.skip.empty) {
		skp = filter_matches(&matcher.skip, elem, n, &pskip);
		if (skp && !pskip)
			*ok = false;
	}
	free(copy);
	return name;
}

/* ---- output ---- */

static char *last_name; /* the chatty printer's */
static alt_buf root_out; /* where the root's children write (stdout) */

static void chatty_update(const char *name, const char *text, size_t n) {
	free(last_name);
	last_name = strdup(name);
	fwrite(text, 1, n, stdout);
	fflush(stdout);
}

static void chatty_print(const char *name, const char *text, size_t n) {
	if (!last_name)
		last_name = strdup(name);
	else if (strcmp(last_name, name)) {
		printf("=== NAME  %s\n", name);
		free(last_name);
		last_name = strdup(name);
	}
	fwrite(text, 1, n, stdout);
	fflush(stdout);
}

/* ---- tests ---- */

typedef struct cleanup {
	void (*fn)(void *);
	void *arg;
	struct cleanup *next;
} cleanup;

struct alt_test {
	char *name;
	int level;
	alt_test *parent;
	void (*fn)(alt_test *t, void *arg);
	void *arg;
	alt_buf output;
	bool failed, skipped, finished, done, parallel, has_sub, ran, bench, setenv_used;
	alt_test **sub; /* the parallel subtests */
	size_t nsub;
	int nsignal; /* Go's t.signal: posts not yet taken (under mu) */
	pthread_cond_t signal;
	bool barrier_open;
	pthread_cond_t barrier;
	pthread_t thread;
	jmp_buf jb;
	alt_time start;
	alt_dur duration;
	cleanup *cleanups;
	char *tempdir;
	int tempseq;
	/* benchmarks */
	size_t n;
	bool timer_on;
	alt_time bstart;
	alt_dur bdur;
	int64_t bytes;
	struct {
		char *unit;
		double v;
	} extra[16];
	int nextra;
	uint64_t loop_n, loop_i;
	bool loop_done;
	alt_dur benchtime;
	size_t benchn;
	struct running *run_entry;
};

/* the running tests, for the timeout and crash messages */
typedef struct running {
	alt_test *t;
	struct running *next, *prev;
} running;
static running *running_list;

static void running_add(alt_test *t) {
	running *r = calloc(1, sizeof *r);
	r->t = t;
	r->next = running_list;
	if (running_list)
		running_list->prev = r;
	running_list = r;
	t->run_entry = r;
}

static void running_del(alt_test *t) {
	running *r = t->run_entry;
	if (!r)
		return;
	if (r->prev)
		r->prev->next = r->next;
	else
		running_list = r->next;
	if (r->next)
		r->next->prev = r->prev;
	free(r);
	t->run_entry = NULL;
}

/* the parallelism limit (Go's testState) */
static struct {
	int max, running, waiting;
	pthread_cond_t start;
} tstate = {.start = PTHREAD_COND_INITIALIZER};
static unsigned starts_pending;

static void wait_parallel(void) {
	if (tstate.running < tstate.max) {
		tstate.running++;
		return;
	}
	tstate.waiting++;
	while (!starts_pending)
		pthread_cond_wait(&tstate.start, &mu);
	starts_pending--;
}

static void release_parallel(void) {
	if (tstate.waiting == 0) {
		tstate.running--;
		return;
	}
	tstate.waiting--;
	starts_pending++;
	pthread_cond_signal(&tstate.start);
}

static __thread alt_test *current;
static alt_test *root_test;

const char *alt_test_name(const alt_test *t) { return t->name; }
bool alt_test_failed(const alt_test *t) {
	pthread_mutex_lock(&mu);
	bool f = t->failed;
	pthread_mutex_unlock(&mu);
	return f;
}
bool alt_test_skipped(const alt_test *t) {
	pthread_mutex_lock(&mu);
	bool s = t->skipped;
	pthread_mutex_unlock(&mu);
	return s;
}

static void mark_failed(alt_test *t) {
	for (alt_test *p = t; p; p = p->parent) {
		if (p->failed)
			break; /* Go's Fail also fails the parents */
		p->failed = true;
	}
}

void alt_test_fail(alt_test *t) {
	pthread_mutex_lock(&mu);
	if (t->done)
		fprintf(stderr, "Fail in goroutine after %s has completed\n", t->name);
	mark_failed(t);
	pthread_mutex_unlock(&mu);
}

/* leaves the current test's function (Goexit) */
static void __attribute__((noreturn)) test_exit(alt_test *t) {
	alt_test *c = current;
	(void)t;
	if (c)
		longjmp(c->jb, 1);
	pthread_exit(NULL);
}

void alt_test_fail_now(alt_test *t) {
	alt_test_fail(t);
	pthread_mutex_lock(&mu);
	t->finished = true;
	pthread_mutex_unlock(&mu);
	test_exit(t);
}

void alt_test_skip_now(alt_test *t) {
	pthread_mutex_lock(&mu);
	t->skipped = true;
	t->finished = true;
	pthread_mutex_unlock(&mu);
	test_exit(t);
}

/* appends the indented lines of s to w's output (Go's indenter) */
static void indent_into(alt_buf *w, const char *s, size_t n) {
	while (n > 0) {
		const char *nl = memchr(s, '\n', n);
		size_t end = nl ? (size_t)(nl - s) + 1 : n;
		alt_buf_append(w, "    ", 4);
		alt_buf_append(w, s, end);
		s += end, n -= end;
	}
}

/* writes a log line (with its call site) to the test or its nearest
   unfinished parent (mu held) */
static void write_log(alt_test *t, const char *line, size_t n) {
	alt_test *d = t;
	while (d && d->done)
		d = d->parent;
	if (!d || d == root_test) {
		fprintf(stderr, "panic: Log in goroutine after %s has completed: %.*s", t->name, (int)n, line);
		return;
	}
	if (!flags.verbose || d->done) {
		indent_into(&d->output, line, n);
		return;
	}
	alt_buf b = ALT_BUF_INIT;
	indent_into(&b, line, n);
	if (d->bench) {
		fwrite(b.p, 1, b.n, stdout);
		fflush(stdout);
	} else
		chatty_print(d->name, b.p, b.n);
	alt_buf_free(&b);
}

void alt__test_logf(alt_test *t, const char *file, int line, int kind, const char *fmt, ...) {
	alt_buf msg = ALT_BUF_INIT, s = ALT_BUF_INIT;
	va_list ap;
	va_start(ap, fmt);
	alt_buf_vprintf(&msg, fmt, ap);
	va_end(ap);
	size_t n = msg.n;
	while (n > 0 && msg.p[n - 1] == '\n')
		n--;
	const char *base = strrchr(file, '/');
	alt_buf_printf(&s, "%s:%d: ", base ? base + 1 : file, line);
	for (size_t i = 0; i < n; i++) {
		if (msg.p[i] == '\n')
			alt_buf_append(&s, "\n    ", 5);
		else
			alt_buf_append(&s, msg.p + i, 1);
	}
	alt_buf_append(&s, "\n", 1);
	pthread_mutex_lock(&mu);
	write_log(t, s.p, s.n);
	pthread_mutex_unlock(&mu);
	alt_buf_free(&msg);
	alt_buf_free(&s);
	switch (kind) {
	case ALT__TEST_ERROR:
		alt_test_fail(t);
		break;
	case ALT__TEST_FATAL:
		alt_test_fail_now(t);
	case ALT__TEST_SKIP:
		alt_test_skip_now(t);
	}
}

void alt_test_cleanup(alt_test *t, void (*fn)(void *arg), void *arg) {
	cleanup *c = malloc(sizeof *c);
	c->fn = fn, c->arg = arg;
	pthread_mutex_lock(&mu);
	c->next = t->cleanups;
	t->cleanups = c;
	pthread_mutex_unlock(&mu);
}

static void run_cleanups(alt_test *t) {
	for (;;) {
		pthread_mutex_lock(&mu);
		cleanup *c = t->cleanups;
		if (c)
			t->cleanups = c->next;
		pthread_mutex_unlock(&mu);
		if (!c)
			break;
		/* a cleanup may call Fatalf: it ends that cleanup only */
		alt_test *saved = current;
		jmp_buf save;
		memcpy(save, t->jb, sizeof save);
		current = t;
		if (!setjmp(t->jb))
			c->fn(c->arg);
		memcpy(t->jb, save, sizeof save);
		current = saved;
		free(c);
	}
}

static int rm_entry(const char *path, const struct stat *st, int type, struct FTW *ftw) {
	(void)st, (void)type, (void)ftw;
	remove(path);
	return 0;
}

static void remove_dir(void *dir) {
	nftw(dir, rm_entry, 16, FTW_DEPTH | FTW_PHYS);
	free(dir);
}

const char *alt_test_temp_dir(alt_test *t) {
	pthread_mutex_lock(&mu);
	if (!t->tempdir) {
		/* the pattern is the test's name with the unsafe bytes dropped, as Go */
		alt_buf pat = ALT_BUF_INIT;
		const char *tmp = getenv("TMPDIR");
		alt_buf_printf(&pat, "%s/", tmp && *tmp ? tmp : "/tmp");
		for (const char *p = t->name; *p && p - t->name < 64; p++)
			if ((unsigned char)*p >= 0x80 || strchr("<>:\"/\\|?*", *p) == NULL)
				alt_buf_append(&pat, p, 1);
		alt_buf_append_cstr(&pat, "XXXXXXXXX");
		if (!mkdtemp(pat.p)) {
			pthread_mutex_unlock(&mu);
			alt_test_fatalf(t, "TempDir: %s", strerror(errno));
		}
		t->tempdir = pat.p;
		pthread_mutex_unlock(&mu);
		alt_test_cleanup(t, remove_dir, strdup(t->tempdir));
		pthread_mutex_lock(&mu);
	}
	char *dir = malloc(strlen(t->tempdir) + 8);
	sprintf(dir, "%s/%03d", t->tempdir, ++t->tempseq);
	pthread_mutex_unlock(&mu);
	if (mkdir(dir, 0777) < 0)
		alt_test_fatalf(t, "TempDir: %s", strerror(errno));
	/* kept until the test ends (with the parent directory) */
	alt_test_cleanup(t, free, dir);
	return dir;
}

typedef struct {
	char *key, *old;
} envsave;

static void restore_env(void *arg) {
	envsave *e = arg;
	if (e->old)
		setenv(e->key, e->old, 1);
	else
		unsetenv(e->key);
	free(e->key);
	free(e->old);
	free(e);
}

void alt_test_setenv(alt_test *t, const char *key, const char *value) {
	pthread_mutex_lock(&mu);
	bool par = false;
	for (alt_test *p = t; p; p = p->parent)
		par |= p->parallel;
	if (!par)
		t->setenv_used = true;
	pthread_mutex_unlock(&mu);
	if (par) {
		fprintf(stderr, "panic: testing: test using t.Setenv or t.Chdir can not use t.Parallel\n");
		alt_test_fail_now(t);
	}
	envsave *e = calloc(1, sizeof *e);
	e->key = strdup(key);
	const char *old = getenv(key);
	e->old = old ? strdup(old) : NULL;
	setenv(key, value, 1);
	alt_test_cleanup(t, restore_env, e);
}

static void report(alt_test *t);
static void signal_post(alt_test *t);
static void signal_wait(alt_test *t);

/* an error from the runner itself (no call site in the test) */
static void alt__test_logf_quiet(alt_test *t, const char *msg) {
	pthread_mutex_lock(&mu);
	alt_buf s = ALT_BUF_INIT;
	alt_buf_printf(&s, "testing.c: %s\n", msg);
	write_log(t, s.p, s.n);
	alt_buf_free(&s);
	mark_failed(t);
	pthread_mutex_unlock(&mu);
}

static void *trunner(void *arg) {
	alt_test *t = arg;
	current = t;
	stack_t ss = {.ss_sp = malloc(64 << 10), .ss_size = 64 << 10};
	sigaltstack(&ss, NULL);
	if (!setjmp(t->jb))
		t->fn(t, t->arg);
	pthread_mutex_lock(&mu);
	if (!t->bench)
		t->finished = true; /* for benchmarks: only by FailNow and SkipNow */
	bool has_par = t->nsub > 0;
	pthread_mutex_unlock(&mu);
	if (!has_par)
		run_cleanups(t);
	pthread_mutex_lock(&mu);
	t->duration += alt_since(t->start);
	if (t->nsub > 0) {
		release_parallel();
		running_del(t);
		t->barrier_open = true;
		pthread_cond_broadcast(&t->barrier);
		size_t nsub = t->nsub;
		pthread_mutex_unlock(&mu);
		for (size_t i = 0; i < nsub; i++)
			signal_wait(t->sub[i]);
		alt_time cs = alt_now();
		run_cleanups(t);
		pthread_mutex_lock(&mu);
		t->duration += alt_since(cs);
		if (!t->parallel)
			wait_parallel();
	} else if (t->parallel)
		release_parallel();
	report(t);
	running_del(t);
	t->done = true;
	if (t->parent && !t->has_sub)
		for (alt_test *p = t; p; p = p->parent)
			p->ran = true;
	if (t->failed && flags.failfast)
		nfailed++;
	pthread_mutex_unlock(&mu);
	sigaltstack(&(stack_t){.ss_flags = SS_DISABLE}, NULL);
	free(ss.ss_sp);
	current = NULL;
	signal_post(t);
	return NULL;
}

static void fmt_duration(char *out, alt_dur d) { sprintf(out, "%.2fs", (double)d / 1e9); }

/* writes the "--- X: name (d)" line and the output to the parent (mu held) */
static void flush_to_parent(alt_test *t, const char *status) {
	alt_test *p = t->parent;
	char dstr[64];
	fmt_duration(dstr, t->duration);
	alt_buf b = ALT_BUF_INIT;
	alt_buf_printf(&b, "--- %s: %s (%s)\n", status, t->name, dstr);
	alt_buf_append(&b, t->output.p, t->output.n);
	alt_buf_reset(&t->output);
	if (p == root_test) {
		if (flags.verbose)
			chatty_update(t->name, b.p, b.n);
		else {
			fwrite(b.p, 1, b.n, stdout);
			fflush(stdout);
		}
	} else
		indent_into(&p->output, b.p, b.n);
	alt_buf_free(&b);
}

static void report(alt_test *t) {
	if (!t->parent || t->bench)
		return;
	if (t->failed)
		flush_to_parent(t, "FAIL");
	else if (flags.verbose)
		flush_to_parent(t, t->skipped ? "SKIP" : "PASS");
}

static alt_test *new_test(alt_test *parent, char *name) {
	alt_test *t = calloc(1, sizeof *t);
	t->name = name;
	t->parent = parent;
	t->level = parent ? parent->level + 1 : 0;
	pthread_cond_init(&t->signal, NULL);
	pthread_cond_init(&t->barrier, NULL);
	return t;
}

static void signal_post(alt_test *t) {
	pthread_mutex_lock(&mu);
	t->nsignal++;
	pthread_cond_broadcast(&t->signal);
	pthread_mutex_unlock(&mu); /* the poster no longer touches t */
}

static void signal_wait(alt_test *t) {
	pthread_mutex_lock(&mu);
	while (t->nsignal == 0)
		pthread_cond_wait(&t->signal, &mu);
	t->nsignal--;
	pthread_mutex_unlock(&mu);
}

static void free_test(alt_test *t) {
	for (size_t i = 0; i < t->nsub; i++)
		free_test(t->sub[i]);
	free(t->sub);
	alt_buf_free(&t->output);
	pthread_cond_destroy(&t->signal);
	pthread_cond_destroy(&t->barrier);
	for (int i = 0; i < t->nextra; i++)
		free(t->extra[i].unit);
	free(t->tempdir);
	free(t->name);
	free(t);
}

static void start_thread(alt_test *t) {
	pthread_attr_t pa;
	pthread_attr_init(&pa);
	pthread_attr_setstacksize(&pa, 64 << 20);
	pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&t->thread, &pa, trunner, t) != 0) {
		fprintf(stderr, "testing: cannot start a thread for %s\n", t->name);
		exit(2);
	}
	pthread_attr_destroy(&pa);
}

bool alt_test_run(alt_test *t, const char *name, void (*fn)(alt_test *t, void *arg), void *arg) {
	pthread_mutex_lock(&mu);
	if (t->bench) {
		pthread_mutex_unlock(&mu);
		fprintf(stderr, "testing: alt_test_run called on a benchmark (use alt_bench_run)\n");
		return false;
	}
	t->has_sub = true;
	bool ok;
	char *full = full_name(t->level, t->name, name, &ok);
	if (!ok || (flags.failfast && nfailed > 0)) {
		pthread_mutex_unlock(&mu);
		free(full);
		return true;
	}
	alt_test *s = new_test(t, full);
	s->fn = fn, s->arg = arg;
	if (flags.verbose) {
		alt_buf b = ALT_BUF_INIT;
		alt_buf_printf(&b, "=== RUN   %s\n", full);
		chatty_update(full, b.p, b.n);
		alt_buf_free(&b);
	}
	running_add(s);
	s->start = alt_now();
	pthread_mutex_unlock(&mu);
	start_thread(s);
	signal_wait(s);
	pthread_mutex_lock(&mu);
	bool passed = !s->failed;
	bool par = s->parallel;
	pthread_mutex_unlock(&mu);
	if (!par)
		free_test(s); /* a parallel one goes with its parent */
	return passed;
}

void alt_test_parallel(alt_test *t) {
	pthread_mutex_lock(&mu);
	if (t->parallel) {
		pthread_mutex_unlock(&mu);
		fprintf(stderr, "panic: testing: t.Parallel called multiple times\n");
		alt_test_fail_now(t);
	}
	if (t->setenv_used) {
		pthread_mutex_unlock(&mu);
		fprintf(stderr, "panic: testing: t.Parallel called after t.Setenv; cannot set environment variables in parallel tests\n");
		alt_test_fail_now(t);
	}
	if (t->bench || !t->parent) {
		pthread_mutex_unlock(&mu);
		return;
	}
	t->parallel = true;
	alt_test *p = t->parent;
	p->sub = realloc(p->sub, (p->nsub + 1) * sizeof *p->sub);
	p->sub[p->nsub++] = t;
	t->duration += alt_since(t->start);
	if (flags.verbose) {
		alt_buf b = ALT_BUF_INIT;
		alt_buf_printf(&b, "=== PAUSE %s\n", t->name);
		chatty_update(t->name, b.p, b.n);
		alt_buf_free(&b);
	}
	running_del(t);
	t->nsignal++; /* the parent's Run returns */
	pthread_cond_broadcast(&t->signal);
	while (!p->barrier_open)
		pthread_cond_wait(&p->barrier, &mu);
	wait_parallel();
	if (flags.verbose) {
		alt_buf b = ALT_BUF_INIT;
		alt_buf_printf(&b, "=== CONT  %s\n", t->name);
		chatty_update(t->name, b.p, b.n);
		alt_buf_free(&b);
	}
	running_add(t);
	t->start = alt_now();
	pthread_mutex_unlock(&mu);
}

/* ---- benchmarks ---- */

size_t alt_bench_n(const alt_bench *b) { return b->n; }

void alt_bench_start_timer(alt_bench *b) {
	if (!b->timer_on) {
		b->bstart = alt_now();
		b->timer_on = true;
	}
}

void alt_bench_stop_timer(alt_bench *b) {
	if (b->timer_on) {
		b->bdur += alt_since(b->bstart);
		b->timer_on = false;
	}
}

void alt_bench_reset_timer(alt_bench *b) {
	if (b->timer_on)
		b->bstart = alt_now();
	b->bdur = 0;
}

void alt_bench_set_bytes(alt_bench *b, int64_t n) { b->bytes = n; }

void alt_bench_report_metric(alt_bench *b, double n, const char *unit) {
	if (!unit || !*unit || strpbrk(unit, " \t\n\r\v\f")) {
		fprintf(stderr, "panic: metric unit must not be empty or contain whitespace\n");
		return;
	}
	for (int i = 0; i < b->nextra; i++)
		if (!strcmp(b->extra[i].unit, unit)) {
			b->extra[i].v = n;
			return;
		}
	if (b->nextra < 16) {
		b->extra[b->nextra].unit = strdup(unit);
		b->extra[b->nextra++].v = n;
	}
}

static alt_dur elapsed(const alt_bench *b) { return b->bdur + (b->timer_on ? alt_since(b->bstart) : 0); }

static int64_t predict_n(int64_t goalns, int64_t prev_iters, int64_t prevns, int64_t last) {
	if (prevns == 0)
		prevns = 1;
	/* in 128 bits: goalns * prev_iters may not fit */
	__int128 n = (__int128)goalns * prev_iters / prevns;
	n += n / 5;
	if (n > (__int128)100 * last)
		n = (__int128)100 * last;
	if (n < last + 1)
		n = last + 1;
	if (n > 1000000000)
		n = 1000000000;
	return (int64_t)n;
}

bool alt_bench_loop(alt_bench *b) {
	if (b->loop_i < b->loop_n) {
		b->loop_i++;
		return true;
	}
	if (!b->timer_on)
		alt_test_fatalf(b, "B.Loop called with timer stopped");
	if (b->loop_n == 0) {
		b->loop_n = b->benchn > 0 ? b->benchn : 1;
		b->n = 0;
		alt_bench_reset_timer(b);
		b->loop_i++;
		return true;
	}
	bool more;
	if (b->benchn > 0)
		more = false;
	else {
		alt_dur t = elapsed(b);
		if (t >= b->benchtime)
			more = false;
		else {
			uint64_t prev = b->loop_n;
			b->loop_n = (uint64_t)predict_n(b->benchtime, (int64_t)prev, t, (int64_t)prev);
			more = prev < b->loop_n;
		}
	}
	if (!more) {
		alt_bench_stop_timer(b);
		b->n = b->loop_n;
		b->loop_done = true;
		return false;
	}
	b->loop_i++;
	return true;
}

typedef struct {
	alt_bench *b;
	void (*fn)(alt_bench *b, void *arg);
	void *arg;
} bench_call;

static void bench_body(alt_test *t, void *arg) {
	bench_call *c = arg;
	(void)t;
	alt_bench_start_timer(c->b); /* on the benchmark's thread, as Go's runN */
	c->fn(c->b, c->arg);
}

/* runs the body once with N = n on a thread of its own */
static void run_n(alt_bench *b, void (*fn)(alt_bench *, void *), void *arg, size_t n) {
	b->n = n;
	b->loop_n = 0, b->loop_i = 0, b->loop_done = false;
	b->bdur = 0;
	b->timer_on = false;
	b->has_sub = false;
	bench_call c = {b, fn, arg};
	b->fn = bench_body, b->arg = &c;
	b->finished = false;
	b->done = false;
	b->start = alt_now();
	start_thread(b);
	signal_wait(b);
	alt_bench_stop_timer(b);
	if (b->loop_n > 0 && !b->loop_done && !b->failed) {
		/* the body left the loop early */
		alt__test_logf_quiet(b, "benchmark function returned without B.Loop() == false (break or return in loop?)");
	}
	if (b->loop_n)
		b->n = b->loop_n;
}

static size_t bench_maxlen;
static int bench_procs;
static bool bench_header_done;

static void pretty_print(alt_buf *w, double x, const char *unit) {
	double y = x < 0 ? -x : x;
	int width, prec;
	if (y == 0 || y >= 999.95)
		width = 10, prec = 0;
	else if (y >= 99.995)
		width = 12, prec = 1;
	else if (y >= 9.9995)
		width = 13, prec = 2;
	else if (y >= 0.99995)
		width = 14, prec = 3;
	else if (y >= 0.099995)
		width = 15, prec = 4;
	else if (y >= 0.0099995)
		width = 16, prec = 5;
	else if (y >= 0.00099995)
		width = 17, prec = 6;
	else
		width = 18, prec = 7;
	alt_buf_printf(w, "%*.*f %s", width, prec, x, unit);
}

static int cmp_extra(const void *a, const void *b) {
	return strcmp(*(char *const *)a, *(char *const *)b);
}

/* the result line after the name: N, ns/op, MB/s, the extra metrics */
static void bench_result(alt_buf *w, alt_bench *b) {
	alt_buf_printf(w, "%8zu", b->n);
	double ns = -1;
	for (int i = 0; i < b->nextra; i++)
		if (!strcmp(b->extra[i].unit, "ns/op"))
			ns = b->extra[i].v;
	if (ns < 0)
		ns = b->n ? (double)b->bdur / (double)b->n : 0;
	if (ns != 0) {
		alt_buf_append(w, "\t", 1);
		pretty_print(w, ns, "ns/op");
	}
	if (b->bytes > 0 && b->bdur > 0 && b->n > 0) {
		double mbs = ((double)b->bytes * (double)b->n / 1e6) / ((double)b->bdur / 1e9);
		if (mbs != 0)
			alt_buf_printf(w, "\t%7.2f MB/s", mbs);
	}
	char *keys[16];
	int nk = 0;
	for (int i = 0; i < b->nextra; i++)
		if (strcmp(b->extra[i].unit, "ns/op") && strcmp(b->extra[i].unit, "MB/s") && strcmp(b->extra[i].unit, "B/op") &&
			strcmp(b->extra[i].unit, "allocs/op"))
			keys[nk++] = b->extra[i].unit;
	qsort(keys, (size_t)nk, sizeof *keys, cmp_extra);
	for (int k = 0; k < nk; k++)
		for (int i = 0; i < b->nextra; i++)
			if (b->extra[i].unit == keys[k]) {
				alt_buf_append(w, "\t", 1);
				pretty_print(w, b->extra[i].v, keys[k]);
			}
}

static bool bench_failed;

static void bench_name(alt_buf *w, const char *name) {
	if (bench_procs != 1)
		alt_buf_printf(w, "%s-%d", name, bench_procs);
	else
		alt_buf_append_cstr(w, name);
}

static void bench_header(void) {
	if (bench_header_done)
		return;
	bench_header_done = true;
	printf("goos: linux\ngoarch: amd64\n");
	FILE *f = fopen("/proc/cpuinfo", "r");
	char line[512];
	while (f && fgets(line, sizeof line, f))
		if (!strncmp(line, "model name", 10)) {
			char *v = strchr(line, ':');
			if (v) {
				v++;
				while (*v == ' ' || *v == '\t')
					v++;
				printf("cpu: %s", v);
			}
			break;
		}
	if (f)
		fclose(f);
	fflush(stdout);
}

static alt_bench *new_bench(alt_test *parent, const char *name) {
	alt_bench *b = new_test(parent, strdup(name));
	b->bench = true;
	b->benchtime = flags.benchtime;
	b->benchn = flags.benchn;
	return b;
}

static void free_bench(alt_bench *b) { free_test(b); }

static void print_output(const char *tag, const char *name, alt_bench *b) {
	printf("--- %s: %s\n%.*s", tag, name, (int)b->output.n, b->output.p ? b->output.p : "");
}

/* launch: grows N until the body runs for -benchtime */
static void launch(alt_bench *b, void (*fn)(alt_bench *, void *), void *arg) {
	if (b->loop_n != 0)
		return; /* b.Loop ran its own iterations in run1 */
	if (b->benchn > 0) {
		if (b->benchn > 1)
			run_n(b, fn, arg, b->benchn);
		return;
	}
	for (int64_t n = 1; !b->failed && b->bdur < b->benchtime && n < 1000000000;) {
		int64_t last = n;
		n = predict_n(b->benchtime, (int64_t)b->n, b->bdur, last);
		run_n(b, fn, arg, (size_t)n);
	}
}

/* Go's B.Run for a (sub)benchmark: run1, then processBench when it has
   no subbenchmarks */
static void process_bench(alt_test *parent, const char *name, void (*fn)(alt_bench *, void *), void *arg) {
	alt_bench *b = new_bench(parent, name);
	if (flags.verbose) {
		bench_header();
		printf("%s\n", name);
	}
	pthread_mutex_lock(&mu);
	running_add(b);
	pthread_mutex_unlock(&mu);
	run_n(b, fn, arg, 1);
	pthread_mutex_lock(&mu);
	running_del(b);
	pthread_mutex_unlock(&mu);
	if (b->failed) {
		bench_failed = true;
		print_output("FAIL", b->name, b);
		fflush(stdout);
		free_bench(b);
		return;
	}
	if (b->has_sub || b->finished) {
		if (flags.verbose && (b->output.n || b->finished))
			print_output(b->skipped ? "SKIP" : "BENCH", b->name, b);
		fflush(stdout);
		free_bench(b);
		return;
	}
	bench_header();
	for (unsigned j = 0; j < flags.count; j++) {
		alt_buf nm = ALT_BUF_INIT, line = ALT_BUF_INIT;
		bench_name(&nm, name);
		if (j > 0) {
			free_bench(b);
			b = new_bench(parent, name);
			run_n(b, fn, arg, 1);
		}
		pthread_mutex_lock(&mu);
		running_add(b);
		pthread_mutex_unlock(&mu);
		launch(b, fn, arg);
		pthread_mutex_lock(&mu);
		running_del(b);
		pthread_mutex_unlock(&mu);
		if (b->failed) {
			bench_failed = true;
			print_output("FAIL", nm.p, b);
		} else {
			alt_buf_printf(&line, "%-*s\t", (int)bench_maxlen, nm.p);
			bench_result(&line, b);
			printf("%s\n", alt_buf_cstr(&line));
			if (b->output.n)
				print_output("BENCH", nm.p, b);
		}
		fflush(stdout);
		alt_buf_free(&nm);
		alt_buf_free(&line);
	}
	free_bench(b);
}

bool alt_bench_run(alt_bench *b, const char *name, void (*fn)(alt_bench *b, void *arg), void *arg) {
	b->has_sub = true;
	pthread_mutex_lock(&mu);
	bool ok;
	char *full = full_name(b->level, b->name, name, &ok);
	pthread_mutex_unlock(&mu);
	if (!ok) {
		free(full);
		return true;
	}
	/* run1 widens the name column (with slack) */
	size_t ext = bench_procs != 1 ? 1 + (size_t)snprintf(NULL, 0, "%d", bench_procs) : 0;
	if (strlen(full) + ext + 1 > bench_maxlen)
		bench_maxlen = strlen(full) + ext + 1 + 8;
	process_bench(b, full, fn, arg);
	free(full);
	return !b->failed;
}

/* ---- the binary ---- */

struct alt_testing_m {
	const alt_test_def *tests, *benches;
	size_t ntests, nbenches;
	bool ran;
};

typedef struct {
	const alt_test_def *tests;
	size_t n;
} test_list;

static void call_def(alt_test *t, void *arg) { ((void (*)(alt_test *))arg)(t); }

static void root_fn(alt_test *t, void *arg) {
	test_list *l = arg;
	for (size_t i = 0; i < l->n; i++)
		alt_test_run(t, l->tests[i].name, call_def, (void *)l->tests[i].fn);
}

static void bench_root(alt_bench *b, void *arg) {
	test_list *l = arg;
	for (size_t i = 0; i < l->n; i++) {
		bool ok;
		pthread_mutex_lock(&mu);
		char *full = full_name(0, "", l->tests[i].name, &ok);
		pthread_mutex_unlock(&mu);
		if (ok) {
			process_bench(b, full, (void (*)(alt_bench *, void *))call_def, (void *)l->tests[i].fn);
		}
		free(full);
	}
}

static void timeout_text(alt_buf *b) {
	alt_buf_append_cstr(b, "panic: test timed out after ");
	alt_dur_format(b, flags.timeout);
	if (running_list) {
		alt_buf_append_cstr(b, "\nrunning tests:");
		for (running *r = running_list; r; r = r->next) {
			alt_buf_printf(b, "\n\t%s (", r->t->name);
			alt_dur d = alt_since(r->t->start);
			alt_dur_format(b, (d + ALT_SEC / 2) / ALT_SEC * ALT_SEC); /* Round(time.Second) */
			alt_buf_append_cstr(b, ")");
		}
	}
	alt_buf_append_cstr(b, "\n");
}

static void *watchdog(void *arg) {
	(void)arg;
	struct timespec ts = {(time_t)(flags.timeout / ALT_SEC), (long)(flags.timeout % ALT_SEC)};
	while (nanosleep(&ts, &ts) < 0 && errno == EINTR)
		;
	pthread_mutex_lock(&mu);
	alt_buf b = ALT_BUF_INIT;
	timeout_text(&b);
	fflush(stdout);
	fwrite(b.p, 1, b.n, stderr);
	fflush(stderr);
	_exit(2);
}

static void crash(int sig) {
	alt_test *t = current;
	char line[256];
	int n;
	fflush(stdout);
	if (t && t->name) {
		n = snprintf(line, sizeof line, "--- FAIL: %s\n", t->name);
		if (write(1, line, (size_t)n) < 0)
			_exit(2);
		if (t->output.n && write(1, t->output.p, t->output.n) < 0)
			_exit(2);
	}
	const char *what = sig == SIGSEGV ? "SIGSEGV: segmentation violation"
		: sig == SIGBUS		  ? "SIGBUS: bus error"
		: sig == SIGFPE		  ? "SIGFPE: floating-point exception"
		: sig == SIGILL		  ? "SIGILL: illegal instruction"
					  : "SIGABRT: abort";
	n = snprintf(line, sizeof line, "panic: signal %s\nFAIL\n", what);
	if (write(2, line, (size_t)n) < 0)
		_exit(2);
	_exit(2);
}

static void usage_exit(const char *msg, const char *a) {
	fprintf(stderr, "%s%s\n", msg, a);
	exit(2);
}

static bool parse_bool(const char *v, const char *name) {
	if (!v || !strcmp(v, "true") || !strcmp(v, "1") || !strcmp(v, "t") || !strcmp(v, "T") || !strcmp(v, "TRUE") || !strcmp(v, "True"))
		return true;
	if (!strcmp(v, "false") || !strcmp(v, "0") || !strcmp(v, "f") || !strcmp(v, "F") || !strcmp(v, "FALSE") || !strcmp(v, "False"))
		return false;
	fprintf(stderr, "invalid boolean value \"%s\" for -test.%s\n", v, name);
	exit(2);
}

static void parse_flags(int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		char *a = argv[i];
		if (a[0] != '-')
			break;
		if (!strcmp(a, "--"))
			break;
		a += a[1] == '-' ? 2 : 1;
		if (!strncmp(a, "test.", 5))
			a += 5;
		char *eq = strchr(a, '=');
		char name[64];
		size_t nl = eq ? (size_t)(eq - a) : strlen(a);
		if (nl >= sizeof name)
			usage_exit("flag provided but not defined: ", argv[i]);
		memcpy(name, a, nl);
		name[nl] = 0;
		const char *v = eq ? eq + 1 : NULL;
		bool isbool = !strcmp(name, "v") || !strcmp(name, "short") || !strcmp(name, "failfast");
		static const char *const known[] = {"run", "skip", "bench", "list", "count", "parallel", "timeout", "benchtime",
			"paniconexit0", "timeout_ignored", NULL};
		bool isknown = isbool;
		for (int k = 0; known[k] && !isknown; k++)
			isknown = !strcmp(name, known[k]);
		if (!isknown)
			usage_exit("flag provided but not defined: ", argv[i]);
		if (!isbool && !v) {
			if (i + 1 >= argc)
				usage_exit("flag needs an argument: ", argv[i]);
			v = argv[++i];
		}
		if (!strcmp(name, "v"))
			flags.verbose = parse_bool(v, name);
		else if (!strcmp(name, "short"))
			flags.short_ = parse_bool(v, name);
		else if (!strcmp(name, "failfast"))
			flags.failfast = parse_bool(v, name);
		else if (!strcmp(name, "run"))
			flags.run = v;
		else if (!strcmp(name, "skip"))
			flags.skip = v;
		else if (!strcmp(name, "bench"))
			flags.bench = v;
		else if (!strcmp(name, "list"))
			flags.list = v;
		else if (!strcmp(name, "count")) {
			char *end;
			long c = strtol(v, &end, 10);
			if (*end || c < 0)
				usage_exit("invalid value for flag -test.count: ", v);
			flags.count = (unsigned)c;
		} else if (!strcmp(name, "parallel")) {
			char *end;
			long c = strtol(v, &end, 10);
			if (*end || c < 1) {
				fprintf(stderr, "testing: -parallel can only be given a positive integer\n");
				exit(2);
			}
			flags.parallel = (int)c;
		} else if (!strcmp(name, "timeout")) {
			if (alt_dur_parse(alt_str_make(v, strlen(v)), &flags.timeout) < 0)
				usage_exit("invalid value for flag -test.timeout: ", v);
		} else if (!strcmp(name, "benchtime")) {
			size_t vl = strlen(v);
			if (vl > 1 && v[vl - 1] == 'x') {
				char *end;
				long long c = strtoll(v, &end, 10);
				if (end != v + vl - 1 || c <= 0)
					usage_exit("invalid value for flag -test.benchtime: ", v);
				flags.benchn = (size_t)c;
			} else if (alt_dur_parse(alt_str_make(v, vl), &flags.benchtime) < 0 || flags.benchtime <= 0)
				usage_exit("invalid value for flag -test.benchtime: ", v);
		} else if (!strcmp(name, "paniconexit0") || !strcmp(name, "timeout_ignored"))
			;
		else
			usage_exit("flag provided but not defined: ", argv[i]);
	}
}

static void list_names(const alt_test_def *defs, size_t n) {
	alt_regexp *re;
	alt_buf err = ALT_BUF_INIT;
	if (alt_regexp_compile(&re, alt_str_make(flags.list, strlen(flags.list)), &err) < 0) {
		fprintf(stderr, "testing: invalid regexp in -test.list (\"%s\"): %s\n", flags.list, alt_buf_cstr(&err));
		exit(1);
	}
	for (size_t i = 0; i < n; i++)
		if (match_string(re, defs[i].name))
			printf("%s\n", defs[i].name);
	alt_regexp_free(re);
}

static alt_testing_m the_m;

int alt_testing_m_run(alt_testing_m *m) {
	if (flags.list) {
		list_names(m->tests, m->ntests);
		list_names(m->benches, m->nbenches);
		return 0;
	}
	pthread_t wd;
	if (flags.timeout > 0) {
		pthread_attr_t pa;
		pthread_attr_init(&pa);
		pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);
		pthread_create(&wd, &pa, watchdog, NULL);
		pthread_attr_destroy(&pa);
	}
	bool ok = true, ran = false;
	test_list tl = {m->tests, m->ntests};
	for (unsigned i = 0; i < flags.count; i++) {
		if (flags.failfast && nfailed > 0)
			break;
		if (i > 0 && !ran)
			break;
		pthread_mutex_lock(&mu);
		subnames_clear();
		tstate.max = flags.parallel;
		tstate.running = 1;
		tstate.waiting = 0;
		alt_test *root = new_test(NULL, strdup(""));
		root_test = root;
		root->fn = root_fn, root->arg = &tl;
		root->start = alt_now();
		pthread_mutex_unlock(&mu);
		start_thread(root);
		signal_wait(root);
		ok = ok && !root->failed;
		ran = ran || root->ran;
		pthread_mutex_lock(&mu);
		root_test = NULL;
		pthread_mutex_unlock(&mu);
		free_test(root);
	}
	if (!ran && !(flags.bench && *flags.bench))
		fprintf(stderr, "testing: warning: no tests to run\n");
	if (flags.bench && *flags.bench) {
		/* benchmarks run one at a time, after the tests */
		pthread_mutex_lock(&mu);
		subnames_clear();
		filter_free(&matcher.run);
		pthread_mutex_unlock(&mu);
		if (!filter_init(&matcher.run, flags.bench, "-test.bench"))
			exit(1);
		test_list bl = {m->benches, m->nbenches};
		for (size_t i = 0; i < bl.n; i++) {
			bool mok;
			char *full = full_name(0, "", bl.tests[i].name, &mok);
			if (mok) {
				alt_buf nm = ALT_BUF_INIT;
				bench_name(&nm, full);
				if (nm.n + 1 + (bench_procs != 1 ? 1 + (size_t)snprintf(NULL, 0, "%d", bench_procs) : 0) > bench_maxlen)
					bench_maxlen = nm.n + 1 + (bench_procs != 1 ? 1 + (size_t)snprintf(NULL, 0, "%d", bench_procs) : 0);
				alt_buf_free(&nm);
			}
			free(full);
		}
		alt_bench *main_b = new_test(NULL, strdup("Main"));
		main_b->bench = true;
		root_test = NULL;
		bench_root(main_b, &bl);
		free_test(main_b);
		if (bench_failed)
			ok = false;
	}
	fflush(stdout);
	if (!ok) {
		printf("FAIL\n");
		fflush(stdout);
		return 1;
	}
	printf("PASS\n");
	fflush(stdout);
	return 0;
}

int alt_testing_main(int argc, char **argv, const alt_test_def *tests, size_t ntests, const alt_test_def *benches,
	size_t nbenches, int (*test_main)(alt_testing_m *m)) {
	long np = sysconf(_SC_NPROCESSORS_ONLN);
	flags.parallel = np > 0 ? (int)np : 1;
	bench_procs = flags.parallel;
	parse_flags(argc, argv);
	if (!filter_init(&matcher.run, flags.run, "-test.run") || !filter_init(&matcher.skip, flags.skip, "-test.skip"))
		exit(1);
	struct sigaction sa = {0};
	sa.sa_handler = crash;
	sa.sa_flags = SA_ONSTACK;
	int sigs[] = {SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT};
	for (size_t i = 0; i < sizeof sigs / sizeof *sigs; i++)
		sigaction(sigs[i], &sa, NULL);
	the_m = (alt_testing_m){tests, benches, ntests, nbenches, false};
	if (test_main)
		return test_main(&the_m);
	return alt_testing_m_run(&the_m);
}
