/* AltC <sys/socket.h> - sockets (glibc-compatible, x86-64 Linux).  */

#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H 1

#include <features.h>
#include <_altc/types.h>
#include <_altc/iovec.h>
#include <sys/types.h>

#define __need_size_t
#include <stddef.h>

#ifndef __ssize_t_defined
typedef __ssize_t ssize_t;
# define __ssize_t_defined
#endif
#ifndef __socklen_t_defined
typedef __socklen_t socklen_t;
# define __socklen_t_defined
#endif
#ifndef __pid_t_defined
typedef __pid_t pid_t;
# define __pid_t_defined
#endif
#ifndef __uid_t_defined
typedef __uid_t uid_t;
# define __uid_t_defined
#endif
#ifndef __gid_t_defined
typedef __gid_t gid_t;
# define __gid_t_defined
#endif

__BEGIN_DECLS

/* Socket types, protocol families, levels, message flags
   (<bits/socket.h>, <bits/socket_type.h>).  */
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define SOCK_RAW 3
#define SOCK_RDM 4
#define SOCK_SEQPACKET 5
#define SOCK_DCCP 6
#define SOCK_PACKET 10
#define SOCK_CLOEXEC 0x80000
#define SOCK_NONBLOCK 2048
#define PF_UNSPEC 0
#define PF_LOCAL 1
#define PF_UNIX PF_LOCAL
#define PF_FILE PF_LOCAL
#define PF_INET 2
#define PF_AX25 3
#define PF_IPX 4
#define PF_APPLETALK 5
#define PF_NETROM 6
#define PF_BRIDGE 7
#define PF_ATMPVC 8
#define PF_X25 9
#define PF_INET6 10
#define PF_ROSE 11
#define PF_DECnet 12
#define PF_NETBEUI 13
#define PF_SECURITY 14
#define PF_KEY 15
#define PF_NETLINK 16
#define PF_ROUTE PF_NETLINK
#define PF_PACKET 17
#define PF_ASH 18
#define PF_ECONET 19
#define PF_ATMSVC 20
#define PF_RDS 21
#define PF_SNA 22
#define PF_IRDA 23
#define PF_PPPOX 24
#define PF_WANPIPE 25
#define PF_LLC 26
#define PF_IB 27
#define PF_MPLS 28
#define PF_CAN 29
#define PF_TIPC 30
#define PF_BLUETOOTH 31
#define PF_IUCV 32
#define PF_RXRPC 33
#define PF_ISDN 34
#define PF_PHONET 35
#define PF_IEEE802154 36
#define PF_CAIF 37
#define PF_ALG 38
#define PF_NFC 39
#define PF_VSOCK 40
#define PF_KCM 41
#define PF_QIPCRTR 42
#define PF_SMC 43
#define PF_XDP 44
#define PF_MCTP 45
#define PF_MAX 46
#define AF_UNSPEC PF_UNSPEC
#define AF_LOCAL PF_LOCAL
#define AF_UNIX PF_UNIX
#define AF_FILE PF_FILE
#define AF_INET PF_INET
#define AF_AX25 PF_AX25
#define AF_IPX PF_IPX
#define AF_APPLETALK PF_APPLETALK
#define AF_NETROM PF_NETROM
#define AF_BRIDGE PF_BRIDGE
#define AF_ATMPVC PF_ATMPVC
#define AF_X25 PF_X25
#define AF_INET6 PF_INET6
#define AF_ROSE PF_ROSE
#define AF_DECnet PF_DECnet
#define AF_NETBEUI PF_NETBEUI
#define AF_SECURITY PF_SECURITY
#define AF_KEY PF_KEY
#define AF_NETLINK PF_NETLINK
#define AF_ROUTE PF_ROUTE
#define AF_PACKET PF_PACKET
#define AF_ASH PF_ASH
#define AF_ECONET PF_ECONET
#define AF_ATMSVC PF_ATMSVC
#define AF_RDS PF_RDS
#define AF_SNA PF_SNA
#define AF_IRDA PF_IRDA
#define AF_PPPOX PF_PPPOX
#define AF_WANPIPE PF_WANPIPE
#define AF_LLC PF_LLC
#define AF_IB PF_IB
#define AF_MPLS PF_MPLS
#define AF_CAN PF_CAN
#define AF_TIPC PF_TIPC
#define AF_BLUETOOTH PF_BLUETOOTH
#define AF_IUCV PF_IUCV
#define AF_RXRPC PF_RXRPC
#define AF_ISDN PF_ISDN
#define AF_PHONET PF_PHONET
#define AF_IEEE802154 PF_IEEE802154
#define AF_CAIF PF_CAIF
#define AF_ALG PF_ALG
#define AF_NFC PF_NFC
#define AF_VSOCK PF_VSOCK
#define AF_KCM PF_KCM
#define AF_QIPCRTR PF_QIPCRTR
#define AF_SMC PF_SMC
#define AF_XDP PF_XDP
#define AF_MCTP PF_MCTP
#define AF_MAX PF_MAX
#define SOL_RAW 255
#define SOL_DECNET 261
#define SOL_X25 262
#define SOL_PACKET 263
#define SOL_ATM 264
#define SOL_AAL 265
#define SOL_IRDA 266
#define SOL_NETBEUI 267
#define SOL_LLC 268
#define SOL_DCCP 269
#define SOL_NETLINK 270
#define SOL_TIPC 271
#define SOL_RXRPC 272
#define SOL_PPPOL2TP 273
#define SOL_BLUETOOTH 274
#define SOL_PNPIPE 275
#define SOL_RDS 276
#define SOL_IUCV 277
#define SOL_CAIF 278
#define SOL_ALG 279
#define SOL_NFC 280
#define SOL_KCM 281
#define SOL_TLS 282
#define SOL_XDP 283
#define SOL_MPTCP 284
#define SOL_MCTP 285
#define SOL_SMC 286
#define SOL_VSOCK 287
#define SOMAXCONN 4096
#define MSG_OOB 1
#define MSG_PEEK 2
#define MSG_DONTROUTE 4
#ifdef __USE_GNU
# define MSG_TRYHARD MSG_DONTROUTE
#endif
#define MSG_CTRUNC 8
#define MSG_PROXY 16
#define MSG_TRUNC 32
#define MSG_DONTWAIT 64
#define MSG_EOR 128
#define MSG_WAITALL 256
#define MSG_FIN 512
#define MSG_SYN 1024
#define MSG_CONFIRM 2048
#define MSG_RST 0x1000
#define MSG_ERRQUEUE 0x2000
#define MSG_NOSIGNAL 0x4000
#define MSG_MORE 0x8000
#define MSG_WAITFORONE 0x10000
#define MSG_BATCH 0x40000
#define MSG_SOCK_DEVMEM 0x2000000
#define MSG_ZEROCOPY 0x4000000
#define MSG_FASTOPEN 0x20000000
#define MSG_CMSG_CLOEXEC 0x40000000
#define SOL_SOCKET 1
#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2

