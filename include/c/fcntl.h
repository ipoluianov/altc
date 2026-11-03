/* AltC <fcntl.h> - file control (glibc-compatible, x86-64 Linux).  */

#ifndef _FCNTL_H
#define _FCNTL_H 1

#include <features.h>
#include <_altc/types.h>

__BEGIN_DECLS

/* <bits/fcntl-linux.h> */
#define O_ACCMODE 0003
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_CREAT 0100
#define O_EXCL 0200
#define O_NOCTTY 0400
#define O_TRUNC 01000
#define O_APPEND 02000
#define O_NONBLOCK 04000
#define O_NDELAY O_NONBLOCK
#define O_SYNC 04010000
#define O_FSYNC O_SYNC
#define O_ASYNC 020000
#define __O_LARGEFILE 0
#define __O_DIRECTORY 0200000
#define __O_NOFOLLOW 0400000
#define __O_CLOEXEC 02000000
#define __O_DIRECT 040000
#define __O_NOATIME 01000000
#define __O_PATH 010000000
#define __O_DSYNC 010000
#define __O_TMPFILE (020000000 | __O_DIRECTORY)

#define F_GETLK F_GETLK64
#define F_SETLK F_SETLK64
#define F_SETLKW F_SETLKW64
#define F_GETLK64 5
#define F_SETLK64 6
#define F_SETLKW64 7
#define F_OFD_GETLK 36
#define F_OFD_SETLK 37
#define F_OFD_SETLKW 38

#ifdef __USE_LARGEFILE64
# define O_LARGEFILE __O_LARGEFILE
#endif

#define O_DIRECTORY __O_DIRECTORY
#define O_NOFOLLOW __O_NOFOLLOW
#define O_CLOEXEC __O_CLOEXEC
#define O_DSYNC __O_DSYNC
#define O_RSYNC O_SYNC
#ifdef __USE_GNU
# define O_DIRECT __O_DIRECT
# define O_NOATIME __O_NOATIME
# define O_PATH __O_PATH
# define O_TMPFILE __O_TMPFILE
#endif

#define F_DUPFD 0
#define F_GETFD 1
#define F_SETFD 2
#define F_GETFL 3
#define F_SETFL 4

#define __F_SETOWN 8
#define __F_GETOWN 9
#define F_SETOWN __F_SETOWN
#define F_GETOWN __F_GETOWN
#define __F_SETSIG 10
#define __F_GETSIG 11
#define __F_SETOWN_EX 15
#define __F_GETOWN_EX 16

#ifdef __USE_GNU
# define F_SETSIG __F_SETSIG
# define F_GETSIG __F_GETSIG
# define F_SETOWN_EX __F_SETOWN_EX
# define F_GETOWN_EX __F_GETOWN_EX
# define F_SETLEASE 1024
# define F_GETLEASE 1025
# define F_NOTIFY 1026
# define F_DUPFD_QUERY 1027
# define F_CREATED_QUERY 1028
# define F_SETPIPE_SZ 1031
# define F_GETPIPE_SZ 1032
# define F_ADD_SEALS 1033
# define F_GET_SEALS 1034
# define F_SEAL_SEAL 0x0001
# define F_SEAL_SHRINK 0x0002
# define F_SEAL_GROW 0x0004
# define F_SEAL_WRITE 0x0008
# define F_SEAL_FUTURE_WRITE 0x0010
# define F_SEAL_EXEC 0x0020
# define F_GET_RW_HINT 1035
# define F_SET_RW_HINT 1036
# define F_GET_FILE_RW_HINT 1037
# define F_SET_FILE_RW_HINT 1038
# define RWH_WRITE_LIFE_NOT_SET 0
# define RWF_WRITE_LIFE_NOT_SET RWH_WRITE_LIFE_NOT_SET
# define RWH_WRITE_LIFE_NONE 1
# define RWH_WRITE_LIFE_SHORT 2
# define RWH_WRITE_LIFE_MEDIUM 3
# define RWH_WRITE_LIFE_LONG 4
# define RWH_WRITE_LIFE_EXTREME 5
#endif

#define F_DUPFD_CLOEXEC 1030

#define FD_CLOEXEC 1

/* Also defined by <unistd.h>.  */
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0
#define F_ULOCK 0
#define F_LOCK 1
#define F_TLOCK 2
#define F_TEST 3

#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2

#define F_EXLCK 4
#define F_SHLCK 8

#define LOCK_SH 1
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8

#ifdef __USE_GNU
# define LOCK_MAND 32
# define LOCK_READ 64
# define LOCK_WRITE 128
# define LOCK_RW 192

# define DN_ACCESS 0x00000001
# define DN_MODIFY 0x00000002
# define DN_CREATE 0x00000004
# define DN_DELETE 0x00000008
# define DN_RENAME 0x00000010
# define DN_ATTRIB 0x00000020
# define DN_MULTISHOT 0x80000000

enum __pid_type
  {
    F_OWNER_TID = 0,
    F_OWNER_PID,
    F_OWNER_PGRP,
    F_OWNER_GID = F_OWNER_PGRP
  };

struct f_owner_ex
  {
    enum __pid_type type;
    __pid_t pid;
  };
#endif

#define FAPPEND O_APPEND
#define FFSYNC O_FSYNC
#define FASYNC O_ASYNC
#define FNONBLOCK O_NONBLOCK
#define FNDELAY O_NDELAY

#define __POSIX_FADV_DONTNEED 4
#define __POSIX_FADV_NOREUSE 5
#define POSIX_FADV_NORMAL 0
#define POSIX_FADV_RANDOM 1
#define POSIX_FADV_SEQUENTIAL 2
#define POSIX_FADV_WILLNEED 3
#define POSIX_FADV_DONTNEED __POSIX_FADV_DONTNEED
#define POSIX_FADV_NOREUSE __POSIX_FADV_NOREUSE

