/* AltC <sys/poll.h> - wait for events on file descriptors.  */

#ifndef _SYS_POLL_H
#define _SYS_POLL_H 1

#include <features.h>

/* <bits/poll.h> */
#define POLLIN 0x001
#define POLLPRI 0x002
#define POLLOUT 0x004
#define POLLRDNORM 0x040
#define POLLRDBAND 0x080
#define POLLWRNORM 0x100
#define POLLWRBAND 0x200
#ifdef __USE_GNU
# define POLLMSG 0x400
# define POLLREMOVE 0x1000
# define POLLRDHUP 0x2000
#endif
#define POLLERR 0x008
#define POLLHUP 0x010
#define POLLNVAL 0x020

#ifdef __USE_GNU
# include <_altc/sigset_t.h>
# include <_altc/time_types.h>
#endif

__BEGIN_DECLS

typedef unsigned long int nfds_t;

struct pollfd
  {
    int fd;
    short int events;
    short int revents;
  };

extern int poll (struct pollfd *__fds, nfds_t __nfds, int __timeout);

#ifdef __USE_GNU
extern int ppoll (struct pollfd *__fds, nfds_t __nfds,
                  const struct timespec *__timeout,
                  const __sigset_t *__ss);
#endif

__END_DECLS

#endif /* sys/poll.h */
