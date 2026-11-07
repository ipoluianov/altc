/* AltC standard library: networking (Go's net, net/netip).

   Connections and listeners are TCP ("tcp", "tcp4", "tcp6"), UDP ("udp",
   "udp4", "udp6") or Unix domain ("unix", "unixgram"). Their operations
   block the calling task (other tasks of its thread keep running) or
   thread. Addresses are "host:port" (host a name, an IPv4 address or an
   IPv6 address in brackets; an empty host is the unspecified address for
   listening and the local host for dialing) or, for Unix sockets, a path
   (a leading '@' is the abstract namespace).

   Names are resolved by AltC's own resolver: /etc/hosts first, then the
   name servers of /etc/resolv.conf (UDP, TCP for truncated answers, the
   search domains and the ndots, timeout and attempts options). The
   resolver waits in the poller: it does not block the thread.

   alt_conn_close wakes every operation still waiting on the connection
   (they return -ALT_ECLOSED), waits until they have returned, closes the
   descriptor and frees the connection; listeners alike. */
#ifndef ALT_NET_H
#define ALT_NET_H

#include <alt/errors.h>
#include <alt/context.h>
#include <alt/io.h>
#include <alt/str.h>
#include <alt/time.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- IP addresses (net/netip.Addr) ---- */

typedef struct {
	uint8_t b[16];  /* IPv4 as ::ffff:a.b.c.d */
	bool is4;       /* an IPv4 address */
	bool valid;
	char zone[16];  /* IPv6 scope ("eth0", "3"), "" for none */
} alt_ip;

/* "1.2.3.4", "::1", "::ffff:1.2.3.4", "fe80::1%eth0", as netip.ParseAddr:
   0 or -ALT_ENET_ADDR */
int alt_ip_parse(alt_str s, alt_ip *ip);
/* formats as Go's netip does (RFC 5952 for IPv6) */
int alt_ip_format(const alt_ip *ip, alt_buf *b);
alt_ip alt_ip_v4(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
bool alt_ip_equal(const alt_ip *a, const alt_ip *b);
bool alt_ip_is_loopback(const alt_ip *ip);
bool alt_ip_is_unspecified(const alt_ip *ip);
bool alt_ip_is_private(const alt_ip *ip);

/* ---- socket addresses ---- */

typedef struct {
	struct sockaddr_storage ss;
	socklen_t len;
} alt_addr;

/* "1.2.3.4:80", "[::1]:80", "/tmp/sock" */
int alt_addr_format(const alt_addr *a, alt_buf *b);
/* an address from an IP and a port */
alt_addr alt_addr_from_ip(const alt_ip *ip, uint16_t port);
/* the IP and port of an IP address; false for other families */
bool alt_addr_ip(const alt_addr *a, alt_ip *ip, uint16_t *port);

/* splits "host:port", "[ipv6]:port" (net.SplitHostPort) */
int alt_split_host_port(alt_str hostport, alt_str *host, alt_str *port);
/* "host:port", brackets around hosts with ':' */
int alt_join_host_port(alt_buf *b, alt_str host, alt_str port);
/* a numeric port or a service name ("http"): 0 or -ALT_ENET_ADDR */
int alt_lookup_port(const char *network, alt_str service, uint16_t *port);

/* ---- resolution ---- */

/* the addresses of host (IPv4 and IPv6, in the order to try them; at most
   max): the count, or -ALT_ENET_NOHOST, -ALT_ENET_DNS, -ETIMEDOUT... A
   numeric host is returned as is. */
int alt_lookup_host(alt_ctx *ctx, const char *host, alt_ip *out, int max);

/* a resolver with explicit settings (tests, custom servers) */
typedef struct {
	alt_addr servers[8];
	int nservers;
	char search[6][256];
	int nsearch;
	int ndots;          /* default 1 */
	alt_dur timeout;    /* per query; default 5 s */
	int attempts;       /* default 2 */
	const char *hosts;  /* the hosts file; NULL: /etc/hosts, "" none */
	bool rotate;
	bool prefer_go_order; /* reserved */
} alt_resolver;

/* the settings of /etc/resolv.conf (path NULL) */
int alt_resolver_load(alt_resolver *r, const char *path);
int alt_resolver_lookup(const alt_resolver *r, alt_ctx *ctx, const char *host, alt_ip *out, int max);

/* ---- connections ---- */

typedef struct alt_conn alt_conn;
typedef struct alt_listener alt_listener;

/* connects (for "udp" networks: sets the default destination) */
int alt_dial(alt_conn **out, const char *network, const char *address, alt_ctx *ctx);
int alt_dial_timeout(alt_conn **out, const char *network, const char *address, alt_dur timeout);

/* the number of bytes read, 0 at the end of the stream */
ssize_t alt_conn_read(alt_conn *c, void *buf, size_t n);
/* writes everything: n or an error */
ssize_t alt_conn_write(alt_conn *c, const void *buf, size_t n);
int alt_conn_close(alt_conn *c);
/* shuts the sending side down (the peer reads the end of the stream) */
int alt_conn_close_write(alt_conn *c);

/* deadlines of the operations (ALT_FOREVER: none); changing one
   affects operations already waiting */
int alt_conn_set_deadline(alt_conn *c, alt_time t);
int alt_conn_set_read_deadline(alt_conn *c, alt_time t);
int alt_conn_set_write_deadline(alt_conn *c, alt_time t);

const alt_addr *alt_conn_local_addr(const alt_conn *c);
const alt_addr *alt_conn_remote_addr(const alt_conn *c);
int alt_conn_fd(const alt_conn *c);
int alt_conn_set_nodelay(alt_conn *c, bool on);
int alt_conn_set_keepalive(alt_conn *c, alt_dur period); /* 0 disables */
alt_reader alt_conn_reader(alt_conn *c);
alt_writer alt_conn_writer(alt_conn *c);

/* datagrams: an unconnected UDP or unixgram socket from alt_listen_packet */
int alt_listen_packet(alt_conn **out, const char *network, const char *address);
ssize_t alt_conn_read_from(alt_conn *c, void *buf, size_t n, alt_addr *from);
ssize_t alt_conn_write_to(alt_conn *c, const void *buf, size_t n, const alt_addr *to);

/* ---- listeners ---- */

/* "tcp" ":8080" listens on all addresses (IPv6 and IPv4), "127.0.0.1:0"
   picks a free port; "unix" "/path" (an existing socket file is not
   removed) */
int alt_listen(alt_listener **out, const char *network, const char *address);
int alt_accept(alt_listener *l, alt_conn **out, alt_ctx *ctx);
int alt_listener_close(alt_listener *l);
const alt_addr *alt_listener_addr(const alt_listener *l);
int alt_listener_fd(const alt_listener *l);
/* wraps an already listening (or connected) socket */
int alt_listener_from_fd(alt_listener **out, int fd);
int alt_conn_from_fd(alt_conn **out, int fd);

#ifdef __cplusplus
}
#endif

#endif
