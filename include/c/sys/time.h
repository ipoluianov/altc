/* AltC <sys/time.h> - time types and interval timers.  */

#ifndef _SYS_TIME_H
#define _SYS_TIME_H 1

#include <features.h>
#include <_altc/types.h>
#include <_altc/time_types.h>

#ifndef __suseconds_t_defined
typedef __suseconds_t suseconds_t;
# define __suseconds_t_defined
#endif

#include <sys/select.h>

__BEGIN_DECLS

struct timezone
  {
    int tz_minuteswest;
    int tz_dsttime;
  };

extern int gettimeofday (struct timeval *__restrict __tv,
                         void *__restrict __tz);
extern int settimeofday (const struct timeval *__tv,
                         const struct timezone *__tz);
extern int adjtime (const struct timeval *__delta,
                    struct timeval *__olddelta);

enum __itimer_which
  {
    ITIMER_REAL = 0,
#define ITIMER_REAL ITIMER_REAL
    ITIMER_VIRTUAL = 1,
#define ITIMER_VIRTUAL ITIMER_VIRTUAL
    ITIMER_PROF = 2
#define ITIMER_PROF ITIMER_PROF
  };

struct itimerval
  {
    struct timeval it_interval;
    struct timeval it_value;
  };

typedef int __itimer_which_t;

extern int getitimer (__itimer_which_t __which, struct itimerval *__value);
extern int setitimer (__itimer_which_t __which,
                      const struct itimerval *__restrict __new,
                      struct itimerval *__restrict __old);

extern int utimes (const char *__file, const struct timeval __tvp[2]);
extern int lutimes (const char *__file, const struct timeval __tvp[2]);
extern int futimes (int __fd, const struct timeval __tvp[2]);
#ifdef __USE_GNU
extern int futimesat (int __fd, const char *__file,
                      const struct timeval __tvp[2]);
#endif

#define timerisset(tvp) ((tvp)->tv_sec || (tvp)->tv_usec)
#define timerclear(tvp) ((tvp)->tv_sec = (tvp)->tv_usec = 0)
#define timercmp(a, b, CMP) \
  (((a)->tv_sec == (b)->tv_sec) \
   ? ((a)->tv_usec CMP (b)->tv_usec) \
   : ((a)->tv_sec CMP (b)->tv_sec))
#define timeradd(a, b, result) \
  do { \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec; \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
    if ((result)->tv_usec >= 1000000) \
      { \
        ++(result)->tv_sec; \
        (result)->tv_usec -= 1000000; \
      } \
  } while (0)
#define timersub(a, b, result) \
  do { \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
    if ((result)->tv_usec < 0) { \
      --(result)->tv_sec; \
      (result)->tv_usec += 1000000; \
    } \
  } while (0)

#define TIMEVAL_TO_TIMESPEC(tv, ts) { \
  (ts)->tv_sec = (tv)->tv_sec; \
  (ts)->tv_nsec = (tv)->tv_usec * 1000; \
}
#define TIMESPEC_TO_TIMEVAL(tv, ts) { \
  (tv)->tv_sec = (ts)->tv_sec; \
  (tv)->tv_usec = (ts)->tv_nsec / 1000; \
}

__END_DECLS

#endif /* sys/time.h */
