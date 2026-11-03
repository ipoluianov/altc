/* AltC <ucontext.h> - user context switching (glibc-compatible).  */

#ifndef _UCONTEXT_H
#define _UCONTEXT_H 1

#include <features.h>
#include <sys/ucontext.h>

__BEGIN_DECLS

extern int getcontext (ucontext_t *__ucp) __attribute__ ((__returns_twice__));
extern int setcontext (const ucontext_t *__ucp);
extern int swapcontext (ucontext_t *__restrict __oucp,
                        const ucontext_t *__restrict __ucp);
extern void makecontext (ucontext_t *__ucp, void (*__func) (void),
                         int __argc, ...);

__END_DECLS

#endif /* ucontext.h */
