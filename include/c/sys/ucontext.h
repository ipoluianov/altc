/* AltC <sys/ucontext.h> - user context (glibc-compatible, x86-64).  */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H 1

#include <features.h>
#include <_altc/types.h>
#include <_altc/sigset_t.h>
#include <_altc/stack_t.h>

#ifdef __USE_MISC
# define __ctx(fld) fld
#else
# define __ctx(fld) __ ## fld
#endif

typedef long long int greg_t;

#define __NGREG 23
#ifdef __USE_MISC
# define NGREG __NGREG
#endif

typedef greg_t gregset_t[__NGREG];

#ifdef __USE_GNU
# define REG_R8 0
# define REG_R9 1
# define REG_R10 2
# define REG_R11 3
# define REG_R12 4
# define REG_R13 5
# define REG_R14 6
# define REG_R15 7
# define REG_RDI 8
# define REG_RSI 9
# define REG_RBP 10
# define REG_RBX 11
# define REG_RDX 12
# define REG_RAX 13
# define REG_RCX 14
# define REG_RSP 15
# define REG_RIP 16
# define REG_EFL 17
# define REG_CSGSFS 18
# define REG_ERR 19
# define REG_TRAPNO 20
# define REG_OLDMASK 21
# define REG_CR2 22
#endif

struct _libc_fpxreg
{
  unsigned short int __ctx(significand)[4];
  unsigned short int __ctx(exponent);
  unsigned short int __glibc_reserved1[3];
};

struct _libc_xmmreg
{
  __uint32_t __ctx(element)[4];
};

struct _libc_fpstate
{
  __uint16_t __ctx(cwd);
  __uint16_t __ctx(swd);
  __uint16_t __ctx(ftw);
  __uint16_t __ctx(fop);
  __uint64_t __ctx(rip);
  __uint64_t __ctx(rdp);
  __uint32_t __ctx(mxcsr);
  __uint32_t __ctx(mxcr_mask);
  struct _libc_fpxreg _st[8];
  struct _libc_xmmreg _xmm[16];
  __uint32_t __glibc_reserved1[24];
};

typedef struct _libc_fpstate *fpregset_t;

typedef struct
  {
    gregset_t __ctx(gregs);
    fpregset_t __ctx(fpregs);
    unsigned long long __reserved1 [8];
} mcontext_t;

typedef struct ucontext_t
  {
    unsigned long int __ctx(uc_flags);
    struct ucontext_t *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
    unsigned long long int __ssp[4];
  } ucontext_t;

#undef __ctx

#endif /* sys/ucontext.h */
