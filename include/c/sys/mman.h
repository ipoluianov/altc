/* AltC <sys/mman.h> - memory management (x86-64 Linux).  */

#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H 1

#include <features.h>
#include <_altc/types.h>

#define __need_size_t
#include <stddef.h>

#ifndef __off_t_defined
typedef __off_t off_t;
# define __off_t_defined
#endif
#ifndef __mode_t_defined
typedef __mode_t mode_t;
# define __mode_t_defined
#endif

__BEGIN_DECLS

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0
#define PROT_GROWSDOWN 0x01000000
#define PROT_GROWSUP 0x02000000

#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_SHARED_VALIDATE 0x03
#define MAP_DROPPABLE 0x08
#define MAP_TYPE 0x0f
#define MAP_FIXED 0x10
#define MAP_FILE 0
#define MAP_ANONYMOUS 0x20
#define MAP_ANON MAP_ANONYMOUS
#define MAP_32BIT 0x40
#define MAP_ABOVE4G 0x80
#define MAP_HUGE_SHIFT 26
#define MAP_HUGE_MASK 0x3f
#define MAP_HUGE_16KB (14 << MAP_HUGE_SHIFT)
#define MAP_HUGE_64KB (16 << MAP_HUGE_SHIFT)
#define MAP_HUGE_512KB (19 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1MB (20 << MAP_HUGE_SHIFT)
#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_8MB (23 << MAP_HUGE_SHIFT)
#define MAP_HUGE_16MB (24 << MAP_HUGE_SHIFT)
#define MAP_HUGE_32MB (25 << MAP_HUGE_SHIFT)
#define MAP_HUGE_256MB (28 << MAP_HUGE_SHIFT)
#define MAP_HUGE_512MB (29 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB (30 << MAP_HUGE_SHIFT)
#define MAP_HUGE_2GB (31 << MAP_HUGE_SHIFT)
#define MAP_HUGE_16GB (34U << MAP_HUGE_SHIFT)

#define MAP_GROWSDOWN 0x00100
#define MAP_DENYWRITE 0x00800
#define MAP_EXECUTABLE 0x01000
#define MAP_LOCKED 0x02000
#define MAP_NORESERVE 0x04000
#define MAP_POPULATE 0x08000
#define MAP_NONBLOCK 0x10000
#define MAP_STACK 0x20000
#define MAP_HUGETLB 0x40000
#define MAP_SYNC 0x80000
#define MAP_FIXED_NOREPLACE 0x100000

#define MAP_FAILED ((void *) -1)

#define MS_ASYNC 1
#define MS_SYNC 4
#define MS_INVALIDATE 2

#define MCL_CURRENT 1
#define MCL_FUTURE 2
#define MCL_ONFAULT 4

#define MADV_NORMAL 0
#define MADV_RANDOM 1
#define MADV_SEQUENTIAL 2
#define MADV_WILLNEED 3
#define MADV_DONTNEED 4
#define MADV_FREE 8
#define MADV_REMOVE 9
#define MADV_DONTFORK 10
#define MADV_DOFORK 11
#define MADV_MERGEABLE 12
#define MADV_UNMERGEABLE 13
#define MADV_HUGEPAGE 14
#define MADV_NOHUGEPAGE 15
#define MADV_DONTDUMP 16
#define MADV_DODUMP 17
#define MADV_WIPEONFORK 18
#define MADV_KEEPONFORK 19
#define MADV_COLD 20
#define MADV_PAGEOUT 21
#define MADV_POPULATE_READ 22
#define MADV_POPULATE_WRITE 23
#define MADV_DONTNEED_LOCKED 24
#define MADV_COLLAPSE 25
#define MADV_HWPOISON 100

#define POSIX_MADV_NORMAL 0
#define POSIX_MADV_RANDOM 1
#define POSIX_MADV_SEQUENTIAL 2
#define POSIX_MADV_WILLNEED 3
#define POSIX_MADV_DONTNEED 4

#ifdef __USE_GNU
# include <_altc/iovec.h>
# define MREMAP_MAYMOVE 1
# define MREMAP_FIXED 2
# define MREMAP_DONTUNMAP 4
# define MFD_CLOEXEC 1U
# define MFD_ALLOW_SEALING 2U
# define MFD_HUGETLB 4U
# define MFD_NOEXEC_SEAL 8U
# define MFD_EXEC 0x10U
# define MLOCK_ONFAULT 1U
# define PKEY_DISABLE_ACCESS 0x1
# define PKEY_DISABLE_WRITE 0x2
#endif

extern void *mmap (void *__addr, size_t __len, int __prot,
                   int __flags, int __fd, __off_t __offset);
#ifdef __USE_LARGEFILE64
extern void *mmap64 (void *__addr, size_t __len, int __prot,
                     int __flags, int __fd, __off64_t __offset);
#endif
extern int munmap (void *__addr, size_t __len);
extern int mprotect (void *__addr, size_t __len, int __prot);
extern int msync (void *__addr, size_t __len, int __flags);
extern int madvise (void *__addr, size_t __len, int __advice);
extern int posix_madvise (void *__addr, size_t __len, int __advice);
extern int mlock (const void *__addr, size_t __len);
extern int munlock (const void *__addr, size_t __len);
extern int mlockall (int __flags);
extern int munlockall (void);
extern int mincore (void *__start, size_t __len, unsigned char *__vec);

#ifdef __USE_GNU
extern void *mremap (void *__addr, size_t __old_len, size_t __new_len,
                     int __flags, ...);
extern int remap_file_pages (void *__start, size_t __size, int __prot,
                             size_t __pgoff, int __flags);
extern int memfd_create (const char *__name, unsigned int __flags);
extern int mlock2 (const void *__addr, size_t __length, unsigned int __flags);
extern int pkey_alloc (unsigned int __flags, unsigned int __access_rights);
extern int pkey_set (int __key, unsigned int __access_rights);
extern int pkey_get (int __key);
extern int pkey_free (int __key);
extern int pkey_mprotect (void *__addr, size_t __len, int __prot, int __pkey);
extern __ssize_t process_madvise (int __pid_fd, const struct iovec *__iov,
                                  size_t __count, int __advice,
                                  unsigned __flags);
extern int process_mrelease (int pidfd, unsigned int flags);
#endif

extern int shm_open (const char *__name, int __oflag, mode_t __mode);
extern int shm_unlink (const char *__name);

__END_DECLS

#endif /* sys/mman.h */
