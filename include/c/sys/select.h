/* AltC <sys/select.h> - synchronous I/O multiplexing.  */

#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H 1

#include <features.h>
#include <_altc/types.h>
#include <_altc/sigset_t.h>
#include <_altc/time_types.h>

#ifndef __suseconds_t_defined
typedef __suseconds_t suseconds_t;
# define __suseconds_t_defined
#endif

__BEGIN_DECLS

typedef long int __fd_mask;

#undef __NFDBITS
#define __NFDBITS (8 * (int) sizeof (__fd_mask))
#define __FD_ELT(d) ((d) / __NFDBITS)
#define __FD_MASK(d) ((__fd_mask) (1UL << ((d) % __NFDBITS)))

typedef struct
  {
#ifdef __USE_XOPEN
    __fd_mask fds_bits[__FD_SETSIZE / __NFDBITS];
# define __FDS_BITS(set) ((set)->fds_bits)
#else
    __fd_mask __fds_bits[__FD_SETSIZE / __NFDBITS];
# define __FDS_BITS(set) ((set)->__fds_bits)
#endif
  } fd_set;

#define FD_SETSIZE __FD_SETSIZE

typedef __fd_mask fd_mask;
#define NFDBITS __NFDBITS

#define __FD_ZERO(s) \
  do { \
    unsigned int __i; \
    fd_set *__arr = (s); \
    for (__i = 0; __i < sizeof (fd_set) / sizeof (__fd_mask); ++__i) \
      __FDS_BITS (__arr)[__i] = 0; \
  } while (0)
#define __FD_SET(d, s) \
  ((void) (__FDS_BITS (s)[__FD_ELT(d)] |= __FD_MASK(d)))
#define __FD_CLR(d, s) \
  ((void) (__FDS_BITS (s)[__FD_ELT(d)] &= ~__FD_MASK(d)))
#define __FD_ISSET(d, s) \
  ((__FDS_BITS (s)[__FD_ELT (d)] & __FD_MASK (d)) != 0)

#define FD_SET(fd, fdsetp) __FD_SET (fd, fdsetp)
#define FD_CLR(fd, fdsetp) __FD_CLR (fd, fdsetp)
#define FD_ISSET(fd, fdsetp) __FD_ISSET (fd, fdsetp)
#define FD_ZERO(fdsetp) __FD_ZERO (fdsetp)

extern int select (int __nfds, fd_set *__restrict __readfds,
                   fd_set *__restrict __writefds,
                   fd_set *__restrict __exceptfds,
                   struct timeval *__restrict __timeout);

extern int pselect (int __nfds, fd_set *__restrict __readfds,
                    fd_set *__restrict __writefds,
                    fd_set *__restrict __exceptfds,
                    const struct timespec *__restrict __timeout,
                    const __sigset_t *__restrict __sigmask);

__END_DECLS

#endif /* sys/select.h */
