// The sound chip; see apu.h. Everything steps once per CPU cycle, which on a PC is cheap enough, and
// keeps the code as plain as the wiki's description of it.
#include "apu.h"

#include <math.h>
#include <string.h>

Apu g_apu;

enum {
  kCpuClock = 1789773,
  // The frame counter's steps, in CPU cycles since it started over.
  kStep1 = 7457,
  kStep2 = 14913,
  kStep3 = 22371,
  kStep4 = 29829,   // 4-step mode: also the end, a cycle later
  kStep5 = 37281,   // 5-step mode only
  kFilterSteps = 4,  // steps of the filter's input per sample of output
  kOutputScale = 20000,  // about as loud as Mesen2 at its default volume, with room to spare
};

static const uint8_t kLengths[32] = {
  10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
  12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30,
};
// The duties as they play out from a write to $4003, step by step. The position counts down, and the
// NESdev wiki has the chip read this table with it (0, 7, 6, ...), which starts each pulse later; Mesen2
// goes through it in this order. It can't be heard, but the waveforms can only be compared if they agree.
static const uint8_t kDuty[4][8] = {
  {0, 1, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 0, 0, 0},
  {1, 0, 0, 1, 1, 1, 1, 1},
};
static const uint8_t kTriangle[32] = {
  15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
};
static const uint16_t kNoisePeriods[16] = {
  4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068,
};

// The chip's nonlinear mixer: the output for each sum of the pulses, and of triangle and noise.
static double mix_pulse[31];
static double mix_tnd[16][16];

// A windowed sinc, cutting off just below half the sample rate.
static double filter[kApuFilterTaps];

static void make_filter(void) {
  static const double kPi = 3.14159265358979323846;
  static const double kCutoff = 0.80 / (2.0 * kFilterSteps);  // of the input's rate
  double total = 0;
  for(int i = 0; i < kApuFilterTaps; i++) {
    double x = i - (kApuFilterTaps - 1) / 2.0;
    double sinc = sin(2 * kPi * kCutoff * x) / (kPi * x);
    double window = 0.42 + 0.5 * cos(2 * kPi * x / kApuFilterTaps) + 0.08 * cos(4 * kPi * x / kApuFilterTaps);
    filter[i] = sinc * window;
    total += filter[i];
  }
  for(int i = 0; i < kApuFilterTaps; i++) {
    filter[i] /= total;
  }
}

static void make_mix_tables(void) {
  for(int n = 1; n < 31; n++) {
    mix_pulse[n] = 95.88 / (8128.0 / n + 100.0);
  }
  for(int t = 0; t < 16; t++) {
    for(int n = 0; n < 16; n++) {
      double sum = t / 8227.0 + n / 12241.0;
      mix_tnd[t][n] = sum > 0 ? 159.79 / (1.0 / sum + 100.0) : 0;
    }
  }
}

void apu_reset(unsigned sample_rate, int16_t *samples, size_t room) {
  memset(&g_apu, 0, sizeof(g_apu));
  make_mix_tables();
  make_filter();
  g_apu.pulse[0].negate_extra = 1;
  g_apu.noise.shift = 1;
  g_apu.noise.period = (uint16_t)(kNoisePeriods[0] - 1);
  g_apu.samples = samples;
  g_apu.sample_room = room;
  g_apu.cycles_per_step = (double)kCpuClock / sample_rate / kFilterSteps;
  g_apu.until_step = g_apu.cycles_per_step;
  g_apu.steps_to_sample = kFilterSteps;
  // The triangle rests at its first step, 15, from power-on. The output starts settled there, so there
  // is no click.
  double resting = mix_tnd[kTriangle[0]][0];
  for(int i = 0; i < kApuFilterTaps; i++) {
    g_apu.history[i] = resting;
  }
  g_apu.previous_in = resting;
}

// ---- Registers -----------------------------------------------------------------------------

static void length_load(ApuLength *l, uint8_t value) {
  if(l->enabled) {
    l->pending_load = kLengths[value >> 3];
    l->counter_at_load = l->counter;
  }
}

static void length_enable(ApuLength *l, int on) {
  l->enabled = (uint8_t)on;
  if(!on) {
    l->counter = 0;
  }
}

static void envelope_write(ApuEnvelope *e, ApuLength *l, uint8_t value) {
  e->loop = value >> 5 & 1;
  l->pending_halt = e->loop;
  e->constant = value >> 4 & 1;
  e->volume = value & 0x0F;
}

static void pulse_set_period(ApuPulse *p, unsigned period) {
  p->period = (uint16_t)period;
  p->timer_period = (uint16_t)(period * 2 + 1);
}

