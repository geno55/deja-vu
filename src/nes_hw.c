// The C side's NES hardware; see nes_hw.h. Register effects follow Mesen2 2.1.1 where the
// game can see the difference (palette mirroring, OAM attribute bits, controller open bus).
#include "nes_hw.h"

#include <string.h>

Nes g_nes;

enum {
  kFrameCycles = 29781,       // CPU cycles per NTSC frame (341 x 262 dots / 3, rounded)
  kVblankCycles = 2273,       // 20 scanlines
  kFirstVblankCycles = 27394, // from power-on to the first vblank (241 scanlines)
};

void nes_init(const uint8_t *prg, const uint8_t *chr) {
  memset(&g_nes, 0, sizeof(g_nes));
  g_nes.prg = prg;
  g_nes.chr = chr;
  // MMC3 power-on bank registers, as Mesen2 sets them.
  static const uint8_t kBanks[8] = {0, 2, 4, 5, 6, 7, 0, 1};
  memcpy(g_nes.bank_regs, kBanks, sizeof(kBanks));
}

void nes_power_on(uint64_t cycle) {
  g_nes.cpu.cycles = cycle;
  g_nes.vblank_cycle = cycle + kFirstVblankCycles;
}

void nes_set_vblank(uint64_t cycle) {
  g_nes.vblank_cycle = cycle;
}

void nes_set_ppu_reads(const NesAccess *reads, size_t count) {
  g_nes.ppu_reads = reads;
  g_nes.ppu_read_count = count;
  g_nes.ppu_read_pos = 0;
  g_nes.ppu_read_miss = 0;
}

// ---- PRG banking -------------------------------------------------------------------------

uint32_t nes_prg_offset(uint16_t addr) {
  int swapped = g_nes.bank_select & 0x40;  // PRG mode 1 swaps $8000 and $C000
  uint8_t bank;
  switch((addr >> 13) & 3) {
    case 0: bank = swapped ? 14 : g_nes.bank_regs[6]; break;
    case 1: bank = g_nes.bank_regs[7]; break;
    case 2: bank = swapped ? g_nes.bank_regs[6] : 14; break;
    default: bank = 15; break;
  }
  return (uint32_t)(bank & 15) * 0x2000 + (addr & 0x1FFF);
}

static void mmc3_write(uint16_t addr, uint8_t v) {
  switch(addr & 0xE001) {
    case 0x8000: g_nes.bank_select = v; break;
    case 0x8001: g_nes.bank_regs[g_nes.bank_select & 7] = v; break;
    case 0xA000: g_nes.mirroring = v & 1; break;
    case 0xA001: g_nes.ram_protect = v; break;
    case 0xC000: g_nes.irq_regs[0] = v; break;
    case 0xC001: g_nes.irq_regs[1] = v; break;
    case 0xE000: g_nes.irq_regs[2] = v; break;
    default: g_nes.irq_regs[3] = v; break;
  }
}

// ---- PPU ---------------------------------------------------------------------------------

// The vblank and sprite-0 flags at the current cycle. Approximate (see nes_hw.h): vblank
// for 20 scanlines from vblank_cycle; sprite 0 hit from the scanline of sprite 0 until the
// next pre-render line, when sprites and background are both on.
static uint8_t ppu_flags(void) {
  uint64_t t = g_nes.cpu.cycles;
  while(t >= g_nes.vblank_cycle + kFrameCycles) {
    g_nes.vblank_cycle += kFrameCycles;
  }
  if(t < g_nes.vblank_cycle) {
    return 0;
  }
  uint64_t phase = t - g_nes.vblank_cycle;
  uint8_t flags = 0;
  if(phase < kVblankCycles && g_nes.vblank_read_cycle < g_nes.vblank_cycle) {
    flags |= 0x80;
  }
  uint64_t hit = kVblankCycles + (uint64_t)(g_nes.oam[0] + 1) * 341 / 3;
  if((g_nes.ppu_mask & 0x18) == 0x18 && (phase < kVblankCycles || phase >= hit)) {
    flags |= 0x40;
  }
  return flags;
}

static void vram_write(uint16_t addr, uint8_t v) {
  if(addr < 0x2000) {
    return;  // CHR ROM
  }
  if(addr < 0x3F00) {
    int table = (addr >> 10) & 3;
    int page = g_nes.mirroring ? table >> 1 : table & 1;
    g_nes.ciram[page * 0x400 + (addr & 0x3FF)] = v;
    return;
  }
  // Entries 0, 4, 8 and C are shared between the two halves; Mesen2 stores both copies.
  addr &= 0x1F;
  v &= 0x3F;
  if((addr & 0x03) == 0) {
    g_nes.palette[addr & 0x0F] = v;
    g_nes.palette[(addr & 0x0F) | 0x10] = v;
  } else {
    g_nes.palette[addr] = v;
  }
}

static void oam_write(uint8_t v) {
  if((g_nes.oam_addr & 0x03) == 0x02) {
    v &= 0xE3;  // sprite attribute bits 2-4 don't exist
  }
  g_nes.oam[g_nes.oam_addr++] = v;
}

