// Checks the APU (src/apu.c) without the ROM, against what the NESdev wiki says it does. The harness
// compares it with Mesen2 at every NMI of the movies; this keeps what can be told from the docs alone
// checked in CI.
#include <stdio.h>
#include <stdlib.h>

#include "apu.h"

static int failures;

#define CHECK(cond)                                                    \
  do {                                                                 \
    if(!(cond)) {                                                      \
      fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond); \
      failures++;                                                      \
    }                                                                  \
  } while(0)

enum {
  kRate = 48000,
  kRoom = 200000,
  kAfterWrite = 4,   // a write to $4017 takes effect by then
  kQuarterFrame = 7457,
  kHalfFrame = 14913,
  kFourStepEnd = 29830,
};

static int16_t samples[kRoom];

static void start(void) {
  apu_reset(kRate, samples, kRoom);
  apu_write(0x4017, 0x00);
  apu_run(kAfterWrite);
}

static void test_length_counter(void) {
  start();
  ApuLength *l = &g_apu.pulse[0].length;
  apu_write(0x4003, 3 << 3);  // not enabled yet: no load
  apu_run(1);
  CHECK(l->counter == 0);
  apu_write(0x4015, 0x01);
  apu_write(0x4003, 3 << 3);  // entry 3 of the table is 2
  apu_run(1);
  CHECK(l->counter == 2);
  apu_run(kHalfFrame);
  CHECK(l->counter == 1);
  apu_run(kFourStepEnd - kHalfFrame);
  CHECK(l->counter == 0);
  // $4015 clears it at once, and the halt flag stops it counting.
  start();
  apu_write(0x4015, 0x01);
  apu_write(0x4003, 1 << 3);  // 254
  apu_run(1);
  CHECK(l->counter == 254);
  apu_write(0x4000, 0x20);
  apu_run(kFourStepEnd * 2);
  CHECK(l->counter == 254);
  apu_write(0x4015, 0x00);
  CHECK(l->counter == 0);
}

static void test_envelope(void) {
  start();
  ApuEnvelope *e = &g_apu.pulse[0].envelope;
  apu_write(0x4000, 0x02);  // period 2, no loop
  apu_write(0x4003, 0x00);  // starts it
  CHECK(e->start == 1);
  apu_run(kQuarterFrame);
  CHECK(e->start == 0 && e->decay == 15 && e->divider == 2);
  apu_run(kHalfFrame - kQuarterFrame);  // divider 1
  CHECK(e->decay == 15 && e->divider == 1);
  apu_run(22371 - kHalfFrame);  // divider 0
  CHECK(e->decay == 15 && e->divider == 0);
  apu_run(29829 - 22371);  // the third clock since it started: one step down
  CHECK(e->decay == 14 && e->divider == 2);
}

static void test_pulse_timer(void) {
  start();
  apu_write(0x4015, 0x01);
  apu_write(0x4002, 0x10);
  apu_write(0x4003, 0x00);
  ApuPulse *p = &g_apu.pulse[0];
  CHECK(p->period == 0x10 && p->duty_pos == 0);
  // The timer takes what's left of its old count first; then a step every 2(p + 1) CPU cycles.
  while(p->duty_pos == 0) {
    apu_run(1);
  }
  unsigned first = p->duty_pos;
  CHECK(first == 7);
  int cycles = 0;
  while(p->duty_pos == first) {
    apu_run(1);
    cycles++;
  }
  CHECK(cycles == 2 * (0x10 + 1));
  CHECK(p->duty_pos == 6);
}

static void test_sweep_mute(void) {
  start();
  ApuPulse *p = &g_apu.pulse[0];
  apu_write(0x4002, 0x07);
  CHECK(apu_pulse_muted(p));  // period under 8
  apu_write(0x4002, 0x08);
  CHECK(!apu_pulse_muted(p));
  // A target over $7FF mutes it, even with the sweep off, unless it's going down.
  apu_write(0x4001, 0x01);
  apu_write(0x4003, 0x07);
  apu_write(0x4002, 0xFF);
  CHECK(apu_pulse_muted(p));
  apu_write(0x4001, 0x09);
  CHECK(!apu_pulse_muted(p));
}