static void pulse_write(ApuPulse *p, unsigned reg, uint8_t value) {
  switch(reg) {
    case 0:
      p->duty = value >> 6;
      envelope_write(&p->envelope, &p->length, value);
      break;
    case 1:
      p->sweep_on = value >> 7;
      p->sweep_period = value >> 4 & 7;
      p->sweep_negate = value >> 3 & 1;
      p->sweep_shift = value & 7;
      p->sweep_reload = 1;
      break;
    case 2:
      pulse_set_period(p, (p->period & 0x700) | value);
      break;
    default:
      pulse_set_period(p, (p->period & 0xFF) | (value & 7) << 8);
      length_load(&p->length, value);
      p->duty_pos = 0;
      p->envelope.start = 1;
      break;
  }
}

void apu_write(uint16_t addr, uint8_t value) {
  Apu *a = &g_apu;
  switch(addr) {
    case 0x4000: case 0x4001: case 0x4002: case 0x4003:
      pulse_write(&a->pulse[0], addr & 3, value);
      break;
    case 0x4004: case 0x4005: case 0x4006: case 0x4007:
      pulse_write(&a->pulse[1], addr & 3, value);
      break;
    case 0x4008:
      a->triangle.control = value >> 7;
      a->triangle.length.pending_halt = value >> 7;
      a->triangle.linear_reload_value = value & 0x7F;
      break;
    case 0x400A:
      a->triangle.period = (uint16_t)((a->triangle.period & 0x700) | value);
      break;
    case 0x400B:
      a->triangle.period = (uint16_t)((a->triangle.period & 0xFF) | (value & 7) << 8);
      length_load(&a->triangle.length, value);
      a->triangle.linear_reload = 1;
      break;
    case 0x400C:
      envelope_write(&a->noise.envelope, &a->noise.length, value);
      break;
    case 0x400E:
      a->noise.mode = value >> 7;
      a->noise.period = (uint16_t)(kNoisePeriods[value & 15] - 1);
      break;
    case 0x400F:
      length_load(&a->noise.length, value);
      a->noise.envelope.start = 1;
      break;
    case 0x4015:
      length_enable(&a->pulse[0].length, value & 1);
      length_enable(&a->pulse[1].length, value >> 1 & 1);
      length_enable(&a->triangle.length, value >> 2 & 1);
      length_enable(&a->noise.length, value >> 3 & 1);
      break;
    case 0x4017:
      a->frame_write_value = value;
      // After 3 cycles, or 4 if this one falls between two of the APU's (every other CPU cycle).
      a->frame_write_delay = a->cycle & 1 ? 3 : 4;
      break;
    default:
      break;
  }
}

// ---- The frame counter's clocks ------------------------------------------------------------

static void clock_envelope(ApuEnvelope *e) {
  if(e->start) {
    e->start = 0;
    e->decay = 15;
    e->divider = e->volume;
  } else if(e->divider) {
    e->divider--;
  } else {
    e->divider = e->volume;
    if(e->decay) {
      e->decay--;
    } else if(e->loop) {
      e->decay = 15;
    }
  }
}

static unsigned pulse_target(const ApuPulse *p) {
  unsigned change = p->period >> p->sweep_shift;
  if(!p->sweep_negate) {
    return p->period + change;
  }
  unsigned less = change + p->negate_extra;
  return p->period >= less ? p->period - less : 0;
}

int apu_pulse_muted(const ApuPulse *p) {
  return p->period < 8 || (!p->sweep_negate && pulse_target(p) > 0x7FF);
}

static void clock_sweep(ApuPulse *p) {
  if(!p->sweep_divider && p->sweep_on && p->sweep_shift && !apu_pulse_muted(p)) {
    pulse_set_period(p, pulse_target(p));
  }
  if(!p->sweep_divider || p->sweep_reload) {
    p->sweep_divider = p->sweep_period;
    p->sweep_reload = 0;
  } else {
    p->sweep_divider--;
  }
}

static void clock_length(ApuLength *l) {
  if(l->counter && !l->halt) {
    l->counter--;
  }
}

static void clock_quarter(Apu *a) {
  clock_envelope(&a->pulse[0].envelope);
  clock_envelope(&a->pulse[1].envelope);
  clock_envelope(&a->noise.envelope);
  ApuTriangle *t = &a->triangle;
  if(t->linear_reload) {
    t->linear_counter = t->linear_reload_value;
  } else if(t->linear_counter) {
    t->linear_counter--;
  }
  if(!t->control) {
    t->linear_reload = 0;
  }
}

