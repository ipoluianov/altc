/* AltC private header: struct rusage (glibc guard; the anonymous unions of
   glibc's definition are replaced by their identical long members).  */

#include <_altc/time_types.h>

#ifndef __rusage_defined
#define __rusage_defined 1
struct rusage
  {
    struct timeval ru_utime;
    struct timeval ru_stime;
    long int ru_maxrss;
    long int ru_ixrss;
    long int ru_idrss;
    long int ru_isrss;
    long int ru_minflt;
    long int ru_majflt;
    long int ru_nswap;
    long int ru_inblock;
    long int ru_oublock;
    long int ru_msgsnd;
    long int ru_msgrcv;
    long int ru_nsignals;
    long int ru_nvcsw;
    long int ru_nivcsw;
  };
#endif
