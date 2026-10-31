// The sound driver (the data is in docs/sound.md): play_sound starts a
// sound, and the NMI's sound_update steps nine channel slots of 24 bytes at $0700, then writes the
// APU's registers from the slots that have each APU channel.
//
// No NMI lands in what only the handler runs, as the handler turns the NMI off first. One can land
// in what play_sound runs, but it sees system_flags bit 2, so it leaves the sound alone and asks for
// a double step next time. None of these wait, so a guard at the top of each is enough. $44-$4B and
// $51-$53 are the driver's scratch.
#include "funcs.h"
#include "variables.h"

// A channel slot's bytes.
enum {
  kChFlags = 0,       // kChPlaying, kChNewNote, kChRest
  kChStreamLo = 1,    // the next command
  kChStreamHi = 2,
  kChLengthLo = 3,    // a note adds 8 times this to the time left
  kChLengthHi = 4,
  kChTimeLo = 5,      // counts down by the speed; the next command comes when it goes negative
  kChTimeHi = 6,
  kChSpeedLo = 7,
  kChSpeedHi = 8,
  kChEnvSpeed = 9,    // pulse: frames per envelope step, bit 7 no envelope
  kChEnvLo = 10,      // pulse: the volume envelope; noise: 3 register bytes per note
  kChEnvHi = 11,
  kChEnvOn = 12,      // pulse: the envelope runs; triangle: the note's countdown runs
  kChEnvPos = 13,     // pulse: the envelope's step; triangle: what the note's countdown multiplies
  kChEnvWait = 14,
  kChLoopIndex = 15,  // into the loop stacks at $0110
  kChReg0 = 16,       // what goes to the APU channel's four registers
  kChReg1 = 17,
  kChReg2 = 18,
  kChReg3 = 19,
  kChTriPeriod = 20,  // triangle: the note's period, high byte
  kChTriMode = 21,    // triangle: 1 and 2 cut the note short after a time
  kChTriTimeLo = 22,  // triangle: the note's countdown
  kChTriTimeHi = 23,

  kChPlaying = 0x80,
  kChNewNote = 0x40,  // write registers 1-3 too, which restarts the note
  kChRest = 0x20,

  kStreamEnd = 0xFF,
  kStreamWait = 0x7E,
  kStreamRest = 0x7F,
  kStreamLoopEnd = 0xCF,
};

// sound_channels+offset,X. A made-up X runs on past the end of RAM into its mirror, as on the
// console.
#define CH(offset) (g_ram[(0x700u + (offset) + g_nes.cpu.x) & 0x7FFu])

// $0110+offset,Y: an entry of the loop stacks, 4 bytes (passes done, passes, where the loop starts).
static uint8_t *loop_entry(int offset) {
  uint16_t addr = (uint16_t)(0x110 + offset + g_nes.cpu.y);
  emu_note_loop_stack(addr);
  return &g_ram[addr];
}

// sound_reset (7:F3F5): all channels off, nothing to repeat. A and X are kept (it pushes them), with
// the flags of the PLA and the carry of its CPX.
void sound_reset(void) {
  RUN_ORIGINAL_IF_LAG(kSoundReset);
  for(int i = 0; i < 0xD8; i++) {
    sound_channels[i] = 0;
  }
  repeat_sound = 0;
  emu_cmp(0xD8, 0xD8);
  emu_set_nz(g_nes.cpu.a);
}

// play_sound (7:F409): sound number from 1, 0 stops all. The music that repeats isn't started again.
void play_sound(uint8_t sound) {
  emu_set_a(sound);
  RUN_ORIGINAL_IF_LAG(kPlaySound);
  system_flags |= 0x04;
  push_and_switch_bank(5);
  g_ram[0x44] = sound;
  emu_set_a(sound);
  if(!sound) {
    sound_reset();
  } else if(sound != repeat_sound) {
    g_ram[0x44]--;
    start_sound();
  }
  pop_bank();
  system_flags &= 0xFB;
  emu_set_a(system_flags);
}