/* Socket options (<asm/socket.h>, x86-64 values; the *_OLD variants are
   the 64-bit time_t ones).  */
#ifndef __ASM_GENERIC_SOCKET_H
#define __ASM_GENERIC_SOCKET_H 1
#define SOL_SOCKET 1
#define SO_DEBUG 1
#define SO_REUSEADDR 2
#define SO_TYPE 3
#define SO_ERROR 4
#define SO_DONTROUTE 5
#define SO_BROADCAST 6
#define SO_SNDBUF 7
#define SO_RCVBUF 8
#define SO_SNDBUFFORCE 32
#define SO_RCVBUFFORCE 33
#define SO_KEEPALIVE 9
#define SO_OOBINLINE 10
#define SO_NO_CHECK 11
#define SO_PRIORITY 12
#define SO_LINGER 13
#define SO_BSDCOMPAT 14
#define SO_REUSEPORT 15
#define SO_PASSCRED 16
#define SO_PEERCRED 17
#define SO_RCVLOWAT 18
#define SO_SNDLOWAT 19
#define SO_RCVTIMEO_OLD 20
#define SO_SNDTIMEO_OLD 21
#define SO_SECURITY_AUTHENTICATION 22
#define SO_SECURITY_ENCRYPTION_TRANSPORT 23
#define SO_SECURITY_ENCRYPTION_NETWORK 24
#define SO_BINDTODEVICE 25
#define SO_ATTACH_FILTER 26
#define SO_DETACH_FILTER 27
#define SO_GET_FILTER SO_ATTACH_FILTER
#define SO_PEERNAME 28
#define SO_ACCEPTCONN 30
#define SO_PEERSEC 31
#define SO_PASSSEC 34
#define SO_MARK 36
#define SO_PROTOCOL 38
#define SO_DOMAIN 39
#define SO_RXQ_OVFL 40
#define SO_WIFI_STATUS 41
#define SCM_WIFI_STATUS SO_WIFI_STATUS
#define SO_PEEK_OFF 42
#define SO_NOFCS 43
#define SO_LOCK_FILTER 44
#define SO_SELECT_ERR_QUEUE 45
#define SO_BUSY_POLL 46
#define SO_MAX_PACING_RATE 47
#define SO_BPF_EXTENSIONS 48
#define SO_INCOMING_CPU 49
#define SO_ATTACH_BPF 50
#define SO_DETACH_BPF SO_DETACH_FILTER
#define SO_ATTACH_REUSEPORT_CBPF 51
#define SO_ATTACH_REUSEPORT_EBPF 52
#define SO_CNX_ADVICE 53
#define SCM_TIMESTAMPING_OPT_STATS 54
#define SO_MEMINFO 55
#define SO_INCOMING_NAPI_ID 56
#define SO_COOKIE 57
#define SCM_TIMESTAMPING_PKTINFO 58
#define SO_PEERGROUPS 59
#define SO_ZEROCOPY 60
#define SO_TXTIME 61
#define SCM_TXTIME SO_TXTIME
#define SO_BINDTOIFINDEX 62
#define SO_TIMESTAMP_OLD 29
#define SO_TIMESTAMPNS_OLD 35
#define SO_TIMESTAMPING_OLD 37
#define SO_TIMESTAMP_NEW 63
#define SO_TIMESTAMPNS_NEW 64
#define SO_TIMESTAMPING_NEW 65
#define SO_RCVTIMEO_NEW 66
#define SO_SNDTIMEO_NEW 67
#define SO_DETACH_REUSEPORT_BPF 68
#define SO_PREFER_BUSY_POLL 69
#define SO_BUSY_POLL_BUDGET 70
#define SO_NETNS_COOKIE 71
#define SO_BUF_LOCK 72
#define SO_RESERVE_MEM 73
#define SO_TXREHASH 74
#define SO_RCVMARK 75
#define SO_PASSPIDFD 76
#define SO_PEERPIDFD 77
#define SO_DEVMEM_LINEAR 78
#define SCM_DEVMEM_LINEAR SO_DEVMEM_LINEAR
#define SO_DEVMEM_DMABUF 79
#define SCM_DEVMEM_DMABUF SO_DEVMEM_DMABUF
#define SO_DEVMEM_DONTNEED 80
#define SO_TIMESTAMP SO_TIMESTAMP_OLD
#define SO_TIMESTAMPNS SO_TIMESTAMPNS_OLD
#define SO_TIMESTAMPING SO_TIMESTAMPING_OLD
#define SO_RCVTIMEO SO_RCVTIMEO_OLD
#define SO_SNDTIMEO SO_SNDTIMEO_OLD
#define SCM_TIMESTAMP SO_TIMESTAMP
#define SCM_TIMESTAMPNS SO_TIMESTAMPNS
#define SCM_TIMESTAMPING SO_TIMESTAMPING
#define FIOSETOWN 0x8901
#define SIOCSPGRP 0x8902
#define FIOGETOWN 0x8903
#define SIOCGPGRP 0x8904
#define SIOCATMARK 0x8905
#define SIOCGSTAMP_OLD 0x8906
#define SIOCGSTAMPNS_OLD 0x8907
#endif

