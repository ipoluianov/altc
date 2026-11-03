/* AltC <stdio.h> - standard input/output (glibc-compatible, x86-64).  */

#ifndef _STDIO_H
#define _STDIO_H 1

#include <features.h>

#define __need_size_t
#define __need_NULL
#include <stddef.h>

#define __need___va_list
#include <stdarg.h>

#include <_altc/types.h>
#include <_altc/mbstate_t.h>
#include <_altc/FILE.h>
#include <_altc/struct_FILE.h>

#ifndef _____fpos_t_defined
#define _____fpos_t_defined 1
typedef struct _G_fpos_t
{
  __off_t __pos;
  __mbstate_t __state;
} __fpos_t;
#endif

#ifndef _____fpos64_t_defined
#define _____fpos64_t_defined 1
typedef struct _G_fpos64_t
{
  __off64_t __pos;
  __mbstate_t __state;
} __fpos64_t;
#endif

__BEGIN_DECLS

#if !defined _VA_LIST_DEFINED && !defined _VA_LIST_ && !defined _VA_LIST \
    && !defined _VA_LIST_T_H && !defined __va_list__
typedef __gnuc_va_list va_list;
# define _VA_LIST_DEFINED
#endif

#ifndef __off_t_defined
typedef __off_t off_t;
# define __off_t_defined
#endif
#if defined __USE_LARGEFILE64 && !defined __off64_t_defined
typedef __off64_t off64_t;
# define __off64_t_defined
#endif

#ifndef __ssize_t_defined
typedef __ssize_t ssize_t;
# define __ssize_t_defined
#endif

typedef __fpos_t fpos_t;
#ifdef __USE_LARGEFILE64
typedef __fpos64_t fpos64_t;
#endif

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

#define BUFSIZ 8192

#define EOF (-1)

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#ifdef __USE_GNU
# define SEEK_DATA 3
# define SEEK_HOLE 4
#endif

#define P_tmpdir "/tmp"

#define L_tmpnam 20
#define TMP_MAX 238328
#define FILENAME_MAX 4096
#define L_ctermid 9
#define L_cuserid 9
#define FOPEN_MAX 16

#if __GLIBC_USE (ISOC23)
# define _PRINTF_NAN_LEN_MAX 4
#endif
#if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
# define __STDC_VERSION_STDIO_H__ 202311L
#endif

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
#define stdin stdin
#define stdout stdout
#define stderr stderr

extern int remove (const char *__filename);
extern int rename (const char *__old, const char *__new);
extern int renameat (int __oldfd, const char *__old, int __newfd,
                     const char *__new);
#ifdef __USE_GNU
# define RENAME_NOREPLACE (1 << 0)
# define RENAME_EXCHANGE (1 << 1)
# define RENAME_WHITEOUT (1 << 2)
extern int renameat2 (int __oldfd, const char *__old, int __newfd,
                      const char *__new, unsigned int __flags);
#endif

extern FILE *tmpfile (void);
extern char *tmpnam (char __s[20]);
extern char *tmpnam_r (char __s[20]);
extern char *tempnam (const char *__dir, const char *__pfx);

extern int fclose (FILE *__stream);
extern int fflush (FILE *__stream);
extern int fflush_unlocked (FILE *__stream);
#ifdef __USE_GNU
extern int fcloseall (void);
#endif

extern FILE *fopen (const char *__restrict __filename,
                    const char *__restrict __modes);
extern FILE *freopen (const char *__restrict __filename,
                      const char *__restrict __modes,
                      FILE *__restrict __stream);
#ifdef __USE_LARGEFILE64
extern FILE *fopen64 (const char *__restrict __filename,
                      const char *__restrict __modes);
extern FILE *freopen64 (const char *__restrict __filename,
                        const char *__restrict __modes,
                        FILE *__restrict __stream);
extern FILE *tmpfile64 (void);
#endif
extern FILE *fdopen (int __fd, const char *__modes);

typedef __ssize_t cookie_read_function_t (void *__cookie, char *__buf,
                                          size_t __nbytes);
typedef __ssize_t cookie_write_function_t (void *__cookie, const char *__buf,
                                           size_t __nbytes);
