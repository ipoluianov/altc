/* AltC private header: FILE and __FILE (glibc guards and struct tag).  */

#ifndef ____FILE_defined
#define ____FILE_defined 1
struct _IO_FILE;
typedef struct _IO_FILE __FILE;
#endif

#ifndef __FILE_defined
#define __FILE_defined 1
struct _IO_FILE;
typedef struct _IO_FILE FILE;
#endif
