/* AltC standard library: X.509 certificates (RFC 5280; Go's
   crypto/x509).

   alt_x509_parse checks a DER certificate as Go's ParseCertificate does
   and fills an alt_x509_cert whose parts live in the given arena (views
   into the DER bytes, which must outlive it). */
#ifndef ALT_CRYPTO_X509_H
#define ALT_CRYPTO_X509_H

#include <alt/arena.h>
#include <alt/encoding/asn1.h>
#include <alt/errors.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	ALT_X509_SIG_UNKNOWN,
	ALT_X509_MD2_RSA, /* (unused: Go has the value) */
	ALT_X509_MD5_RSA,
	ALT_X509_SHA1_RSA,
	ALT_X509_SHA256_RSA,
	ALT_X509_SHA384_RSA,
	ALT_X509_SHA512_RSA,
	ALT_X509_DSA_SHA1,
	ALT_X509_DSA_SHA256,
	ALT_X509_ECDSA_SHA1,
	ALT_X509_ECDSA_SHA256,
	ALT_X509_ECDSA_SHA384,
	ALT_X509_ECDSA_SHA512,
	ALT_X509_SHA256_RSAPSS,
	ALT_X509_SHA384_RSAPSS,
	ALT_X509_SHA512_RSAPSS,
	ALT_X509_ED25519,
} alt_x509_sig_alg; /* Go's SignatureAlgorithm values */

typedef enum {
	ALT_X509_PK_UNKNOWN,
	ALT_X509_PK_RSA,
	ALT_X509_PK_DSA,
	ALT_X509_PK_ECDSA,
	ALT_X509_PK_ED25519,
	ALT_X509_PK_MLDSA,
} alt_x509_pk_alg; /* Go's PublicKeyAlgorithm values */

enum { ALT_X509_P256 = 1, ALT_X509_P384, ALT_X509_P521 };

/* key usage bits as Go's KeyUsage */
enum {
	ALT_X509_KU_DIGITAL_SIGNATURE = 1 << 0,
	ALT_X509_KU_CONTENT_COMMITMENT = 1 << 1,
	ALT_X509_KU_KEY_ENCIPHERMENT = 1 << 2,
	ALT_X509_KU_DATA_ENCIPHERMENT = 1 << 3,
	ALT_X509_KU_KEY_AGREEMENT = 1 << 4,
	ALT_X509_KU_CERT_SIGN = 1 << 5,
	ALT_X509_KU_CRL_SIGN = 1 << 6,
	ALT_X509_KU_ENCIPHER_ONLY = 1 << 7,
	ALT_X509_KU_DECIPHER_ONLY = 1 << 8,
};

/* extended key usages as Go's ExtKeyUsage */
enum {
	ALT_X509_EKU_ANY,
	ALT_X509_EKU_SERVER_AUTH,
	ALT_X509_EKU_CLIENT_AUTH,
	ALT_X509_EKU_CODE_SIGNING,
	ALT_X509_EKU_EMAIL_PROTECTION,
	ALT_X509_EKU_IPSEC_END_SYSTEM,
	ALT_X509_EKU_IPSEC_TUNNEL,
	ALT_X509_EKU_IPSEC_USER,
	ALT_X509_EKU_TIME_STAMPING,
	ALT_X509_EKU_OCSP_SIGNING,
	ALT_X509_EKU_MS_SERVER_GATED_CRYPTO,
	ALT_X509_EKU_NS_SERVER_GATED_CRYPTO,
	ALT_X509_EKU_MS_COMMERCIAL_CODE_SIGNING,
	ALT_X509_EKU_MS_KERNEL_CODE_SIGNING,
};

/* an attribute of a distinguished name (pkix.AttributeTypeAndValue);
   string values are decoded to UTF-8 */
typedef struct {
	alt_oid type;
	unsigned tag;          /* of the value */
	alt_str value;         /* strings decoded; other types their contents */
	int set;               /* the index of the RDN set it belongs to */
} alt_x509_attr;

typedef struct {
	alt_x509_attr *attrs;
	size_t n;
} alt_x509_name;

/* the first value of an attribute type (2.5.4.3 is the common name), empty if none */
alt_str alt_x509_name_get(const alt_x509_name *n, const alt_oid *type);

typedef struct {
	alt_oid id;            /* n is 0 for OIDs of more than 32 arcs */
	alt_der raw_id;        /* the OID's contents */
	bool critical;
	alt_der value;
} alt_x509_ext;

typedef struct {
	uint8_t ip[16], mask[16];
	int len;               /* 4 or 16 */
} alt_x509_ipnet;

typedef struct {
	alt_str *items;
	size_t n;
} alt_x509_strs;

typedef struct {
	alt_der issuer, subject;  /* the OIDs' contents */
} alt_x509_policy_mapping;

