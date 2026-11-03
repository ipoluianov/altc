/* AltC private header: struct stat and file mode bits.  Mirrors glibc's
   <bits/stat.h> and <bits/struct_stat.h> for x86-64 (same guards).  */

#include <_altc/types.h>
#include <_altc/time_types.h>

#ifndef _BITS_STRUCT_STAT_H
#define _BITS_STRUCT_STAT_H 1
struct stat
  {
    __dev_t st_dev;
    __ino_t st_ino;
    __nlink_t st_nlink;
    __mode_t st_mode;
    __uid_t st_uid;
    __gid_t st_gid;
    int __pad0;
    __dev_t st_rdev;
    __off_t st_size;
    __blksize_t st_blksize;
    __blkcnt_t st_blocks;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
#define st_atime st_atim.tv_sec
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
    __syscall_slong_t __glibc_reserved[3];
  };

#ifdef __USE_LARGEFILE64
struct stat64
  {
    __dev_t st_dev;
    __ino64_t st_ino;
    __nlink_t st_nlink;
    __mode_t st_mode;
    __uid_t st_uid;
    __gid_t st_gid;
    int __pad0;
    __dev_t st_rdev;
    __off_t st_size;
    __blksize_t st_blksize;
    __blkcnt64_t st_blocks;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    __syscall_slong_t __glibc_reserved[3];
  };
#endif

#define _STATBUF_ST_BLKSIZE
#define _STATBUF_ST_RDEV
#define _STATBUF_ST_NSEC
#endif /* _BITS_STRUCT_STAT_H */

#ifndef _BITS_STAT_H
#define _BITS_STAT_H 1
#define __S_IFMT 0170000
#define __S_IFDIR 0040000
#define __S_IFCHR 0020000
#define __S_IFBLK 0060000
#define __S_IFREG 0100000
#define __S_IFIFO 0010000
#define __S_IFLNK 0120000
#define __S_IFSOCK 0140000
#define __S_TYPEISMQ(buf) ((buf)->st_mode - (buf)->st_mode)
#define __S_TYPEISSEM(buf) ((buf)->st_mode - (buf)->st_mode)
#define __S_TYPEISSHM(buf) ((buf)->st_mode - (buf)->st_mode)
#define __S_ISUID 04000
#define __S_ISGID 02000
#define __S_ISVTX 01000
#define __S_IREAD 0400
#define __S_IWRITE 0200
#define __S_IEXEC 0100
#define UTIME_NOW ((1l << 30) - 1l)
#define UTIME_OMIT ((1l << 30) - 2l)
#endif /* _BITS_STAT_H */

/* Public mode constants (shared by <sys/stat.h> and <fcntl.h>).  */
#ifndef S_IFMT
# define S_IFMT __S_IFMT
# define S_IFDIR __S_IFDIR
# define S_IFCHR __S_IFCHR
# define S_IFBLK __S_IFBLK
# define S_IFREG __S_IFREG
# define S_IFIFO __S_IFIFO
# define S_IFLNK __S_IFLNK
# define S_IFSOCK __S_IFSOCK
#endif
#ifndef S_ISUID
# define S_ISUID __S_ISUID
# define S_ISGID __S_ISGID
# define S_ISVTX __S_ISVTX
#endif
#ifndef S_IRUSR
# define S_IRUSR __S_IREAD
# define S_IWUSR __S_IWRITE
# define S_IXUSR __S_IEXEC
# define S_IRWXU (__S_IREAD|__S_IWRITE|__S_IEXEC)
# define S_IRGRP (S_IRUSR >> 3)
# define S_IWGRP (S_IWUSR >> 3)
# define S_IXGRP (S_IXUSR >> 3)
# define S_IRWXG (S_IRWXU >> 3)
# define S_IROTH (S_IRGRP >> 3)
# define S_IWOTH (S_IWGRP >> 3)
# define S_IXOTH (S_IXGRP >> 3)
# define S_IRWXO (S_IRWXG >> 3)
#endif
