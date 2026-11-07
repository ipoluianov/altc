/* Sockets: connections and listeners on poll descriptors (after Go's
   net: fd_unix.go, dial.go, ipsock_posix.go) */
#define _GNU_SOURCE
#include "../runtime/rt.h"
#include "net_internal.h"
#include <alt/errors.h>
#include <alt/net.h>
#include <alt/sync.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

struct alt_conn {
	int fd;
	int family, type;
	alt__pd *pd;
	alt_addr laddr, raddr;
	_Atomic alt_time rdl, wdl;
	_Atomic bool closed;
	alt_waitgroup ops; /* operations in progress */
	_Atomic(alt_ctx *) ctx; /* operations also end when it is done (internal) */
};

struct alt_listener {
	alt_conn c; /* the socket; its addresses */
};

/* ---- operations hold a reference, so that close waits for them ---- */

static bool enter(alt_conn *c) {
	alt_waitgroup_add(&c->ops, 1);
	if (atomic_load(&c->closed)) {
		alt_waitgroup_done(&c->ops);
		return false;
	}
	return true;
}

static void leave(alt_conn *c) { alt_waitgroup_done(&c->ops); }

static int conn_init(alt_conn *c, int fd, int family, int type) {
	memset(c, 0, sizeof *c);
	c->fd = fd;
	c->family = family;
	c->type = type;
	atomic_store(&c->rdl, ALT_FOREVER);
	atomic_store(&c->wdl, ALT_FOREVER);
	c->ops = (alt_waitgroup)ALT_WAITGROUP_INIT;
	atomic_store(&c->ctx, (alt_ctx *)0);
	c->pd = alt__pd_open(fd);
	if (!c->pd)
		return -ENOMEM;
	c->laddr.len = sizeof c->laddr.ss;
	getsockname(fd, (struct sockaddr *)&c->laddr.ss, &c->laddr.len);
	c->raddr.len = sizeof c->raddr.ss;
	if (getpeername(fd, (struct sockaddr *)&c->raddr.ss, &c->raddr.len) < 0)
		c->raddr.len = 0;
	return 0;
}

static void conn_shutdown(alt_conn *c) {
	alt__pd_close(c->pd);
	alt_waitgroup_wait(&c->ops);
	close(c->fd);
	alt__pd_free(c->pd);
}

static bool expired(alt_time t) { return t != ALT_FOREVER && alt_until(t) <= 0; }

/* waits for readiness; -EINTR means "the deadline changed, look again" */
static int wait_io(alt_conn *c, int mode, alt_ctx *ctx) {
	alt_time dl = atomic_load(mode == ALT_POLL_IN ? &c->rdl : &c->wdl);
	if (expired(dl))
		return -ETIMEDOUT;
	if (!ctx)
		ctx = atomic_load(&c->ctx);
	return alt__pd_wait(c->pd, mode, dl, ctx);
}

void alt__conn_set_ctx(alt_conn *c, alt_ctx *ctx) { atomic_store(&c->ctx, ctx); }

/* the error of the connection's context, 0 while it runs */
static int ctx_err(alt_conn *c) { return alt_ctx_err(atomic_load(&c->ctx)); }

ssize_t alt_conn_read(alt_conn *c, void *buf, size_t n) {
	if (!enter(c))
		return -ALT_ECLOSED;
	ssize_t rc;
	for (;;) {
		if (expired(atomic_load(&c->rdl))) {
			rc = -ETIMEDOUT;
			break;
		}
		if ((rc = ctx_err(c)))
			break;
		ssize_t k = read(c->fd, buf, n);
		if (k >= 0) {
			rc = k;
			break;
		}
		if (errno == EINTR)
			continue;
		if (errno != EAGAIN) {
			rc = -errno;
			break;
		}
		int w = wait_io(c, ALT_POLL_IN, 0);
		if (w < 0 && w != -EINTR) {
			rc = w;
			break;
		}
	}
	leave(c);
	return rc;
}