// sound_check_repeat (7:F433), when a channel's stream has ended: if no music channel plays any
// more, starts the repeating music again, or else stops everything, effects too.
void sound_check_repeat(void) {
  RUN_ORIGINAL_IF_LAG(kSoundCheckRepeat);
  emu_set_a(sound_channels[0x00] | sound_channels[0x18] | sound_channels[0x30] |
            sound_channels[0x48]);
  if(g_nes.cpu.a & kChPlaying) {
    return;
  }
  emu_set_a(repeat_sound);
  g_ram[0x44] = repeat_sound;
  if(!repeat_sound) {
    sound_reset();
    return;
  }
  g_ram[0x44]--;
  start_sound();
}

// start_sound (7:F44D): $44 = number - 1. Keeps X and Y; returns A = X.
void start_sound(void) {
  RUN_ORIGINAL_IF_LAG(kStartSound);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t x = c->x, y = c->y;
  c->y = (uint8_t)(g_ram[0x44] << 1);
  g_ram[0x45] = emu_read_ind_y(0x3B, c->y);  // sound_table_ptr
  c->y++;
  g_ram[0x46] = emu_read_ind_y(0x3B, c->y);
  sound_start_by_type();
  c->y = y;
  c->x = x;
  emu_set_a(x);
}

// sound_start_by_type (7:F466), with sound_jump_table (7:FADA) inline, which leaves $53 = Y, $51/$52
// the address, A its low byte, and the flags of its last LDY. $45/$46 = the sound's header. A type
// past the list only comes with a made-up header: the original jumps wherever the bytes after it say.
void sound_start_by_type(void) {
  Cpu6502 *c = &g_nes.cpu;
  if(emu_lag_in(kSoundStartByType) || emu_lag_in(kSoundJumpTable)) {
    emu_call_original(kSoundStartByType);
    return;
  }
  c->y = 0;
  uint8_t type = emu_read_ind_y(0x45, 0);
  if(type >= 5) {
    emu_note_bad_index();
    emu_call_original(kSoundStartByType);
    return;
  }
  g_ram[0x53] = 0;
  uint16_t entry = (uint16_t)(0xF46D + type * 2);  // the list after the JSR at 7:F46A
  g_ram[0x51] = cpu_bus_read(entry);
  g_ram[0x52] = cpu_bus_read((uint16_t)(entry + 1));
  emu_set_a(g_ram[0x51]);
  emu_set_nz(0);
  emu_set_flag(kFlagC, 0);
  switch(type) {
    case 0: sound_start_music(); break;
    case 1: sound_start_repeating_music(); break;
    case 2: sound_start_effect_3ch(); break;
    default: sound_start_effect_2ch(); break;
  }
}

// sound_start_music (7:F477): the four music channels, from the header's four streams.
void sound_start_music(void) {
  RUN_ORIGINAL_IF_LAG(kSoundStartMusic);
  Cpu6502 *c = &g_nes.cpu;
  c->x = 0x00;
  c->y = 1;
  sound_start_channel();
  sound_start_channel();
  sound_start_channel();
  sound_start_channel();
  c->y = (uint8_t)(c->y + 2);
  uint8_t speed = emu_read_ind_y(0x45, c->y);
  sound_channels[0x00 + kChSpeedLo] = speed;
  sound_channels[0x18 + kChSpeedLo] = speed;
  sound_channels[0x30 + kChSpeedLo] = speed;
  sound_channels[0x48 + kChSpeedLo] = speed;
  sound_channels[0x00 + kChLoopIndex] = 0x00;
  sound_channels[0x18 + kChLoopIndex] = 0x10;
  sound_channels[0x30 + kChLoopIndex] = 0x20;
  sound_channels[0x48 + kChLoopIndex] = 0x30;
  repeat_sound = 0;
  emu_set_a(0);
}

// sound_start_channel (7:F4B0): the slot at X from the stream address at header byte Y; a stream of
// 0 stays off. Moves Y past it and X to the next slot. Returns A = X, the flags of the ADC.
void sound_start_channel(void) {
  RUN_ORIGINAL_IF_LAG(kSoundStartChannel);
  Cpu6502 *c = &g_nes.cpu;
  CH(kChFlags) = kChPlaying;
  CH(kChStreamLo) = emu_read_ind_y(0x45, c->y);
  c->y++;
  CH(kChStreamHi) = emu_read_ind_y(0x45, c->y);
  c->y++;
  CH(kChSpeedHi) = 0;
  CH(kChTimeHi) = 0;
  CH(kChTimeLo) = 1;
  if(!(CH(kChStreamLo) | CH(kChStreamHi))) {
    CH(kChFlags) = 0;
  }
  emu_set_flag(kFlagC, 0);
  c->a = emu_adc(c->x, 0x18);
  c->x = c->a;
}

