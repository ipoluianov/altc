/* AltC private header: stack_t (glibc guard).  */

#ifndef __stack_t_defined
#define __stack_t_defined 1
#define __need_size_t
#include <stddef.h>
typedef struct
  {
    void *ss_sp;
    int ss_flags;
    size_t ss_size;
  } stack_t;
#endif
