/* AltC <pwd.h> - password database.  */

#ifndef _PWD_H
#define _PWD_H 1

#include <features.h>
#include <_altc/types.h>
#include <_altc/FILE.h>

#define __need_size_t
#include <stddef.h>

#ifndef __gid_t_defined
typedef __gid_t gid_t;
# define __gid_t_defined
#endif
#ifndef __uid_t_defined
typedef __uid_t uid_t;
# define __uid_t_defined
#endif

__BEGIN_DECLS

struct passwd
{
  char *pw_name;
  char *pw_passwd;
  __uid_t pw_uid;
  __gid_t pw_gid;
  char *pw_gecos;
  char *pw_dir;
  char *pw_shell;
};

#define NSS_BUFLEN_PASSWD 1024

extern void setpwent (void);
extern void endpwent (void);
extern struct passwd *getpwent (void);
extern struct passwd *fgetpwent (FILE *__stream);
extern int putpwent (const struct passwd *__restrict __p,
                     FILE *__restrict __f);
extern struct passwd *getpwuid (__uid_t __uid);
extern struct passwd *getpwnam (const char *__name);

extern int getpwent_r (struct passwd *__restrict __resultbuf,
                       char *__restrict __buffer, size_t __buflen,
                       struct passwd **__restrict __result);
extern int getpwuid_r (__uid_t __uid,
                       struct passwd *__restrict __resultbuf,
                       char *__restrict __buffer, size_t __buflen,
                       struct passwd **__restrict __result);
extern int getpwnam_r (const char *__restrict __name,
                       struct passwd *__restrict __resultbuf,
                       char *__restrict __buffer, size_t __buflen,
                       struct passwd **__restrict __result);
extern int fgetpwent_r (FILE *__restrict __stream,
                        struct passwd *__restrict __resultbuf,
                        char *__restrict __buffer, size_t __buflen,
                        struct passwd **__restrict __result);

#ifdef __USE_GNU
extern int getpw (__uid_t __uid, char *__buffer);
#endif

__END_DECLS

#endif /* pwd.h */
