/* AltC standard library: tar archives (Go's archive/tar).

   The reader understands V7, USTAR, PAX (local 'x' and global 'g'
   headers, fractional times, any records), GNU (long names 'L'/'K',
   base-256 numbers, atime/ctime, old sparse files 'S') and STAR
   headers, and the GNU sparse formats in PAX (0.0, 0.1, 1.0): the data
   of a sparse file reads as the expanded file, holes as zeros.

	alt_tar_reader *tr;
	alt_tar_reader_new(&tr, src);
	alt_tar_header *h;
	int rc;
	while ((rc = alt_tar_next(tr, &h)) == 0) {
		... h->name, h->size; alt_tar_read(tr, buf, n) reads the data ...
	}
	if (rc != -ALT_EEOF)
		... alt_tar_reader_error(tr) has Go's message ...
	alt_tar_reader_free(tr);

   The writer produces the bytes Go's tar.Writer does for the same
   headers and data: each header in the first format that can encode it
   (USTAR, then PAX, then GNU) unless h->format restricts the choice; PAX
   extended headers named dir/PaxHeaders.0/file with sorted records; GNU
   long names in ././@LongLink entries. Without a format the modification
   time is rounded to the nearest second and atime/ctime are dropped, as
   in Go. Sparse files are not written (Go's writer does not either).

   Errors are Go's: -ALT_ETAR_HEADER (ErrHeader, and the sparse data
   errors), -ALT_ETAR_FIELD (ErrFieldTooLong, a header that cannot be
   encoded, a sparse map too long), -ALT_ETAR_WRITE_TOO_LONG,
   -ALT_EUNEXPECTED_EOF, -ALT_EEOF at the end of the archive, -ALT_ECLOSED
   after alt_tar_writer_close, or an error of the stream.
   alt_tar_reader_error and alt_tar_writer_error give the exact text Go's
   error would have. */
#ifndef ALT_ARCHIVE_TAR_H
#define ALT_ARCHIVE_TAR_H

#include <alt/arena.h>
#include <alt/errors.h>
#include <alt/io.h>
#include <alt/str.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* type flags */
enum {
	ALT_TAR_REG = '0',
	ALT_TAR_REGA = '\0',          /* legacy: read as ALT_TAR_REG or ALT_TAR_DIR */
	ALT_TAR_LINK = '1',           /* hard link */
	ALT_TAR_SYMLINK = '2',
	ALT_TAR_CHAR = '3',
	ALT_TAR_BLOCK = '4',
	ALT_TAR_DIR = '5',
	ALT_TAR_FIFO = '6',
	ALT_TAR_CONT = '7',           /* reserved */
	ALT_TAR_XHEADER = 'x',        /* PAX records of the next entry (handled by the library) */
	ALT_TAR_XGLOBAL = 'g',        /* PAX global records: returned as an entry of their own */
	ALT_TAR_GNU_SPARSE = 'S',
	ALT_TAR_GNU_LONGNAME = 'L',   /* handled by the library */
	ALT_TAR_GNU_LONGLINK = 'K',   /* handled by the library */
};

/* formats (Go's tar.Format values; they combine as bit sets) */
enum {
	ALT_TAR_FORMAT_UNKNOWN = 0,
	ALT_TAR_FORMAT_V7 = 1,        /* only reported by the reader */
	ALT_TAR_FORMAT_USTAR = 2,
	ALT_TAR_FORMAT_PAX = 4,
	ALT_TAR_FORMAT_GNU = 8,
	ALT_TAR_FORMAT_STAR = 16,     /* only reported by the reader */
};

/* an instant as Unix seconds and nanoseconds (0..999999999); set false
   is Go's zero time.Time (as is the instant of year 1, sec
   -62135596800). Times outside alt_wall's range (1678..2262) are
   representable, as in tar headers. */
typedef struct {
	int64_t sec;
	int32_t nsec;
	bool set;
} alt_tar_time;

/* Go's time.Unix(sec, nsec): nsec outside 0..999999999 is carried into sec */
alt_tar_time alt_tar_unix(int64_t sec, int64_t nsec);
static inline alt_tar_time alt_tar_from_wall(int64_t wall) { return alt_tar_unix(0, wall); }
/* Go's Time.IsZero */
bool alt_tar_time_is_zero(alt_tar_time t);

