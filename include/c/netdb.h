/* AltC <netdb.h> - network database operations (glibc-compatible).  */

#ifndef _NETDB_H
#define _NETDB_H 1

#include <features.h>
#include <netinet/in.h>

#define __need_size_t
#include <stddef.h>

__BEGIN_DECLS

#define _PATH_HEQUIV "/etc/hosts.equiv"
#define _PATH_HOSTS "/etc/hosts"
#define _PATH_NETWORKS "/etc/networks"
#define _PATH_NSSWITCH_CONF "/etc/nsswitch.conf"
#define _PATH_PROTOCOLS "/etc/protocols"
#define _PATH_SERVICES "/etc/services"

#define h_errno (*__h_errno_location ())

#define HOST_NOT_FOUND 1
#define TRY_AGAIN 2
#define NO_RECOVERY 3
#define NO_DATA 4
#define NETDB_INTERNAL -1
#define NETDB_SUCCESS 0
#define NO_ADDRESS NO_DATA
#define IPPORT_RESERVED 1024
#ifdef __USE_GNU
# define SCOPE_DELIMITER 37
#endif
#ifdef __USE_GNU
# define GAI_WAIT 0
#endif
#ifdef __USE_GNU
# define GAI_NOWAIT 1
#endif
#define AI_PASSIVE 0x0001
#define AI_CANONNAME 0x0002
#define AI_NUMERICHOST 0x0004
#define AI_V4MAPPED 0x0008
#define AI_ALL 0x0010
#define AI_ADDRCONFIG 0x0020
#ifdef __USE_GNU
# define AI_IDN 0x0040
#endif
#ifdef __USE_GNU
# define AI_CANONIDN 0x0080
#endif
#ifdef __USE_GNU
# define AI_IDN_ALLOW_UNASSIGNED 256
#endif
#ifdef __USE_GNU
# define AI_IDN_USE_STD3_ASCII_RULES 512
#endif
#define AI_NUMERICSERV 0x0400
#define EAI_BADFLAGS -1
#define EAI_NONAME -2
#define EAI_AGAIN -3
#define EAI_FAIL -4
#define EAI_FAMILY -6
#define EAI_SOCKTYPE -7
#define EAI_SERVICE -8
#define EAI_MEMORY -10
#define EAI_SYSTEM -11
#define EAI_OVERFLOW -12
#ifdef __USE_GNU
# define EAI_NODATA -5
#endif
#ifdef __USE_GNU
# define EAI_ADDRFAMILY -9
#endif
#ifdef __USE_GNU
# define EAI_INPROGRESS -100
#endif
#ifdef __USE_GNU
# define EAI_CANCELED -101
#endif
#ifdef __USE_GNU
# define EAI_NOTCANCELED -102
#endif
#ifdef __USE_GNU
# define EAI_ALLDONE -103
#endif
#ifdef __USE_GNU
# define EAI_INTR -104
#endif
#ifdef __USE_GNU
# define EAI_IDN_ENCODE -105
#endif
#define NI_MAXHOST 1025
#define NI_MAXSERV 32
#define NI_NUMERICHOST 1
#define NI_NUMERICSERV 2
#define NI_NOFQDN 4
#define NI_NAMEREQD 8
#define NI_DGRAM 16
#ifdef __USE_GNU
# define NI_IDN 32
#endif
#ifdef __USE_GNU
# define NI_IDN_ALLOW_UNASSIGNED 64
#endif
#ifdef __USE_GNU
# define NI_IDN_USE_STD3_ASCII_RULES 128
#endif

struct hostent
{
  char *h_name;
  char **h_aliases;
  int h_addrtype;
  int h_length;
  char **h_addr_list;
#define h_addr h_addr_list[0]
};

struct netent
{
  char *n_name;
  char **n_aliases;
  int n_addrtype;
  uint32_t n_net;
};

struct servent
{
  char *s_name;
  char **s_aliases;
  int s_port;
  char *s_proto;
};

struct protoent
{
  char *p_name;
  char **p_aliases;
  int p_proto;
};

struct addrinfo
{
  int ai_flags;
  int ai_family;
  int ai_socktype;
  int ai_protocol;
  socklen_t ai_addrlen;
  struct sockaddr *ai_addr;
  char *ai_canonname;
  struct addrinfo *ai_next;
};

#ifdef __USE_GNU
struct gaicb
{
  const char *ar_name;
  const char *ar_service;
  const struct addrinfo *ar_request;
  struct addrinfo *ar_result;
  int __return;
  int __glibc_reserved[5];
};
#endif

extern int *__h_errno_location (void);
extern void herror (const char *__str);
extern const char *hstrerror (int __err_num);
extern void sethostent (int __stay_open);
extern void endhostent (void);
extern struct hostent *gethostent (void);
extern struct hostent *gethostbyaddr (const void *__addr, __socklen_t __len,
                                      int __type);
extern struct hostent *gethostbyname (const char *__name);
extern struct hostent *gethostbyname2 (const char *__name, int __af);
extern int gethostent_r (struct hostent *__restrict __result_buf,
                         char *__restrict __buf, size_t __buflen,
                         struct hostent **__restrict __result,
                         int *__restrict __h_errnop);
extern int gethostbyaddr_r (const void *__restrict __addr, __socklen_t __len,
                            int __type,
                            struct hostent *__restrict __result_buf,
                            char *__restrict __buf, size_t __buflen,
                            struct hostent **__restrict __result,
                            int *__restrict __h_errnop);
extern int gethostbyname_r (const char *__restrict __name,
                            struct hostent *__restrict __result_buf,
                            char *__restrict __buf, size_t __buflen,
                            struct hostent **__restrict __result,
                            int *__restrict __h_errnop);
