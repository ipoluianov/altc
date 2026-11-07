/* A small test framework for the standard library's C tests: each test
   file is a program whose main runs its tests with RUN and returns
   check_done(). Failed checks print file:line and the values. */
#ifndef ALT_TESTS_CHECK_H
#define ALT_TESTS_CHECK_H

#include <alt/errors.h>
#include <alt/str.h>
#include <inttypes.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static _Atomic int check_failures, check_count; /* checks run in threads too */
static inline alt_str check_as_str(alt_str s) { return s; }
static inline alt_str check_as_cstr(const char *s) { return alt_str_cstr(s ? s : "(null)"); }
static const char *check_current = "";

#define FAILED(...) \
	do { \
		check_failures++; \
		fprintf(stderr, "%s:%d: [%s] ", __FILE__, __LINE__, check_current); \
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr); \
	} while (0)

#define CHECK(c) \
	do { \
		check_count++; \
		if (!(c)) \
			FAILED("CHECK(%s) failed", #c); \
	} while (0)

#define CHECK_INT(a, b) \
	do { \
		long long check_a = (long long)(a), check_b = (long long)(b); \
		check_count++; \
		if (check_a != check_b) \
			FAILED("%s == %lld, want %lld (%s)", #a, check_a, check_b, #b); \
	} while (0)

/* an alt_str or C string against a C string */
#define CHECK_STR(a, b) \
	do { \
		alt_str check_s = _Generic((a), alt_str: check_as_str, default: check_as_cstr)(a); \
		const char *check_w = (b); \
		check_count++; \
		if (!alt_str_eq_cstr(check_s, check_w)) \
			FAILED("%s == \"%.*s\", want \"%s\"", #a, (int)check_s.n, check_s.p, check_w); \
	} while (0)

/* an alt return code that must be success */
#define CHECK_OK(e) \
	do { \
		long long check_rc = (long long)(e); \
		check_count++; \
		if (check_rc < 0) \
			FAILED("%s failed: %s (%lld)", #e, alt_strerror((int)check_rc), check_rc); \
	} while (0)

#define CHECK_ERR(e, code) \
	do { \
		long long check_rc = (long long)(e); \
		check_count++; \
		if (check_rc != (long long)(code)) \
			FAILED("%s == %lld (%s), want %s", #e, check_rc, check_rc < 0 ? alt_strerror((int)check_rc) : "success", #code); \
	} while (0)

#define RUN(fn) \
	do { \
		check_current = #fn; \
		int check_before = check_failures; \
		struct timespec check_t0, check_t1; \
		clock_gettime(CLOCK_MONOTONIC, &check_t0); \
		fn(); \
		clock_gettime(CLOCK_MONOTONIC, &check_t1); \
		printf("%s %s (%.3fs)\n", check_failures == check_before ? "ok  " : "FAIL", #fn, \
			(double)(check_t1.tv_sec - check_t0.tv_sec) + (check_t1.tv_nsec - check_t0.tv_nsec) / 1e9); \
		fflush(stdout); \
	} while (0)

static inline int check_done(void) {
	printf("%d checks, %d failed\n", check_count, check_failures);
	return check_failures ? 1 : 0;
}

#endif
