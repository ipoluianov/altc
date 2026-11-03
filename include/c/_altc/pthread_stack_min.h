/* AltC private header: PTHREAD_STACK_MIN (shared by <limits.h> and
   <pthread.h>).  */

#ifndef PTHREAD_STACK_MIN
# if defined __USE_DYNAMIC_STACK_SIZE && __USE_DYNAMIC_STACK_SIZE
extern long int __sysconf (int __name);
#  define PTHREAD_STACK_MIN __sysconf (75)
# else
#  define PTHREAD_STACK_MIN 16384
# endif
#endif
