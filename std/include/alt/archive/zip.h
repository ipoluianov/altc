/* AltC standard library: ZIP archives (Go's archive/zip).

   Reading needs random access: an alt_zip_reader_at (memory, a file
   descriptor, or anything with a read_at function) and the size of the
   archive. The central directory is read at once, files are opened as
   streams that check the CRC-32 and the size when they end:

	alt_zip_reader *zr;
	if (alt_zip_open(&zr, "a.zip") < 0) ...
	for (size_t i = 0; i < zr->nfile; i++) {
		alt_zip_file *f = &zr->file[i];
		alt_zip_file_reader *fr;
		if (alt_zip_file_open(f, &fr) < 0) ...
		... alt_zip_file_read(fr, buf, n) until 0 or an error ...
		alt_zip_file_close(fr);
	}
	alt_zip_reader_free(zr);

   Writing streams to an alt_writer: each entry is created and its
   contents written to the returned alt_writer before the next one;
   alt_zip_writer_close writes the central directory:

	alt_zip_writer *zw;
	alt_zip_writer_new(&zw, out);
	alt_writer fw;
	alt_zip_create(zw, ALT_STR("hello.txt"), &fw);
	alt_write_all(fw, "hello", 5);
	alt_zip_writer_close(zw);
	alt_zip_writer_free(zw);

   The methods are Store and Deflate. Archives written with Store are
   byte for byte what Go's zip.Writer writes for the same headers (with a
   data descriptor after each file, the UTF-8 flag set as Go sets it, an
   extended timestamp for a set modification time, Zip64 records when
   needed). Deflated data differs from Go's (another compressor) but
   either side reads the other's archives. Errors are Go's:
   -ALT_EZIP_FORMAT (ErrFormat), -ALT_EZIP_ALGORITHM (ErrAlgorithm),
   -ALT_ECHECKSUM (ErrChecksum), -ALT_EZIP_INSECURE_PATH
   (ErrInsecurePath), -ALT_EUNEXPECTED_EOF, -ALT_EEOF (io.EOF where Go
   returns it), -ALT_EFLATE_CORRUPT, -EINVAL (misuse and Go's other
   errors), -ENOMEM. Disk spanning and encryption are not supported. */
#ifndef ALT_ARCHIVE_ZIP_H
#define ALT_ARCHIVE_ZIP_H

#include <alt/arena.h>
#include <alt/errors.h>
#include <alt/io.h>
#include <alt/str.h>
#include <alt/time.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* compression methods */
enum {
	ALT_ZIP_STORE = 0,
	ALT_ZIP_DEFLATE = 8,
};

/* the first byte of creator_version (the host system) */
enum {
	ALT_ZIP_CREATOR_FAT = 0,
	ALT_ZIP_CREATOR_UNIX = 3,
	ALT_ZIP_CREATOR_NTFS = 11,
	ALT_ZIP_CREATOR_VFAT = 14,
	ALT_ZIP_CREATOR_MACOSX = 19,
};

/* flags bits */
enum {
	ALT_ZIP_FLAG_DATA_DESCRIPTOR = 0x8, /* sizes and CRC-32 follow the data */
	ALT_ZIP_FLAG_UTF8 = 0x800,          /* name and comment are UTF-8 */
};

/* A file header (Go's FileHeader). Strings are not NUL-terminated
   (except those of a reader, which are). */
typedef struct {
	alt_str name;    /* slash-separated; a trailing slash makes a directory */
	alt_str comment;
	/* name and comment are not UTF-8 (the writer clears the UTF-8
	   flag); else the writer sets that flag when they need it */
	bool non_utf8;
	uint16_t creator_version, reader_version, flags;
	uint16_t method; /* ALT_ZIP_STORE or ALT_ZIP_DEFLATE */

	/* The modification time. modified_set false is Go's zero Modified:
	   the writer then keeps modified_time and modified_date as given;
	   else it encodes the MS-DOS fields from modified in the zone
	   modified_offset (seconds east of UTC) and adds an extended
	   timestamp. A reader always sets it: from an extended timestamp,
	   NTFS or Unix extra field when there is one (with
	   modified_has_zone and the zone offset that makes the MS-DOS
	   fields agree, as Go does, when they are not zero), else from the
	   MS-DOS fields in UTC. Times beyond alt_wall's years 1677..2262
	   are clamped. */
	bool modified_set;
	bool modified_has_zone; /* false: UTC */
	alt_wall modified;
	int modified_offset;
	uint16_t modified_time, modified_date; /* MS-DOS */

	uint32_t crc32;
	/* 0xffffffff when either size needs 64 bits */
	uint32_t compressed_size, uncompressed_size;
	uint64_t compressed_size64, uncompressed_size64;
	alt_str extra;   /* the extra fields */
	uint32_t external_attrs; /* their meaning depends on creator_version */
} alt_zip_header;

/* the permission and type bits as a Unix st_mode (Go's Mode, mapped
   back as SetMode maps it: a regular file when the creator is unknown) */
unsigned alt_zip_header_mode(const alt_zip_header *h);
/* sets creator_version to Unix and external_attrs from a st_mode (with
   the MS-DOS directory and read-only bits, as Go's SetMode) */
void alt_zip_header_set_mode(alt_zip_header *h, unsigned mode);
/* the MS-DOS fields as a time in UTC (Go's ModTime) */
alt_wall alt_zip_header_mod_time(const alt_zip_header *h);
/* modified and the MS-DOS fields from t in UTC (Go's SetModTime) */
void alt_zip_header_set_mod_time(alt_zip_header *h, alt_wall t);

