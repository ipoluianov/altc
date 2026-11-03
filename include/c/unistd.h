/* AltC <unistd.h> - POSIX standard symbolic constants and types
   (glibc-compatible, x86-64 Linux).  */

#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <features.h>

__BEGIN_DECLS

/* POSIX/X-Open version and option macros (<bits/posix_opt.h>,
   <bits/environments.h>); values as reported by glibc 2.41.  */
#define __ILP32_OFF32_CFLAGS "-m32"
#define __ILP32_OFF32_LDFLAGS "-m32"
#define __ILP32_OFFBIG_CFLAGS "-m32 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64"
#define __ILP32_OFFBIG_LDFLAGS "-m32"
#define _LFS64_ASYNCHRONOUS_IO 1
#define _LFS64_LARGEFILE 1
#define _LFS64_STDIO 1
#define _LFS_ASYNCHRONOUS_IO 1
#define _LFS_LARGEFILE 1
#define __LP64_OFF64_CFLAGS "-m64"
#define __LP64_OFF64_LDFLAGS "-m64"
#define _POSIX2_C_BIND __POSIX2_THIS_VERSION
#define _POSIX2_C_DEV __POSIX2_THIS_VERSION
#define _POSIX2_CHAR_TERM 200809L
#define _POSIX2_C_VERSION __POSIX2_THIS_VERSION
#define _POSIX2_LOCALEDEF __POSIX2_THIS_VERSION
#define _POSIX2_SW_DEV __POSIX2_THIS_VERSION
#define __POSIX2_THIS_VERSION 200809L
#define _POSIX2_VERSION __POSIX2_THIS_VERSION
#define _POSIX_ADVISORY_INFO 200809L
#define _POSIX_ASYNCHRONOUS_IO 200809L
#define _POSIX_ASYNC_IO 1
#define _POSIX_BARRIERS 200809L
#define _POSIX_CHOWN_RESTRICTED 0
#define _POSIX_CLOCK_SELECTION 200809L
#define _POSIX_CPUTIME 0
#define _POSIX_FSYNC 200809L
#define _POSIX_IPV6 200809L
#define _POSIX_JOB_CONTROL 1
#define _POSIX_MAPPED_FILES 200809L
#define _POSIX_MEMLOCK 200809L
#define _POSIX_MEMLOCK_RANGE 200809L
#define _POSIX_MEMORY_PROTECTION 200809L
#define _POSIX_MESSAGE_PASSING 200809L
#define _POSIX_MONOTONIC_CLOCK 0
#define _POSIX_NO_TRUNC 1
#define _POSIX_PRIORITIZED_IO 200809L
#define _POSIX_PRIORITY_SCHEDULING 200809L
#define _POSIX_RAW_SOCKETS 200809L
#define _POSIX_READER_WRITER_LOCKS 200809L
#define _POSIX_REALTIME_SIGNALS 200809L
#define _POSIX_REENTRANT_FUNCTIONS 1
#define _POSIX_REGEXP 1
#define _POSIX_SAVED_IDS 1
#define _POSIX_SEMAPHORES 200809L
#define _POSIX_SHARED_MEMORY_OBJECTS 200809L
#define _POSIX_SHELL 1
#define _POSIX_SPAWN 200809L
#define _POSIX_SPIN_LOCKS 200809L
#define _POSIX_SPORADIC_SERVER -1
#define _POSIX_SYNCHRONIZED_IO 200809L
#define _POSIX_THREAD_ATTR_STACKADDR 200809L
#define _POSIX_THREAD_ATTR_STACKSIZE 200809L
#define _POSIX_THREAD_CPUTIME 0
#define _POSIX_THREAD_PRIO_INHERIT 200809L
#define _POSIX_THREAD_PRIO_PROTECT 200809L
#define _POSIX_THREAD_PRIORITY_SCHEDULING 200809L
#define _POSIX_THREAD_PROCESS_SHARED 200809L
#define _POSIX_THREAD_ROBUST_PRIO_INHERIT 200809L
#define _POSIX_THREAD_ROBUST_PRIO_PROTECT -1
#define _POSIX_THREADS 200809L
#define _POSIX_THREAD_SAFE_FUNCTIONS 200809L
#define _POSIX_THREAD_SPORADIC_SERVER -1
#define _POSIX_TIMEOUTS 200809L
#define _POSIX_TIMERS 200809L
#define _POSIX_TRACE -1
#define _POSIX_TRACE_EVENT_FILTER -1
#define _POSIX_TRACE_INHERIT -1
#define _POSIX_TRACE_LOG -1
#define _POSIX_TYPED_MEMORY_OBJECTS -1
#define _POSIX_V6_LP64_OFF64 1
#define _POSIX_V6_LPBIG_OFFBIG -1
#define _POSIX_V7_LP64_OFF64 1
#define _POSIX_V7_LPBIG_OFFBIG -1
#define _POSIX_VDISABLE '\0'
#define _POSIX_VERSION 200809L
#define _XOPEN_ENH_I18N 1
#define _XOPEN_LEGACY 1
#define _XOPEN_REALTIME 1
#define _XOPEN_REALTIME_THREADS 1
#define _XOPEN_SHM 1
#define _XOPEN_UNIX 1
#define _XOPEN_VERSION 700
#define _XOPEN_XCU_VERSION 4
#define _XOPEN_XPG2 1
#define _XOPEN_XPG3 1
#define _XOPEN_XPG4 1

