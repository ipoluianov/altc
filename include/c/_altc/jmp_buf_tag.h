/* AltC private header: struct __jmp_buf_tag (glibc guards).  */

#include <_altc/sigset_t.h>

#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H 1
typedef long int __jmp_buf[8];
#endif

#ifndef __jmp_buf_tag_defined
#define __jmp_buf_tag_defined 1
struct __jmp_buf_tag
  {
    __jmp_buf __jmpbuf;
    int __mask_was_saved;
    __sigset_t __saved_mask;
  };
#endif
