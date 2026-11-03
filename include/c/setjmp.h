/* AltC <setjmp.h> - non-local jumps (glibc-compatible, x86-64).

   The compiler must treat _setjmp, __sigsetjmp and setjmp as functions
   that may return twice.  */

#ifndef _SETJMP_H
#define _SETJMP_H 1

#include <features.h>
#include <_altc/jmp_buf_tag.h>

__BEGIN_DECLS

typedef struct __jmp_buf_tag jmp_buf[1];

extern int setjmp (jmp_buf __env) __attribute__ ((__returns_twice__));
extern int __sigsetjmp (struct __jmp_buf_tag __env[1], int __savemask)
     __attribute__ ((__returns_twice__));
extern int _setjmp (struct __jmp_buf_tag __env[1])
     __attribute__ ((__returns_twice__));

/* ISO C setjmp does not save the signal mask.  */
#define setjmp(env) _setjmp (env)

extern void longjmp (struct __jmp_buf_tag __env[1], int __val)
     __attribute__ ((__noreturn__));
extern void _longjmp (struct __jmp_buf_tag __env[1], int __val)
     __attribute__ ((__noreturn__));

typedef struct __jmp_buf_tag sigjmp_buf[1];

#define sigsetjmp(env, savemask) __sigsetjmp (env, savemask)

extern void siglongjmp (sigjmp_buf __env, int __val)
     __attribute__ ((__noreturn__));

#if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
# define __STDC_VERSION_SETJMP_H__ 202311L
#endif

__END_DECLS

#endif /* setjmp.h */