#define _XBS5_LP64_OFF64 1
#define _XBS5_LPBIG_OFFBIG -1

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#include <_altc/types.h>

#ifndef __ssize_t_defined
typedef __ssize_t ssize_t;
# define __ssize_t_defined
#endif

#define __need_size_t
#define __need_NULL
#include <stddef.h>

#ifndef __gid_t_defined
typedef __gid_t gid_t;
# define __gid_t_defined
#endif
#ifndef __uid_t_defined
typedef __uid_t uid_t;
# define __uid_t_defined
#endif
#ifndef __off_t_defined
typedef __off_t off_t;
# define __off_t_defined
#endif
#if defined __USE_LARGEFILE64 && !defined __off64_t_defined
typedef __off64_t off64_t;
# define __off64_t_defined
#endif
#ifndef __useconds_t_defined
typedef __useconds_t useconds_t;
# define __useconds_t_defined
#endif
#ifndef __pid_t_defined
typedef __pid_t pid_t;
# define __pid_t_defined
#endif
#ifndef __intptr_t_defined
typedef __intptr_t intptr_t;
# define __intptr_t_defined
#endif
#ifndef __socklen_t_defined
typedef __socklen_t socklen_t;
# define __socklen_t_defined
#endif

#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0

extern int access (const char *__name, int __type);
#ifdef __USE_GNU
extern int euidaccess (const char *__name, int __type);
extern int eaccess (const char *__name, int __type);
extern int execveat (int __fd, const char *__path, char *const __argv[],
                     char *const __envp[], int __flags);
#endif
extern int faccessat (int __fd, const char *__file, int __type, int __flag);

#ifndef _STDIO_H
# define SEEK_SET 0
# define SEEK_CUR 1
# define SEEK_END 2
# ifdef __USE_GNU
#  define SEEK_DATA 3
#  define SEEK_HOLE 4
# endif
#endif

#ifndef L_SET
# define L_SET SEEK_SET
# define L_INCR SEEK_CUR
# define L_XTND SEEK_END
#endif

extern __off_t lseek (int __fd, __off_t __offset, int __whence);
#ifdef __USE_LARGEFILE64
extern __off64_t lseek64 (int __fd, __off64_t __offset, int __whence);
#endif

extern int close (int __fd);
extern void closefrom (int __lowfd);

extern ssize_t read (int __fd, void *__buf, size_t __nbytes);
extern ssize_t write (int __fd, const void *__buf, size_t __n);
extern ssize_t pread (int __fd, void *__buf, size_t __nbytes,
                      __off_t __offset);
extern ssize_t pwrite (int __fd, const void *__buf, size_t __n,
                       __off_t __offset);