#include <_altc/sockaddr.h>

struct sockaddr
  {
    __SOCKADDR_COMMON (sa_);
    char sa_data[14];
  };

#define __ss_aligntype unsigned long int
#define _SS_PADSIZE \
  (_SS_SIZE - __SOCKADDR_COMMON_SIZE - sizeof (__ss_aligntype))

struct sockaddr_storage
  {
    __SOCKADDR_COMMON (ss_);
    char __ss_padding[_SS_PADSIZE];
    __ss_aligntype __ss_align;
  };

struct msghdr
  {
    void *msg_name;
    socklen_t msg_namelen;
    struct iovec *msg_iov;
    size_t msg_iovlen;
    void *msg_control;
    size_t msg_controllen;
    int msg_flags;
  };

struct cmsghdr
  {
    size_t cmsg_len;
    int cmsg_level;
    int cmsg_type;
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
    unsigned char __cmsg_data[];
#endif
  };

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
# define CMSG_DATA(cmsg) ((cmsg)->__cmsg_data)
#else
# define CMSG_DATA(cmsg) ((unsigned char *) ((struct cmsghdr *) (cmsg) + 1))
#endif
#define CMSG_NXTHDR(mhdr, cmsg) __cmsg_nxthdr (mhdr, cmsg)
#define CMSG_FIRSTHDR(mhdr) \
  ((size_t) (mhdr)->msg_controllen >= sizeof (struct cmsghdr) \
   ? (struct cmsghdr *) (mhdr)->msg_control : (struct cmsghdr *) 0)
