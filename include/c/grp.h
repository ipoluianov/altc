/* AltC <grp.h> - group database.  */

#ifndef _GRP_H
#define _GRP_H 1

#include <features.h>
#include <_altc/types.h>
#include <_altc/FILE.h>

#define __need_size_t
#include <stddef.h>

#ifndef __gid_t_defined
typedef __gid_t gid_t;
# define __gid_t_defined
#endif

__BEGIN_DECLS

struct group
  {
    char *gr_name;
    char *gr_passwd;
    __gid_t gr_gid;
    char **gr_mem;
  };

#define NSS_BUFLEN_GROUP 1024

extern void setgrent (void);
extern void endgrent (void);
extern struct group *getgrent (void);
extern struct group *fgetgrent (FILE *__stream);
#ifdef __USE_GNU
extern int putgrent (const struct group *__restrict __p,
                     FILE *__restrict __f);
#endif
extern struct group *getgrgid (__gid_t __gid);
extern struct group *getgrnam (const char *__name);

#ifdef __USE_GNU
extern int getgrent_r (struct group *__restrict __resultbuf,
                       char *__restrict __buffer, size_t __buflen,
                       struct group **__restrict __result);
#endif
extern int getgrgid_r (__gid_t __gid, struct group *__restrict __resultbuf,
                       char *__restrict __buffer, size_t __buflen,
                       struct group **__restrict __result);
extern int getgrnam_r (const char *__restrict __name,
                       struct group *__restrict __resultbuf,
                       char *__restrict __buffer, size_t __buflen,
                       struct group **__restrict __result);
extern int fgetgrent_r (FILE *__restrict __stream,
                        struct group *__restrict __resultbuf,
                        char *__restrict __buffer, size_t __buflen,
                        struct group **__restrict __result);

extern int setgroups (size_t __n, const __gid_t *__groups);
extern int getgrouplist (const char *__user, __gid_t __group,
                         __gid_t *__groups, int *__ngroups);
extern int initgroups (const char *__user, __gid_t __group);

__END_DECLS

#endif /* grp.h */