typedef struct {
	alt_str key, value;
} alt_tar_record;

typedef struct {
	int typeflag;
	alt_str name;
	alt_str linkname;             /* of ALT_TAR_LINK and ALT_TAR_SYMLINK */
	int64_t size;                 /* the logical size (of a sparse file: the expanded one) */
	int64_t mode;                 /* permission and mode bits */
	int64_t uid, gid;
	alt_str uname, gname;
	alt_tar_time mtime;           /* without a format the writer rounds it to seconds */
	alt_tar_time atime, ctime;    /* written only for PAX or GNU */
	int64_t devmajor, devminor;
	/* the PAX records (Go's PAXRecords: the reader gives them sorted by
	   key; the writer takes the last of duplicate keys). Extended
	   attributes are the SCHILY.xattr.* records (Go's deprecated Xattrs
	   field has no counterpart). */
	const alt_tar_record *pax;
	size_t npax;
	int format;                   /* reader: its best guess; writer: the formats allowed (0: any) */
} alt_tar_header;

/* the value of a PAX record of h: true when it exists */
bool alt_tar_header_pax(const alt_tar_header *h, alt_str key, alt_str *value);

/* ---- reading ---- */

typedef struct alt_tar_reader alt_tar_reader;

/* a reader of the archive in src (read without buffering: give it an
   alt_bufreader for small reads from a descriptor) */
int alt_tar_reader_new(alt_tar_reader **out, alt_reader src);
void alt_tar_reader_free(alt_tar_reader *r);
/* when on, alt_tar_next fails an entry whose name is not local
   (absolute, with .., ...) with -ALT_ETAR_INSECURE_PATH, still giving
   *h, as Go does with GODEBUG=tarinsecurepath=0; the default is off */
void alt_tar_reader_check_paths(alt_tar_reader *r, bool on);
/* advances to the next entry, skipping the rest of the current one:
   0 with *h (valid until the next call), -ALT_EEOF at the end of the
   archive, or an error, which stays (later calls return it again) */
int alt_tar_next(alt_tar_reader *r, alt_tar_header **h);
/* the data of the current entry: a count, 0 at its end, or an error */
ssize_t alt_tar_read(alt_tar_reader *r, void *buf, size_t n);
alt_reader alt_tar_reader_reader(alt_tar_reader *r);
/* Go's text of the error last returned ("archive/tar: invalid tar
   header", "unexpected EOF", "EOF"...); "" when none */
const char *alt_tar_reader_error(const alt_tar_reader *r);

/* ---- writing ---- */

typedef struct alt_tar_writer alt_tar_writer;

int alt_tar_writer_new(alt_tar_writer **out, alt_writer dst);
void alt_tar_writer_free(alt_tar_writer *w);
/* writes the header of the next entry (finishing the current one, which
   must have got all its data). A header that cannot be encoded fails
   with -ALT_ETAR_FIELD and leaves the writer usable; errors of dst stay. */
int alt_tar_write_header(alt_tar_writer *w, const alt_tar_header *h);
/* the data of the entry: n, or fewer bytes when n exceeds what the
   header's size has left, then -ALT_ETAR_WRITE_TOO_LONG */
ssize_t alt_tar_write(alt_tar_writer *w, const void *p, size_t n);
alt_writer alt_tar_writer_writer(alt_tar_writer *w);
/* writes the padding of the current entry (complete) */
int alt_tar_writer_flush(alt_tar_writer *w);
/* ends the archive with two zero blocks; dst is not closed. Later
   writes fail with -ALT_ECLOSED. */
int alt_tar_writer_close(alt_tar_writer *w);
/* Go's text of the error last returned ("archive/tar: cannot encode
   header: ...", "archive/tar: missed writing 3 bytes"...); "" when none.
   Times in it are shown in UTC (Go uses the local zone). */
const char *alt_tar_writer_error(const alt_tar_writer *w);

#ifdef __cplusplus
}
#endif

#endif
