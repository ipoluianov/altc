/* ZIP archives after Go's archive/zip (BSD-3-Clause, see LICENSE-Go):
   the parts shared by the reader and the writer (struct.go): file modes,
   MS-DOS times, detectUTF8, and the random access sources. */
#include "zip_internal.h"
#include <alt/unicode/utf8.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

/* Go's fs.FileMode bits */
enum {
	M_DIR = 1u << 31,
	M_SYMLINK = 1u << 27,
	M_DEVICE = 1u << 26,
	M_PIPE = 1u << 25,
	M_SOCKET = 1u << 24,
	M_SETUID = 1u << 23,
	M_SETGID = 1u << 22,
	M_CHAR = 1u << 21,
	M_STICKY = 1u << 20,
	M_IRREGULAR = 1u << 19,
	M_TYPE = M_DIR | M_SYMLINK | M_PIPE | M_SOCKET | M_DEVICE | M_CHAR | M_IRREGULAR,
};

/* Unix st_mode bits */
enum {
	S_FMT = 0xf000,
	S_SOCK = 0xc000,
	S_LNK = 0xa000,
	S_REG = 0x8000,
	S_BLK = 0x6000,
	S_DIR = 0x4000,
	S_CHR = 0x2000,
	S_FIFO = 0x1000,
	S_SUID = 0x800,
	S_SGID = 0x400,
	S_SVTX = 0x200,
	MSDOS_DIR = 0x10,
	MSDOS_READONLY = 0x01,
};

static uint32_t unix_to_filemode(uint32_t m) {
	uint32_t mode = m & 0777;
	switch (m & S_FMT) {
	case S_BLK: mode |= M_DEVICE; break;
	case S_CHR: mode |= M_DEVICE | M_CHAR; break;
	case S_DIR: mode |= M_DIR; break;
	case S_FIFO: mode |= M_PIPE; break;
	case S_LNK: mode |= M_SYMLINK; break;
	case S_SOCK: mode |= M_SOCKET; break;
	}
	if (m & S_SGID)
		mode |= M_SETGID;
	if (m & S_SUID)
		mode |= M_SETUID;
	if (m & S_SVTX)
		mode |= M_STICKY;
	return mode;
}

static uint32_t filemode_to_unix(uint32_t mode) {
	uint32_t m;
	switch (mode & M_TYPE) {
	case M_DIR: m = S_DIR; break;
	case M_SYMLINK: m = S_LNK; break;
	case M_PIPE: m = S_FIFO; break;
	case M_SOCKET: m = S_SOCK; break;
	case M_DEVICE: m = S_BLK; break;
	case M_DEVICE | M_CHAR: m = S_CHR; break;
	default: m = S_REG;
	}
	if (mode & M_SETUID)
		m |= S_SUID;
	if (mode & M_SETGID)
		m |= S_SGID;
	if (mode & M_STICKY)
		m |= S_SVTX;
	return m | (mode & 0777);
}

unsigned alt_zip_header_mode(const alt_zip_header *h) {
	uint32_t mode = 0;
	switch (h->creator_version >> 8) {
	case ALT_ZIP_CREATOR_UNIX:
	case ALT_ZIP_CREATOR_MACOSX: mode = unix_to_filemode(h->external_attrs >> 16); break;
	case ALT_ZIP_CREATOR_NTFS:
	case ALT_ZIP_CREATOR_VFAT:
	case ALT_ZIP_CREATOR_FAT:
		mode = h->external_attrs & MSDOS_DIR ? M_DIR | 0777 : 0666;
		if (h->external_attrs & MSDOS_READONLY)
			mode &= ~0222u;
		break;
	}
	if (h->name.n && h->name.p[h->name.n - 1] == '/')
		mode |= M_DIR;
	return filemode_to_unix(mode);
}