ssize_t alt_conn_write(alt_conn *c, const void *buf, size_t n) {
	if (!enter(c))
		return -ALT_ECLOSED;
	size_t done = 0;
	ssize_t rc = 0;
	while (done < n || n == 0) {
		if (expired(atomic_load(&c->wdl))) {
			rc = -ETIMEDOUT;
			break;
		}
		if ((rc = ctx_err(c)))
			break;
		ssize_t k = send(c->fd, (const char *)buf + done, n - done, MSG_NOSIGNAL);
		if (k >= 0) {
			done += (size_t)k;
			if (n == 0)
				break;
			continue;
		}
		if (errno == EINTR)
			continue;
		if (errno == ENOTSOCK) {
			k = write(c->fd, (const char *)buf + done, n - done);
			if (k >= 0) {
				done += (size_t)k;
				continue;
			}
		}
		if (errno != EAGAIN) {
			rc = -errno;
			break;
		}
		int w = wait_io(c, ALT_POLL_OUT, 0);
		if (w < 0 && w != -EINTR) {
			rc = w;
			break;
		}
	}
	leave(c);
	return rc < 0 ? rc : (ssize_t)done;
}

ssize_t alt_conn_read_from(alt_conn *c, void *buf, size_t n, alt_addr *from) {
	if (!enter(c))
		return -ALT_ECLOSED;
	ssize_t rc;
	for (;;) {
		if (expired(atomic_load(&c->rdl))) {
			rc = -ETIMEDOUT;
			break;
		}
		alt_addr a;
		a.len = sizeof a.ss;
		ssize_t k = recvfrom(c->fd, buf, n, 0, (struct sockaddr *)&a.ss, &a.len);
		if (k >= 0) {
			if (from)
				*from = a;
			rc = k;
			break;
		}
		if (errno == EINTR)
			continue;
		if (errno != EAGAIN) {
			rc = -errno;
			break;
		}
		int w = wait_io(c, ALT_POLL_IN, 0);
		if (w < 0 && w != -EINTR) {
			rc = w;
			break;
		}
	}
	leave(c);
	return rc;
}

ssize_t alt_conn_write_to(alt_conn *c, const void *buf, size_t n, const alt_addr *to) {
	if (!enter(c))
		return -ALT_ECLOSED;
	ssize_t rc;
	for (;;) {
		if (expired(atomic_load(&c->wdl))) {
			rc = -ETIMEDOUT;
			break;
		}
		ssize_t k = sendto(c->fd, buf, n, MSG_NOSIGNAL, to ? (const struct sockaddr *)&to->ss : 0, to ? to->len : 0);
		if (k >= 0) {
			rc = k;
			break;
		}
		if (errno == EINTR)
			continue;
		if (errno != EAGAIN) {
			rc = -errno;
			break;
		}
		int w = wait_io(c, ALT_POLL_OUT, 0);
		if (w < 0 && w != -EINTR) {
			rc = w;
			break;
		}
	}
	leave(c);
	return rc;
}

int alt_conn_close(alt_conn *c) {
	if (!c)
		return -EINVAL;
	if (atomic_exchange(&c->closed, true))
		return -ALT_ECLOSED;
	conn_shutdown(c);
	free(c);
	return 0;
}

int alt_conn_close_write(alt_conn *c) {
	if (!enter(c))
		return -ALT_ECLOSED;
	int rc = shutdown(c->fd, SHUT_WR) < 0 ? -errno : 0;
	leave(c);
	return rc;
}

int alt_conn_set_read_deadline(alt_conn *c, alt_time t) {
	atomic_store(&c->rdl, t);
	alt__pd_interrupt(c->pd, ALT_POLL_IN, -EINTR);
	return 0;
}

int alt_conn_set_write_deadline(alt_conn *c, alt_time t) {
	atomic_store(&c->wdl, t);
	alt__pd_interrupt(c->pd, ALT_POLL_OUT, -EINTR);
	return 0;
}

int alt_conn_set_deadline(alt_conn *c, alt_time t) {
	alt_conn_set_read_deadline(c, t);
	return alt_conn_set_write_deadline(c, t);
}

const alt_addr *alt_conn_local_addr(const alt_conn *c) { return &c->laddr; }
const alt_addr *alt_conn_remote_addr(const alt_conn *c) { return &c->raddr; }
int alt_conn_fd(const alt_conn *c) { return c->fd; }

