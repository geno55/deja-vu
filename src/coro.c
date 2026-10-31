// Coroutines on Windows fibers or POSIX ucontext; see coro.h.
#ifndef _WIN32
#define _XOPEN_SOURCE 700  // ucontext; before any header
#endif

#include "coro.h"

#include <stdlib.h>

static Coro *current;

#ifdef _WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600  // IsThreadAFiber
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct Coro {
  void *fiber, *caller;
  void (*fn)(void *);
  void *arg;
  int done;
};

static void CALLBACK fiber_main(void *p) {
  Coro *c = p;
  c->fn(c->arg);
  c->done = 1;
  for(;;) {
    SwitchToFiber(c->caller);  // a fiber's function must never return
  }
}

Coro *coro_create(void (*fn)(void *), void *arg, size_t stack_size) {
  Coro *c = calloc(1, sizeof(*c));
  if(!c) {
    return NULL;
  }
  c->fn = fn;
  c->arg = arg;
  c->fiber = CreateFiber(stack_size, fiber_main, c);
  if(!c->fiber) {
    free(c);
    return NULL;
  }
  return c;
}

void coro_resume(Coro *c) {
  if(c->done) {
    return;
  }
  if(!IsThreadAFiber()) {
    ConvertThreadToFiber(NULL);
  }
  c->caller = GetCurrentFiber();
  Coro *prev = current;
  current = c;
  SwitchToFiber(c->fiber);
  current = prev;
}

void coro_yield(void) {
  SwitchToFiber(current->caller);
}

void coro_destroy(Coro *c) {
  if(c) {
    DeleteFiber(c->fiber);
    free(c);
  }
}

#else  // ucontext

#ifdef __APPLE__
#pragma clang diagnostic ignored "-Wdeprecated-declarations"  // ucontext still works there
#endif
#include <ucontext.h>

struct Coro {
  ucontext_t ctx, caller;
  void *stack;
  void (*fn)(void *);
  void *arg;
  int done;
};

static void coro_main(void) {
  Coro *c = current;
  c->fn(c->arg);
  c->done = 1;
  swapcontext(&c->ctx, &c->caller);  // never resumed after this
}

Coro *coro_create(void (*fn)(void *), void *arg, size_t stack_size) {
  Coro *c = calloc(1, sizeof(*c));
  if(!c || !(c->stack = malloc(stack_size)) || getcontext(&c->ctx)) {
    if(c) {
      free(c->stack);
    }
    free(c);
    return NULL;
  }
  c->fn = fn;
  c->arg = arg;
  c->ctx.uc_stack.ss_sp = c->stack;
  c->ctx.uc_stack.ss_size = stack_size;
  c->ctx.uc_link = NULL;
  makecontext(&c->ctx, coro_main, 0);
  return c;
}

void coro_resume(Coro *c) {
  if(c->done) {
    return;
  }
  Coro *prev = current;
  current = c;
  swapcontext(&c->caller, &c->ctx);
  current = prev;
}

void coro_yield(void) {
  Coro *c = current;
  swapcontext(&c->ctx, &c->caller);
}

void coro_destroy(Coro *c) {
  if(c) {
    free(c->stack);
    free(c);
  }
}

#endif

Coro *coro_current(void) {
  return current;
}

int coro_done(const Coro *c) {
  return c->done;
}
