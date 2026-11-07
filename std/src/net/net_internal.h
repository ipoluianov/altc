/* The net package's internal interface */
#ifndef ALT_NET_INTERNAL_H
#define ALT_NET_INTERNAL_H

#include <alt/net.h>

typedef struct {
	int family; /* AF_UNSPEC, AF_INET, AF_INET6, AF_UNIX */
	int type;   /* SOCK_STREAM, SOCK_DGRAM, SOCK_SEQPACKET */
	bool ip;
} alt__network;

int alt__net_parse(const char *network, alt__network *n);
/* connects to one address */
int alt__net_dial_addr(alt_conn **out, int type, const alt_addr *a, alt_time deadline, alt_ctx *ctx);

/* operations of c also end when ctx is done (NULL: none) */
void alt__conn_set_ctx(alt_conn *c, alt_ctx *ctx);

/* DNS messages (dns.c) */
enum { ALT__DNS_A = 1, ALT__DNS_CNAME = 5, ALT__DNS_AAAA = 28 };

/* builds a query: the length, or <0 */
int alt__dns_query(uint8_t *buf, size_t cap, uint16_t id, const char *name, int qtype);
/* parses an answer to the query (id, name, type): addresses appended to
   out (n so far, max), rcode in *rcode; the number of addresses or <0 */
int alt__dns_answer(const uint8_t *msg, size_t len, uint16_t id, const char *name, int qtype, alt_ip *out, int n, int max,
	int *rcode, bool *truncated);

#endif