// sound_start_repeating_music (7:F4E1)
void sound_start_repeating_music(void) {
  RUN_ORIGINAL_IF_LAG(kSoundStartRepeatingMusic);
  sound_start_music();
  g_nes.cpu.a = g_ram[0x44];
  repeat_sound = (uint8_t)(g_ram[0x44] + 1);
  emu_set_nz(repeat_sound);
}

// sound_start_effect_3ch (7:F4EB): slots 4-6, from the first three streams.
void sound_start_effect_3ch(void) {
  RUN_ORIGINAL_IF_LAG(kSoundStartEffect3ch);
  Cpu6502 *c = &g_nes.cpu;
  c->x = 0x60;
  c->y = 1;
  sound_start_channel();
  sound_start_channel();
  sound_start_channel();
  c->y = (uint8_t)(c->y + 4);
  uint8_t speed = emu_read_ind_y(0x45, c->y);
  sound_channels[0x60 + kChSpeedLo] = speed;
  sound_channels[0x78 + kChSpeedLo] = speed;
  sound_channels[0x90 + kChSpeedLo] = speed;
  sound_channels[0x60 + kChLoopIndex] = 0x40;
  sound_channels[0x78 + kChLoopIndex] = 0x50;
  sound_channels[0x90 + kChLoopIndex] = 0x60;
  emu_set_a(0x60);
}

// sound_start_effect_2ch (7:F517): slots 7 and 8, from the 2nd and 4th streams.
void sound_start_effect_2ch(void) {
  RUN_ORIGINAL_IF_LAG(kSoundStartEffect2ch);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 3;
  c->x = 0xA8;
  sound_start_channel();
  c->y = (uint8_t)(c->y + 2);
  sound_start_channel();
  c->y = 0x0B;
  uint8_t speed = emu_read_ind_y(0x45, c->y);
  sound_channels[0xA8 + kChSpeedLo] = speed;
  sound_channels[0xC0 + kChSpeedLo] = speed;
  sound_channels[0xA8 + kChLoopIndex] = 0x70;
  sound_channels[0xC0 + kChLoopIndex] = 0x80;
  emu_set_a(0x80);
}

// sound_update (7:F538), from the NMI. It clears the wait_vblank flag. If the game is in play_sound
// or switching banks (bits 2 and 3), it only asks for a double step next time (bit 7).
void sound_update(void) {
  RUN_ORIGINAL_IF_LAG(kSoundUpdate);
  system_flags &= 0xFD;
  if(system_flags & 0x0C) {
    system_flags |= 0x80;
    emu_set_a(system_flags);
    return;
  }
  switch_to_sound_bank();
  sound_update_all();
  restore_current_bank();
}

// sound_update_all (7:F553)
void sound_update_all(void) {
  RUN_ORIGINAL_IF_LAG(kSoundUpdateAll);
  if(system_flags & 0x80) {
    system_flags &= 0x7F;
    sound_step_all_channels();
  }
  sound_step_all_channels();
}

// sound_step_all_channels (7:F55E)
void sound_step_all_channels(void) {
  RUN_ORIGINAL_IF_LAG(kSoundStepAllChannels);
  Cpu6502 *c = &g_nes.cpu;
  c->x = 0x00;
  sound_step_pulse();
  sound_pulse_envelope();
  c->x = 0x18;
  sound_step_pulse();
  sound_pulse_envelope();
  c->x = 0x30;
  sound_step_triangle();
  sound_triangle_countdown();
  c->x = 0x48;
  sound_step_noise();
  c->x = 0x60;
  sound_step_pulse();
  sound_pulse_envelope();
  c->x = 0x78;
  sound_step_pulse();
  sound_pulse_envelope();
  c->x = 0x90;
  sound_step_triangle();
  sound_triangle_countdown();
  c->x = 0xA8;
  sound_step_pulse();
  sound_pulse_envelope();
  c->x = 0xC0;
  sound_step_noise();
  sound_write_apu();
}

