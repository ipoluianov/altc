#include "check.h"
#include <alt/flag.h>

static void names(const alt_flag *f, void *arg) { alt_buf_printf(arg, "%s=%s,", f->name, f->def_value); }

static int collect(void *arg, const char *v) { return alt_buf_printf(arg, "[%s]", v) < 0 ? -ENOMEM : 0; }

static void command_line(void) {
	long long port;
	bool verbose;
	const char *host;
	alt_dur timeout;
	alt_buf seen = ALT_BUF_INIT;
	CHECK_OK(alt_flag_int(NULL, &port, "port", 8080, "listen on `port`"));
	CHECK_OK(alt_flag_bool(NULL, &verbose, "v", false, "verbose"));
	CHECK_OK(alt_flag_string(NULL, &host, "host", "localhost", "host name"));
	CHECK_OK(alt_flag_dur(NULL, &timeout, "timeout", 30 * ALT_SEC, "timeout"));
	CHECK_OK(alt_flag_func(NULL, "add", "adds", collect, &seen));
	CHECK_INT(port, 8080);
	CHECK_STR(host, "localhost");
	CHECK_INT(alt_flag_int(NULL, &port, "port", 1, "again"), -EEXIST);
	CHECK_INT(alt_flag_int(NULL, &port, "a=b", 1, ""), -EINVAL);
	CHECK_INT(port, 8080);
	char *argv[] = {"/bin/prog", "-port=9000", "--v", "-host", "example.com", "-add", "x", "-add=y", "-timeout", "1m30s",
		"file1", "-notaflag", 0};
	alt_flag_parse(12, argv);
	CHECK_INT(port, 9000);
	CHECK(verbose);
	CHECK_STR(host, "example.com");
	CHECK_INT(timeout, 90 * ALT_SEC);
	CHECK_STR(alt_buf_str(&seen), "[x][y]");
	alt_flagset *fs = alt_flag_command_line();
	CHECK(alt_flagset_parsed(fs));
	CHECK_STR(alt_flagset_name(fs), "/bin/prog");
	CHECK_INT(alt_flagset_nargs(fs), 2);
	CHECK_STR(alt_flagset_arg(fs, 0), "file1");
	CHECK_STR(alt_flagset_arg(fs, 1), "-notaflag");
	CHECK(alt_flagset_arg(fs, 2) == 0);
	CHECK_INT(alt_flagset_nflag(fs), 5);
	const alt_flag *f = alt_flagset_lookup(NULL, "port");
	CHECK(f && f->kind == ALT_FLAGK_INT && f->p == &port);
	CHECK_STR(f->def_value, "8080");
	CHECK(alt_flagset_lookup(NULL, "nope") == 0);
	CHECK_OK(alt_flagset_set(NULL, "port", "0x10"));
	CHECK_INT(port, 16);
	CHECK_INT(alt_flagset_set(NULL, "port", "x"), -ALT_ESYNTAX);
	CHECK_INT(alt_flagset_set(NULL, "nope", "1"), -ENOENT);
	alt_buf b = ALT_BUF_INIT;
	alt_flagset_visit_all(fs, names, &b);
	CHECK_STR(alt_buf_str(&b), "add=,host=localhost,port=8080,timeout=30s,v=false,");
	alt_buf name = ALT_BUF_INIT, usage = ALT_BUF_INIT;
	alt_flag_unquote_usage(f, &name, &usage);
	CHECK_STR(alt_buf_str(&name), "port");
	CHECK_STR(alt_buf_str(&usage), "listen on port");
	alt_buf_free(&b);
	alt_buf_free(&name);
	alt_buf_free(&usage);
	alt_buf_free(&seen);
}

static void custom_usage_fn(alt_flagset *fs, void *arg) {
	(void)fs;
	(*(int *)arg)++;
}

static void custom_usage(void) {
	alt_flagset *fs = alt_flagset_new("t", ALT_FLAG_CONTINUE);
	alt_buf out = ALT_BUF_INIT;
	alt_flagset_set_output(fs, alt_buf_writer(&out));
	int calls = 0;
	alt_flagset_set_usage(fs, custom_usage_fn, &calls);
	char *a1[] = {"-zz"};
	CHECK_INT(alt_flagset_parse(fs, 1, a1), -ALT_EFLAG_PARSE);
	CHECK_STR(alt_flagset_error(fs), "flag provided but not defined: -zz");
	CHECK_STR(alt_buf_str(&out), "flag provided but not defined: -zz\n");
	char *a2[] = {"-h"};
	CHECK_INT(alt_flagset_parse(fs, 1, a2), -ALT_EFLAG_HELP);
	CHECK_INT(calls, 2);
	/* many flags: the sorted table grows */
	long long v[100];
	for (int i = 0; i < 100; i++) {
		char n[8];
		snprintf(n, sizeof n, "f%02d", 99 - i);
		CHECK_OK(alt_flag_int(fs, &v[i], n, i, ""));
	}
	char *a3[] = {"-f00=5", "-f99", "7"};
	CHECK_OK(alt_flagset_parse(fs, 3, a3));
	CHECK_INT(v[99], 5);
	CHECK_INT(v[0], 7);
	CHECK_INT(v[50], 50);
	alt_flagset_free(fs);
	alt_buf_free(&out);
}

int main(void) {
	RUN(command_line);
	RUN(custom_usage);
	return check_done();
}