// A read of a PPU register other than $2002 itself, which only glitches make: the value comes
// from the reference (see nes_set_ppu_reads); the side effects are applied here.
static uint8_t ppu_glitch_read(uint16_t addr) {
  uint8_t v = g_nes.ppu_latch;  // without the reference's value: the last write
  if(g_nes.ppu_read_pos < g_nes.ppu_read_count &&
     g_nes.ppu_reads[g_nes.ppu_read_pos].addr == addr) {
    v = g_nes.ppu_reads[g_nes.ppu_read_pos].value;
  } else if(!g_nes.ppu_read_miss) {
    g_nes.ppu_read_miss = addr;
  }
  g_nes.ppu_read_pos++;
  if((addr & 7) == 2) {
    if(v & 0x80) {
      g_nes.vblank_read_cycle = g_nes.cpu.cycles;
    }
    g_nes.write_toggle = 0;
  } else if((addr & 7) == 7) {
    g_nes.vram_addr = (g_nes.vram_addr + (g_nes.ppu_ctrl & 0x04 ? 32 : 1)) & 0x7FFF;
  }
  return v;
}

static uint8_t ppu_read(uint16_t addr) {
  if(addr != 0x2002) {
    return ppu_glitch_read(addr);
  }
  uint8_t v = ppu_flags() | (g_nes.ppu_latch & 0x1F);
  if(v & 0x80) {
    g_nes.vblank_read_cycle = g_nes.cpu.cycles;
  }
  g_nes.write_toggle = 0;
  g_nes.ppu_latch = (g_nes.ppu_latch & 0x1F) | (v & 0xE0);
  return v;
}

static void ppu_write(uint16_t addr, uint8_t v) {
  g_nes.ppu_latch = v;
  switch(addr & 7) {
    case 0:
      g_nes.ppu_ctrl = v;
      g_nes.vram_temp = (g_nes.vram_temp & ~0x0C00) | (v & 0x03) << 10;
      break;
    case 1: g_nes.ppu_mask = v; break;
    case 3: g_nes.oam_addr = v; break;
    case 4: oam_write(v); break;
    case 5:
      if(!g_nes.write_toggle) {
        g_nes.fine_x = v & 7;
        g_nes.vram_temp = (g_nes.vram_temp & ~0x001F) | v >> 3;
      } else {
        g_nes.vram_temp = (g_nes.vram_temp & ~0x73E0) | (v & 0xF8) << 2 | (v & 0x07) << 12;
      }
      g_nes.write_toggle ^= 1;
      break;
    case 6:
      if(!g_nes.write_toggle) {
        g_nes.vram_temp = (g_nes.vram_temp & ~0xFF00) | (v & 0x3F) << 8;
      } else {
        g_nes.vram_temp = (g_nes.vram_temp & ~0x00FF) | v;
        g_nes.vram_addr = g_nes.vram_temp;
      }
      g_nes.write_toggle ^= 1;
      break;
    case 7:
      vram_write(g_nes.vram_addr & 0x3FFF, v);
      g_nes.vram_addr = (g_nes.vram_addr + (g_nes.ppu_ctrl & 0x04 ? 32 : 1)) & 0x7FFF;
      break;
  }
}

static void oam_dma(uint8_t page) {
  uint64_t start = g_nes.cpu.cycles;
  for(int i = 0; i < 256; i++) {
    oam_write(cpu_bus_read((uint16_t)(page << 8 | i)));
  }
  g_nes.cpu.cycles = start + 513 + (start & 1);
}

// ---- Controllers -------------------------------------------------------------------------

static uint8_t pad_read(int port) {
  if(g_nes.pad_strobe) {
    g_nes.pad_shift[port] = g_nes.pad[port];
  }
  uint8_t bit = g_nes.pad_shift[port] & 1;
  g_nes.pad_shift[port] = g_nes.pad_shift[port] >> 1 | 0x80;  // then 1s, as a real pad
  return (g_nes.open_bus & 0xE0) | bit;  // the top bits aren't driven
}

static void pad_write(uint8_t v) {
  if(g_nes.pad_strobe && !(v & 1)) {
    g_nes.pad_shift[0] = g_nes.pad[0];
    g_nes.pad_shift[1] = g_nes.pad[1];
  }
  g_nes.pad_strobe = v & 1;
}

// ---- The CPU's bus -----------------------------------------------------------------------

uint8_t cpu_bus_read(uint16_t addr) {
  uint8_t v;
  if(addr < 0x2000) {
    v = g_nes.ram[addr & 0x7FF];
  } else if(addr < 0x4000) {
    v = ppu_read(addr);
  } else if(addr == 0x4016 || addr == 0x4017) {
    v = pad_read(addr - 0x4016);
  } else if(addr < 0x6000) {
    v = g_nes.open_bus;
  } else if(addr < 0x8000) {
    v = g_nes.ram_protect & 0x80 ? g_nes.sram[addr - 0x6000] : g_nes.open_bus;
  } else {
    v = g_nes.prg[nes_prg_offset(addr)];
  }
  g_nes.open_bus = v;
  return v;
}

void cpu_bus_write(uint16_t addr, uint8_t v) {
  g_nes.open_bus = v;
  if(addr < 0x2000) {
    g_nes.ram[addr & 0x7FF] = v;
  } else if(addr < 0x4000) {
    ppu_write(addr, v);
  } else if(addr == 0x4014) {
    oam_dma(v);
  } else if(addr == 0x4016) {
    pad_write(v);
  } else if(addr <= 0x4017) {
    if(g_nes.apu_count < kApuLogMax) {
      g_nes.apu_log[g_nes.apu_count] = (NesAccess){addr, v};
    }
    g_nes.apu_count++;
  } else if(addr >= 0x8000) {
    mmc3_write(addr, v);
  } else if(addr >= 0x6000 && (g_nes.ram_protect & 0xC0) == 0x80) {
    g_nes.sram[addr - 0x6000] = v;
  }
}
