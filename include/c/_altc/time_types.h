/* AltC private header: time-related shared types.  Each definition uses
   the same "already defined" guard as the corresponding glibc
   <bits/types/...> header.  */

#include <_altc/types.h>

#ifndef __time_t_defined
#define __time_t_defined 1
typedef __time_t time_t;
#endif

#ifndef __clock_t_defined
#define __clock_t_defined 1
typedef __clock_t clock_t;
#endif

#ifndef __clockid_t_defined
#define __clockid_t_defined 1
typedef __clockid_t clockid_t;
#endif

#ifndef __timer_t_defined
#define __timer_t_defined 1
typedef __timer_t timer_t;
#endif

#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC 1
struct timespec
{
  __time_t tv_sec;
  __syscall_slong_t tv_nsec;
};
#endif

#ifndef __timeval_defined
#define __timeval_defined 1
struct timeval
{
  __time_t tv_sec;
  __suseconds_t tv_usec;
};
#endif
