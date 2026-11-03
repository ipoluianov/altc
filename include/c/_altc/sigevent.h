/* AltC private header: struct sigevent and union sigval (glibc guards).  */

#include <_altc/types.h>

#ifndef ____sigval_t_defined
#define ____sigval_t_defined
union sigval
{
  int sival_int;
  void *sival_ptr;
};
typedef union sigval __sigval_t;
#endif

#include <_altc/pthreadtypes.h>
#ifndef __sigevent_t_defined
#define __sigevent_t_defined 1
#define __SIGEV_MAX_SIZE 64
#define __SIGEV_PAD_SIZE ((__SIGEV_MAX_SIZE / sizeof (int)) - 4)
typedef struct sigevent
  {
    __sigval_t sigev_value;
    int sigev_signo;
    int sigev_notify;
    union
      {
        int _pad[__SIGEV_PAD_SIZE];
        __pid_t _tid;
        struct
          {
            void (*_function) (__sigval_t);
            pthread_attr_t *_attribute;
          } _sigev_thread;
      } _sigev_un;
  } sigevent_t;
#define sigev_notify_function _sigev_un._sigev_thread._function
#define sigev_notify_attributes _sigev_un._sigev_thread._attribute
#endif