typedef struct alt_x509_cert {
	alt_der raw, raw_tbs, raw_sig_alg, raw_issuer, raw_subject, raw_spki;
	int version;           /* 1, 2 or 3 */
	alt_der serial;        /* the INTEGER's bytes (two's complement) */
	alt_x509_sig_alg sig_alg;
	alt_x509_name issuer, subject;
	int64_t not_before, not_after; /* Unix seconds */
	alt_x509_pk_alg pk_alg;
	struct {
		alt_der n;         /* RSA modulus, no leading zeros */
		int64_t e;
		int curve;         /* ECDSA: ALT_X509_P256... */
		alt_der point;     /* ECDSA: the uncompressed point; Ed25519, ML-DSA: the key; DSA: y */
	} pub;
	alt_der signature;
	alt_x509_ext *exts;
	size_t nexts;
	int key_usage;
	bool basic_constraints_valid, is_ca;
	int max_path_len;      /* -1 when absent */
	bool max_path_len_zero;
	alt_der subject_key_id, authority_key_id;
	alt_x509_strs dns_names, emails, uris, crl_dps, ocsp, issuing_urls;
	struct {
		uint8_t b[16];
		int len;
	} * ips;
	size_t nips;
	int *ext_key_usage;
	size_t neku;
	alt_oid *unknown_eku, *policies, *unhandled_critical;
	size_t nunknown_eku, npolicies, nunhandled_critical;
	/* the policies' OIDs as their DER contents (all of them, as Go's
	   Policies; policies above has n 0 for those that do not fit) */
	alt_der *policy_oids;
	alt_x509_policy_mapping *policy_mappings;
	size_t npolicy_mappings;
	/* policy constraints and inhibitAnyPolicy, as Go's fields: a value
	   counts when it is > 0 or its _zero flag is set */
	int64_t require_explicit_policy, inhibit_policy_mapping, inhibit_any_policy;
	bool require_explicit_policy_zero, inhibit_policy_mapping_zero, inhibit_any_policy_zero;
	/* name constraints */
	bool permitted_dns_critical;
	alt_x509_strs permitted_dns, excluded_dns, permitted_emails, excluded_emails, permitted_uris, excluded_uris;
	alt_x509_ipnet *permitted_ips, *excluded_ips;
	size_t npermitted_ips, nexcluded_ips;
} alt_x509_cert;

/* a private key: PKCS #8, PKCS #1 (RSA) or SEC 1 (EC), as
   crypto/tls's parsePrivateKey tries them; the parts point into der or
   the arena */
typedef struct {
	alt_x509_pk_alg alg;   /* RSA, ECDSA or Ed25519 */
	struct {
		alt_der n, d, p, q, dp, dq, qinv;
		int64_t e;
	} rsa;
	int curve;             /* ECDSA: ALT_X509_P256... */
	uint8_t ec_priv[66], ec_pub[133]; /* scalar of the curve's size, uncompressed point */
	uint8_t ed25519[64];   /* seed and public key */
} alt_x509_private_key;

int alt_x509_parse_private_key(alt_arena *a, const void *der, size_t n, alt_x509_private_key *out, const char **why);

/* 0, or -ALT_EX509 with *why (when not NULL) set to Go's message */
int alt_x509_parse(alt_arena *a, const void *der, size_t n, alt_x509_cert **out, const char **why);

#ifdef __cplusplus
}
#endif

#endif

#ifndef ALT_CRYPTO_X509_VERIFY_H
#define ALT_CRYPTO_X509_VERIFY_H
#ifdef __cplusplus
extern "C" {
#endif

/* a set of certificates (Go's CertPool): roots or intermediates */
typedef struct alt_x509_pool alt_x509_pool;

alt_x509_pool *alt_x509_pool_new(void);
void alt_x509_pool_free(alt_x509_pool *p);
/* adds a DER certificate (copied): 0 or the parse error */
int alt_x509_pool_add_der(alt_x509_pool *p, const void *der, size_t n);
/* adds the CERTIFICATE blocks of PEM text: how many were added */
int alt_x509_pool_add_pem(alt_x509_pool *p, const void *pem, size_t n);
size_t alt_x509_pool_len(const alt_x509_pool *p);
const alt_x509_cert *alt_x509_pool_get(const alt_x509_pool *p, size_t i);
/* the system's roots, as Go finds them on Linux (SSL_CERT_FILE,
   SSL_CERT_DIR, /etc/ssl/certs/ca-certificates.crt...): loaded once,
   NULL when there are none */
const alt_x509_pool *alt_x509_system_roots(void);

typedef struct {
	const char *dns_name;              /* the host name or IP to match, NULL: none */
	const alt_x509_pool *roots;        /* NULL: the system roots */
	const alt_x509_pool *intermediates;
	int64_t now;                       /* Unix seconds, 0: the current time */
	const int *key_usages;             /* ALT_X509_EKU_*, NULL: server auth */
	size_t nkey_usages;
	/* the acceptable policies (Go's CertificatePolicies) as OID
	   contents; none: any policy */
	const alt_der *certificate_policies;
	size_t ncertificate_policies;
} alt_x509_verify_opts;

typedef struct {
	const alt_x509_cert **certs;       /* leaf first, root last */
	size_t n;
} alt_x509_chain;

/* Go's Certificate.Verify: 0 and the valid chains (in a), or
   -ALT_EX509_VERIFY with Go's message in *why. Certificate policies are
   validated as Go does (RFC 5280 6.1 with RFC 9618's policy graph). */
int alt_x509_verify(const alt_x509_cert *leaf, const alt_x509_verify_opts *opts, alt_arena *a, alt_x509_chain **chains,
	size_t *nchains, const char **why);
/* Go's VerifyHostname */
int alt_x509_verify_hostname(const alt_x509_cert *c, const char *host, alt_arena *a, const char **why);
/* Go's CheckSignatureFrom */
int alt_x509_check_signature_from(const alt_x509_cert *c, const alt_x509_cert *parent, const char **why);
/* checks a signature made by the key of c (Go's Certificate.CheckSignature) */
int alt_x509_check_signature(const alt_x509_cert *c, alt_x509_sig_alg alg, const void *signed_data, size_t n,
	const void *sig, size_t nsig, const char **why);

#ifdef __cplusplus
}
#endif
#endif
