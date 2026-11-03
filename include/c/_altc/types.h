/* AltC private header: glibc's internal fixed-width and system types.

   Mirrors glibc's <bits/types.h>, <bits/typesizes.h>, <bits/timesize.h>
   and <bits/time64.h> for x86-64 under the same include guards and with
   token-identical macro definitions, so that whichever of AltC's or
   glibc's copy is seen first wins and the other is skipped harmlessly.  */

#ifndef _BITS_TYPES_H
#define _BITS_TYPES_H 1

#include <features.h>

/* <bits/timesize.h> */
#define __TIMESIZE	__WORDSIZE

typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;

typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;

typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;

typedef long int __quad_t;
typedef unsigned long int __u_quad_t;

typedef long int __intmax_t;
typedef unsigned long int __uintmax_t;

#define	__S16_TYPE		short int
#define __U16_TYPE		unsigned short int
#define	__S32_TYPE		int
#define __U32_TYPE		unsigned int
#define __SLONGWORD_TYPE	long int
#define __ULONGWORD_TYPE	unsigned long int
#define __SQUAD_TYPE		long int
#define __UQUAD_TYPE		unsigned long int
#define __SWORD_TYPE		long int
#define __UWORD_TYPE		unsigned long int
#define __SLONG32_TYPE		int
#define __ULONG32_TYPE		unsigned int
#define __S64_TYPE		long int
#define __U64_TYPE		unsigned long int
#define __STD_TYPE		typedef

/* <bits/typesizes.h> */
#ifndef _BITS_TYPESIZES_H
#define _BITS_TYPESIZES_H 1
#define __SYSCALL_SLONG_TYPE	__SLONGWORD_TYPE
#define __SYSCALL_ULONG_TYPE	__ULONGWORD_TYPE
#define __DEV_T_TYPE		__UQUAD_TYPE
#define __UID_T_TYPE		__U32_TYPE
#define __GID_T_TYPE		__U32_TYPE
#define __INO_T_TYPE		__SYSCALL_ULONG_TYPE
#define __INO64_T_TYPE		__UQUAD_TYPE
#define __MODE_T_TYPE		__U32_TYPE
#define __NLINK_T_TYPE		__SYSCALL_ULONG_TYPE
#define __FSWORD_T_TYPE	__SYSCALL_SLONG_TYPE
#define __OFF_T_TYPE		__SYSCALL_SLONG_TYPE
#define __OFF64_T_TYPE		__SQUAD_TYPE
#define __PID_T_TYPE		__S32_TYPE
#define __RLIM_T_TYPE		__SYSCALL_ULONG_TYPE
#define __RLIM64_T_TYPE		__UQUAD_TYPE
#define __BLKCNT_T_TYPE		__SYSCALL_SLONG_TYPE
#define __BLKCNT64_T_TYPE	__SQUAD_TYPE
#define __FSBLKCNT_T_TYPE	__SYSCALL_ULONG_TYPE
#define __FSBLKCNT64_T_TYPE	__UQUAD_TYPE
#define __FSFILCNT_T_TYPE	__SYSCALL_ULONG_TYPE
#define __FSFILCNT64_T_TYPE	__UQUAD_TYPE
#define __ID_T_TYPE		__U32_TYPE
#define __CLOCK_T_TYPE		__SYSCALL_SLONG_TYPE
#define __TIME_T_TYPE		__SYSCALL_SLONG_TYPE
#define __USECONDS_T_TYPE	__U32_TYPE
#define __SUSECONDS_T_TYPE	__SYSCALL_SLONG_TYPE
#define __SUSECONDS64_T_TYPE	__SQUAD_TYPE
#define __DADDR_T_TYPE		__S32_TYPE
#define __KEY_T_TYPE		__S32_TYPE
#define __CLOCKID_T_TYPE	__S32_TYPE
#define __TIMER_T_TYPE		void *
#define __BLKSIZE_T_TYPE	__SYSCALL_SLONG_TYPE
#define __FSID_T_TYPE		struct { int __val[2]; }
#define __SSIZE_T_TYPE		__SWORD_TYPE
#define __CPU_MASK_TYPE 	__SYSCALL_ULONG_TYPE
#define __OFF_T_MATCHES_OFF64_T	1
#define __INO_T_MATCHES_INO64_T	1
#define __RLIM_T_MATCHES_RLIM64_T	1
#define __STATFS_MATCHES_STATFS64  1
#define __KERNEL_OLD_TIMEVAL_MATCHES_TIMEVAL64 1
#define __FD_SETSIZE		1024
#endif

/* <bits/time64.h> */
#ifndef _BITS_TIME64_H
#define _BITS_TIME64_H 1
#define __TIME64_T_TYPE		__TIME_T_TYPE
#endif

typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct { int __val[2]; } __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;
typedef long int __suseconds64_t;
typedef int __daddr_t;
typedef int __key_t;
typedef int __clockid_t;
typedef void * __timer_t;
typedef long int __blksize_t;
typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;
typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;
typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;
typedef long int __fsword_t;
typedef long int __ssize_t;
typedef long int __syscall_slong_t;
typedef unsigned long int __syscall_ulong_t;

typedef __off64_t __loff_t;
typedef char *__caddr_t;
typedef long int __intptr_t;
typedef unsigned int __socklen_t;
typedef int __sig_atomic_t;

#undef __STD_TYPE

#endif /* _BITS_TYPES_H */