#ifdef __USE_LARGEFILE64
extern ssize_t pread64 (int __fd, void *__buf, size_t __nbytes,
                        __off64_t __offset);
extern ssize_t pwrite64 (int __fd, const void *__buf, size_t __n,
                         __off64_t __offset);
#endif

extern int pipe (int __pipedes[2]);
#ifdef __USE_GNU
extern int pipe2 (int __pipedes[2], int __flags);
#endif

extern unsigned int alarm (unsigned int __seconds);
extern unsigned int sleep (unsigned int __seconds);
extern __useconds_t ualarm (__useconds_t __value, __useconds_t __interval);
extern int usleep (__useconds_t __useconds);
extern int pause (void);

extern int chown (const char *__file, __uid_t __owner, __gid_t __group);
extern int fchown (int __fd, __uid_t __owner, __gid_t __group);
extern int lchown (const char *__file, __uid_t __owner, __gid_t __group);
extern int fchownat (int __fd, const char *__file, __uid_t __owner,
                     __gid_t __group, int __flag);

extern int chdir (const char *__path);
extern int fchdir (int __fd);

extern char *getcwd (char *__buf, size_t __size);
#ifdef __USE_GNU
extern char *get_current_dir_name (void);
#endif
extern char *getwd (char *__buf) __attribute__ ((__deprecated__));

extern int dup (int __fd);
extern int dup2 (int __fd, int __fd2);
#ifdef __USE_GNU
extern int dup3 (int __fd, int __fd2, int __flags);
#endif

extern char **__environ;
#ifdef __USE_GNU
extern char **environ;
#endif

extern int execve (const char *__path, char *const __argv[],
                   char *const __envp[]);
extern int fexecve (int __fd, char *const __argv[], char *const __envp[]);
extern int execv (const char *__path, char *const __argv[]);
extern int execle (const char *__path, const char *__arg, ...);
extern int execl (const char *__path, const char *__arg, ...);
extern int execvp (const char *__file, char *const __argv[]);
extern int execlp (const char *__file, const char *__arg, ...);
#ifdef __USE_GNU
extern int execvpe (const char *__file, char *const __argv[],
                    char *const __envp[]);
#endif

extern int nice (int __inc);

extern void _exit (int __status) __attribute__ ((__noreturn__));

/* Values for the NAME argument to pathconf, sysconf and confstr
   (<bits/confname.h>).  */
#define _PC_LINK_MAX 0
#define _PC_MAX_CANON 1
#define _PC_MAX_INPUT 2
#define _PC_NAME_MAX 3
#define _PC_PATH_MAX 4
#define _PC_PIPE_BUF 5
#define _PC_CHOWN_RESTRICTED 6
#define _PC_NO_TRUNC 7
#define _PC_VDISABLE 8
#define _PC_SYNC_IO 9
#define _PC_ASYNC_IO 10
#define _PC_PRIO_IO 11
#define _PC_SOCK_MAXBUF 12
#define _PC_FILESIZEBITS 13
#define _PC_REC_INCR_XFER_SIZE 14
#define _PC_REC_MAX_XFER_SIZE 15
#define _PC_REC_MIN_XFER_SIZE 16
#define _PC_REC_XFER_ALIGN 17
#define _PC_ALLOC_SIZE_MIN 18
#define _PC_SYMLINK_MAX 19
#define _PC_2_SYMLINKS 20

