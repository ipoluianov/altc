/* AltC private header: __mbstate_t / mbstate_t (glibc guards).  */

#ifndef ____mbstate_t_defined
#define ____mbstate_t_defined 1
typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;
#endif

#ifndef __mbstate_t_defined
#define __mbstate_t_defined 1
typedef __mbstate_t mbstate_t;
#endif