// How the three channel steps start: if the slot at X plays and its time is up, the next command,
// or the end of the stream. Returns 1 with a note in A, and the flags of CMP #$7F.
static int sound_next_note(void) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(CH(kChFlags));
  if(!(c->a & kChPlaying)) {
    return 0;
  }
  sound_tick();
  if(!(c->p & kFlagN)) {
    return 0;
  }
  sound_read_command();
  if(c->p & kFlagC) {
    sound_check_repeat();
    return 0;
  }
  emu_cmp(c->a, kStreamRest);
  return 1;
}

// A rest, at 7:F5FD. The triangle's step jumps here too.
static void sound_rest(void) {
  CH(kChEnvOn) = 0;
  emu_set_a(CH(kChFlags) | kChRest);
  CH(kChFlags) = g_nes.cpu.a;
}

// A new note's end in all three steps.
static void sound_new_note(void) {
  emu_set_a((CH(kChFlags) | kChNewNote) & ~kChRest);
  CH(kChFlags) = g_nes.cpu.a;
}

// sound_step_pulse (7:F5A4)
void sound_step_pulse(void) {
  RUN_ORIGINAL_IF_LAG(kSoundStepPulse);
  Cpu6502 *c = &g_nes.cpu;
  if(!sound_next_note()) {
    return;
  }
  if(c->a == kStreamRest) {
    sound_rest();
    return;
  }
  emu_set_flag(kFlagC, c->a & 0x80);
  c->y = (uint8_t)(c->a << 1);
  CH(kChReg3) = (uint8_t)((CH(kChReg3) & 0xF8) | cpu_bus_read((uint16_t)(kNotePeriods + c->y)));
  CH(kChReg2) = cpu_bus_read((uint16_t)(kNotePeriods + 1 + c->y));
  if(CH(kChEnvSpeed) & 0x80) {
    sound_new_note();
    CH(kChEnvOn) = 0;
    emu_set_a(0);
    return;
  }
  CH(kChEnvPos) = 0;
  CH(kChEnvWait) = 0;
  CH(kChEnvOn) = 1;
  sound_new_note();
}

// sound_pulse_envelope (7:F60F): the volume from the envelope's step, which stays on the last of 64.
void sound_pulse_envelope(void) {
  RUN_ORIGINAL_IF_LAG(kSoundPulseEnvelope);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(CH(kChEnvOn));
  if(!c->a) {
    return;
  }
  c->y = CH(kChEnvPos);
  emu_cmp(c->y, 0x3F);
  if(c->y != 0x3F) {
    CH(kChEnvWait)--;
    if(CH(kChEnvWait) & 0x80) {
      CH(kChEnvWait) = CH(kChEnvSpeed);
      CH(kChEnvPos)++;
    }
  }
  g_ram[0x46] = CH(kChEnvLo);
  g_ram[0x47] = CH(kChEnvHi);
  g_ram[0x48] = emu_read_ind_y(0x46, c->y) & 0x0F;
  emu_set_a((CH(kChReg0) & 0xC0) | 0x30 | g_ram[0x48]);
  CH(kChReg0) = c->a;
}

// The end of sound_step_triangle (7:F6D2): the note on, with the linear counter at its longest.
static void sound_triangle_note_on(void) {
  CH(kChReg0) = 0xBF;
  CH(kChReg3) = CH(kChTriPeriod) | 0x08;
  sound_new_note();
}

