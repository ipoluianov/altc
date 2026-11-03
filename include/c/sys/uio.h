/* AltC <sys/uio.h> - vector I/O.  */

#ifndef _SYS_UIO_H
#define _SYS_UIO_H 1

#include <features.h>
#include <_altc/types.h>
#include <_altc/iovec.h>
#include <sys/types.h>

#ifndef __ssize_t_defined
typedef __ssize_t ssize_t;
# define __ssize_t_defined
#endif

#define UIO_MAXIOV 1024

__BEGIN_DECLS

extern ssize_t readv (int __fd, const struct iovec *__iovec, int __count);
extern ssize_t writev (int __fd, const struct iovec *__iovec, int __count);
extern ssize_t preadv (int __fd, const struct iovec *__iovec, int __count,
                       __off_t __offset);
extern ssize_t pwritev (int __fd, const struct iovec *__iovec, int __count,
                        __off_t __offset);

#ifdef __USE_GNU
# define RWF_HIPRI 0x00000001
# define RWF_DSYNC 0x00000002
# define RWF_SYNC 0x00000004
# define RWF_NOWAIT 0x00000008
# define RWF_APPEND 0x00000010
# define RWF_NOAPPEND 0x00000020
# define RWF_ATOMIC 0x00000040
extern ssize_t preadv2 (int __fp, const struct iovec *__iovec, int __count,
                        __off_t __offset, int ___flags);
extern ssize_t pwritev2 (int __fd, const struct iovec *__iodev, int __count,
                         __off_t __offset, int __flags);
extern ssize_t preadv64 (int __fd, const struct iovec *__iovec, int __count,
                         __off64_t __offset);
extern ssize_t pwritev64 (int __fd, const struct iovec *__iovec, int __count,
                          __off64_t __offset);
extern ssize_t preadv64v2 (int __fp, const struct iovec *__iovec,
                           int __count, __off64_t __offset, int ___flags);
extern ssize_t pwritev64v2 (int __fd, const struct iovec *__iodev,
                            int __count, __off64_t __offset, int __flags);
extern ssize_t process_vm_readv (__pid_t __pid, const struct iovec *__lvec,
                                 unsigned long int __liovcnt,
                                 const struct iovec *__rvec,
                                 unsigned long int __riovcnt,
                                 unsigned long int __flags);
extern ssize_t process_vm_writev (__pid_t __pid, const struct iovec *__lvec,
                                  unsigned long int __liovcnt,
                                  const struct iovec *__rvec,
                                  unsigned long int __riovcnt,
                                  unsigned long int __flags);
#endif

__END_DECLS

#endif /* sys/uio.h */