struct flock
  {
    short int l_type;
    short int l_whence;
    __off_t l_start;
    __off_t l_len;
    __pid_t l_pid;
  };

#ifdef __USE_LARGEFILE64
struct flock64
  {
    short int l_type;
    short int l_whence;
    __off64_t l_start;
    __off64_t l_len;
    __pid_t l_pid;
  };
#endif

#ifdef __USE_GNU
# define SYNC_FILE_RANGE_WAIT_BEFORE 1
# define SYNC_FILE_RANGE_WRITE 2
# define SYNC_FILE_RANGE_WAIT_AFTER 4
# define SYNC_FILE_RANGE_WRITE_AND_WAIT \
  (SYNC_FILE_RANGE_WRITE | SYNC_FILE_RANGE_WAIT_BEFORE \
   | SYNC_FILE_RANGE_WAIT_AFTER)
# define SPLICE_F_MOVE 1
# define SPLICE_F_NONBLOCK 2
# define SPLICE_F_MORE 4
# define SPLICE_F_GIFT 8
# define FALLOC_FL_KEEP_SIZE 0x01
# define FALLOC_FL_PUNCH_HOLE 0x02
# define FALLOC_FL_NO_HIDE_STALE 0x04
# define FALLOC_FL_COLLAPSE_RANGE 0x08
# define FALLOC_FL_ZERO_RANGE 0x10
# define FALLOC_FL_INSERT_RANGE 0x20
# define FALLOC_FL_UNSHARE_RANGE 0x40
# define FALLOC_FL_ALLOCATE_RANGE 0x00
# define MAX_HANDLE_SZ 128

struct file_handle
{
  unsigned int handle_bytes;
  int handle_type;
  unsigned char f_handle __flexarr;
};
#endif

/* *at() flags.  */
#define AT_FDCWD -100
#define AT_SYMLINK_NOFOLLOW 0x100
#define AT_REMOVEDIR 0x200
#define AT_SYMLINK_FOLLOW 0x400
#define AT_EACCESS 0x200
#ifdef __USE_GNU
# define AT_NO_AUTOMOUNT 0x800
# define AT_EMPTY_PATH 0x1000
# define AT_STATX_SYNC_TYPE 0x6000
# define AT_STATX_SYNC_AS_STAT 0x0000
# define AT_STATX_FORCE_SYNC 0x2000
# define AT_STATX_DONT_SYNC 0x4000
# define AT_RECURSIVE 0x8000
# define AT_HANDLE_FID AT_REMOVEDIR
#endif

#ifndef __mode_t_defined
typedef __mode_t mode_t;
# define __mode_t_defined
#endif
#ifndef __off_t_defined
typedef __off_t off_t;
# define __off_t_defined
#endif
#if defined __USE_LARGEFILE64 && !defined __off64_t_defined
typedef __off64_t off64_t;
# define __off64_t_defined
#endif
#ifndef __pid_t_defined
typedef __pid_t pid_t;
# define __pid_t_defined
#endif

#include <_altc/time_types.h>
#include <_altc/stat.h>

#ifndef _STDIO_H
# ifndef SEEK_SET
#  define SEEK_SET 0
#  define SEEK_CUR 1
#  define SEEK_END 2
# endif
#endif

extern int fcntl (int __fd, int __cmd, ...);
extern int open (const char *__file, int __oflag, ...);
extern int openat (int __fd, const char *__file, int __oflag, ...);
extern int creat (const char *__file, mode_t __mode);
#ifdef __USE_LARGEFILE64
extern int fcntl64 (int __fd, int __cmd, ...);
extern int open64 (const char *__file, int __oflag, ...);
extern int openat64 (int __fd, const char *__file, int __oflag, ...);
extern int creat64 (const char *__file, mode_t __mode);
#endif

extern int lockf (int __fd, int __cmd, off_t __len);
extern int posix_fadvise (int __fd, off_t __offset, off_t __len,
                          int __advise);
extern int posix_fallocate (int __fd, off_t __offset, off_t __len);
#ifdef __USE_LARGEFILE64
extern int lockf64 (int __fd, int __cmd, off64_t __len);
extern int posix_fadvise64 (int __fd, off64_t __offset, off64_t __len,
                            int __advise);
extern int posix_fallocate64 (int __fd, off64_t __offset, off64_t __len);
#endif

#ifdef __USE_GNU
# include <_altc/iovec.h>
extern __ssize_t readahead (int __fd, __off64_t __offset, size_t __count);
extern int sync_file_range (int __fd, __off64_t __offset, __off64_t __count,
                            unsigned int __flags);
extern __ssize_t vmsplice (int __fdout, const struct iovec *__iov,
                           size_t __count, unsigned int __flags);
extern __ssize_t splice (int __fdin, __off64_t *__offin, int __fdout,
                         __off64_t *__offout, size_t __len,
                         unsigned int __flags);
extern __ssize_t tee (int __fdin, int __fdout, size_t __len,
                      unsigned int __flags);
extern int fallocate (int __fd, int __mode, __off_t __offset, __off_t __len);
extern int fallocate64 (int __fd, int __mode, __off64_t __offset,
                        __off64_t __len);
extern int name_to_handle_at (int __dfd, const char *__name,
                              struct file_handle *__handle, int *__mnt_id,
                              int __flags);
extern int open_by_handle_at (int __mountdirfd, struct file_handle *__handle,
                              int __flags);
#endif

__END_DECLS

#endif /* fcntl.h */