// sound_step_triangle (7:F646). Modes 1 and 2 stop the note after a time: 1 after `setting` frames, 2
// (it looks like) `setting - 1` frames before the note's end. In mode 2 the original writes both bytes
// of that time to kChTriTimeLo, leaving kChTriTimeHi as it was, so this does too.
void sound_step_triangle(void) {
  RUN_ORIGINAL_IF_LAG(kSoundStepTriangle);
  Cpu6502 *c = &g_nes.cpu;
  if(!sound_next_note()) {
    return;
  }
  if(c->a == kStreamRest) {
    sound_rest();
    return;
  }
  emu_set_flag(kFlagC, c->a & 0x80);
  c->y = (uint8_t)(c->a << 1);
  CH(kChReg3) &= 0xF8;
  CH(kChTriPeriod) = cpu_bus_read((uint16_t)(kNotePeriods + c->y));
  CH(kChReg3) |= CH(kChTriPeriod);
  CH(kChReg2) = cpu_bus_read((uint16_t)(kNotePeriods + 1 + c->y));
  uint8_t mode = CH(kChTriMode);
  emu_cmp(mode, 1);
  if(mode != 1) {
    emu_cmp(mode, 2);
  }
  if(mode != 1 && mode != 2) {
    CH(kChEnvOn) = 0;
    sound_new_note();
    return;
  }
  CH(kChEnvOn) = 1;
  emu_set_a(CH(kChEnvPos));
  if(mode == 2) {
    emu_set_flag(kFlagC, 1);
    c->a = emu_sbc(c->a, 1);
  }
  if(mode == 1 || !(c->p & kFlagC)) {
    multiply_8x8();
    CH(kChTriTimeLo) = g_ram[0x44];
    CH(kChTriTimeHi) = g_ram[0x45];
    sound_triangle_note_on();
    return;
  }
  multiply_8x8();
  emu_set_flag(kFlagC, 1);
  CH(kChTriTimeLo) = emu_sbc(CH(kChTimeLo), g_ram[0x44]);
  c->a = emu_sbc(CH(kChTimeHi), g_ram[0x45]);
  CH(kChTriTimeLo) = c->a;
  if(!(c->p & kFlagC)) {
    sound_rest();
    return;
  }
  sound_triangle_note_on();
}

// multiply_8x8 (7:F6EA): $45:$44 = A * the speed. Keeps X and Y; returns A = X, the carry from the
// last ROR.
void multiply_8x8(void) {
  RUN_ORIGINAL_IF_LAG(kMultiply8x8);
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x49] = c->a;
  g_ram[0x48] = CH(kChSpeedLo);
  g_ram[0x45] = 0;
  g_ram[0x44] = 0;
  int carry = 0;
  for(int i = 0; i < 8; i++) {
    carry = g_ram[0x49] & 1;
    g_ram[0x49] >>= 1;
    if(carry) {
      unsigned sum = (unsigned)g_ram[0x48] + g_ram[0x45];
      g_ram[0x45] = (uint8_t)sum;
      carry = (int)(sum >> 8);
    }
    int low = g_ram[0x45] & 1;
    g_ram[0x45] = (uint8_t)(g_ram[0x45] >> 1 | carry << 7);
    carry = g_ram[0x44] & 1;
    g_ram[0x44] = (uint8_t)(g_ram[0x44] >> 1 | low << 7);
  }
  emu_set_flag(kFlagC, carry);
  emu_set_a(c->x);
}

// sound_triangle_countdown (7:F710): when a note's time is over, silences it.
void sound_triangle_countdown(void) {
  RUN_ORIGINAL_IF_LAG(kSoundTriangleCountdown);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(CH(kChEnvOn));
  if(!c->a) {
    return;
  }
  emu_set_flag(kFlagC, 1);
  CH(kChTriTimeLo) = emu_sbc(CH(kChTriTimeLo), CH(kChSpeedLo));
  c->a = emu_sbc(CH(kChTriTimeHi), CH(kChSpeedHi));
  CH(kChTriTimeHi) = c->a;
  if(!(c->p & kFlagC)) {
    emu_set_a(0);
    CH(kChReg0) = 0;
    CH(kChEnvOn) = 0;
  }
}

// sound_step_noise (7:F733): a note is 3 register bytes from the table at the "envelope" address,
// 12 notes that repeat every octave, from note 4. Below that it rests.
void sound_step_noise(void) {
  RUN_ORIGINAL_IF_LAG(kSoundStepNoise);
  Cpu6502 *c = &g_nes.cpu;
  if(!sound_next_note()) {
    return;
  }
  int rest = c->a == kStreamRest;
  if(!rest) {
    emu_set_flag(kFlagC, 1);
    c->a = emu_sbc(c->a, 4);
    rest = !(c->p & kFlagC);
  }
  if(rest) {
    emu_set_a(CH(kChFlags) | kChRest);
    CH(kChFlags) = c->a;
    return;
  }
  while(c->p & kFlagC) {
    c->a = emu_sbc(c->a, 0x0C);
  }
  c->a = emu_adc(c->a, 0x0C);
  g_ram[0x48] = c->a;
  emu_set_flag(kFlagC, c->a & 0x80);
  c->a = emu_adc((uint8_t)(c->a << 1), g_ram[0x48]);
  c->y = c->a;
  g_ram[0x44] = CH(kChEnvLo);
  g_ram[0x45] = CH(kChEnvHi);
  CH(kChReg0) = emu_read_ind_y(0x44, c->y);
  c->y++;
  CH(kChReg2) = emu_read_ind_y(0x44, c->y);
  c->y++;
  CH(kChReg3) = emu_read_ind_y(0x44, c->y);
  sound_new_note();
}