typedef int cookie_seek_function_t (void *__cookie, __off64_t *__pos, int __w);
typedef int cookie_close_function_t (void *__cookie);
# ifndef __cookie_io_functions_t_defined
#  define __cookie_io_functions_t_defined 1
typedef struct _IO_cookie_io_functions_t
{
  cookie_read_function_t *read;
  cookie_write_function_t *write;
  cookie_seek_function_t *seek;
  cookie_close_function_t *close;
} cookie_io_functions_t;
# endif
extern FILE *fopencookie (void *__restrict __magic_cookie,
                          const char *__restrict __modes,
                          cookie_io_functions_t __io_funcs);

extern FILE *fmemopen (void *__s, size_t __len, const char *__modes);
extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc);

extern void setbuf (FILE *__restrict __stream, char *__restrict __buf);
extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
                    int __modes, size_t __n);
extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
                       size_t __size);
extern void setlinebuf (FILE *__stream);

extern int fprintf (FILE *__restrict __stream,
                    const char *__restrict __format, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));
extern int printf (const char *__restrict __format, ...)
     __attribute__ ((__format__ (__printf__, 1, 2)));
extern int sprintf (char *__restrict __s,
                    const char *__restrict __format, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));
extern int vfprintf (FILE *__restrict __s, const char *__restrict __format,
                     __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int vprintf (const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 1, 0)));
extern int vsprintf (char *__restrict __s, const char *__restrict __format,
                     __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int snprintf (char *__restrict __s, size_t __maxlen,
                     const char *__restrict __format, ...)
     __attribute__ ((__format__ (__printf__, 3, 4)));
extern int vsnprintf (char *__restrict __s, size_t __maxlen,
                      const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 3, 0)));
extern int vasprintf (char **__restrict __ptr, const char *__restrict __f,
                      __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int __asprintf (char **__restrict __ptr,
                       const char *__restrict __fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));
extern int asprintf (char **__restrict __ptr,
                     const char *__restrict __fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));
extern int vdprintf (int __fd, const char *__restrict __fmt,
                     __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int dprintf (int __fd, const char *__restrict __fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));

extern int fscanf (FILE *__restrict __stream,
                   const char *__restrict __format, ...);
extern int scanf (const char *__restrict __format, ...);
extern int sscanf (const char *__restrict __s,
                   const char *__restrict __format, ...);
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format,
                    __gnuc_va_list __arg);
extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg);
extern int vsscanf (const char *__restrict __s,
                    const char *__restrict __format, __gnuc_va_list __arg);

/* Like glibc without asm redirection: ISO C99 (resp. C23) conforming
   scanf implementations are selected by renaming.  */
#if !__GLIBC_USE (DEPRECATED_SCANF)
# if __GLIBC_USE (C23_STRTOL)
extern int __isoc23_fscanf (FILE *__restrict __stream,
                            const char *__restrict __format, ...);
extern int __isoc23_scanf (const char *__restrict __format, ...);
extern int __isoc23_sscanf (const char *__restrict __s,
                            const char *__restrict __format, ...);
extern int __isoc23_vfscanf (FILE *__restrict __s,
                             const char *__restrict __format,
                             __gnuc_va_list __arg);
extern int __isoc23_vscanf (const char *__restrict __format,
                            __gnuc_va_list __arg);
extern int __isoc23_vsscanf (const char *__restrict __s,
                             const char *__restrict __format,
                             __gnuc_va_list __arg);
#  define fscanf __isoc23_fscanf
#  define scanf __isoc23_scanf
#  define sscanf __isoc23_sscanf
#  define vfscanf __isoc23_vfscanf
#  define vscanf __isoc23_vscanf
#  define vsscanf __isoc23_vsscanf
# else
extern int __isoc99_fscanf (FILE *__restrict __stream,
                            const char *__restrict __format, ...);
extern int __isoc99_scanf (const char *__restrict __format, ...);
extern int __isoc99_sscanf (const char *__restrict __s,
                            const char *__restrict __format, ...);
extern int __isoc99_vfscanf (FILE *__restrict __s,
                             const char *__restrict __format,
                             __gnuc_va_list __arg);
extern int __isoc99_vscanf (const char *__restrict __format,
                            __gnuc_va_list __arg);
