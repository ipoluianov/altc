/* AltC <sys/times.h> - process times.  */

#ifndef _SYS_TIMES_H
#define _SYS_TIMES_H 1

#include <features.h>
#include <_altc/time_types.h>

__BEGIN_DECLS

struct tms
  {
    clock_t tms_utime;
    clock_t tms_stime;
    clock_t tms_cutime;
    clock_t tms_cstime;
  };

extern clock_t times (struct tms *__buffer);

__END_DECLS

#endif /* sys/times.h */
