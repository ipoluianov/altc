/* AltC <sys/un.h> - UNIX domain sockets.  */

#ifndef _SYS_UN_H
#define _SYS_UN_H 1

#include <features.h>
#include <_altc/sockaddr.h>
#include <string.h>

__BEGIN_DECLS

struct sockaddr_un
  {
    sa_family_t sun_family;
    char sun_path[108];
  };

#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path) \
                      + strlen ((ptr)->sun_path))

__END_DECLS

#endif /* sys/un.h */
