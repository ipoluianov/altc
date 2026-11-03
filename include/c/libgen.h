/* AltC <libgen.h> - POSIX path name manipulation.  The POSIX basename is
   exported by glibc as __xpg_basename (it may modify its argument).  */

#ifndef _LIBGEN_H
#define _LIBGEN_H 1

#include <features.h>

__BEGIN_DECLS

extern char *dirname (char *__path);

extern char *__xpg_basename (char *__path);
#define basename __xpg_basename

__END_DECLS

#endif /* libgen.h */