int alt_conn_set_nodelay(alt_conn *c, bool on) {
	int v = on;
	return setsockopt(c->fd, IPPROTO_TCP, TCP_NODELAY, &v, sizeof v) < 0 ? -errno : 0;
}

int alt_conn_set_keepalive(alt_conn *c, alt_dur period) {
	int on = period > 0;
	if (setsockopt(c->fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on) < 0)
		return -errno;
	if (!on)
		return 0;
	int secs = (int)((period + ALT_SEC - 1) / ALT_SEC), cnt = 9;
	setsockopt(c->fd, IPPROTO_TCP, TCP_KEEPIDLE, &secs, sizeof secs);
	setsockopt(c->fd, IPPROTO_TCP, TCP_KEEPINTVL, &secs, sizeof secs);
	setsockopt(c->fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof cnt);
	return 0;
}

static ssize_t conn_read(void *obj, void *buf, size_t n) { return alt_conn_read(obj, buf, n); }
static ssize_t conn_write(void *obj, const void *buf, size_t n) { return alt_conn_write(obj, buf, n); }
alt_reader alt_conn_reader(alt_conn *c) { return (alt_reader){conn_read, c}; }
alt_writer alt_conn_writer(alt_conn *c) { return (alt_writer){conn_write, c}; }

/* ---- networks ---- */

int alt__net_parse(const char *network, alt__network *n) {
	static const struct {
		const char *name;
		int family, type;
		bool ip;
	} nets[] = {
		{"tcp", AF_UNSPEC, SOCK_STREAM, true}, {"tcp4", AF_INET, SOCK_STREAM, true}, {"tcp6", AF_INET6, SOCK_STREAM, true},
		{"udp", AF_UNSPEC, SOCK_DGRAM, true}, {"udp4", AF_INET, SOCK_DGRAM, true}, {"udp6", AF_INET6, SOCK_DGRAM, true},
		{"unix", AF_UNIX, SOCK_STREAM, false}, {"unixgram", AF_UNIX, SOCK_DGRAM, false},
		{"unixpacket", AF_UNIX, SOCK_SEQPACKET, false},
	};
	for (size_t i = 0; i < sizeof nets / sizeof *nets; i++)
		if (strcmp(network, nets[i].name) == 0) {
			n->family = nets[i].family;
			n->type = nets[i].type;
			n->ip = nets[i].ip;
			return 0;
		}
	return -ALT_ENET_NETWORK;
}

static int unix_addr(const char *path, alt_addr *a) {
	memset(a, 0, sizeof *a);
	struct sockaddr_un *u = (struct sockaddr_un *)&a->ss;
	size_t n = strlen(path);
	if (n == 0 || n >= sizeof u->sun_path)
		return -ALT_ENET_ADDR;
	u->sun_family = AF_UNIX;
	memcpy(u->sun_path, path, n);
	if (path[0] == '@')
		u->sun_path[0] = 0; /* the abstract namespace */
	a->len = (socklen_t)(offsetof(struct sockaddr_un, sun_path) + n + (path[0] == '@' ? 0 : 1));
	return 0;
}

static int new_socket(int family, int type) {
	int fd = socket(family, type | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	return fd < 0 ? -errno : fd;
}

static void tcp_defaults(int fd) {
	int one = 1, secs = 15, cnt = 9;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof one);
	setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &secs, sizeof secs);
	setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &secs, sizeof secs);
	setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof cnt);
}