#define _SC_ARG_MAX 0
#define _SC_CHILD_MAX 1
#define _SC_CLK_TCK 2
#define _SC_NGROUPS_MAX 3
#define _SC_OPEN_MAX 4
#define _SC_STREAM_MAX 5
#define _SC_TZNAME_MAX 6
#define _SC_JOB_CONTROL 7
#define _SC_SAVED_IDS 8
#define _SC_REALTIME_SIGNALS 9
#define _SC_PRIORITY_SCHEDULING 10
#define _SC_TIMERS 11
#define _SC_ASYNCHRONOUS_IO 12
#define _SC_PRIORITIZED_IO 13
#define _SC_SYNCHRONIZED_IO 14
#define _SC_FSYNC 15
#define _SC_MAPPED_FILES 16
#define _SC_MEMLOCK 17
#define _SC_MEMLOCK_RANGE 18
#define _SC_MEMORY_PROTECTION 19
#define _SC_MESSAGE_PASSING 20
#define _SC_SEMAPHORES 21
#define _SC_SHARED_MEMORY_OBJECTS 22
#define _SC_AIO_LISTIO_MAX 23
#define _SC_AIO_MAX 24
#define _SC_AIO_PRIO_DELTA_MAX 25
#define _SC_DELAYTIMER_MAX 26
#define _SC_MQ_OPEN_MAX 27
#define _SC_MQ_PRIO_MAX 28
#define _SC_VERSION 29
#define _SC_PAGE_SIZE 30
#define _SC_PAGESIZE 30
#define _SC_RTSIG_MAX 31
#define _SC_SEM_NSEMS_MAX 32
#define _SC_SEM_VALUE_MAX 33
#define _SC_SIGQUEUE_MAX 34
#define _SC_TIMER_MAX 35
#define _SC_BC_BASE_MAX 36
#define _SC_BC_DIM_MAX 37
#define _SC_BC_SCALE_MAX 38
#define _SC_BC_STRING_MAX 39
#define _SC_COLL_WEIGHTS_MAX 40
#define _SC_EQUIV_CLASS_MAX 41
#define _SC_EXPR_NEST_MAX 42
#define _SC_LINE_MAX 43
#define _SC_RE_DUP_MAX 44
#define _SC_CHARCLASS_NAME_MAX 45
#define _SC_2_VERSION 46
#define _SC_2_C_BIND 47
#define _SC_2_C_DEV 48
#define _SC_2_FORT_DEV 49
#define _SC_2_FORT_RUN 50
#define _SC_2_SW_DEV 51
#define _SC_2_LOCALEDEF 52
#define _SC_PII 53
#define _SC_PII_XTI 54
#define _SC_PII_SOCKET 55
#define _SC_PII_INTERNET 56
#define _SC_PII_OSI 57
#define _SC_POLL 58
#define _SC_SELECT 59
#define _SC_IOV_MAX 60
#define _SC_UIO_MAXIOV 60
#define _SC_PII_INTERNET_STREAM 61
#define _SC_PII_INTERNET_DGRAM 62
#define _SC_PII_OSI_COTS 63
#define _SC_PII_OSI_CLTS 64
#define _SC_PII_OSI_M 65
#define _SC_T_IOV_MAX 66
#define _SC_THREADS 67
#define _SC_THREAD_SAFE_FUNCTIONS 68
#define _SC_GETGR_R_SIZE_MAX 69
#define _SC_GETPW_R_SIZE_MAX 70
#define _SC_LOGIN_NAME_MAX 71
#define _SC_TTY_NAME_MAX 72
#define _SC_THREAD_DESTRUCTOR_ITERATIONS 73
#define _SC_THREAD_KEYS_MAX 74
#define _SC_THREAD_STACK_MIN 75
#define _SC_THREAD_THREADS_MAX 76
#define _SC_THREAD_ATTR_STACKADDR 77
#define _SC_THREAD_ATTR_STACKSIZE 78
#define _SC_THREAD_PRIORITY_SCHEDULING 79
#define _SC_THREAD_PRIO_INHERIT 80
#define _SC_THREAD_PRIO_PROTECT 81
#define _SC_THREAD_PROCESS_SHARED 82
#define _SC_NPROCESSORS_CONF 83
#define _SC_NPROCESSORS_ONLN 84
#define _SC_PHYS_PAGES 85
#define _SC_AVPHYS_PAGES 86
#define _SC_ATEXIT_MAX 87
#define _SC_PASS_MAX 88
#define _SC_XOPEN_VERSION 89
#define _SC_XOPEN_XCU_VERSION 90
#define _SC_XOPEN_UNIX 91
#define _SC_XOPEN_CRYPT 92
#define _SC_XOPEN_ENH_I18N 93
#define _SC_XOPEN_SHM 94
#define _SC_2_CHAR_TERM 95
#define _SC_2_C_VERSION 96
#define _SC_2_UPE 97
#define _SC_XOPEN_XPG2 98
#define _SC_XOPEN_XPG3 99
#define _SC_XOPEN_XPG4 100
#define _SC_CHAR_BIT 101
#define _SC_CHAR_MAX 102
#define _SC_CHAR_MIN 103
#define _SC_INT_MAX 104
#define _SC_INT_MIN 105
#define _SC_LONG_BIT 106
#define _SC_WORD_BIT 107
#define _SC_MB_LEN_MAX 108
#define _SC_NZERO 109
#define _SC_SSIZE_MAX 110
#define _SC_SCHAR_MAX 111
#define _SC_SCHAR_MIN 112
#define _SC_SHRT_MAX 113
#define _SC_SHRT_MIN 114
#define _SC_UCHAR_MAX 115
#define _SC_UINT_MAX 116
#define _SC_ULONG_MAX 117
#define _SC_USHRT_MAX 118
#define _SC_NL_ARGMAX 119
#define _SC_NL_LANGMAX 120
#define _SC_NL_MSGMAX 121
#define _SC_NL_NMAX 122
#define _SC_NL_SETMAX 123
#define _SC_NL_TEXTMAX 124
#define _SC_XBS5_ILP32_OFF32 125
#define _SC_XBS5_ILP32_OFFBIG 126
#define _SC_XBS5_LP64_OFF64 127
#define _SC_XBS5_LPBIG_OFFBIG 128
#define _SC_XOPEN_LEGACY 129
#define _SC_XOPEN_REALTIME 130
#define _SC_XOPEN_REALTIME_THREADS 131
#define _SC_ADVISORY_INFO 132
#define _SC_BARRIERS 133
#define _SC_BASE 134
#define _SC_C_LANG_SUPPORT 135
#define _SC_C_LANG_SUPPORT_R 136
#define _SC_CLOCK_SELECTION 137
#define _SC_CPUTIME 138
#define _SC_THREAD_CPUTIME 139
#define _SC_DEVICE_IO 140
#define _SC_DEVICE_SPECIFIC 141
#define _SC_DEVICE_SPECIFIC_R 142
#define _SC_FD_MGMT 143
#define _SC_FIFO 144
#define _SC_PIPE 145
#define _SC_FILE_ATTRIBUTES 146
#define _SC_FILE_LOCKING 147
#define _SC_FILE_SYSTEM 148
#define _SC_MONOTONIC_CLOCK 149
#define _SC_MULTI_PROCESS 150
#define _SC_SINGLE_PROCESS 151
#define _SC_NETWORKING 152
#define _SC_READER_WRITER_LOCKS 153
#define _SC_SPIN_LOCKS 154
#define _SC_REGEXP 155
#define _SC_REGEX_VERSION 156
#define _SC_SHELL 157
#define _SC_SIGNALS 158
#define _SC_SPAWN 159
#define _SC_SPORADIC_SERVER 160
#define _SC_THREAD_SPORADIC_SERVER 161
#define _SC_SYSTEM_DATABASE 162
#define _SC_SYSTEM_DATABASE_R 163
#define _SC_TIMEOUTS 164
#define _SC_TYPED_MEMORY_OBJECTS 165
#define _SC_USER_GROUPS 166
#define _SC_USER_GROUPS_R 167
#define _SC_2_PBS 168
#define _SC_2_PBS_ACCOUNTING 169
#define _SC_2_PBS_LOCATE 170
#define _SC_2_PBS_MESSAGE 171
#define _SC_2_PBS_TRACK 172
#define _SC_SYMLOOP_MAX 173
#define _SC_STREAMS 174
#define _SC_2_PBS_CHECKPOINT 175
#define _SC_V6_ILP32_OFF32 176
#define _SC_V6_ILP32_OFFBIG 177
#define _SC_V6_LP64_OFF64 178
#define _SC_V6_LPBIG_OFFBIG 179
#define _SC_HOST_NAME_MAX 180
#define _SC_TRACE 181
#define _SC_TRACE_EVENT_FILTER 182
#define _SC_TRACE_INHERIT 183
#define _SC_TRACE_LOG 184
#define _SC_LEVEL1_ICACHE_SIZE 185
#define _SC_LEVEL1_ICACHE_ASSOC 186
#define _SC_LEVEL1_ICACHE_LINESIZE 187
#define _SC_LEVEL1_DCACHE_SIZE 188
#define _SC_LEVEL1_DCACHE_ASSOC 189
#define _SC_LEVEL1_DCACHE_LINESIZE 190
#define _SC_LEVEL2_CACHE_SIZE 191
#define _SC_LEVEL2_CACHE_ASSOC 192
#define _SC_LEVEL2_CACHE_LINESIZE 193
#define _SC_LEVEL3_CACHE_SIZE 194
#define _SC_LEVEL3_CACHE_ASSOC 195
#define _SC_LEVEL3_CACHE_LINESIZE 196
#define _SC_LEVEL4_CACHE_SIZE 197
#define _SC_LEVEL4_CACHE_ASSOC 198
#define _SC_LEVEL4_CACHE_LINESIZE 199
#define _SC_IPV6 235
#define _SC_RAW_SOCKETS 236
#define _SC_V7_ILP32_OFF32 237
#define _SC_V7_ILP32_OFFBIG 238
#define _SC_V7_LP64_OFF64 239
#define _SC_V7_LPBIG_OFFBIG 240
#define _SC_SS_REPL_MAX 241
#define _SC_TRACE_EVENT_NAME_MAX 242
#define _SC_TRACE_NAME_MAX 243
#define _SC_TRACE_SYS_MAX 244
#define _SC_TRACE_USER_EVENT_MAX 245
#define _SC_XOPEN_STREAMS 246
#define _SC_THREAD_ROBUST_PRIO_INHERIT 247
#define _SC_THREAD_ROBUST_PRIO_PROTECT 248
#define _SC_MINSIGSTKSZ 249
#define _SC_SIGSTKSZ 250