static void clock_half(Apu *a) {
  clock_length(&a->pulse[0].length);
  clock_length(&a->pulse[1].length);
  clock_length(&a->triangle.length);
  clock_length(&a->noise.length);
  clock_sweep(&a->pulse[0]);
  clock_sweep(&a->pulse[1]);
}

static void step_frame_counter(Apu *a) {
  a->sequencer++;
  if(a->frame_write_delay && !--a->frame_write_delay) {
    a->five_step = a->frame_write_value >> 7;
    a->sequencer = 0;
    if(a->five_step) {
      clock_quarter(a);
      clock_half(a);
    }
    return;
  }
  switch(a->sequencer) {
    case kStep1:
    case kStep3:
      clock_quarter(a);
      break;
    case kStep2:
      clock_quarter(a);
      clock_half(a);
      break;
    case kStep4:
      if(!a->five_step) {
        clock_quarter(a);
        clock_half(a);
      }
      break;
    case kStep4 + 1:
      if(!a->five_step) {
        a->sequencer = 0;
      }
      break;
    case kStep5:
      clock_quarter(a);
      clock_half(a);
      break;
    case kStep5 + 1:
      a->sequencer = 0;
      break;
    default:
      break;
  }
}

static void apply_length_writes(ApuLength *l) {
  if(l->pending_load) {
    if(l->counter == l->counter_at_load) {
      l->counter = l->pending_load;
    }
    l->pending_load = 0;
  }
  l->halt = l->pending_halt;
}

// ---- The channels and the output -----------------------------------------------------------

static unsigned envelope_output(const ApuEnvelope *e) {
  return e->constant ? e->volume : e->decay;
}

static unsigned pulse_output(const ApuPulse *p) {
  if(!p->length.counter || !kDuty[p->duty][(8 - p->duty_pos) & 7] || apu_pulse_muted(p)) {
    return 0;
  }
  return envelope_output(&p->envelope);
}

// A step of the filter's input, and every so often a sample.
static void filter_step(Apu *a) {
  a->history[a->history_pos] = a->sum / a->sum_cycles;
  a->history_pos = (a->history_pos + 1) % kApuFilterTaps;
  a->sum = 0;
  a->sum_cycles = 0;
  if(--a->steps_to_sample) {
    return;
  }
  a->steps_to_sample = kFilterSteps;
  double in = 0;
  for(int i = 0; i < kApuFilterTaps; i++) {
    in += filter[i] * a->history[(a->history_pos + (uint32_t)i) % kApuFilterTaps];
  }
  // A slow high-pass at about 25 Hz, as the chip's output isn't centered on zero.
  double out = in - a->previous_in + a->previous_out * (1.0 - 1.0 / 300);
  a->previous_in = in;
  a->previous_out = out;
  if(a->sample_count < a->sample_room) {
    double scaled = out * kOutputScale;
    a->samples[a->sample_count++] = (int16_t)(scaled > 32767 ? 32767 : scaled < -32768 ? -32768 : scaled);
  }
}

void apu_run(uint32_t cycles) {
  Apu *a = &g_apu;
  for(uint32_t i = 0; i < cycles; i++) {
    a->cycle++;
    step_frame_counter(a);
    apply_length_writes(&a->pulse[0].length);
    apply_length_writes(&a->pulse[1].length);
    apply_length_writes(&a->triangle.length);
    apply_length_writes(&a->noise.length);

    for(int c = 0; c < 2; c++) {
      ApuPulse *p = &a->pulse[c];
      if(p->timer) {
        p->timer--;
      } else {
        p->timer = p->timer_period;
        p->duty_pos = (p->duty_pos - 1) & 7;
      }
    }
    ApuTriangle *t = &a->triangle;
    if(t->timer) {
      t->timer--;
    } else {
      t->timer = t->period;
      if(t->length.counter && t->linear_counter) {
        t->step = (t->step + 1) & 31;
      }
    }
    ApuNoise *n = &a->noise;
    if(n->timer) {
      n->timer--;
    } else {
      n->timer = n->period;
      unsigned feedback = (n->shift ^ n->shift >> (n->mode ? 6 : 1)) & 1;
      n->shift = (uint16_t)(n->shift >> 1 | feedback << 14);
    }

    unsigned noise = n->length.counter && !(n->shift & 1) ? envelope_output(&n->envelope) : 0;
    a->sum += mix_pulse[pulse_output(&a->pulse[0]) + pulse_output(&a->pulse[1])] +
              mix_tnd[kTriangle[t->step]][noise];
    a->sum_cycles++;
    if(--a->until_step <= 0) {
      a->until_step += a->cycles_per_step;
      filter_step(a);
    }
  }
}

size_t apu_take_samples(void) {
  size_t count = g_apu.sample_count;
  g_apu.sample_count = 0;
  return count;
}