// sound_tick (7:F78B): the time left, less the speed. Returns its high byte in A and the flags of
// the SBC.
void sound_tick(void) {
  RUN_ORIGINAL_IF_LAG(kSoundTick);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_flag(kFlagC, 1);
  CH(kChTimeLo) = emu_sbc(CH(kChTimeLo), CH(kChSpeedLo));
  c->a = emu_sbc(CH(kChTimeHi), CH(kChSpeedHi));
  CH(kChTimeHi) = c->a;
}

// sound_add_length (7:F79F): the time left, plus 8 times the note length.
void sound_add_length(void) {
  RUN_ORIGINAL_IF_LAG(kSoundAddLength);
  Cpu6502 *c = &g_nes.cpu;
  unsigned length = (unsigned)(CH(kChLengthLo) | CH(kChLengthHi) << 8) << 3;
  g_ram[0x48] = (uint8_t)(length >> 8);
  emu_set_flag(kFlagC, 0);
  CH(kChTimeLo) = emu_adc((uint8_t)length, CH(kChTimeLo));
  c->a = emu_adc(g_ram[0x48], CH(kChTimeHi));
  CH(kChTimeHi) = c->a;
}

// $E0-$FE in a stream (7:F827), with Y after the command: an address follows, and $E4 and up do
// nothing with it.
static void sound_address_command(uint8_t cmd) {
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x44] = emu_read_ind_y(0x46, c->y);
  c->y++;
  g_ram[0x45] = emu_read_ind_y(0x46, c->y);
  c->y++;
  if(cmd <= 0xE1) {  // an instrument: 4 bytes, then its envelope
    g_ram[0x4B] = c->y;
    CH(kChReg0) = emu_read_ind_y(0x44, 0);
    CH(kChReg1) = emu_read_ind_y(0x44, 1);
    CH(kChReg3) = emu_read_ind_y(0x44, 2);
    CH(kChEnvSpeed) = emu_read_ind_y(0x44, 3);
    emu_set_flag(kFlagC, 0);
    CH(kChEnvLo) = emu_adc(4, g_ram[0x44]);
    CH(kChEnvHi) = emu_adc(0, g_ram[0x45]);
    c->y = g_ram[0x4B];
  } else if(cmd == 0xE2) {
    g_ram[0x4B] = c->y;
    CH(kChReg0) = emu_read_ind_y(0x44, 0);
    CH(kChReg3) = emu_read_ind_y(0x44, 1);
    CH(kChTriMode) = emu_read_ind_y(0x44, 2);
    CH(kChEnvPos) = emu_read_ind_y(0x44, 3);
    c->y = g_ram[0x4B];
  } else if(cmd == 0xE3) {
    CH(kChEnvLo) = g_ram[0x44];
    CH(kChEnvHi) = g_ram[0x45];
  }
}

