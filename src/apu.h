// The NES's sound chip: our own, from the NESdev wiki's description of the APU. The
// sound driver writes its registers; this makes the sound. The game doesn't use the DMC channel, so
// there is none here.
#ifndef DV_APU_H
#define DV_APU_H

#include <stddef.h>
#include <stdint.h>

enum { kApuFilterTaps = 64 };

typedef struct ApuEnvelope {
  uint8_t start, loop, constant;
  uint8_t volume;   // the register: the constant volume, or the decay's period
  uint8_t divider;
  uint8_t decay;
} ApuEnvelope;

// What pulse, triangle and noise have in common.
typedef struct ApuLength {
  uint8_t enabled;  // $4015
  uint8_t counter;
  uint8_t halt;
  // A write takes effect after the frame counter's step in that cycle, and a load is lost if that
  // step has just clocked the counter.
  uint8_t pending_load, counter_at_load, pending_halt;
} ApuLength;

typedef struct ApuPulse {
  uint8_t duty, duty_pos;
  uint16_t period;  // the register
  uint16_t timer, timer_period;  // in CPU cycles: a period of p is 2p + 1 once anything is written
  uint8_t sweep_on, sweep_negate, sweep_period, sweep_shift, sweep_divider, sweep_reload;
  uint8_t negate_extra;  // pulse 1's sweep subtracts one more than pulse 2's
  ApuLength length;
  ApuEnvelope envelope;
} ApuPulse;

typedef struct ApuTriangle {
  uint16_t period, timer;
  uint8_t step;
  uint8_t control, linear_reload_value, linear_reload, linear_counter;
  ApuLength length;
} ApuTriangle;

typedef struct ApuNoise {
  uint8_t mode;
  uint16_t period, timer;  // in CPU cycles
  uint16_t shift;
  ApuLength length;
  ApuEnvelope envelope;
} ApuNoise;

typedef struct Apu {
  ApuPulse pulse[2];
  ApuTriangle triangle;
  ApuNoise noise;

  uint64_t cycle;      // CPU cycles run
  uint16_t sequencer;  // the frame counter, in CPU cycles since it started over
  uint8_t five_step;
  uint8_t frame_write_delay, frame_write_value;  // a write to $4017 takes effect after 3 or 4 cycles

  // Output. The mix is averaged down to 4 times the sample rate, then a low-pass filter takes it to
  // the sample rate, so that what's above half of that doesn't fold back as noise.
  double cycles_per_step, until_step;
  double sum;
  uint32_t sum_cycles;
  double history[kApuFilterTaps];
  uint32_t history_pos, steps_to_sample;
  double previous_in, previous_out;
  int16_t *samples;
  size_t sample_count, sample_room;
} Apu;

extern Apu g_apu;

// Power-on. Samples come at `sample_rate` into a buffer the caller keeps, of `room` samples, which
// apu_take_samples empties.
void apu_reset(unsigned sample_rate, int16_t *samples, size_t room);

// Writes a register, $4000-$4017, at the current cycle.
void apu_write(uint16_t addr, uint8_t value);

// Runs `cycles` CPU cycles.
void apu_run(uint32_t cycles);

// How many samples are waiting in the buffer, and starts it over: the caller has the samples.
size_t apu_take_samples(void);

// A pulse channel's sweep is holding it silent: its period is too low, or its next would be too high.
int apu_pulse_muted(const ApuPulse *p);

#endif  // DV_APU_H