static void test_sweep_target(void) {
  // Going down, pulse 1 subtracts one more than pulse 2.
  for(int channel = 0; channel < 2; channel++) {
    start();
    uint16_t base = (uint16_t)(0x4000 + channel * 4);
    apu_write((uint16_t)(base + 1), 0x89);  // on, period 0, negate, shift 1
    apu_write((uint16_t)(base + 2), 0x00);
    apu_write((uint16_t)(base + 3), 0x01);  // period $100
    apu_run(kHalfFrame);
    CHECK(g_apu.pulse[channel].period == (channel == 0 ? 0x7F : 0x80));
  }
}

static void test_triangle(void) {
  start();
  ApuTriangle *t = &g_apu.triangle;
  apu_write(0x400A, 0x10);
  apu_write(0x400B, 0x00);
  apu_run(1000);
  CHECK(t->step == 0);  // no length counter, no linear counter: it holds
  apu_write(0x4015, 0x04);
  apu_write(0x4008, 0x7F);
  apu_write(0x400B, 1 << 3);
  apu_run(1);
  CHECK(t->length.counter == 254);
  apu_run(kQuarterFrame);
  CHECK(t->linear_counter == 0x7F);  // the first quarter frame reloads it
  unsigned step = t->step;
  while(t->step == step) {
    apu_run(1);
  }
  step = t->step;
  int cycles = 0;
  while(t->step == step) {
    apu_run(1);
    cycles++;
  }
  CHECK(cycles == 0x10 + 1);  // a step every p + 1 CPU cycles
}

static void test_noise(void) {
  apu_reset(kRate, samples, kRoom);
  ApuNoise *n = &g_apu.noise;
  CHECK(n->shift == 1);
  apu_write(0x400E, 0x00);
  while(n->shift == 1) {
    apu_run(1);
  }
  CHECK(n->shift == 0x4000);  // bit 0 xor bit 1 goes in at the top
  apu_write(0x400E, 0x0F);
  CHECK(n->period == 4068 - 1);
  apu_write(0x400E, 0x80);
  CHECK(n->mode == 1 && n->period == 4 - 1);
}

static void test_five_step(void) {
  apu_reset(kRate, samples, kRoom);
  apu_write(0x4017, 0x80);
  apu_write(0x4015, 0x01);
  apu_write(0x4003, 3 << 3);
  apu_run(1);
  ApuLength *l = &g_apu.pulse[0].length;
  CHECK(l->counter == 2);
  apu_run(kAfterWrite);
  CHECK(l->counter == 1);  // the write clocks a half frame at once in this mode
  while(g_apu.sequencer != kHalfFrame - 1) {
    apu_run(1);
  }
  CHECK(l->counter == 1);
  apu_run(1);
  CHECK(l->counter == 0);
}

static void test_output(void) {
  start();
  apu_take_samples();
  apu_run(1789773);  // a second
  size_t count = apu_take_samples();
  CHECK(count >= kRate - 2 && count <= kRate + 2);
  int loudest = 0;
  for(size_t i = 0; i < count; i++) {
    loudest = abs(samples[i]) > loudest ? abs(samples[i]) : loudest;
  }
  CHECK(loudest == 0);  // silence, and no click at the start
  // A square wave of about 440 Hz: as many rising crossings as that, around zero.
  start();
  apu_write(0x4015, 0x01);
  apu_write(0x4000, 0xBF);  // duty 50%, constant volume 15
  apu_write(0x4002, 0xFD);
  apu_write(0x4003, 0x00);
  apu_take_samples();
  apu_run(1789773);
  count = apu_take_samples();
  int rises = 0;
  for(size_t i = 1; i < count; i++) {
    rises += samples[i - 1] < 0 && samples[i] >= 0;
  }
  CHECK(rises >= 435 && rises <= 445);
}

int main(void) {
  test_length_counter();
  test_envelope();
  test_pulse_timer();
  test_sweep_mute();
  test_sweep_target();
  test_triangle();
  test_noise();
  test_five_step();
  test_output();
  if(failures) {
    fprintf(stderr, "test_apu: %d checks failed\n", failures);
    return 1;
  }
  printf("test_apu: all checks passed\n");
  return 0;
}
