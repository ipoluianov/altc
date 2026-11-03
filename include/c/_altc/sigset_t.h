/* AltC private header: __sigset_t / sigset_t (glibc guards).  */

#ifndef ____sigset_t_defined
#define ____sigset_t_defined
#define _SIGSET_NWORDS (1024 / (8 * sizeof (unsigned long int)))
typedef struct
{
  unsigned long int __val[_SIGSET_NWORDS];
} __sigset_t;
#endif

#ifndef __sigset_t_defined
#define __sigset_t_defined 1
typedef __sigset_t sigset_t;
#endif