extern int gethostbyname2_r (const char *__restrict __name, int __af,
                             struct hostent *__restrict __result_buf,
                             char *__restrict __buf, size_t __buflen,
                             struct hostent **__restrict __result,
                             int *__restrict __h_errnop);
extern void setnetent (int __stay_open);
extern void endnetent (void);
extern struct netent *getnetent (void);
extern struct netent *getnetbyaddr (uint32_t __net, int __type);
extern struct netent *getnetbyname (const char *__name);
extern int getnetent_r (struct netent *__restrict __result_buf,
                        char *__restrict __buf, size_t __buflen,
                        struct netent **__restrict __result,
                        int *__restrict __h_errnop);
extern int getnetbyaddr_r (uint32_t __net, int __type,
                           struct netent *__restrict __result_buf,
                           char *__restrict __buf, size_t __buflen,
                           struct netent **__restrict __result,
                           int *__restrict __h_errnop);
extern int getnetbyname_r (const char *__restrict __name,
                           struct netent *__restrict __result_buf,
                           char *__restrict __buf, size_t __buflen,
                           struct netent **__restrict __result,
                           int *__restrict __h_errnop);
extern void setservent (int __stay_open);
extern void endservent (void);
extern struct servent *getservent (void);
extern struct servent *getservbyname (const char *__name, const char *__proto);
extern struct servent *getservbyport (int __port, const char *__proto);
extern int getservent_r (struct servent *__restrict __result_buf,
                         char *__restrict __buf, size_t __buflen,
                         struct servent **__restrict __result);
extern int getservbyname_r (const char *__restrict __name,
                            const char *__restrict __proto,
                            struct servent *__restrict __result_buf,
                            char *__restrict __buf, size_t __buflen,
                            struct servent **__restrict __result);
extern int getservbyport_r (int __port, const char *__restrict __proto,
                            struct servent *__restrict __result_buf,
                            char *__restrict __buf, size_t __buflen,
                            struct servent **__restrict __result);
extern void setprotoent (int __stay_open);
extern void endprotoent (void);
extern struct protoent *getprotoent (void);
extern struct protoent *getprotobyname (const char *__name);
extern struct protoent *getprotobynumber (int __proto);
extern int getprotoent_r (struct protoent *__restrict __result_buf,
                          char *__restrict __buf, size_t __buflen,
                          struct protoent **__restrict __result);
extern int getprotobyname_r (const char *__restrict __name,
                             struct protoent *__restrict __result_buf,
                             char *__restrict __buf, size_t __buflen,
                             struct protoent **__restrict __result);
extern int getprotobynumber_r (int __proto,
                               struct protoent *__restrict __result_buf,
                               char *__restrict __buf, size_t __buflen,
                               struct protoent **__restrict __result);
extern int setnetgrent (const char *__netgroup);
extern void endnetgrent (void);
extern int getnetgrent (char **__restrict __hostp, char **__restrict __userp,
                        char **__restrict __domainp);
extern int innetgr (const char *__netgroup, const char *__host,
                    const char *__user, const char *__domain);
extern int getnetgrent_r (char **__restrict __hostp,
                          char **__restrict __userp,
                          char **__restrict __domainp,
                          char *__restrict __buffer, size_t __buflen);
extern int rcmd (char **__restrict __ahost, unsigned short int __rport,
                 const char *__restrict __locuser,
                 const char *__restrict __remuser,
                 const char *__restrict __cmd, int *__restrict __fd2p);
extern int rcmd_af (char **__restrict __ahost, unsigned short int __rport,
                    const char *__restrict __locuser,
                    const char *__restrict __remuser,
                    const char *__restrict __cmd, int *__restrict __fd2p,
                    sa_family_t __af);
extern int rexec (char **__restrict __ahost, int __rport,
                  const char *__restrict __name,
                  const char *__restrict __pass, const char *__restrict __cmd,
                  int *__restrict __fd2p);
extern int rexec_af (char **__restrict __ahost, int __rport,
                     const char *__restrict __name,
                     const char *__restrict __pass,
                     const char *__restrict __cmd, int *__restrict __fd2p,
                     sa_family_t __af);
extern int ruserok (const char *__rhost, int __suser, const char *__remuser,
                    const char *__locuser);
extern int ruserok_af (const char *__rhost, int __suser,
                       const char *__remuser, const char *__locuser,
                       sa_family_t __af);
extern int iruserok (uint32_t __raddr, int __suser, const char *__remuser,
                     const char *__locuser);
extern int iruserok_af (const void *__raddr, int __suser,
                        const char *__remuser, const char *__locuser,
                        sa_family_t __af);
extern int rresvport (int *__alport);
extern int rresvport_af (int *__alport, sa_family_t __af);
extern int getaddrinfo (const char *__restrict __name,
                        const char *__restrict __service,
                        const struct addrinfo *__restrict __req,
                        struct addrinfo **__restrict __pai);
extern void freeaddrinfo (struct addrinfo *__ai);
extern const char *gai_strerror (int __ecode);
extern int getnameinfo (const struct sockaddr *__restrict __sa,
                        socklen_t __salen, char *__restrict __host,
                        socklen_t __hostlen, char *__restrict __serv,
                        socklen_t __servlen, int __flags);

#ifdef __USE_GNU
# include <_altc/sigevent.h>
extern int getaddrinfo_a (int __mode, struct gaicb *__list[__restrict],
                          int __ent, struct sigevent *__restrict __sig);
extern int gai_suspend (const struct gaicb *const __list[], int __ent,
                        const struct timespec *__timeout);
extern int gai_error (struct gaicb *__req);
extern int gai_cancel (struct gaicb *__gaicbp);
#endif

__END_DECLS

#endif /* netdb.h */
