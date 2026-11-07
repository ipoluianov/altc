/* AltC standard library: channels (Go's chan and select).

   A channel passes elements of a fixed size by value between tasks and
   threads, in any combination. With capacity 0 a send waits for a
   receiver (and the other way round); with capacity n up to n elements
   wait in the channel. Elements of size 0 make signal channels (closing
   one wakes every receiver).

   Every blocking call takes a deadline: ALT_FOREVER waits as long as
   needed, ALT_NOWAIT never waits (-EAGAIN when the operation cannot
   proceed at once), other instants give -ETIMEDOUT when they pass.

   Closing a channel wakes all receivers once it is empty (they get
   -ALT_ECLOSED and a zeroed element) and all waiting senders (they get
   -ALT_ECLOSED, where Go would panic). Free a channel only when nothing
   uses it any more. */
#ifndef ALT_CHAN_H
#define ALT_CHAN_H

#include <alt/errors.h>
#include <alt/time.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct alt_chan alt_chan;

int alt_chan_new(alt_chan **out, size_t elem_size, size_t cap);
void alt_chan_free(alt_chan *c);

/* 0, -ALT_ECLOSED, -ETIMEDOUT, -EAGAIN (ALT_NOWAIT) */
int alt_chan_send(alt_chan *c, const void *elem, alt_time deadline);
/* 0, -ALT_ECLOSED (closed and empty: *elem zeroed), -ETIMEDOUT, -EAGAIN */
int alt_chan_recv(alt_chan *c, void *elem, alt_time deadline);
/* 0, -ALT_ECLOSED when already closed */
int alt_chan_close(alt_chan *c);

size_t alt_chan_len(alt_chan *c);
size_t alt_chan_cap(const alt_chan *c);
bool alt_chan_closed(alt_chan *c);

enum { ALT_SEND = 1, ALT_RECV = 2 };

typedef struct {
	int op;       /* ALT_SEND or ALT_RECV; 0 (or a NULL ch) disables the case */
	alt_chan *ch;
	void *elem;   /* what to send, where to receive (may be NULL for receives) */
	bool ok;      /* out: false when the case completed because the channel is closed */
} alt_case;

/* waits until one of the cases can proceed and performs it: the index of
   that case (chosen at random among the ready ones), -ETIMEDOUT, or
   -EAGAIN with ALT_NOWAIT when none is ready */
int alt_select(alt_case *cases, int n, alt_time deadline);

#ifdef __cplusplus
}
#endif

#endif