#define _CS_PATH 0
#define _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS 1
#define _CS_V6_WIDTH_RESTRICTED_ENVS 1
#define _CS_GNU_LIBC_VERSION 2
#define _CS_GNU_LIBPTHREAD_VERSION 3
#define _CS_POSIX_V5_WIDTH_RESTRICTED_ENVS 4
#define _CS_V5_WIDTH_RESTRICTED_ENVS 4
#define _CS_POSIX_V7_WIDTH_RESTRICTED_ENVS 5
#define _CS_V7_WIDTH_RESTRICTED_ENVS 5
#define _CS_LFS_CFLAGS 1000
#define _CS_LFS_LDFLAGS 1001
#define _CS_LFS_LIBS 1002
#define _CS_LFS_LINTFLAGS 1003
#define _CS_LFS64_CFLAGS 1004
#define _CS_LFS64_LDFLAGS 1005
#define _CS_LFS64_LIBS 1006
#define _CS_LFS64_LINTFLAGS 1007
#define _CS_XBS5_ILP32_OFF32_CFLAGS 1100
#define _CS_XBS5_ILP32_OFF32_LDFLAGS 1101
#define _CS_XBS5_ILP32_OFF32_LIBS 1102
#define _CS_XBS5_ILP32_OFF32_LINTFLAGS 1103
#define _CS_XBS5_ILP32_OFFBIG_CFLAGS 1104
#define _CS_XBS5_ILP32_OFFBIG_LDFLAGS 1105
#define _CS_XBS5_ILP32_OFFBIG_LIBS 1106
#define _CS_XBS5_ILP32_OFFBIG_LINTFLAGS 1107
#define _CS_XBS5_LP64_OFF64_CFLAGS 1108
#define _CS_XBS5_LP64_OFF64_LDFLAGS 1109
#define _CS_XBS5_LP64_OFF64_LIBS 1110
#define _CS_XBS5_LP64_OFF64_LINTFLAGS 1111
#define _CS_XBS5_LPBIG_OFFBIG_CFLAGS 1112
#define _CS_XBS5_LPBIG_OFFBIG_LDFLAGS 1113
#define _CS_XBS5_LPBIG_OFFBIG_LIBS 1114
#define _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS 1115
#define _CS_POSIX_V6_ILP32_OFF32_CFLAGS 1116
#define _CS_POSIX_V6_ILP32_OFF32_LDFLAGS 1117
#define _CS_POSIX_V6_ILP32_OFF32_LIBS 1118
#define _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS 1119
#define _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS 1120
#define _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS 1121
#define _CS_POSIX_V6_ILP32_OFFBIG_LIBS 1122
#define _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS 1123
#define _CS_POSIX_V6_LP64_OFF64_CFLAGS 1124
#define _CS_POSIX_V6_LP64_OFF64_LDFLAGS 1125
#define _CS_POSIX_V6_LP64_OFF64_LIBS 1126
#define _CS_POSIX_V6_LP64_OFF64_LINTFLAGS 1127
#define _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS 1128
#define _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS 1129
#define _CS_POSIX_V6_LPBIG_OFFBIG_LIBS 1130
#define _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS 1131
#define _CS_POSIX_V7_ILP32_OFF32_CFLAGS 1132
#define _CS_POSIX_V7_ILP32_OFF32_LDFLAGS 1133
#define _CS_POSIX_V7_ILP32_OFF32_LIBS 1134
#define _CS_POSIX_V7_ILP32_OFF32_LINTFLAGS 1135
#define _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS 1136
#define _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS 1137
#define _CS_POSIX_V7_ILP32_OFFBIG_LIBS 1138
#define _CS_POSIX_V7_ILP32_OFFBIG_LINTFLAGS 1139
#define _CS_POSIX_V7_LP64_OFF64_CFLAGS 1140
#define _CS_POSIX_V7_LP64_OFF64_LDFLAGS 1141
#define _CS_POSIX_V7_LP64_OFF64_LIBS 1142
#define _CS_POSIX_V7_LP64_OFF64_LINTFLAGS 1143
#define _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS 1144
#define _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS 1145
#define _CS_POSIX_V7_LPBIG_OFFBIG_LIBS 1146
#define _CS_POSIX_V7_LPBIG_OFFBIG_LINTFLAGS 1147
#define _CS_V6_ENV 1148
#define _CS_V7_ENV 1149