#define CMSG_ALIGN(len) (((len) + sizeof (size_t) - 1) \
                         & (size_t) ~(sizeof (size_t) - 1))
#define CMSG_SPACE(len) (CMSG_ALIGN (len) \
                         + CMSG_ALIGN (sizeof (struct cmsghdr)))
#define CMSG_LEN(len) (CMSG_ALIGN (sizeof (struct cmsghdr)) + (len))

extern struct cmsghdr *__cmsg_nxthdr (struct msghdr *__mhdr,
                                      struct cmsghdr *__cmsg);

#define SCM_RIGHTS 0x01
#ifdef __USE_GNU
# define SCM_CREDENTIALS 0x02
# define SCM_SECURITY 0x03
# define SCM_PIDFD 0x04

struct ucred
{
  pid_t pid;
  uid_t uid;
  gid_t gid;
};

struct mmsghdr
  {
    struct msghdr msg_hdr;
    unsigned int msg_len;
  };
#endif

struct linger
  {
    int l_onoff;
    int l_linger;
  };

#include <_altc/time_types.h>

extern int socket (int __domain, int __type, int __protocol);
extern int socketpair (int __domain, int __type, int __protocol,
                       int __fds[2]);
extern int bind (int __fd, const struct sockaddr *__addr, socklen_t __len);
extern int getsockname (int __fd, struct sockaddr *__restrict __addr,
                        socklen_t *__restrict __len);
extern int connect (int __fd, const struct sockaddr *__addr,
                    socklen_t __len);
extern int getpeername (int __fd, struct sockaddr *__restrict __addr,
                        socklen_t *__restrict __len);
extern ssize_t send (int __fd, const void *__buf, size_t __n, int __flags);
extern ssize_t recv (int __fd, void *__buf, size_t __n, int __flags);
extern ssize_t sendto (int __fd, const void *__buf, size_t __n,
                       int __flags, const struct sockaddr *__addr,
                       socklen_t __addr_len);
extern ssize_t recvfrom (int __fd, void *__restrict __buf, size_t __n,
                         int __flags, struct sockaddr *__restrict __addr,
                         socklen_t *__restrict __addr_len);
extern ssize_t sendmsg (int __fd, const struct msghdr *__message,
                        int __flags);
extern ssize_t recvmsg (int __fd, struct msghdr *__message, int __flags);
#ifdef __USE_GNU
extern int sendmmsg (int __fd, struct mmsghdr *__vmessages,
                     unsigned int __vlen, int __flags);
extern int recvmmsg (int __fd, struct mmsghdr *__vmessages,
                     unsigned int __vlen, int __flags,
                     struct timespec *__tmo);
#endif
extern int getsockopt (int __fd, int __level, int __optname,
                       void *__restrict __optval,
                       socklen_t *__restrict __optlen);
extern int setsockopt (int __fd, int __level, int __optname,
                       const void *__optval, socklen_t __optlen);
extern int listen (int __fd, int __n);
extern int accept (int __fd, struct sockaddr *__restrict __addr,
                   socklen_t *__restrict __addr_len);
#ifdef __USE_GNU
extern int accept4 (int __fd, struct sockaddr *__restrict __addr,
                    socklen_t *__restrict __addr_len, int __flags);
#endif
extern int shutdown (int __fd, int __how);
extern int sockatmark (int __fd);
extern int isfdtype (int __fd, int __fdtype);

__END_DECLS

#endif /* sys/socket.h */
