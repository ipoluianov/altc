/* AltC <time.h> - date and time (glibc-compatible, x86-64).  */

#ifndef _TIME_H
#define _TIME_H 1

#include <features.h>

#define __need_size_t
#define __need_NULL
#include <stddef.h>

#include <_altc/types.h>
#include <_altc/time_types.h>
#include <_altc/struct_tm.h>
#include <_altc/locale_t.h>

/* <bits/time.h> */
#ifndef _BITS_TIME_H
#define _BITS_TIME_H 1
#define CLOCKS_PER_SEC ((__clock_t) 1000000)
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID 3
#define CLOCK_MONOTONIC_RAW 4
#define CLOCK_REALTIME_COARSE 5
#define CLOCK_MONOTONIC_COARSE 6
#define CLOCK_BOOTTIME 7
#define CLOCK_REALTIME_ALARM 8
#define CLOCK_BOOTTIME_ALARM 9
#define CLOCK_TAI 11
#define TIMER_ABSTIME 1
#endif

#ifndef __itimerspec_defined
#define __itimerspec_defined 1
struct itimerspec
  {
    struct timespec it_interval;
    struct timespec it_value;
  };
#endif

struct sigevent;

#ifndef __pid_t_defined
typedef __pid_t pid_t;
# define __pid_t_defined
#endif

#define TIME_UTC 1

__BEGIN_DECLS

extern clock_t clock (void);
extern time_t time (time_t *__timer);
extern double difftime (time_t __time1, time_t __time0)
     __attribute__ ((__const__));
extern time_t mktime (struct tm *__tp);

extern size_t strftime (char *__restrict __s, size_t __maxsize,
                        const char *__restrict __format,
                        const struct tm *__restrict __tp);
extern char *strptime (const char *__restrict __s,
                       const char *__restrict __fmt, struct tm *__tp);
extern size_t strftime_l (char *__restrict __s, size_t __maxsize,
                          const char *__restrict __format,
                          const struct tm *__restrict __tp,
                          locale_t __loc);
#ifdef __USE_GNU
extern char *strptime_l (const char *__restrict __s,
                         const char *__restrict __fmt, struct tm *__tp,
                         locale_t __loc);
#endif

extern struct tm *gmtime (const time_t *__timer);
extern struct tm *localtime (const time_t *__timer);
extern struct tm *gmtime_r (const time_t *__restrict __timer,
                            struct tm *__restrict __tp);
extern struct tm *localtime_r (const time_t *__restrict __timer,
                               struct tm *__restrict __tp);

extern char *asctime (const struct tm *__tp);
extern char *ctime (const time_t *__timer);
extern char *asctime_r (const struct tm *__restrict __tp,
                        char *__restrict __buf);
extern char *ctime_r (const time_t *__restrict __timer,
                      char *__restrict __buf);

extern char *__tzname[2];
extern int __daylight;
extern long int __timezone;
extern char *tzname[2];
extern void tzset (void);
extern int daylight;
extern long int timezone;

#define __isleap(year) \
  ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

extern time_t timegm (struct tm *__tp);
extern time_t timelocal (struct tm *__tp);
extern int dysize (int __year) __attribute__ ((__const__));

extern int nanosleep (const struct timespec *__requested_time,
                      struct timespec *__remaining);
extern int clock_getres (clockid_t __clock_id, struct timespec *__res);
extern int clock_gettime (clockid_t __clock_id, struct timespec *__tp);
extern int clock_settime (clockid_t __clock_id, const struct timespec *__tp);
extern int clock_nanosleep (clockid_t __clock_id, int __flags,
                            const struct timespec *__req,
                            struct timespec *__rem);
extern int clock_getcpuclockid (pid_t __pid, clockid_t *__clock_id);

extern int timer_create (clockid_t __clock_id,
                         struct sigevent *__restrict __evp,
                         timer_t *__restrict __timerid);
extern int timer_delete (timer_t __timerid);
extern int timer_settime (timer_t __timerid, int __flags,
                          const struct itimerspec *__restrict __value,
                          struct itimerspec *__restrict __ovalue);
extern int timer_gettime (timer_t __timerid, struct itimerspec *__value);
extern int timer_getoverrun (timer_t __timerid);

extern int timespec_get (struct timespec *__ts, int __base);
extern int timespec_getres (struct timespec *__ts, int __base);

extern int getdate_err;
extern struct tm *getdate (const char *__string);
#ifdef __USE_GNU
extern int getdate_r (const char *__restrict __string,
                      struct tm *__restrict __resbufp);
#endif

__END_DECLS

#endif /* time.h */