// $C0-$DF in a stream (7:F8A2), with Y after the command: $C0-$CE start a loop of that many passes,
// $CF ends one, $D0-$DF leave it on that pass, skipping to just after its $CF. Leaves Y on the next
// command.
static void sound_loop_command(uint8_t cmd) {
  Cpu6502 *c = &g_nes.cpu;
  if(cmd < kStreamLoopEnd) {
    g_ram[0x48] = cmd & 0x0F;
    emu_set_flag(kFlagC, 0);
    g_ram[0x46] = emu_adc(c->y, g_ram[0x46]);
    g_ram[0x47] = emu_adc(g_ram[0x47], 0);
    c->y = CH(kChLoopIndex);
    *loop_entry(0) = 0;
    *loop_entry(1) = g_ram[0x48];
    *loop_entry(2) = g_ram[0x46];
    *loop_entry(3) = g_ram[0x47];
    sound_loop_push();
    CH(kChLoopIndex) = c->y;
    c->y = 0;
    return;
  }
  if(cmd == kStreamLoopEnd) {
    goto loop_end;
  }
leave_on_pass:  // 7:F913
  g_ram[0x45] = (uint8_t)((cmd & 0x0F) - 1);
  g_ram[0x4B] = c->y;
  c->y = CH(kChLoopIndex);
  sound_loop_pop();
  if(*loop_entry(0) == g_ram[0x45]) {
    goto skip_leaves;
  }
  c->y = g_ram[0x4B];
  for(;;) {  // 7:F92A: on to the next $CF or $Dx
    emu_count_work(14);
    cmd = emu_read_ind_y(0x46, c->y);
    g_ram[0x48] = cmd;
    c->y++;
    if(!c->y) {
      g_ram[0x47]++;
    }
    if(cmd == kStreamLoopEnd) {
      goto loop_end;
    }
    if((cmd & 0xF0) == 0xD0) {
      goto leave_on_pass;
    }
  }
loop_end:  // 7:F8DB
  g_ram[0x4B] = c->y;
  c->y = CH(kChLoopIndex);
  sound_loop_pop();
  if(!(c->p & kFlagC)) {
    uint8_t passes = (uint8_t)(*loop_entry(0) + 1);
    *loop_entry(0) = passes;
    if(passes != *loop_entry(1)) {
      g_ram[0x46] = *loop_entry(2);
      g_ram[0x47] = *loop_entry(3);
      c->y = 0;
      return;
    }
    CH(kChLoopIndex) = c->y;
  }
skip_leaves:  // 7:F904
  c->y = g_ram[0x4B];
  uint8_t next;
  do {
    emu_count_work(14);
    next = emu_read_ind_y(0x46, c->y);
    c->y++;
  } while((next & 0xF0) == 0xD0);
  c->y--;
}

// sound_read_command (7:F7C0): runs the slot's commands up to its next note, or the stream's end.
// Returns a note in A with the carry clear and the stream moved past it; or the carry set for an
// end, with the slot off.
void sound_read_command(void) {
  RUN_ORIGINAL_IF_LAG(kSoundReadCommand);
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x46] = CH(kChStreamLo);
  g_ram[0x47] = CH(kChStreamHi);
  c->y = 0;
  for(;;) {
    emu_count_work(16);
    uint8_t cmd = emu_read_ind_y(0x46, c->y);
    g_ram[0x4A] = cmd;
    c->y++;
    if(cmd == kStreamEnd) {
      emu_set_a(0);
      CH(kChFlags) = 0;
      CH(kChStreamHi) = 0;
      sound_channel_done();
      emu_set_flag(kFlagC, 1);
      return;
    }
    if(cmd == kStreamWait) {
      sound_add_length();
    } else if(cmd < 0x80) {
      sound_add_length();
      emu_set_flag(kFlagC, 0);
      CH(kChStreamLo) = emu_adc(c->y, g_ram[0x46]);
      CH(kChStreamHi) = emu_adc(g_ram[0x47], 0);
      emu_set_a(g_ram[0x4A]);
      emu_set_flag(kFlagC, 0);
      return;
    } else if(cmd < 0xC0) {
      g_ram[0x4B] = c->y;
      uint8_t index = (uint8_t)((cmd & 0x1F) << 1);
      CH(kChLengthLo) = cpu_bus_read((uint16_t)(kNoteLengths + index));
      CH(kChLengthHi) = cpu_bus_read((uint16_t)(kNoteLengths + 1 + index));
    } else if(cmd >= 0xE0) {
      sound_address_command(cmd);
    } else {
      sound_loop_command(cmd);
    }
  }
}

// sound_loop_push (7:F944): Y to the next entry of its 16 bytes, which wrap. Returns A = Y.
void sound_loop_push(void) {
  RUN_ORIGINAL_IF_LAG(kSoundLoopPush);
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x48] = c->y & 0xF0;
  c->y = (uint8_t)(((c->y + 4) & 0x0F) | g_ram[0x48]);
  emu_set_a(c->y);
}

