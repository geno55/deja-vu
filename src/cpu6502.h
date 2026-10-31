// A 6502 interpreter (the NES's 2A03: no decimal mode) for running the game's original code
// on the C side: everything in pass-through mode, and later the routines not yet ported.
// The game's own code uses only official opcodes, but a glitch can run any byte as code (the capsule
// warp, docs/rooms.md), so it has them all, from the NESdev wiki's description of the unofficial ones.
// Cycle counts follow the standard tables, so code between two NMIs takes as many
// cycles as on the reference, which is how the harness places the NMIs that interrupt it.
#ifndef DV_CPU6502_H
#define DV_CPU6502_H

#include <stdint.h>

enum {
  kFlagC = 0x01, kFlagZ = 0x02, kFlagI = 0x04, kFlagD = 0x08,
  kFlagB = 0x10, kFlagU = 0x20, kFlagV = 0x40, kFlagN = 0x80,
};

typedef struct Cpu6502 {
  uint64_t cycles;
  uint16_t pc;
  uint8_t a, x, y, s;
  uint8_t p;  // bits 4 and 5 are kept clear; they only exist on the stack
} Cpu6502;

// The bus, provided by the hardware model (nes_hw.c). It may add cycles (OAM DMA).
uint8_t cpu_bus_read(uint16_t addr);
void cpu_bus_write(uint16_t addr, uint8_t value);

// Runs one instruction. Returns 0, or 1 for an opcode that freezes a real 6502 until a reset ($02 and
// the others like it; pc is left on it).
int cpu_step(Cpu6502 *c);

// Takes an NMI: pushes pc and p, sets I and jumps through $FFFA. 7 cycles.
void cpu_nmi(Cpu6502 *c);

#endif  // DV_CPU6502_H
