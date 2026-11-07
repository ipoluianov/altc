/* hpe op... -> the blocks of one encoder, hex, separated by spaces
	(ops: S<size>, L<size>, F<name hex>:<value hex>, f... sensitive, B ends a block)
   hpd max maxstr block... -> each block's fields "ok name:value[:s],..." or
	"err <message>" (which ends the line), separated by " | "; a block
	"T<size>" changes the allowed table size */
#include "common.h"
#include <alt/net/hpack.h>

static int collect(void *arg, const alt_hpack_field *f) {
	alt_buf *b = arg;
	if (b->n > 3)
		alt_buf_append_byte(b, ',');
	for (size_t i = 0; i < f->name.n; i++)
		alt_buf_printf(b, "%02x", (unsigned char)f->name.p[i]);
	alt_buf_append_byte(b, ':');
	for (size_t i = 0; i < f->value.n; i++)
		alt_buf_printf(b, "%02x", (unsigned char)f->value.p[i]);
	if (f->sensitive)
		alt_buf_append_cstr(b, ":s");
	return 0;
}

int main(void) {
	char *line, *f[4096];
	while ((line = diff_line())) {
		int n = diff_fields(line, f, 4096);
		if (!strcmp(f[0], "hpe")) {
			alt_hpack_encoder e;
			alt_hpack_encoder_init(&e);
			alt_buf b = ALT_BUF_INIT;
			bool first = true;
			for (int i = 1; i < n; i++) {
				char *op = f[i];
				if (op[0] == 'S')
					alt_hpack_encoder_set_max_table_size(&e, (uint32_t)strtoul(op + 1, 0, 10));
				else if (op[0] == 'L')
					alt_hpack_encoder_set_max_table_size_limit(&e, (uint32_t)strtoul(op + 1, 0, 10));
				else if (op[0] == 'F' || op[0] == 'f') {
					char *colon = strchr(op + 1, ':');
					*colon = 0;
					size_t nn = diff_unhex(op + 1), nv = diff_unhex(colon + 1);
					alt_hpack_field fl = {{op + 1, nn}, {colon + 1, nv}, op[0] == 'f'};
					alt_hpack_encode(&e, &b, &fl);
				} else if (op[0] == 'B') {
					if (!first)
						printf(" ");
					first = false;
					diff_puthex(b.p ? b.p : "", b.n);
					alt_buf_reset(&b);
				}
			}
			printf("\n");
			alt_buf_free(&b);
			alt_hpack_encoder_free(&e);
		} else {
			alt_hpack_decoder d;
			alt_hpack_decoder_init(&d, (uint32_t)strtoul(f[1], 0, 10));
			d.max_string = strtoul(f[2], 0, 10);
			bool first = true;
			for (int i = 3; i < n; i++) {
				if (f[i][0] == 'T') {
					alt_hpack_decoder_set_allowed_max_table_size(&d, (uint32_t)strtoul(f[i] + 1, 0, 10));
					continue;
				}
				size_t bn = diff_unhex(f[i]);
				alt_arena a;
				alt_arena_init(&a, 0);
				alt_buf out = ALT_BUF_INIT;
				alt_buf_append_cstr(&out, "ok ");
				int rc = alt_hpack_decode(&d, f[i], bn, &a, collect, &out);
				if (!first)
					printf(" | ");
				first = false;
				if (rc < 0) {
					printf("err %s", alt_hpack_error(&d));
					alt_buf_free(&out);
					alt_arena_free(&a);
					break;
				}
				/* "ok " with no fields is "ok " in Go too */
				printf("%s", alt_buf_cstr(&out));
				alt_buf_free(&out);
				alt_arena_free(&a);
			}
			printf("\n");
			alt_hpack_decoder_free(&d);
		}
		fflush(stdout);
	}
	return 0;
}
