/* F ns offset name layout -> formatted
   P layout value -> ns offset name | err
   D ns -> duration string
   Q text -> ns | err
   C ns offset -> date fields */
#include "common.h"
#include <alt/time.h>
#include <inttypes.h>

int main(void) {
	char *line, *f[6];
	alt_buf b = ALT_BUF_INIT;
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 5);
		alt_buf_reset(&b);
		switch (line[0]) {
		case 'F': {
			if (n < 5)
				break;
			alt_zone z = alt_zone_fixed(f[3], atoi(f[2]));
			alt_wall_format(&b, strtoll(f[1], 0, 10), &z, f[4]);
			printf("%s\n", alt_buf_cstr(&b));
			break;
		}
		case 'P': {
			alt_wall t;
			alt_zone z;
			int rc = alt_wall_parse(alt_str_cstr(f[2]), f[1], &t, &z);
			if (rc < 0)
				printf("err\n");
			else
				printf("%" PRId64 " %d %s\n", t, z.offset, z.name);
			break;
		}
		case 'D':
			alt_dur_format(&b, strtoll(f[1], 0, 10));
			printf("%s\n", alt_buf_cstr(&b));
			break;
		case 'Q': {
			alt_dur d;
			if (alt_dur_parse(alt_str_cstr(f[1]), &d) < 0)
				printf("err\n");
			else
				printf("%" PRId64 "\n", d);
			break;
		}
		case 'C': {
			alt_zone z = alt_zone_fixed("", atoi(f[2]));
			alt_date d;
			alt_wall_date(strtoll(f[1], 0, 10), &z, &d);
			printf("%" PRId64 " %d %d %d %d %d %d %d %d\n", d.year, d.month, d.day, d.hour, d.min, d.sec, d.nsec, d.wday, d.yday);
			break;
		}
		}
	}
	alt_buf_free(&b);
	return 0;
}