// sound_loop_pop (7:F954): Y to the entry before, carry clear; or the carry set if there is none.
void sound_loop_pop(void) {
  RUN_ORIGINAL_IF_LAG(kSoundLoopPop);
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x48] = c->y & 0xF0;
  emu_set_a(c->y & 0x0F);
  if(!c->a) {
    emu_set_flag(kFlagC, 1);
    return;
  }
  c->y = (uint8_t)(((c->y - 4) & 0x0F) | g_ram[0x48]);
  emu_set_a(c->y);
  emu_set_flag(kFlagC, 0);
}

// sound_note_off (7:F96C): the slot's registers 1-3 are written.
void sound_note_off(void) {
  RUN_ORIGINAL_IF_LAG(kSoundNoteOff);
  emu_set_a(CH(kChFlags) & ~kChNewNote);
  CH(kChFlags) = g_nes.cpu.a;
}

// Sets kChNewNote on a slot, as sound_channel_done's ends do.
static void sound_rewrite_slot(uint8_t slot) {
  emu_set_a(sound_channels[slot] | kChNewNote);
  sound_channels[slot] = g_nes.cpu.a;
}

// sound_channel_done (7:F975): an effect's channel has ended, so the APU channel's registers go back
// to the slots under it. Without such a slot (music), A stays, with the flags of the last CPX.
void sound_channel_done(void) {
  RUN_ORIGINAL_IF_LAG(kSoundChannelDone);
  uint8_t x = g_nes.cpu.x;
  switch(x) {
    case 0x60:
      sound_rewrite_slot(0x00);
      break;
    case 0x78:
      sound_rewrite_slot(0x18);
      break;
    case 0x90:
      sound_rewrite_slot(0x30);
      break;
    case 0xA8:
      sound_rewrite_slot(0x78);
      sound_rewrite_slot(0x18);
      break;
    case 0xC0:
      sound_rewrite_slot(0x48);
      break;
    default:
      emu_cmp(x, 0xC0);
      return;
  }
  emu_set_flag(kFlagC, 1);
}

// Whether an APU channel plays: bit `bit` of $53, unless the first of its slots that plays rests.
static void sound_enable_bit(const uint8_t *slots, int count, uint8_t bit) {
  for(int i = 0; i < count; i++) {
    uint8_t flags = sound_channels[slots[i]];
    if(flags & kChPlaying) {
      if(!(flags & kChRest)) {
        g_ram[0x53] |= bit;
      }
      return;
    }
  }
}

// An APU channel's registers at `reg`, from the first of its slots that plays (left in X), or none.
// Registers 1-3 only go out for a new note, as register 3 restarts it.
static void sound_write_registers(const uint8_t *slots, int count, uint16_t reg) {
  Cpu6502 *c = &g_nes.cpu;
  for(int i = 0;; i++) {
    c->x = slots[i];
    emu_set_a(CH(kChFlags));
    if(c->a & kChPlaying) {
      break;
    }
    if(i == count - 1) {
      return;
    }
  }
  cpu_bus_write(reg, CH(kChReg0));
  emu_set_a(CH(kChFlags) & kChNewNote);
  if(!c->a) {
    return;
  }
  cpu_bus_write((uint16_t)(reg + 1), CH(kChReg1));
  cpu_bus_write((uint16_t)(reg + 2), CH(kChReg2));
  cpu_bus_write((uint16_t)(reg + 3), CH(kChReg3));
  sound_note_off();
}

// sound_write_apu (7:F9B6): an effect's slots go before the music's on the same APU channel.
void sound_write_apu(void) {
  static const uint8_t kPulse1[] = {0x60, 0x00};
  static const uint8_t kPulse2[] = {0xA8, 0x78, 0x18};
  static const uint8_t kTriangle[] = {0x90, 0x30};
  static const uint8_t kNoise[] = {0xC0, 0x48};
  RUN_ORIGINAL_IF_LAG(kSoundWriteApu);
  g_ram[0x53] = 0;
  sound_enable_bit(kPulse1, 2, 0x01);
  sound_enable_bit(kPulse2, 3, 0x02);
  sound_enable_bit(kTriangle, 2, 0x04);
  sound_enable_bit(kNoise, 2, 0x08);
  cpu_bus_write(0x4015, g_ram[0x53]);
  sound_write_registers(kPulse1, 2, 0x4000);
  sound_write_registers(kPulse2, 3, 0x4004);
  sound_write_registers(kTriangle, 2, 0x4008);
  sound_write_registers(kNoise, 2, 0x400C);
}
