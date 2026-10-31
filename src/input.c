// The controllers.
#include "funcs.h"
#include "variables.h"

// read_joypads_once (7:EB6E): strobes the controllers and reads 8 bits from each, bit 0 into
// joy_pressed and bit 1 (a Famicom expansion pad) into $10/$11, then ORs the two together.
// Returns with X = 0 and A = joy_pressed+1.
void read_joypads_once(void) {
  RUN_ORIGINAL_IF_LAG(kReadJoypadsOnce);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t strobe = (uint8_t)(g_ram[0xFB] + 1);
  cpu_bus_write(0x4016, strobe);
  cpu_bus_write(0x4016, (uint8_t)(strobe - 1));
  int carry = 0;
  for(int i = 0; i < 8; i++) {
    uint8_t v = cpu_bus_read(0x4016);
    joy_pressed[0] = (uint8_t)(joy_pressed[0] << 1 | (v & 1));
    g_ram[0x10] = (uint8_t)(g_ram[0x10] << 1 | (v >> 1 & 1));
    v = cpu_bus_read(0x4017);
    joy_pressed[1] = (uint8_t)(joy_pressed[1] << 1 | (v & 1));
    carry = g_ram[0x11] >> 7;
    g_ram[0x11] = (uint8_t)(g_ram[0x11] << 1 | (v >> 1 & 1));
  }
  joy_pressed[0] |= g_ram[0x10];
  joy_pressed[1] |= g_ram[0x11];
  c->x = 0;
  emu_set_a(joy_pressed[1]);
  emu_set_flag(kFlagC, carry);  // from the last ROL
}

// read_joypads (7:EB41): reads until two reads in a row agree, then leaves the buttons held in
// joy_held and those newly pressed since the last call in joy_pressed. Keeps X and Y, and
// returns A = Y.
void read_joypads(void) {
  RUN_ORIGINAL_IF_LAG(kReadJoypads);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t x = c->x, y = c->y;
  emu_set_a(x);
  read_joypads_once();
  uint8_t first0, first1;
  do {
    first0 = joy_pressed[0];
    first1 = joy_pressed[1];
    c->y = first0;
    emu_set_a(first1);
    read_joypads_once();
  } while(first1 != joy_pressed[1] || first0 != joy_pressed[0]);
  for(int i = 1; i >= 0; i--) {
    uint8_t held = joy_pressed[i];
    joy_pressed[i] = (held ^ joy_held[i]) & held;
    joy_held[i] = held;
  }
  c->x = x;
  c->y = y;
  emu_set_a(y);
  emu_set_flag(kFlagC, 1);  // from the CPY that found the reads equal
}