int alt__net_dial_addr(alt_conn **out, int type, const alt_addr *a, alt_time deadline, alt_ctx *ctx) {
	int family = a->ss.ss_family;
	int fd = new_socket(family, type);
	if (fd < 0)
		return fd;
	alt_conn *c = malloc(sizeof *c);
	if (!c) {
		close(fd);
		return -ENOMEM;
	}
	int rc = conn_init(c, fd, family, type);
	if (rc) {
		free(c);
		close(fd);
		return rc;
	}
	if (connect(fd, (const struct sockaddr *)&a->ss, a->len) < 0) {
		if (errno != EINPROGRESS && errno != EINTR) {
			rc = -errno;
			goto fail;
		}
		for (;;) {
			rc = alt__pd_wait(c->pd, ALT_POLL_OUT, deadline, ctx);
			if (rc < 0)
				goto fail;
			int err = 0;
			socklen_t len = sizeof err;
			if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0)
				err = errno;
			if (err == 0)
				break;
			if (err != EINPROGRESS && err != EALREADY && err != EINTR) {
				rc = -err;
				goto fail;
			}
		}
	}
	if (family != AF_UNIX && type == SOCK_STREAM)
		tcp_defaults(fd);
	c->laddr.len = sizeof c->laddr.ss;
	getsockname(fd, (struct sockaddr *)&c->laddr.ss, &c->laddr.len);
	c->raddr = *a;
	*out = c;
	return 0;
fail:
	atomic_store(&c->closed, true);
	conn_shutdown(c);
	free(c);
	return rc;
}

/* the deadline for one of n remaining attempts (Go's partialDeadline):
   the time left shared among them, at least 2 s each when possible */
static alt_time partial_deadline(alt_time deadline, int n) {
	if (deadline == ALT_FOREVER)
		return deadline;
	alt_dur left = alt_until(deadline);
	if (left <= 0)
		return deadline;
	alt_dur each = left / n;
	if (each < 2 * ALT_SEC)
		each = left < 2 * ALT_SEC ? left : 2 * ALT_SEC;
	return alt_deadline(each);
}

