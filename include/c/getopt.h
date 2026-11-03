/* AltC <getopt.h> - command-line option parsing (GNU).  */

#ifndef _GETOPT_H
#define _GETOPT_H 1

#include <features.h>

__BEGIN_DECLS

#ifndef _GETOPT_CORE_H
#define _GETOPT_CORE_H 1
extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;
extern int getopt (int ___argc, char *const *___argv, const char *__shortopts);
#endif

#ifndef _GETOPT_EXT_H
#define _GETOPT_EXT_H 1
struct option
{
  const char *name;
  int has_arg;
  int *flag;
  int val;
};

#define no_argument 0
#define required_argument 1
#define optional_argument 2

extern int getopt_long (int ___argc, char *const *___argv,
                        const char *__shortopts,
                        const struct option *__longopts, int *__longind);
extern int getopt_long_only (int ___argc, char *const *___argv,
                             const char *__shortopts,
                             const struct option *__longopts,
                             int *__longind);
#endif

__END_DECLS

#endif /* getopt.h */
