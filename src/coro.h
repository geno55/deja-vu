// A coroutine: a function that runs on its own stack and can pause itself (coro_yield) and be
// carried on later (coro_resume). The game runs as one: it waits for vblank deep
// inside its call stack, and waiting becomes a yield to the frame driver.
//
// Windows fibers on Windows, ucontext elsewhere. One thread only.
#ifndef DV_CORO_H
#define DV_CORO_H

#include <stddef.h>

typedef struct Coro Coro;

// Makes a coroutine that will run fn(arg) when first resumed. NULL if it can't.
Coro *coro_create(void (*fn)(void *arg), void *arg, size_t stack_size);

// Runs the coroutine until it yields or fn returns. Not from inside the same coroutine.
void coro_resume(Coro *c);

// From inside a coroutine: back to the coro_resume that ran it.
void coro_yield(void);

// The coroutine running now, or NULL outside any.
Coro *coro_current(void);

// Whether fn has returned (resuming it then does nothing).
int coro_done(const Coro *c);

// Frees a coroutine that isn't running. Its stack is dropped as it is: anything fn had
// allocated stays allocated.
void coro_destroy(Coro *c);

#endif  // DV_CORO_H