int alt_dial(alt_conn **out, const char *network, const char *address, alt_ctx *ctx) {
	alt__network nw;
	int rc = alt__net_parse(network, &nw);
	if (rc)
		return rc;
	if (!nw.ip) {
		alt_addr a;
		if ((rc = unix_addr(address, &a)))
			return rc;
		return alt__net_dial_addr(out, nw.type, &a, alt_ctx_deadline(ctx), ctx);
	}
	alt_str host, port;
	if ((rc = alt_split_host_port(alt_str_cstr(address), &host, &port)))
		return rc;
	uint16_t pnum;
	if ((rc = alt_lookup_port(network, port, &pnum)))
		return rc;
	alt_ip ips[16];
	int n;
	char hbuf[256];
	if (host.n >= sizeof hbuf)
		return -ALT_ENET_ADDR;
	if (host.n == 0) {
		/* the local system */
		ips[0] = nw.family == AF_INET6 ? (alt_ip){{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, false, true, ""}
					       : alt_ip_v4(127, 0, 0, 1);
		n = 1;
	} else {
		memcpy(hbuf, host.p, host.n);
		hbuf[host.n] = 0;
		n = alt_lookup_host(ctx, hbuf, ips, 16);
		if (n < 0)
			return n;
	}
	/* the addresses of the network's family, those of the first address's
	   family first (Go dials primaries, then fallbacks) */
	alt_addr cand[16];
	int nc = 0;
	for (int pass = 0; pass < 2; pass++)
		for (int i = 0; i < n; i++) {
			bool v4 = ips[i].is4;
			if ((nw.family == AF_INET && !v4) || (nw.family == AF_INET6 && v4))
				continue;
			bool primary = n && v4 == ips[0].is4;
			if ((pass == 0) == primary)
				cand[nc++] = alt_addr_from_ip(&ips[i], pnum);
		}
	if (nc == 0)
		return -ALT_ENET_NOHOST;
	alt_time deadline = alt_ctx_deadline(ctx);
	rc = -ECONNREFUSED;
	for (int i = 0; i < nc; i++) {
		int e = alt_ctx_err(ctx);
		if (e)
			return e;
		rc = alt__net_dial_addr(out, nw.type, &cand[i], partial_deadline(deadline, nc - i), ctx);
		if (rc == 0 || rc == -ECANCELED)
			return rc;
	}
	return rc;
}

int alt_dial_timeout(alt_conn **out, const char *network, const char *address, alt_dur timeout) {
	alt_ctx *ctx;
	int rc = alt_ctx_with_timeout(&ctx, 0, timeout);
	if (rc)
		return rc;
	rc = alt_dial(out, network, address, ctx);
	alt_ctx_free(ctx);
	return rc;
}

/* ---- listening ---- */

static int listen_backlog(void) {
	static int backlog;
	if (backlog)
		return backlog;
	int v = 4096;
	FILE *f = fopen("/proc/sys/net/core/somaxconn", "re");
	if (f) {
		if (fscanf(f, "%d", &v) != 1 || v <= 0)
			v = 4096;
		fclose(f);
	}
	if (v > 65535)
		v = 65535;
	return backlog = v;
}

/* the address to bind for "host:port" */
static int bind_addr(const alt__network *nw, const char *network, const char *address, alt_addr *a, bool *dual) {
	*dual = false;
	if (!nw->ip)
		return unix_addr(address, a);
	alt_str host, port;
	int rc;
	if ((rc = alt_split_host_port(alt_str_cstr(address), &host, &port)))
		return rc;
	uint16_t pnum;
	if ((rc = alt_lookup_port(network, port, &pnum)))
		return rc;
	alt_ip ip;
	if (host.n == 0) {
		if (nw->family == AF_INET)
			ip = alt_ip_v4(0, 0, 0, 0);
		else {
			memset(&ip, 0, sizeof ip);
			ip.valid = true;
			*dual = nw->family == AF_UNSPEC;
		}
	} else if (alt_ip_parse(host, &ip) < 0) {
		char hbuf[256];
		if (host.n >= sizeof hbuf)
			return -ALT_ENET_ADDR;
		memcpy(hbuf, host.p, host.n);
		hbuf[host.n] = 0;
		alt_ip ips[8];
		int n = alt_lookup_host(0, hbuf, ips, 8);
		if (n < 0)
			return n;
		int k = 0;
		while (k < n && ((nw->family == AF_INET && !ips[k].is4) || (nw->family == AF_INET6 && ips[k].is4)))
			k++;
		if (k == n)
			return -ALT_ENET_NOHOST;
		ip = ips[k];
	}
	if ((nw->family == AF_INET && !ip.is4) || (nw->family == AF_INET6 && ip.is4))
		return -ALT_ENET_ADDR;
	*a = alt_addr_from_ip(&ip, pnum);
	return 0;
}

static int bound_socket(const alt__network *nw, const char *network, const char *address, int *fdp) {
	alt_addr a;
	bool dual;
	int rc = bind_addr(nw, network, address, &a, &dual);
	if (rc)
		return rc;
	int fd = new_socket(a.ss.ss_family, nw->type);
	if (fd == -EAFNOSUPPORT && dual) {
		/* no IPv6: all IPv4 addresses */
		alt_ip any = alt_ip_v4(0, 0, 0, 0);
		uint16_t port = ntohs(((struct sockaddr_in6 *)&a.ss)->sin6_port);
		a = alt_addr_from_ip(&any, port);
		dual = false;
		fd = new_socket(AF_INET, nw->type);
	}
	if (fd < 0)
		return fd;
	int one = 1, zero = 0;
	if (nw->ip && nw->type == SOCK_STREAM)
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
	if (nw->ip && nw->type == SOCK_DGRAM)
		setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &one, sizeof one);
	if (a.ss.ss_family == AF_INET6)
		setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, dual ? &zero : &one, sizeof one);
	if (bind(fd, (struct sockaddr *)&a.ss, a.len) < 0) {
		rc = -errno;
		close(fd);
		return rc;
	}
	*fdp = fd;
	return 0;
}

int alt_listen(alt_listener **out, const char *network, const char *address) {
	alt__network nw;
	int rc = alt__net_parse(network, &nw);
	if (rc)
		return rc;
	if (nw.type == SOCK_DGRAM)
		return -ALT_ENET_NETWORK;
	int fd;
	if ((rc = bound_socket(&nw, network, address, &fd)))
		return rc;
	if (listen(fd, listen_backlog()) < 0) {
		rc = -errno;
		close(fd);
		return rc;
	}
	alt_listener *l = malloc(sizeof *l);
	if (!l) {
		close(fd);
		return -ENOMEM;
	}
	if ((rc = conn_init(&l->c, fd, nw.ip ? AF_INET6 : AF_UNIX, nw.type))) {
		close(fd);
		free(l);
		return rc;
	}
	l->c.family = l->c.laddr.ss.ss_family;
	*out = l;
	return 0;
}

