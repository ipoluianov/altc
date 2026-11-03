/* AltC private header: struct iovec (glibc guard).  */

#ifndef __iovec_defined
#define __iovec_defined 1

#define __need_size_t
#include <stddef.h>

struct iovec
  {
    void *iov_base;
    size_t iov_len;
  };

#endif
