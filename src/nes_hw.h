// The C side's NES: the state the harness compares with the reference and
// what the hardware registers do to it. Original code reaches it through the 6502's bus
// (cpu_bus_read/cpu_bus_write); ported code will use it directly.
//
// It models the console as the game uses it, not every detail of the hardware: memory
// layouts and register effects match Mesen2's, so the five memories can be compared byte for
// byte. PPU timing (the vblank and sprite-0 flags in $2002) is approximate: the harness keeps
// the CPU cycle count in step with the reference at every NMI, so nothing depends on it except
// when the game's wait loops end.
#ifndef DV_NES_HW_H
#define DV_NES_HW_H

#include <stddef.h>
#include <stdint.h>

#include "cpu6502.h"

enum {
  kPrgSize = 0x20000,  // 128 KB: 16 banks of 8 KB
  kChrSize = 0x40000,  // 256 KB
  kApuLogMax = 1024,
};

// A read or write of a hardware register.
typedef struct NesAccess {
  uint16_t addr;
  uint8_t value;
} NesAccess;

typedef struct Nes {
  Cpu6502 cpu;

  // The memories compared with the reference, laid out as Mesen2 keeps them.
  uint8_t ram[0x800];
  uint8_t sram[0x2000];
  uint8_t ciram[0x800];    // nametable RAM: two 1 KB pages, mapped by the mirroring
  uint8_t palette[0x20];
  uint8_t oam[0x100];

  const uint8_t *prg;  // from the user's ROM
  const uint8_t *chr;

  // MMC3.
  uint8_t bank_select;   // $8000
  uint8_t bank_regs[8];  // R0-R7, written through $8001
  uint8_t mirroring;     // $A000 bit 0: 0 vertical, 1 horizontal
  uint8_t ram_protect;   // $A001: bit 7 enables save RAM, bit 6 makes it read-only
  uint8_t irq_regs[4];   // $C000, $C001, $E000, $E001 (the game doesn't use the IRQ)

  // PPU registers.
  uint8_t ppu_ctrl, ppu_mask, oam_addr;
  uint8_t ppu_latch;     // the PPU's data bus: the low bits of $2002 reads
  uint16_t vram_addr, vram_temp;  // v and t
  uint8_t fine_x, write_toggle;
  uint64_t vblank_cycle;          // CPU cycle when the current frame's vblank began
  uint64_t vblank_read_cycle;     // when $2002 last cleared the vblank flag
  // The sprite-0 split the NMI handler asks for: C has no time inside a frame, so the handler leaves
  // PPUCTRL as it is below the split, and this says what it was above it (ppu_render.h).
  uint8_t split_on, split_upper_ctrl;

  // Controllers: buttons held (bit 0 A ... bit 7 Right, read in that order), and the shift
  // registers loaded on strobe.
  uint8_t pad[2], pad_shift[2], pad_strobe;

  uint8_t open_bus;  // the last value on the CPU's data bus

  NesAccess apu_log[kApuLogMax];  // writes to $4000-$4013, $4015 and $4017
  uint32_t apu_count;  // writes since the log was last cleared (may exceed kApuLogMax)

  // Values for reads of PPU registers other than $2002 itself (nes_set_ppu_reads).
  const NesAccess *ppu_reads;
  size_t ppu_read_count, ppu_read_pos;
  uint16_t ppu_read_miss;  // the first read that wasn't the next one given (0: none)
} Nes;

extern Nes g_nes;

// Sets up the console with the ROM's PRG and CHR (the caller keeps them alive). Memories,
// CPU and registers are zero, the MMC3 banks as at power-on; the harness copies the
// reference's power-on memories and registers over them.
void nes_init(const uint8_t *prg, const uint8_t *chr);

// Power-on happens at this CPU cycle: sets the cycle count and the first vblank.
void nes_power_on(uint64_t cycle);

// The CPU cycle when a frame's vblank begins, for the PPU flags. The harness sets it at
// every NMI; between NMIs frames follow at the NTSC rate.
void nes_set_vblank(uint64_t cycle);

// Where a CPU address in $8000-$FFFF is in PRG ROM, with the banks mapped now.
uint32_t nes_prg_offset(uint16_t addr);

// Reads of PPU registers other than $2002 itself return these values, in order. The game's
// own code reads only $2002, but a glitch can read the others or the mirrors (TAS #7044 walks
// a pointer into $3247-$324C), and what they return depends on the PPU's exact position and
// open-bus decay, which aren't modelled here. So the harness takes them from the reference;
// ppu_read_pos and ppu_read_miss show whether the C side made the same reads.
void nes_set_ppu_reads(const NesAccess *reads, size_t count);

#endif  // DV_NES_HW_H
