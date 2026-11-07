/* AltC standard library: timers (Go's time.Timer, time.Ticker,
   time.AfterFunc).

   A timer sends the monotonic time (alt_time) on its channel when it
   fires; the channel has capacity 1, so a ticker whose ticks are not
   received drops them, as in Go. alt_timer_after_func runs a function on
   the timer thread instead: it must be short (start a task or use the
   thread pool for real work). */
#ifndef ALT_TIMER_H
#define ALT_TIMER_H

#include <alt/errors.h>
#include <alt/chan.h>
#include <alt/time.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct alt_timer alt_timer;

/* fires once after d */
int alt_timer_new(alt_timer **out, alt_dur d);
/* fires every period */
int alt_ticker_new(alt_timer **out, alt_dur period);
/* calls fn(arg) once after d */
int alt_timer_after_func(alt_timer **out, alt_dur d, void (*fn)(void *), void *arg);

/* the channel of a timer or ticker (NULL for function timers) */
alt_chan *alt_timer_chan(alt_timer *t);
/* stops the timer: true when it had not fired yet (a function timer whose
   function runs is waited for) */
bool alt_timer_stop(alt_timer *t);
/* stops and starts again with a new duration (period for tickers) */
int alt_timer_reset(alt_timer *t, alt_dur d);
/* stops and frees */
void alt_timer_free(alt_timer *t);

#ifdef __cplusplus
}
#endif

#endif
