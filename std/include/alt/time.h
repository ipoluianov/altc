/* AltC standard library: time (Go's time).

   alt_time is an instant of the monotonic clock in nanoseconds: it is not
   affected by changes of the system clock and is what deadlines and
   timeouts use. alt_dur is a duration in nanoseconds. alt_wall is
   calendar time: nanoseconds since 1970-01-01 00:00:00 UTC. The two kinds
   of instants are never mixed.

   Deadlines are absolute: alt_deadline(5 * ALT_SEC) is five seconds from
   now, ALT_FOREVER is no deadline, and ALT_NOWAIT (like any instant in
   the past) means "do not wait".

   Calendar dates are formatted and parsed with Go's reference layouts:
   the layout shows how the reference time Mon Jan 2 15:04:05 MST 2006
   would look (ALT_RFC3339 is "2006-01-02T15:04:05Z07:00"). */
#ifndef ALT_TIME_H
#define ALT_TIME_H

#include <alt/errors.h>
#include <alt/str.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t alt_time;
typedef int64_t alt_dur;
typedef int64_t alt_wall;

#define ALT_NS ((alt_dur)1)
#define ALT_US ((alt_dur)1000)
#define ALT_MS ((alt_dur)1000000)
#define ALT_SEC ((alt_dur)1000000000)
#define ALT_MIN (60 * ALT_SEC)
#define ALT_HOUR (60 * ALT_MIN)

#define ALT_FOREVER INT64_MAX
#define ALT_NOWAIT INT64_MIN

/* the monotonic clock */
alt_time alt_now(void);
/* now + d, saturating; d <= 0 gives an instant already passed */
alt_time alt_deadline(alt_dur d);
/* the time left until t (negative when passed; ALT_FOREVER for ALT_FOREVER) */
alt_dur alt_until(alt_time t);
alt_dur alt_since(alt_time t);
/* the earlier of two deadlines */
static inline alt_time alt_time_min(alt_time a, alt_time b) { return a < b ? a : b; }

/* calendar time */
alt_wall alt_wall_now(void);
static inline int64_t alt_wall_unix(alt_wall t) { return t >= 0 ? t / ALT_SEC : -((-t + ALT_SEC - 1) / ALT_SEC); }
static inline alt_wall alt_wall_from_unix(int64_t sec, int64_t nsec) { return sec * ALT_SEC + nsec; }

/* a time zone: an offset from UTC and its abbreviation */
typedef struct {
	int offset;    /* seconds east of UTC */
	char name[16]; /* "UTC", "CEST", or "" when unknown */
} alt_zone;

extern const alt_zone alt_zone_utc;
/* the local zone at instant t (the TZ variable, /etc/localtime) */
void alt_zone_local(alt_wall t, alt_zone *z);
/* a fixed zone */
alt_zone alt_zone_fixed(const char *name, int offset);

/* the calendar fields of an instant in a zone */
typedef struct {
	int64_t year;
	int month;  /* 1..12 */
	int day;    /* 1..31 */
	int hour, min, sec;
	int nsec;
	int wday;   /* 0 = Sunday */
	int yday;   /* 1..366 */
	int offset; /* seconds east of UTC */
} alt_date;

/* the date of t in z (NULL is UTC) */
void alt_wall_date(alt_wall t, const alt_zone *z, alt_date *d);
/* the instant of a date: fields out of range are normalized as in Go
   (month 13 is January of the next year), wday and yday are ignored */
alt_wall alt_date_wall(const alt_date *d);

/* layouts */
#define ALT_LAYOUT_ANSIC "Mon Jan _2 15:04:05 2006"
#define ALT_LAYOUT_RFC822 "02 Jan 06 15:04 MST"
#define ALT_LAYOUT_RFC850 "Monday, 02-Jan-06 15:04:05 MST"
#define ALT_LAYOUT_RFC1123 "Mon, 02 Jan 2006 15:04:05 MST"
#define ALT_LAYOUT_RFC1123Z "Mon, 02 Jan 2006 15:04:05 -0700"
#define ALT_LAYOUT_RFC3339 "2006-01-02T15:04:05Z07:00"
#define ALT_LAYOUT_RFC3339NANO "2006-01-02T15:04:05.999999999Z07:00"
#define ALT_LAYOUT_DATETIME "2006-01-02 15:04:05"
#define ALT_LAYOUT_DATE "2006-01-02"
#define ALT_LAYOUT_TIME "15:04:05"
/* HTTP dates (RFC 9110): always GMT */
#define ALT_LAYOUT_HTTP "Mon, 02 Jan 2006 15:04:05 GMT"

/* appends t in zone z (NULL is UTC) formatted by layout */
int alt_wall_format(alt_buf *b, alt_wall t, const alt_zone *z, const char *layout);
/* parses value by layout; *z (when not NULL) gets the zone of the value
   (UTC when the layout has none); -ALT_ESYNTAX or -ALT_ERANGE */
int alt_wall_parse(alt_str value, const char *layout, alt_wall *t, alt_zone *z);

/* "1h2m3.5s", "1.5ms", "0s", as Go's Duration.String */
int alt_dur_format(alt_buf *b, alt_dur d);
/* "300ms", "-1.5h", "2h45m" (units ns, us, µs, ms, s, m, h) */
int alt_dur_parse(alt_str s, alt_dur *d);

#ifdef __cplusplus
}
#endif

#endif
