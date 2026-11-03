/* AltC <sys/utsname.h> - system name (Linux, 65-byte fields).  */

#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H 1

#include <features.h>

__BEGIN_DECLS

#define _UTSNAME_LENGTH 65
#define _UTSNAME_DOMAIN_LENGTH _UTSNAME_LENGTH
#define _UTSNAME_SYSNAME_LENGTH _UTSNAME_LENGTH
#define _UTSNAME_NODENAME_LENGTH _UTSNAME_LENGTH
#define _UTSNAME_RELEASE_LENGTH _UTSNAME_LENGTH
#define _UTSNAME_VERSION_LENGTH _UTSNAME_LENGTH
#define _UTSNAME_MACHINE_LENGTH _UTSNAME_LENGTH

struct utsname
  {
    char sysname[_UTSNAME_SYSNAME_LENGTH];
    char nodename[_UTSNAME_NODENAME_LENGTH];
    char release[_UTSNAME_RELEASE_LENGTH];
    char version[_UTSNAME_VERSION_LENGTH];
    char machine[_UTSNAME_MACHINE_LENGTH];
#ifdef __USE_GNU
    char domainname[_UTSNAME_DOMAIN_LENGTH];
#else
    char __domainname[_UTSNAME_DOMAIN_LENGTH];
#endif
  };

#define SYS_NMLN _UTSNAME_LENGTH

extern int uname (struct utsname *__name);

__END_DECLS

#endif /* sys/utsname.h */
