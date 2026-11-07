/* arghex... -> the result of parsing the arguments with a fixed set of
   flags: rc | error | values | rest | set flags | output hex */
#include "common.h"
#include <alt/flag.h>
#include <alt/io.h>
#include <alt/strconv.h>

static alt_buf fnlog;

static int fn(void *arg, const char *v) {
	alt_buf_printf(&fnlog, "%s%s,", (const char *)arg, v);
	return strcmp(v, "bad") == 0 ? -EINVAL : 0;
}

static void visit(const alt_flag *f, void *arg) { alt_buf_printf(arg, "%s,", f->name); }

int main(void) {
	char *line, *f[64];
	while ((line = diff_line())) {
		int n = line[0] ? diff_fields(line, f, 64) : 0;
		char *argv[64];
		for (int i = 0; i < n; i++) {
			f[i][diff_unhex(f[i])] = 0;
			argv[i] = f[i];
		}
		alt_buf out = ALT_BUF_INIT;
		alt_buf_reset(&fnlog);
		alt_flagset *fs = alt_flagset_new("prog", ALT_FLAG_CONTINUE);
		alt_flagset_set_output(fs, alt_buf_writer(&out));
		bool v, t, x;
		long long nn, zero, ln;
		unsigned long long u;
		double fl;
		const char *s, *e;
		alt_dur d, dz;
		alt_flag_bool(fs, &v, "v", false, "verbose");
		alt_flag_bool(fs, &t, "t", true, "on by default");
		alt_flag_bool(fs, &x, "x", false, "");
		alt_flag_int(fs, &nn, "n", 3, "number of `items`");
		alt_flag_int(fs, &zero, "zero", 0, "zero int");
		alt_flag_int(fs, &ln, "long-name-here", -7, "one ` quote");
		alt_flag_uint(fs, &u, "u", 0, "unsigned");
		alt_flag_float(fs, &fl, "f", 0.5, "a float");
		alt_flag_string(fs, &s, "s", "x y", "a string\nsecond line");
		alt_flag_string(fs, &e, "e", "", "empty");
		alt_flag_dur(fs, &d, "d", ALT_SEC, "duration");
		alt_flag_dur(fs, &dz, "dz", 0, "zero dur");
		alt_flag_func(fs, "fn", "func flag", fn, "fn:");
		alt_flag_bool_func(fs, "bf", "bool func", fn, "bf:");
		int dup = alt_flag_int(fs, &zero, "zero", 1, "again");
		int bad = alt_flag_int(fs, &zero, "-bad", 1, "bad");
		int rc = alt_flagset_parse(fs, n, argv);
		printf("%s|%s|%d %d %d %lld %lld %lld %llu ", rc == 0 ? "ok" : rc == -ALT_EFLAG_HELP ? "help" : "parse",
			rc ? alt_flagset_error(fs) : "", v, t, x, nn, zero, ln, u);
		alt_buf q = ALT_BUF_INIT;
		alt_format_float(&q, fl, 'g', 17);
		alt_buf_append_byte(&q, ' ');
		alt_buf_printf(&q, "%s %s %lld %lld %s|", s, e, (long long)d, (long long)dz, alt_buf_cstr(&fnlog));
		for (int i = 0; i < alt_flagset_nargs(fs); i++)
			alt_buf_printf(&q, "%s,", alt_flagset_arg(fs, i));
		alt_buf_printf(&q, "|%d:", alt_flagset_nflag(fs));
		alt_flagset_visit(fs, visit, &q);
		alt_buf_printf(&q, "|%d %d|", dup == -EEXIST, bad == -EINVAL);
		fwrite(q.p, 1, q.n, stdout);
		diff_puthex(out.p, out.n);
		printf("\n");
		alt_buf_free(&q);
		alt_buf_free(&out);
		alt_flagset_free(fs);
	}
	return 0;
}
