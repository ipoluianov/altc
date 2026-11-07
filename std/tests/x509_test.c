#include "check.h"
#include <alt/crypto/x509.h>
#include <alt/encoding/asn1.h>
#include <alt/encoding/pem.h>

/* a chain made by Go: an ECDSA P-256 root, an Ed25519 intermediate, an
   RSA leaf for example.com, *.example.com and 127.0.0.1 (2025-2035) */
static const char root_pem[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBizCCATGgAwIBAgIBATAKBggqhkjOPQQDAjAtMRIwEAYDVQQKEwlBbHRDIFRl\n"
	"c3QxFzAVBgNVBAMTDkFsdEMgVGVzdCBSb290MB4XDTI1MDEwMTAwMDAwMFoXDTM1\n"
	"MDEwMTAwMDAwMFowLTESMBAGA1UEChMJQWx0QyBUZXN0MRcwFQYDVQQDEw5BbHRD\n"
	"IFRlc3QgUm9vdDBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABHp9L7kHbWHbpCdv\n"
	"cOBi2IZyGn5IQ3zUwiHeXzLRlFNGzgaiCyd5fsqVPlA5vOg8A5k817kyGE7KQ94e\n"
	"7f9W41+jQjBAMA4GA1UdDwEB/wQEAwIChDAPBgNVHRMBAf8EBTADAQH/MB0GA1Ud\n"
	"DgQWBBQgatiZqV7ubpvw3v4SMVlVsG4I+zAKBggqhkjOPQQDAgNIADBFAiEAtaGQ\n"
	"G0Zoht62TpPEPb6tn54SfxkMb1IobTyqRZgpQOMCIFguIIRLnzHDT+/L+6ZJWGNx\n"
	"f3LslTGlR+2aFWr+lKd3\n"
	"-----END CERTIFICATE-----\n"
	;
static const char inter_pem[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBhTCCASugAwIBAgIBAjAKBggqhkjOPQQDAjAtMRIwEAYDVQQKEwlBbHRDIFRl\n"
	"c3QxFzAVBgNVBAMTDkFsdEMgVGVzdCBSb290MB4XDTI1MDEwMTAwMDAwMFoXDTM1\n"
	"MDEwMTAwMDAwMFowNTESMBAGA1UEChMJQWx0QyBUZXN0MR8wHQYDVQQDExZBbHRD\n"
	"IFRlc3QgSW50ZXJtZWRpYXRlMCowBQYDK2VwAyEAXqi8ld0wFaWFVsvV3ZtUoKQS\n"
	"RLXU/wKogaCLaYC/b16jYzBhMA4GA1UdDwEB/wQEAwIChDAPBgNVHRMBAf8EBTAD\n"
	"AQH/MB0GA1UdDgQWBBR69/b0uYyeAik4Bnv+xZBUUy1K8DAfBgNVHSMEGDAWgBQg\n"
	"atiZqV7ubpvw3v4SMVlVsG4I+zAKBggqhkjOPQQDAgNIADBFAiEAiH6Jy1d8t1fw\n"
	"PBd9zoVpwn0fIv33vKdvAqHN/7BWN3wCIHVF9nV7fygATKkciDp58Y82YF/saAZ2\n"
	"IKpP61f+z3u+\n"
	"-----END CERTIFICATE-----\n"
	;
static const char leaf_pem[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIChjCCAjigAwIBAgIBAzAFBgMrZXAwNTESMBAGA1UEChMJQWx0QyBUZXN0MR8w\n"
	"HQYDVQQDExZBbHRDIFRlc3QgSW50ZXJtZWRpYXRlMB4XDTI1MDEwMTAwMDAwMFoX\n"
	"DTM1MDEwMTAwMDAwMFowIzESMBAGA1UEChMJQWx0QyBUZXN0MQ0wCwYDVQQDEwRs\n"
	"ZWFmMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA0tXNfrQlQW12+VEH\n"
	"WJCqI1lzQWv41VJXzmKETnS3GDeFDovQmRRSbHBHnYjPaNh0u+TH4LyvKK0bjauL\n"
	"dg593iUJPK0UoTF56kmV9h1/wc051TaWGlUJGG3BMH2k22NbuzrF0LOBDwZETnRF\n"
	"Y2+j3EinmwiFfFGB5dwDI+tVYt3TocGL8nuZZuCITqwVSjbwGcpYWGjZSEo1fZcB\n"
	"gjQyt18B1xBznF0+fE+RGsaVuFEOeWnWRTAgqMrSCSIVPWH/M33vBV92QlCpU9/5\n"
	"eHjE/T/2sShM52FEVIWPT9sd6BMVXV4XODo3D+GdAMaJmny68nVJXVDw65F4u4jk\n"
	"IXyNqQIDAQABo4GEMIGBMA4GA1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEF\n"
	"BQcDATAMBgNVHRMBAf8EAjAAMB8GA1UdIwQYMBaAFHr39vS5jJ4CKTgGe/7FkFRT\n"
	"LUrwMCsGA1UdEQQkMCKCC2V4YW1wbGUuY29tgg0qLmV4YW1wbGUuY29thwR/AAAB\n"
	"MAUGAytlcANBAL5dywHSWrx+d+9yLDIs8ACHGxpgSwS4QGpn5SfwEQH18eJl7QGC\n"
	"NeRQDuxb0AFCLiwGSNgPu5l/xYBfylVPpQ8=\n"
	"-----END CERTIFICATE-----\n"
	;

static void der(void) {
	alt_buf b = ALT_BUF_INIT;
	alt_der_builder d;
	alt_der_builder_init(&d, &b);
	alt_der_begin(&d, ALT_ASN1_SEQUENCE);
	alt_der_add_int64(&d, -129);
	alt_der_add_int64(&d, 127);
	alt_der_add_uint_bytes(&d, "\x00\x80", 2);
	alt_der_add_oid(&d, &ALT_OID(1, 2, 840, 113549, 1, 1, 11));
	alt_der_add_bool(&d, true);
	alt_der_add_null(&d);
	alt_der_add_time(&d, 1700000000);
	alt_der_add_time(&d, 253402300799); /* 9999-12-31: GeneralizedTime */
	char big[300];
	memset(big, 'x', sizeof big);
	alt_der_add(&d, ALT_ASN1_OCTET_STRING, big, sizeof big); /* a long length */
	alt_der_end(&d);
	CHECK_OK(alt_der_builder_err(&d));
	alt_der s = alt_der_make(b.p, b.n), seq, v;
	CHECK(alt_der_read(&s, &seq, ALT_ASN1_SEQUENCE) && alt_der_empty(s));
	int64_t i;
	CHECK(alt_der_read_int64(&seq, &i) && i == -129);
	CHECK(alt_der_read_int64(&seq, &i) && i == 127);
	CHECK(alt_der_read_uint_bytes(&seq, &v) && v.n == 1 && v.p[0] == 0x80);
	alt_oid o;
	CHECK(alt_der_read_oid(&seq, &o) && alt_oid_eq(&o, &ALT_OID(1, 2, 840, 113549, 1, 1, 11)));
	bool bl;
	CHECK(alt_der_read_bool(&seq, &bl) && bl);
	CHECK(alt_der_skip(&seq, ALT_ASN1_NULL));
	int64_t t;
	CHECK(alt_der_read_utc_time(&seq, &t) && t == 1700000000);
	CHECK(alt_der_read_generalized_time(&seq, &t) && t == 253402300799);
	CHECK(alt_der_read(&seq, &v, ALT_ASN1_OCTET_STRING) && v.n == 300 && alt_der_empty(seq));
	alt_buf_free(&b);
}

static void parse_and_verify(void) {
	alt_arena a;
	alt_arena_init(&a, 0);
	alt_str rest = ALT_STR(leaf_pem);
	alt_pem_block blk;
	CHECK(alt_pem_decode(&a, &rest, &blk));
	CHECK_STR(blk.type, "CERTIFICATE");
	CHECK(!alt_pem_decode(&a, &rest, &blk));
	alt_x509_cert *leaf;
	const char *why = "";
	CHECK_OK(alt_x509_parse(&a, blk.bytes.p, blk.bytes.n, &leaf, &why));
	CHECK_INT(leaf->version, 3);
	CHECK_INT(leaf->pk_alg, ALT_X509_PK_RSA);
	CHECK_INT(leaf->sig_alg, ALT_X509_ED25519);
	CHECK_INT(leaf->dns_names.n, 2);
	CHECK_STR(leaf->dns_names.items[1], "*.example.com");
	CHECK_STR(alt_x509_name_get(&leaf->subject, &ALT_OID(2, 5, 4, 3)), "leaf");
	alt_x509_pool *roots = alt_x509_pool_new(), *inters = alt_x509_pool_new();
	CHECK_INT(alt_x509_pool_add_pem(roots, root_pem, strlen(root_pem)), 1);
	CHECK_INT(alt_x509_pool_add_pem(inters, inter_pem, strlen(inter_pem)), 1);
	const char *hosts[] = {"example.com", "www.example.com", "127.0.0.1", "EXAMPLE.COM."};
	for (int i = 0; i < 4; i++) {
		alt_x509_verify_opts o = {.dns_name = hosts[i], .roots = roots, .intermediates = inters, .now = 1800000000};
		alt_x509_chain *chains;
		size_t n;
		CHECK_OK(alt_x509_verify(leaf, &o, &a, &chains, &n, &why));
		CHECK_INT(n, 1);
		CHECK(n == 1 && chains[0].n == 3);
	}
	alt_x509_verify_opts o = {.dns_name = "a.b.example.com", .roots = roots, .intermediates = inters, .now = 1800000000};
	alt_x509_chain *chains;
	size_t n;
	CHECK_INT(alt_x509_verify(leaf, &o, &a, &chains, &n, &why), -ALT_EX509_VERIFY);
	CHECK_STR(why, "x509: certificate is valid for example.com, *.example.com, not a.b.example.com");
	o.dns_name = "example.com";
	o.now = 2100000000; /* after 2035 */
	CHECK_INT(alt_x509_verify(leaf, &o, &a, &chains, &n, &why), -ALT_EX509_VERIFY);
	CHECK(strstr(why, "has expired") != 0);
	o.now = 1800000000;
	o.intermediates = 0;
	CHECK_INT(alt_x509_verify(leaf, &o, &a, &chains, &n, &why), -ALT_EX509_VERIFY);
	CHECK_STR(why, "x509: certificate signed by unknown authority");
	alt_x509_pool_free(roots);
	alt_x509_pool_free(inters);
	alt_arena_free(&a);
}

static void system_roots(void) {
	const alt_x509_pool *p = alt_x509_system_roots();
	if (!p) {
		printf("   (no system roots here)\n");
		return;
	}
	CHECK(alt_x509_pool_len(p) > 10);
	CHECK(alt_x509_system_roots() == p);
}

int main(void) {
	RUN(der);
	RUN(parse_and_verify);
	RUN(system_roots);
	return check_done();
}
