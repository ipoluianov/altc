/* AltC <fenv.h> - floating-point environment (glibc-compatible, x86-64).
   The functions live in libm.so.6.  */

#ifndef _FENV_H
#define _FENV_H 1

#include <features.h>

__BEGIN_DECLS

#define FE_INVALID 0x01
#define __FE_DENORM 0x02
#define FE_DIVBYZERO 0x04
#define FE_OVERFLOW 0x08
#define FE_UNDERFLOW 0x10
#define FE_INEXACT 0x20

#define FE_ALL_EXCEPT \
  (FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

#define FE_TONEAREST 0
#define FE_DOWNWARD 0x400
#define FE_UPWARD 0x800
#define FE_TOWARDZERO 0xc00

typedef unsigned short int fexcept_t;

/* x87 environment followed by the SSE MXCSR register (32 bytes).  The
   bit-fields occupy one 32-bit unit exactly as in glibc.  */
typedef struct
  {
    unsigned short int __control_word;
    unsigned short int __glibc_reserved1;
    unsigned short int __status_word;
    unsigned short int __glibc_reserved2;
    unsigned short int __tags;
    unsigned short int __glibc_reserved3;
    unsigned int __eip;
    unsigned short int __cs_selector;
    unsigned int __opcode:11;
    unsigned int __glibc_reserved4:5;
    unsigned int __data_offset;
    unsigned short int __data_selector;
    unsigned short int __glibc_reserved5;
    unsigned int __mxcsr;
  }
fenv_t;

#define FE_DFL_ENV ((const fenv_t *) -1)
#ifdef __USE_GNU
# define FE_NOMASK_ENV ((const fenv_t *) -2)
#endif

typedef struct
  {
    unsigned short int __control_word;
    unsigned short int __glibc_reserved;
    unsigned int __mxcsr;
  }
femode_t;

#define FE_DFL_MODE ((const femode_t *) -1L)

extern int feclearexcept (int __excepts);
extern int fegetexceptflag (fexcept_t *__flagp, int __excepts);
extern int feraiseexcept (int __excepts);
extern int fesetexceptflag (const fexcept_t *__flagp, int __excepts);
extern int fetestexcept (int __excepts);

extern int fegetround (void);
extern int fesetround (int __rounding_direction);

extern int fegetenv (fenv_t *__envp);
extern int feholdexcept (fenv_t *__envp);
extern int fesetenv (const fenv_t *__envp);
extern int feupdateenv (const fenv_t *__envp);

/* C23 (glibc 2.25+).  */
extern int fesetexcept (int __excepts);
extern int fetestexceptflag (const fexcept_t *__flagp, int __excepts);
extern int fegetmode (femode_t *__modep);
extern int fesetmode (const femode_t *__modep);

/* GNU extensions.  */
extern int feenableexcept (int __excepts);
extern int fedisableexcept (int __excepts);
extern int fegetexcept (void);

#if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
# define __STDC_VERSION_FENV_H__ 202311L
#endif

__END_DECLS

#endif /* fenv.h */