extern long int pathconf (const char *__path, int __name);
extern long int fpathconf (int __fd, int __name);
extern long int sysconf (int __name);
extern size_t confstr (int __name, char *__buf, size_t __len);

extern __pid_t getpid (void);
extern __pid_t getppid (void);
extern __pid_t getpgrp (void);
extern __pid_t __getpgid (__pid_t __pid);
extern __pid_t getpgid (__pid_t __pid);
extern int setpgid (__pid_t __pid, __pid_t __pgid);
extern int setpgrp (void);
extern __pid_t setsid (void);
extern __pid_t getsid (__pid_t __pid);

extern __uid_t getuid (void);
extern __uid_t geteuid (void);
extern __gid_t getgid (void);
extern __gid_t getegid (void);
extern int getgroups (int __size, __gid_t __list[]);
#ifdef __USE_GNU
extern int group_member (__gid_t __gid);
#endif
extern int setuid (__uid_t __uid);
extern int setreuid (__uid_t __ruid, __uid_t __euid);
extern int seteuid (__uid_t __uid);
extern int setgid (__gid_t __gid);
extern int setregid (__gid_t __rgid, __gid_t __egid);
extern int setegid (__gid_t __gid);
#ifdef __USE_GNU
extern int getresuid (__uid_t *__ruid, __uid_t *__euid, __uid_t *__suid);
extern int getresgid (__gid_t *__rgid, __gid_t *__egid, __gid_t *__sgid);
extern int setresuid (__uid_t __ruid, __uid_t __euid, __uid_t __suid);
extern int setresgid (__gid_t __rgid, __gid_t __egid, __gid_t __sgid);
#endif

