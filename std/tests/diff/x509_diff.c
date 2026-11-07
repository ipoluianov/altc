/* der hex -> a summary of the parsed certificate, or "error <message>"
   (the same summary as tests/stdx509_test.go makes with Go) */
#include "common.h"
#include <alt/crypto/x509.h>
#include <alt/net/url.h>
#include <inttypes.h>

static void hex(alt_buf *b, alt_der d) {
	for (size_t i = 0; i < d.n; i++)
		alt_buf_printf(b, "%02x", d.p[i]);
}

static void strs(alt_buf *b, const char *name, const alt_x509_strs *s) {
	alt_buf_printf(b, "|%s=", name);
	for (size_t i = 0; i < s->n; i++)
	{
		if (i)
			alt_buf_append_byte(b, ',');
		hex(b, alt_der_make(s->items[i].p, s->items[i].n));
	}
}

static void oids(alt_buf *b, const char *name, const alt_oid *o, size_t n) {
	alt_buf_printf(b, "|%s=", name);
	for (size_t i = 0; i < n; i++) {
		if (i)
			alt_buf_append_byte(b, ',');
		alt_oid_format(b, &o[i]);
	}
}

static void name(alt_buf *b, const char *label, const alt_x509_name *n) {
	alt_buf_printf(b, "|%s=", label);
	for (size_t i = 0; i < n->n; i++) {
		const alt_x509_attr *a = &n->attrs[i];
		if (i)
			alt_buf_append_byte(b, ';');
		alt_oid_format(b, &a->type);
		bool str = a->tag == ALT_ASN1_T61_STRING || a->tag == ALT_ASN1_PRINTABLE_STRING || a->tag == ALT_ASN1_UTF8_STRING ||
			   a->tag == ALT_ASN1_BMP_STRING || a->tag == ALT_ASN1_IA5_STRING || a->tag == 18;
		if (str) {
			alt_buf_append_byte(b, '=');
			hex(b, alt_der_make(a->value.p, a->value.n));
		}
		else
			alt_buf_append_cstr(b, "=?");
	}
}

static void nets(alt_buf *b, const char *label, const alt_x509_ipnet *n, size_t k) {
	alt_buf_printf(b, "|%s=", label);
	for (size_t i = 0; i < k; i++) {
		if (i)
			alt_buf_append_byte(b, ',');
		hex(b, alt_der_make(n[i].ip, (size_t)n[i].len));
		alt_buf_append_byte(b, '/');
		hex(b, alt_der_make(n[i].mask, (size_t)n[i].len));
	}
}

int main(void) {
	char *line;
	while ((line = diff_line())) {
		size_t len = diff_unhex(line);
		alt_arena a;
		alt_arena_init(&a, 0);
		alt_x509_cert *c;
		const char *why = "";
		if (alt_x509_parse(&a, line, len, &c, &why) < 0) {
			printf("error %s\n", why);
			alt_arena_free(&a);
			continue;
		}
		alt_buf b = ALT_BUF_INIT;
		alt_buf_printf(&b, "ok|v=%d|serial=", c->version);
		alt_der sn = c->serial;
		while (sn.n > 1 && sn.p[0] == 0)
			sn.p++, sn.n--;
		hex(&b, sn);
		alt_buf_printf(&b, "|sig=%d|pk=%d", c->sig_alg, c->pk_alg);
		name(&b, "iss", &c->issuer);
		name(&b, "sub", &c->subject);
		alt_buf_printf(&b, "|nb=%" PRId64 "|na=%" PRId64 "|key=", c->not_before, c->not_after);
		switch (c->pk_alg) {
		case ALT_X509_PK_RSA:
			hex(&b, c->pub.n);
			alt_buf_printf(&b, ",%" PRId64, c->pub.e);
			break;
		case ALT_X509_PK_ECDSA: alt_buf_printf(&b, "%d,", c->pub.curve); /* fall through */
		default: hex(&b, c->pub.point);
		}
		alt_buf_printf(&b, "|ku=%d|bc=%d,%d,%d,%d|skid=", c->key_usage, c->basic_constraints_valid, c->is_ca, c->max_path_len,
			c->max_path_len_zero);
		hex(&b, c->subject_key_id);
		alt_buf_append_cstr(&b, "|akid=");
		hex(&b, c->authority_key_id);
		strs(&b, "dns", &c->dns_names);
		strs(&b, "em", &c->emails);
		alt_buf_append_cstr(&b, "|ip=");
		for (size_t i = 0; i < c->nips; i++) {
			if (i)
				alt_buf_append_byte(&b, ',');
			hex(&b, alt_der_make(c->ips[i].b, (size_t)c->ips[i].len));
		}
		alt_buf_append_cstr(&b, "|uri=");
		for (size_t i = 0; i < c->uris.n; i++) {
			alt_url u;
			alt_buf us = ALT_BUF_INIT;
			alt_url_parse(&a, c->uris.items[i], &u);
			alt_url_string(&u, &us);
			if (i)
				alt_buf_append_byte(&b, ',');
			hex(&b, alt_der_make(us.p, us.n));
			alt_buf_free(&us);
		}
		alt_buf_append_cstr(&b, "|eku=");
		for (size_t i = 0; i < c->neku; i++)
			alt_buf_printf(&b, "%s%d", i ? "," : "", c->ext_key_usage[i]);
		oids(&b, "ueku", c->unknown_eku, c->nunknown_eku);
		oids(&b, "pol", c->policies, c->npolicies);
		oids(&b, "uc", c->unhandled_critical, c->nunhandled_critical);
		strs(&b, "crl", &c->crl_dps);
		strs(&b, "ocsp", &c->ocsp);
		strs(&b, "issurl", &c->issuing_urls);
		strs(&b, "pdns", &c->permitted_dns);
		strs(&b, "edns", &c->excluded_dns);
		nets(&b, "pip", c->permitted_ips, c->npermitted_ips);
		nets(&b, "eip", c->excluded_ips, c->nexcluded_ips);
		strs(&b, "pem", &c->permitted_emails);
		strs(&b, "eem", &c->excluded_emails);
		strs(&b, "puri", &c->permitted_uris);
		strs(&b, "euri", &c->excluded_uris);
		alt_buf_printf(&b, "|pcrit=%d|sigv=", c->permitted_dns_critical);
		hex(&b, c->signature);
		printf("%s\n", alt_buf_cstr(&b));
		alt_buf_free(&b);
		alt_arena_free(&a);
	}
	return 0;
}