int alt_listen_packet(alt_conn **out, const char *network, const char *address) {
	alt__network nw;
	int rc = alt__net_parse(network, &nw);
	if (rc)
		return rc;
	if (nw.type != SOCK_DGRAM)
		return -ALT_ENET_NETWORK;
	int fd;
	if ((rc = bound_socket(&nw, network, address, &fd)))
		return rc;
	alt_conn *c = malloc(sizeof *c);
	if (!c) {
		close(fd);
		return -ENOMEM;
	}
	if ((rc = conn_init(c, fd, AF_UNSPEC, nw.type))) {
		close(fd);
		free(c);
		return rc;
	}
	c->family = c->laddr.ss.ss_family;
	*out = c;
	return 0;
}

int alt_accept(alt_listener *l, alt_conn **out, alt_ctx *ctx) {
	alt_conn *lc = &l->c;
	if (!enter(lc))
		return -ALT_ECLOSED;
	int rc;
	for (;;) {
		alt_addr a;
		a.len = sizeof a.ss;
		int fd = accept4(lc->fd, (struct sockaddr *)&a.ss, &a.len, SOCK_NONBLOCK | SOCK_CLOEXEC);
		if (fd >= 0) {
			alt_conn *c = malloc(sizeof *c);
			if (!c) {
				close(fd);
				rc = -ENOMEM;
				break;
			}
			if ((rc = conn_init(c, fd, a.ss.ss_family, SOCK_STREAM))) {
				close(fd);
				free(c);
				break;
			}
			if (a.ss.ss_family != AF_UNIX)
				tcp_defaults(fd);
			*out = c;
			rc = 0;
			break;
		}
		if (errno == EINTR || errno == ECONNABORTED)
			continue;
		if (errno != EAGAIN) {
			rc = -errno;
			break;
		}
		int w = wait_io(lc, ALT_POLL_IN, ctx);
		if (w < 0 && w != -EINTR) {
			rc = w;
			break;
		}
	}
	leave(lc);
	return rc;
}

int alt_listener_close(alt_listener *l) {
	if (!l)
		return -EINVAL;
	if (atomic_exchange(&l->c.closed, true))
		return -ALT_ECLOSED;
	conn_shutdown(&l->c);
	if (l->c.family == AF_UNIX) {
		/* Go removes the socket file of a listener it created */
		struct sockaddr_un *u = (struct sockaddr_un *)&l->c.laddr.ss;
		if (u->sun_path[0])
			unlink(u->sun_path);
	}
	free(l);
	return 0;
}

const alt_addr *alt_listener_addr(const alt_listener *l) { return &l->c.laddr; }
int alt_listener_fd(const alt_listener *l) { return l->c.fd; }

static int nonblocking(int fd) {
	int fl = fcntl(fd, F_GETFL);
	if (fl < 0 || fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0)
		return -errno;
	return 0;
}

int alt_listener_from_fd(alt_listener **out, int fd) {
	int rc = nonblocking(fd);
	if (rc)
		return rc;
	alt_listener *l = malloc(sizeof *l);
	if (!l)
		return -ENOMEM;
	if ((rc = conn_init(&l->c, fd, 0, SOCK_STREAM))) {
		free(l);
		return rc;
	}
	l->c.family = l->c.laddr.ss.ss_family;
	*out = l;
	return 0;
}

int alt_conn_from_fd(alt_conn **out, int fd) {
	int rc = nonblocking(fd);
	if (rc)
		return rc;
	int type = SOCK_STREAM;
	socklen_t len = sizeof type;
	getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &len);
	alt_conn *c = malloc(sizeof *c);
	if (!c)
		return -ENOMEM;
	if ((rc = conn_init(c, fd, 0, type))) {
		free(c);
		return rc;
	}
	c->family = c->laddr.ss.ss_family;
	*out = c;
	return 0;
}
