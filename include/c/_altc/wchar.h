/* AltC private header: WCHAR_MIN/WCHAR_MAX helpers.
   Same include guard as glibc's <bits/wchar.h>.  wchar_t is int.  */

#ifndef _BITS_WCHAR_H
#define _BITS_WCHAR_H 1

#define __WCHAR_MAX 0x7fffffff
#define __WCHAR_MIN (-__WCHAR_MAX - 1)

#endif
