/* AltC standard library: contexts (Go's context).

   A context carries a cancellation signal, a deadline and values through
   a chain of operations: blocking operations of the network and HTTP
   packages take one and give up with -ECANCELED when it is cancelled and
   -ETIMEDOUT at its deadline. NULL is the background context: never done,
   no deadline, no values.

   Contexts form trees: cancelling a context cancels its descendants, and
   a child's deadline is never later than its parent's. Free a context
   with alt_ctx_free when done with it (it is cancelled first); freeing a
   parent detaches its children, which stay valid until freed. All
   functions are thread-safe. */
#ifndef ALT_CONTEXT_H
#define ALT_CONTEXT_H

#include <alt/errors.h>
#include <alt/time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct alt_ctx alt_ctx;
struct alt_chan;

/* a cancellable child of parent (NULL: background) */
int alt_ctx_new(alt_ctx **out, alt_ctx *parent);
/* a child done at the deadline (or earlier, with its parent) */
int alt_ctx_with_deadline(alt_ctx **out, alt_ctx *parent, alt_time deadline);
int alt_ctx_with_timeout(alt_ctx **out, alt_ctx *parent, alt_dur d);
/* a child carrying a value for key (compared by address) */
int alt_ctx_with_value(alt_ctx **out, alt_ctx *parent, const void *key, void *value);

void alt_ctx_cancel(alt_ctx *c);
void alt_ctx_free(alt_ctx *c);

/* 0 while not done; -ECANCELED once cancelled, -ETIMEDOUT past the deadline */
int alt_ctx_err(alt_ctx *c);
/* ALT_FOREVER without a deadline */
alt_time alt_ctx_deadline(const alt_ctx *c);
/* the value of the nearest context carrying key, NULL if none */
void *alt_ctx_value(const alt_ctx *c, const void *key);
/* waits until c is done or the deadline passes: the context's error, or
   0 at the deadline (with c still running) */
int alt_ctx_wait(alt_ctx *c, alt_time deadline);
/* a channel that is closed when c is done (for alt_select); NULL for the
   background context, which is never done */
struct alt_chan *alt_ctx_done(alt_ctx *c);

#ifdef __cplusplus
}
#endif

#endif
