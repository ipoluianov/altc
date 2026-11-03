/* AltC <sys/stat.h> - file status (glibc-compatible, x86-64).  */

#ifndef _SYS_STAT_H
#define _SYS_STAT_H 1

#include <features.h>
#include <_altc/types.h>
#include <_altc/time_types.h>

#ifndef __dev_t_defined
typedef __dev_t dev_t;
# define __dev_t_defined
#endif
#ifndef __gid_t_defined
typedef __gid_t gid_t;
# define __gid_t_defined
#endif
#ifndef __ino_t_defined
typedef __ino_t ino_t;
# define __ino_t_defined
#endif
#ifndef __mode_t_defined
typedef __mode_t mode_t;
# define __mode_t_defined
#endif
#ifndef __nlink_t_defined
typedef __nlink_t nlink_t;
# define __nlink_t_defined
#endif
#ifndef __off_t_defined
typedef __off_t off_t;
# define __off_t_defined
#endif
#ifndef __uid_t_defined
typedef __uid_t uid_t;
# define __uid_t_defined
#endif
#ifndef __blkcnt_t_defined
typedef __blkcnt_t blkcnt_t;
# define __blkcnt_t_defined
#endif
#ifndef __blksize_t_defined
typedef __blksize_t blksize_t;
# define __blksize_t_defined
#endif

__BEGIN_DECLS

#include <_altc/stat.h>

#define S_IFMT __S_IFMT
#define __S_ISTYPE(mode, mask) (((mode) & __S_IFMT) == (mask))

#define S_ISDIR(mode) __S_ISTYPE((mode), __S_IFDIR)
#define S_ISCHR(mode) __S_ISTYPE((mode), __S_IFCHR)
#define S_ISBLK(mode) __S_ISTYPE((mode), __S_IFBLK)
#define S_ISREG(mode) __S_ISTYPE((mode), __S_IFREG)
#define S_ISFIFO(mode) __S_ISTYPE((mode), __S_IFIFO)
#define S_ISLNK(mode) __S_ISTYPE((mode), __S_IFLNK)
#define S_ISSOCK(mode) __S_ISTYPE((mode), __S_IFSOCK)

#define S_TYPEISMQ(buf) __S_TYPEISMQ(buf)
#define S_TYPEISSEM(buf) __S_TYPEISSEM(buf)
#define S_TYPEISSHM(buf) __S_TYPEISSHM(buf)

#define S_IREAD S_IRUSR
#define S_IWRITE S_IWUSR
#define S_IEXEC S_IXUSR

#define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO)
#define ALLPERMS (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)
#define DEFFILEMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

#define S_BLKSIZE 512

extern int stat (const char *__restrict __file,
                 struct stat *__restrict __buf);
extern int fstat (int __fd, struct stat *__buf);
extern int fstatat (int __fd, const char *__restrict __file,
                    struct stat *__restrict __buf, int __flag);
extern int lstat (const char *__restrict __file,
                  struct stat *__restrict __buf);
#ifdef __USE_LARGEFILE64
extern int stat64 (const char *__restrict __file,
                   struct stat64 *__restrict __buf);
extern int fstat64 (int __fd, struct stat64 *__buf);
extern int fstatat64 (int __fd, const char *__restrict __file,
                      struct stat64 *__restrict __buf, int __flag);
extern int lstat64 (const char *__restrict __file,
                    struct stat64 *__restrict __buf);
#endif

extern int chmod (const char *__file, __mode_t __mode);
extern int lchmod (const char *__file, __mode_t __mode);
extern int fchmod (int __fd, __mode_t __mode);
extern int fchmodat (int __fd, const char *__file, __mode_t __mode,
                     int __flag);

extern __mode_t umask (__mode_t __mask);

extern int mkdir (const char *__path, __mode_t __mode);
extern int mkdirat (int __fd, const char *__path, __mode_t __mode);
extern int mknod (const char *__path, __mode_t __mode, __dev_t __dev);
extern int mknodat (int __fd, const char *__path, __mode_t __mode,
                    __dev_t __dev);
extern int mkfifo (const char *__path, __mode_t __mode);
extern int mkfifoat (int __fd, const char *__path, __mode_t __mode);

extern int utimensat (int __fd, const char *__path,
                      const struct timespec __times[2], int __flags);
extern int futimens (int __fd, const struct timespec __times[2]);

__END_DECLS

#endif /* sys/stat.h */
