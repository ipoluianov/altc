/* AltC private header: the layout of glibc's FILE object (struct _IO_FILE),
   matching glibc 2.41 on x86-64.  Same guard as <bits/types/struct_FILE.h>.
   Programs should treat FILE as opaque; the full definition only exists so
   that sizeof (FILE) and glibc's own headers agree.  */

#ifndef __struct_FILE_defined
#define __struct_FILE_defined 1

#include <_altc/types.h>

struct _IO_FILE;
struct _IO_marker;
struct _IO_codecvt;
struct _IO_wide_data;

#ifndef _IO_lock_t_defined
typedef void _IO_lock_t;
#endif

struct _IO_FILE
{
  int _flags;
  char *_IO_read_ptr;
  char *_IO_read_end;
  char *_IO_read_base;
  char *_IO_write_base;
  char *_IO_write_ptr;
  char *_IO_write_end;
  char *_IO_buf_base;
  char *_IO_buf_end;
  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;
  struct _IO_marker *_markers;
  struct _IO_FILE *_chain;
  int _fileno;
  int _flags2:24;
  char _short_backupbuf[1];
  __off_t _old_offset;
  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];
  _IO_lock_t *_lock;
  __off64_t _offset;
  struct _IO_codecvt *_codecvt;
  struct _IO_wide_data *_wide_data;
  struct _IO_FILE *_freeres_list;
  void *_freeres_buf;
  struct _IO_FILE **_prevchain;
  int _mode;
  char _unused2[15 * sizeof (int) - 5 * sizeof (void *)];
};

#define _IO_EOF_SEEN 0x0010
#define _IO_ERR_SEEN 0x0020
#define _IO_USER_LOCK 0x8000

#endif /* __struct_FILE_defined */
