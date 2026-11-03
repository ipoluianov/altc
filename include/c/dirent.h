/* AltC <dirent.h> - directory entries (glibc-compatible, x86-64 Linux).  */

#ifndef _DIRENT_H
#define _DIRENT_H 1

#include <features.h>
#include <_altc/types.h>

__BEGIN_DECLS

#ifndef __ino_t_defined
typedef __ino_t ino_t;
# define __ino_t_defined
#endif
#if defined __USE_LARGEFILE64 && !defined __ino64_t_defined
typedef __ino64_t ino64_t;
# define __ino64_t_defined
#endif

/* <bits/dirent.h> */
struct dirent
  {
    __ino_t d_ino;
    __off_t d_off;
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256];
  };

#ifdef __USE_LARGEFILE64
struct dirent64
  {
    __ino64_t d_ino;
    __off64_t d_off;
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256];
  };
#endif

#define d_fileno d_ino

#define _DIRENT_HAVE_D_RECLEN
#define _DIRENT_HAVE_D_OFF
#define _DIRENT_HAVE_D_TYPE
#define _DIRENT_MATCHES_DIRENT64 1

enum
  {
    DT_UNKNOWN = 0,
# define DT_UNKNOWN DT_UNKNOWN
    DT_FIFO = 1,
# define DT_FIFO DT_FIFO
    DT_CHR = 2,
# define DT_CHR DT_CHR
    DT_DIR = 4,
# define DT_DIR DT_DIR
    DT_BLK = 6,
# define DT_BLK DT_BLK
    DT_REG = 8,
# define DT_REG DT_REG
    DT_LNK = 10,
# define DT_LNK DT_LNK
    DT_SOCK = 12,
# define DT_SOCK DT_SOCK
    DT_WHT = 14
# define DT_WHT DT_WHT
  };

#define IFTODT(mode) (((mode) & 0170000) >> 12)
#define DTTOIF(dirtype) ((dirtype) << 12)

typedef struct __dirstream DIR;

extern DIR *opendir (const char *__name);
extern DIR *fdopendir (int __fd);
extern int closedir (DIR *__dirp);
extern struct dirent *readdir (DIR *__dirp);
extern int readdir_r (DIR *__restrict __dirp,
                      struct dirent *__restrict __entry,
                      struct dirent **__restrict __result)
     __attribute__ ((__deprecated__));
#ifdef __USE_LARGEFILE64
extern struct dirent64 *readdir64 (DIR *__dirp);
extern int readdir64_r (DIR *__restrict __dirp,
                        struct dirent64 *__restrict __entry,
                        struct dirent64 **__restrict __result)
     __attribute__ ((__deprecated__));
#endif
extern void rewinddir (DIR *__dirp);
extern void seekdir (DIR *__dirp, long int __pos);
extern long int telldir (DIR *__dirp);
extern int dirfd (DIR *__dirp);

#define MAXNAMLEN 255

#define __need_size_t
#include <stddef.h>

extern int scandir (const char *__restrict __dir,
                    struct dirent ***__restrict __namelist,
                    int (*__selector) (const struct dirent *),
                    int (*__cmp) (const struct dirent **,
                                  const struct dirent **));
extern int alphasort (const struct dirent **__e1, const struct dirent **__e2);
#ifdef __USE_GNU
extern int scandirat (int __dfd, const char *__restrict __dir,
                      struct dirent ***__restrict __namelist,
                      int (*__selector) (const struct dirent *),
                      int (*__cmp) (const struct dirent **,
                                    const struct dirent **));
extern int versionsort (const struct dirent **__e1,
                        const struct dirent **__e2);
extern __ssize_t getdents64 (int __fd, void *__buffer, size_t __length);
#endif
#ifdef __USE_LARGEFILE64
extern int scandir64 (const char *__restrict __dir,
                      struct dirent64 ***__restrict __namelist,
                      int (*__selector) (const struct dirent64 *),
                      int (*__cmp) (const struct dirent64 **,
                                    const struct dirent64 **));
extern int alphasort64 (const struct dirent64 **__e1,
                        const struct dirent64 **__e2);
extern __ssize_t getdirentries64 (int __fd, char *__restrict __buf,
                                  size_t __nbytes,
                                  __off64_t *__restrict __basep);
# ifdef __USE_GNU
extern int scandirat64 (int __dfd, const char *__restrict __dir,
                        struct dirent64 ***__restrict __namelist,
                        int (*__selector) (const struct dirent64 *),
                        int (*__cmp) (const struct dirent64 **,
                                      const struct dirent64 **));
extern int versionsort64 (const struct dirent64 **__e1,
                          const struct dirent64 **__e2);
# endif
#endif

extern __ssize_t getdirentries (int __fd, char *__restrict __buf,
                                size_t __nbytes,
                                __off_t *__restrict __basep);

__END_DECLS

#endif /* dirent.h */