extern __pid_t fork (void);
extern __pid_t vfork (void) __attribute__ ((__returns_twice__));
#ifdef __USE_GNU
extern __pid_t _Fork (void);
#endif

extern char *ttyname (int __fd);
extern int ttyname_r (int __fd, char *__buf, size_t __buflen);
extern int isatty (int __fd);
extern int ttyslot (void);

extern int link (const char *__from, const char *__to);
extern int linkat (int __fromfd, const char *__from, int __tofd,
                   const char *__to, int __flags);
extern int symlink (const char *__from, const char *__to);
extern ssize_t readlink (const char *__restrict __path,
                         char *__restrict __buf, size_t __len);
extern int symlinkat (const char *__from, int __tofd, const char *__to);
extern ssize_t readlinkat (int __fd, const char *__restrict __path,
                           char *__restrict __buf, size_t __len);
extern int unlink (const char *__name);
extern int unlinkat (int __fd, const char *__name, int __flag);
extern int rmdir (const char *__path);

extern __pid_t tcgetpgrp (int __fd);
extern int tcsetpgrp (int __fd, __pid_t __pgrp_id);

extern char *getlogin (void);
extern int getlogin_r (char *__name, size_t __name_len);
extern int setlogin (const char *__name);

/* POSIX getopt (also in <getopt.h>).  */
#ifndef _GETOPT_POSIX_H
# define _GETOPT_POSIX_H 1
# ifndef _GETOPT_CORE_H
#  define _GETOPT_CORE_H 1
extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;
extern int getopt (int ___argc, char *const *___argv, const char *__shortopts);
# endif
#endif