/* ---- random access sources (io.ReaderAt) ---- */

/* read_at reads up to n bytes at off: the count, fewer than n only at
   the end of the data (0 at or past it), or an error (-EINVAL for a
   negative offset) */
typedef struct {
	ssize_t (*read_at)(void *obj, void *buf, size_t n, int64_t off);
	void *obj;
} alt_zip_reader_at;

static inline ssize_t alt_zip_read_at(alt_zip_reader_at r, void *buf, size_t n, int64_t off) {
	return r.read_at(r.obj, buf, n, off);
}
/* the bytes of s (m must outlive its use) */
typedef struct {
	alt_str s;
} alt_zip_mem;
alt_zip_reader_at alt_zip_mem_reader_at(alt_zip_mem *m, alt_str s);
/* pread on *fd */
alt_zip_reader_at alt_zip_fd_reader_at(int *fd);

/* ---- reading ---- */

typedef struct alt_zip_reader alt_zip_reader;

typedef struct {
	alt_zip_header hdr;
	alt_zip_reader *zip;
	int64_t header_offset; /* of the local header (the base offset included) */
} alt_zip_file;

struct alt_zip_reader {
	alt_zip_file *file; /* the central directory in order */
	size_t nfile;
	alt_str comment;    /* of the end of central directory record */
	/* private */
	alt_zip_reader_at ra;
	int64_t size, base_offset;
	alt_zip_mem mem;
	int fd;
	alt_arena arena;
};

/* reads the directory of the archive of size bytes in r (Go's
   NewReader), allowing for data before the archive (a self-extracting
   prefix) as Go does: 0, -ALT_EZIP_FORMAT, -ALT_EUNEXPECTED_EOF,
   -ALT_EEOF, an error of r, -EINVAL for a negative size. r must stay
   valid while the reader is used. */
int alt_zip_reader_new(alt_zip_reader **out, alt_zip_reader_at r, int64_t size);
/* an archive in memory (data must outlive the reader) */
int alt_zip_reader_new_mem(alt_zip_reader **out, alt_str data);
/* opens a file (Go's OpenReader); the descriptor is closed by
   alt_zip_reader_free; -errno when it cannot be opened */
int alt_zip_open(alt_zip_reader **out, const char *path);
void alt_zip_reader_free(alt_zip_reader *r);
/* Go's check under GODEBUG=zipinsecurepath=0: -ALT_EZIP_INSECURE_PATH
   when a name is not local (filepath.IsLocal) or has a backslash, else 0 */
int alt_zip_reader_check_paths(const alt_zip_reader *r);

/* the offset of the file's (compressed) data in the archive (Go's
   DataOffset) */
int alt_zip_file_data_offset(const alt_zip_file *f, int64_t *off);

typedef struct alt_zip_file_reader alt_zip_file_reader;

/* opens the decompressed contents of f (Go's File.Open): reading checks
   the size, the data descriptor and the CRC-32 at the end; several files
   may be read at once. -ALT_EZIP_ALGORITHM for another method. */
int alt_zip_file_open(const alt_zip_file *f, alt_zip_file_reader **out);
/* the compressed bytes as they are (Go's OpenRaw) */
int alt_zip_file_open_raw(const alt_zip_file *f, alt_zip_file_reader **out);
/* a count, 0 at the end, or an error (after the data before it):
   -ALT_ECHECKSUM, -ALT_EZIP_FORMAT (more data than the header says, or
   data in a directory), -ALT_EUNEXPECTED_EOF, -ALT_EFLATE_CORRUPT, or an
   error of the source. Errors are sticky. */
ssize_t alt_zip_file_read(alt_zip_file_reader *fr, void *buf, size_t n);
alt_reader alt_zip_file_stream(alt_zip_file_reader *fr);
void alt_zip_file_close(alt_zip_file_reader *fr);

/* ---- writing ---- */

typedef struct alt_zip_writer alt_zip_writer;

/* writes an archive to w (buffered: see alt_zip_writer_flush) */
int alt_zip_writer_new(alt_zip_writer **out, alt_writer w);
/* frees the writer (without closing the archive) */
void alt_zip_writer_free(alt_zip_writer *w);
/* the offset of the archive in the file it is appended to (call before
   writing anything, else -EINVAL) */
int alt_zip_writer_set_offset(alt_zip_writer *w, int64_t n);
/* the end of central directory comment: -EINVAL beyond 65535 bytes */
int alt_zip_writer_set_comment(alt_zip_writer *w, alt_str comment);
int alt_zip_writer_flush(alt_zip_writer *w);
/* finishes the current file and writes the central directory (the
   underlying writer is not closed) */
int alt_zip_writer_close(alt_zip_writer *w);

/* adds a deflated file named name; *fw gets the writer of its contents,
   valid until the next entry or close */
int alt_zip_create(alt_zip_writer *w, alt_str name, alt_writer *fw);
/* adds a file described by h (copied: flags, versions, the MS-DOS time,
   sizes and the CRC-32 are then filled in as Go does); a name with a
   trailing slash is a directory that takes no data */
int alt_zip_create_header(alt_zip_writer *w, const alt_zip_header *h, alt_writer *fw);
/* adds a file whose data, written to *fw, is already compressed: h is
   written as given (the data descriptor too when h->flags has 0x8) */
int alt_zip_create_raw(alt_zip_writer *w, const alt_zip_header *h, alt_writer *fw);
/* copies f of a reader as it is, without decompressing (Go's Copy) */
int alt_zip_copy(alt_zip_writer *w, const alt_zip_file *f);

#ifdef __cplusplus
}
#endif

#endif
