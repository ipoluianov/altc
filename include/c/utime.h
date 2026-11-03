/* AltC <utime.h> - file access and modification times.  */

#ifndef _UTIME_H
#define _UTIME_H 1

#include <features.h>
#include <_altc/types.h>
#include <_altc/time_types.h>

__BEGIN_DECLS

struct utimbuf
  {
    __time_t actime;
    __time_t modtime;
  };

extern int utime (const char *__file, const struct utimbuf *__file_times);

__END_DECLS

#endif /* utime.h */