extern int gethostname (char *__name, size_t __len);
extern int sethostname (const char *__name, size_t __len);
extern int sethostid (long int __id);
extern int getdomainname (char *__name, size_t __len);
extern int setdomainname (const char *__name, size_t __len);
extern int vhangup (void);
extern int revoke (const char *__file);
extern int profil (unsigned short int *__sample_buffer, size_t __size,
                   size_t __offset, unsigned int __scale);
extern int acct (const char *__name);
extern char *getusershell (void);
extern void endusershell (void);
extern void setusershell (void);
extern int daemon (int __nochdir, int __noclose);
extern int chroot (const char *__path);
extern char *getpass (const char *__prompt);

extern int fsync (int __fd);
#ifdef __USE_GNU
extern int syncfs (int __fd);
#endif
extern long int gethostid (void);
extern void sync (void);
extern int getpagesize (void) __attribute__ ((__const__));
extern int getdtablesize (void);

extern int truncate (const char *__file, __off_t __length);
extern int ftruncate (int __fd, __off_t __length);
#ifdef __USE_LARGEFILE64
extern int truncate64 (const char *__file, __off64_t __length);
extern int ftruncate64 (int __fd, __off64_t __length);
#endif

extern int brk (void *__addr);
extern void *sbrk (intptr_t __delta);

extern long int syscall (long int __sysno, ...);

#define F_ULOCK 0
#define F_LOCK 1
#define F_TLOCK 2
#define F_TEST 3
extern int lockf (int __fd, int __cmd, __off_t __len);
#ifdef __USE_LARGEFILE64
extern int lockf64 (int __fd, int __cmd, __off64_t __len);
#endif

extern int fdatasync (int __fildes);

extern void swab (const void *__restrict __from, void *__restrict __to,
                  ssize_t __n);

extern int getentropy (void *__buffer, size_t __length);

#ifdef __USE_GNU
extern ssize_t copy_file_range (int __infd, __off64_t *__pinoff,
                                int __outfd, __off64_t *__poutoff,
                                size_t __length, unsigned int __flags);
extern __pid_t gettid (void);
# define CLOSE_RANGE_UNSHARE (1U << 1)
# define CLOSE_RANGE_CLOEXEC (1U << 2)
extern int close_range (unsigned int __fd, unsigned int __max_fd,
                        int __flags);
#endif

__END_DECLS

#endif /* unistd.h */