extern int __isoc99_vsscanf (const char *__restrict __s,
                             const char *__restrict __format,
                             __gnuc_va_list __arg);
#  define fscanf __isoc99_fscanf
#  define scanf __isoc99_scanf
#  define sscanf __isoc99_sscanf
#  define vfscanf __isoc99_vfscanf
#  define vscanf __isoc99_vscanf
#  define vsscanf __isoc99_vsscanf
# endif
#endif

extern int fgetc (FILE *__stream);
extern int getc (FILE *__stream);
extern int getchar (void);
extern int getc_unlocked (FILE *__stream);
extern int getchar_unlocked (void);
extern int fgetc_unlocked (FILE *__stream);

extern int fputc (int __c, FILE *__stream);
extern int putc (int __c, FILE *__stream);
extern int putchar (int __c);
extern int fputc_unlocked (int __c, FILE *__stream);
extern int putc_unlocked (int __c, FILE *__stream);
extern int putchar_unlocked (int __c);

extern int getw (FILE *__stream);
extern int putw (int __w, FILE *__stream);

extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream);
#if __GLIBC_USE (DEPRECATED_GETS)
extern char *gets (char *__s) __attribute__ ((__deprecated__));
#endif
#ifdef __USE_GNU
extern char *fgets_unlocked (char *__restrict __s, int __n,
                             FILE *__restrict __stream);
#endif

extern __ssize_t __getdelim (char **__restrict __lineptr,
                             size_t *__restrict __n, int __delimiter,
                             FILE *__restrict __stream);
extern __ssize_t getdelim (char **__restrict __lineptr,
                           size_t *__restrict __n, int __delimiter,
                           FILE *__restrict __stream);
extern __ssize_t getline (char **__restrict __lineptr,
                          size_t *__restrict __n,
                          FILE *__restrict __stream);

extern int fputs (const char *__restrict __s, FILE *__restrict __stream);
extern int puts (const char *__s);
extern int ungetc (int __c, FILE *__stream);

extern size_t fread (void *__restrict __ptr, size_t __size,
                     size_t __n, FILE *__restrict __stream);
extern size_t fwrite (const void *__restrict __ptr, size_t __size,
                      size_t __n, FILE *__restrict __s);
#ifdef __USE_GNU
extern int fputs_unlocked (const char *__restrict __s,
                           FILE *__restrict __stream);
#endif
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
                              size_t __n, FILE *__restrict __stream);
extern size_t fwrite_unlocked (const void *__restrict __ptr, size_t __size,
                               size_t __n, FILE *__restrict __stream);

extern int fseek (FILE *__stream, long int __off, int __whence);
extern long int ftell (FILE *__stream);
extern void rewind (FILE *__stream);
extern int fseeko (FILE *__stream, __off_t __off, int __whence);
extern __off_t ftello (FILE *__stream);
extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos);
extern int fsetpos (FILE *__stream, const fpos_t *__pos);
#ifdef __USE_LARGEFILE64
extern int fseeko64 (FILE *__stream, __off64_t __off, int __whence);
extern __off64_t ftello64 (FILE *__stream);
extern int fgetpos64 (FILE *__restrict __stream, fpos64_t *__restrict __pos);
extern int fsetpos64 (FILE *__stream, const fpos64_t *__pos);
#endif

extern void clearerr (FILE *__stream);
extern int feof (FILE *__stream);
extern int ferror (FILE *__stream);
extern void clearerr_unlocked (FILE *__stream);
extern int feof_unlocked (FILE *__stream);
extern int ferror_unlocked (FILE *__stream);

extern void perror (const char *__s);

extern int fileno (FILE *__stream);
extern int fileno_unlocked (FILE *__stream);

extern int pclose (FILE *__stream);
extern FILE *popen (const char *__command, const char *__modes);

extern char *ctermid (char *__s);
extern char *cuserid (char *__s);

extern void flockfile (FILE *__stream);
extern int ftrylockfile (FILE *__stream);
extern void funlockfile (FILE *__stream);

extern int __uflow (FILE *);
extern int __overflow (FILE *, int);

__END_DECLS

#endif /* stdio.h */
