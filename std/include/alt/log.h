/* AltC standard library: structured logging (Go's log/slog).

   A record has a time, a level, a message and attributes; the text
   format writes them as slog's TextHandler does

     time=2026-11-07T10:00:00.123+01:00 level=INFO msg="request done" path=/x ms=12

   and the JSON format as its JSONHandler. Loggers are safe to use from
   any task or thread. */
#ifndef ALT_LOG_H
#define ALT_LOG_H

#include <alt/errors.h>
#include <alt/io.h>
#include <alt/str.h>
#include <alt/time.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { ALT_LOG_DEBUG = -4, ALT_LOG_INFO = 0, ALT_LOG_WARN = 4, ALT_LOG_ERROR = 8 };
enum { ALT_LOG_TEXT, ALT_LOG_JSON };
enum { ALT_LOGV_STRING, ALT_LOGV_INT, ALT_LOGV_UINT, ALT_LOGV_FLOAT, ALT_LOGV_BOOL, ALT_LOGV_DUR, ALT_LOGV_ERR };

typedef struct {
	const char *key;
	int kind;
	union {
		alt_str s;
		long long i;
		unsigned long long u;
		double f;
		bool b;
		alt_dur d;
		int err;
	};
} alt_log_attr;

#define ALT_LOG_STR(k, v) ((alt_log_attr){.key = (k), .kind = ALT_LOGV_STRING, .s = alt_str_cstr(v)})
#define ALT_LOG_S(k, v) ((alt_log_attr){.key = (k), .kind = ALT_LOGV_STRING, .s = (v)})
#define ALT_LOG_INT(k, v) ((alt_log_attr){.key = (k), .kind = ALT_LOGV_INT, .i = (v)})
#define ALT_LOG_UINT(k, v) ((alt_log_attr){.key = (k), .kind = ALT_LOGV_UINT, .u = (v)})
#define ALT_LOG_FLOAT(k, v) ((alt_log_attr){.key = (k), .kind = ALT_LOGV_FLOAT, .f = (v)})
#define ALT_LOG_BOOL(k, v) ((alt_log_attr){.key = (k), .kind = ALT_LOGV_BOOL, .b = (v)})
#define ALT_LOG_DUR(k, v) ((alt_log_attr){.key = (k), .kind = ALT_LOGV_DUR, .d = (v)})
/* an error code, written with alt_strerror */
#define ALT_LOG_ERR(k, v) ((alt_log_attr){.key = (k), .kind = ALT_LOGV_ERR, .err = (v)})

typedef struct alt_logger alt_logger;

typedef struct {
	alt_writer out;     /* alt_fd_writer of stderr when .write is NULL */
	int format;         /* ALT_LOG_TEXT or ALT_LOG_JSON */
	int level;          /* records below are dropped */
	bool no_time;       /* leave the time out (tests) */
	bool utc;           /* times in UTC instead of the local zone */
} alt_logger_config;

int alt_logger_new(alt_logger **out, const alt_logger_config *cfg);
void alt_logger_free(alt_logger *l);
/* a logger adding attrs to every record (slog's With) */
int alt_logger_with(alt_logger **out, alt_logger *parent, const alt_log_attr *attrs, size_t n);
void alt_logger_set_level(alt_logger *l, int level);
bool alt_logger_enabled(const alt_logger *l, int level);
/* the default logger: text on stderr, level INFO */
alt_logger *alt_log_default(void);

void alt_log_attrs(alt_logger *l, int level, const char *msg, const alt_log_attr *attrs, size_t n);
void alt_logf(alt_logger *l, int level, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

/* alt_log_info(l, "msg", ALT_LOG_INT("n", 3), ALT_LOG_STR("who", "me")) */
#define ALT__LOG(l, level, msg, ...) \
	alt_log_attrs((l), (level), (msg), ((const alt_log_attr[]){{0}, ##__VA_ARGS__}) + 1, \
		sizeof((const alt_log_attr[]){{0}, ##__VA_ARGS__}) / sizeof(alt_log_attr) - 1)
#define alt_log_debug(l, msg, ...) ALT__LOG(l, ALT_LOG_DEBUG, msg, ##__VA_ARGS__)
#define alt_log_info(l, msg, ...) ALT__LOG(l, ALT_LOG_INFO, msg, ##__VA_ARGS__)
#define alt_log_warn(l, msg, ...) ALT__LOG(l, ALT_LOG_WARN, msg, ##__VA_ARGS__)
#define alt_log_error(l, msg, ...) ALT__LOG(l, ALT_LOG_ERROR, msg, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