void alt_zip_header_set_mode(alt_zip_header *h, unsigned st_mode) {
	uint32_t mode = unix_to_filemode(st_mode);
	h->creator_version = (uint16_t)((h->creator_version & 0xff) | ALT_ZIP_CREATOR_UNIX << 8);
	h->external_attrs = filemode_to_unix(mode) << 16;
	if (mode & M_DIR)
		h->external_attrs |= MSDOS_DIR;
	if (!(mode & 0200))
		h->external_attrs |= MSDOS_READONLY;
}

/* days since 1970-01-01 of a proleptic Gregorian date (month 1..12) */
static int64_t days_from_civil(int64_t y, int m, int d) {
	y -= m <= 2;
	int64_t era = (y >= 0 ? y : y - 399) / 400;
	int64_t yoe = y - era * 400;
	int64_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
	int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
	return era * 146097 + doe - 719468;
}

int64_t alt__zip_dos_unix(uint16_t date, uint16_t time) {
	int64_t year = (date >> 9) + 1980;
	int m = (date >> 5 & 0xf) - 1; /* normalized as time.Date does */
	if (m < 0)
		m += 12, year--;
	else if (m >= 12)
		m -= 12, year++;
	int64_t days = days_from_civil(year, m + 1, 1) + (date & 0x1f) - 1;
	return days * 86400 + (time >> 11) * 3600 + (time >> 5 & 0x3f) * 60 + (time & 0x1f) * 2;
}

void alt__zip_dos_time(alt_wall t, int offset, uint16_t *date, uint16_t *time) {
	alt_zone z = alt_zone_fixed("", offset);
	alt_date d;
	alt_wall_date(t, &z, &d);
	*date = (uint16_t)(d.day + d.month * 32 + (d.year - 1980) * 512);
	*time = (uint16_t)(d.sec / 2 + d.min * 32 + d.hour * 2048);
}

alt_wall alt_zip_header_mod_time(const alt_zip_header *h) {
	return alt__zip_dos_unix(h->modified_date, h->modified_time) * ALT_SEC;
}

void alt_zip_header_set_mod_time(alt_zip_header *h, alt_wall t) {
	h->modified_set = true;
	h->modified_has_zone = false;
	h->modified = t;
	h->modified_offset = 0;
	alt__zip_dos_time(t, 0, &h->modified_date, &h->modified_time);
}

void alt__zip_detect_utf8(alt_str s, bool *valid, bool *require) {
	*valid = true;
	*require = false;
	for (size_t i = 0; i < s.n;) {
		int32_t r;
		size_t k = alt_utf8_decode(s.p + i, s.n - i, &r);
		i += k;
		if (r < 0x20 || r > 0x7d || r == 0x5c) {
			if (!alt_utf8_valid_rune(r) || (r == ALT_UTF8_RUNE_ERROR && k == 1)) {
				*valid = false;
				*require = false;
				return;
			}
			*require = true;
		}
	}
}

static ssize_t mem_read_at(void *obj, void *buf, size_t n, int64_t off) {
	alt_zip_mem *m = obj;
	if (off < 0)
		return -EINVAL;
	if ((uint64_t)off >= m->s.n)
		return 0;
	size_t k = m->s.n - (size_t)off;
	if (k > n)
		k = n;
	memcpy(buf, m->s.p + off, k);
	return (ssize_t)k;
}

alt_zip_reader_at alt_zip_mem_reader_at(alt_zip_mem *m, alt_str s) {
	m->s = s;
	return (alt_zip_reader_at){mem_read_at, m};
}

static ssize_t fd_read_at(void *obj, void *buf, size_t n, int64_t off) {
	int fd = *(int *)obj;
	if (off < 0)
		return -EINVAL;
	size_t got = 0;
	while (got < n) {
		ssize_t k = pread(fd, (char *)buf + got, n - got, (off_t)(off + (int64_t)got));
		if (k < 0) {
			if (errno == EINTR)
				continue;
			return -errno;
		}
		if (k == 0)
			break;
		got += (size_t)k;
	}
	return (ssize_t)got;
}

alt_zip_reader_at alt_zip_fd_reader_at(int *fd) { return (alt_zip_reader_at){fd_read_at, fd}; }
