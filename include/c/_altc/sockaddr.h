/* AltC private header: sa_family_t (same guard as glibc's
   <bits/sockaddr.h>).  */

#ifndef _BITS_SOCKADDR_H
#define _BITS_SOCKADDR_H 1

typedef unsigned short int sa_family_t;

#define __SOCKADDR_COMMON(sa_prefix) \
  sa_family_t sa_prefix##family

#define __SOCKADDR_COMMON_SIZE (sizeof (unsigned short int))

#define _SS_SIZE 128

#endif
