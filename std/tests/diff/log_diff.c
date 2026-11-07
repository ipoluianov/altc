/* format level msghex [key kind value]... -> the record (format 0 text,
   1 json) without time; kinds s (hex string), i, u, f, b, d (ns) */
#include "common.h"
#include <alt/log.h>

int main(void) {
	char *line, *f[64];
	alt_buf out = ALT_BUF_INIT;
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 64);
		alt_buf_reset(&out);
		alt_logger *l;
		alt_logger_config cfg = {.out = alt_buf_writer(&out), .format = atoi(f[0]), .level = ALT_LOG_DEBUG - 10, .no_time = true};
		alt_logger_new(&l, &cfg);
		alt_log_attr attrs[20];
		int na = 0;
		f[2][diff_unhex(f[2])] = 0;
		for (int i = 3; i + 2 < n; i += 3) {
			alt_log_attr a = {0};
			f[i][diff_unhex(f[i])] = 0;
			a.key = f[i];
			char *v = f[i + 2];
			switch (f[i + 1][0]) {
			case 's': a.kind = ALT_LOGV_STRING, a.s = alt_str_make(v, diff_unhex(v)); break;
			case 'i': a.kind = ALT_LOGV_INT, a.i = strtoll(v, 0, 10); break;
			case 'u': a.kind = ALT_LOGV_UINT, a.u = strtoull(v, 0, 10); break;
			case 'f': a.kind = ALT_LOGV_FLOAT, a.f = strtod(v, 0); break;
			case 'b': a.kind = ALT_LOGV_BOOL, a.b = v[0] == '1'; break;
			case 'd': a.kind = ALT_LOGV_DUR, a.d = strtoll(v, 0, 10); break;
			}
			attrs[na++] = a;
		}
		alt_log_attrs(l, atoi(f[1]), f[2], attrs, (size_t)na);
		alt_logger_free(l);
		/* one answer line: the record's newline is the line end */
		fwrite(out.p, 1, out.n, stdout);
	}
	return 0;
}
