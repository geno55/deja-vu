// 6502 interpreter; see cpu6502.h. Dummy reads and writes are not made: the game never uses
// an indexed or read-modify-write access on a hardware register, where they would matter.
#include "cpu6502.h"

// Base cycles per opcode, the unofficial ones too. Reads with abs,X / abs,Y / (zp),Y take one more cycle when the index crosses a page, and
// branches one more when taken and another when the target is on another page.
static const uint8_t kCycles[256] = {
  // 0 1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
  7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,  // 0
  2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,  // 1
  6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,  // 2
  2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,  // 3
  6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,  // 4
  2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,  // 5
  6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,  // 6
  2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,  // 7
  2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,  // 8
  2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,  // 9
  2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,  // A
  2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,  // B
  2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,  // C
  2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,  // D
  2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,  // E
  2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,  // F
};

static uint8_t fetch(Cpu6502 *c) {
  return cpu_bus_read(c->pc++);
}

static uint16_t fetch16(Cpu6502 *c) {
  uint8_t lo = fetch(c);
  return lo | fetch(c) << 8;
}

// A little-endian word, low byte first: the order matters, because the last byte read stays
// on the data bus and is what unmapped addresses read back.
static uint16_t read16(uint16_t lo_addr, uint16_t hi_addr) {
  uint8_t lo = cpu_bus_read(lo_addr);
  return lo | cpu_bus_read(hi_addr) << 8;
}

static void push(Cpu6502 *c, uint8_t v) {
  cpu_bus_write(0x100 | c->s--, v);
}

static uint8_t pull(Cpu6502 *c) {
  return cpu_bus_read(0x100 | ++c->s);
}

static void set_nz(Cpu6502 *c, uint8_t v) {
  c->p = (c->p & ~(kFlagN | kFlagZ)) | (v & kFlagN) | (v ? 0 : kFlagZ);
}

static void set_c(Cpu6502 *c, int on) {
  c->p = on ? c->p | kFlagC : c->p & ~kFlagC;
}

// Addressing modes; each returns the effective address. `cross` adds the page-cross cycle.
static uint16_t zp_x(Cpu6502 *c) { return (uint8_t)(fetch(c) + c->x); }
static uint16_t zp_y(Cpu6502 *c) { return (uint8_t)(fetch(c) + c->y); }

static uint16_t abs_i(Cpu6502 *c, uint8_t i, int cross) {
  uint16_t base = fetch16(c), addr = base + i;
  if(cross && (addr ^ base) & 0xFF00) {
    c->cycles++;
  }
  return addr;
}

static uint16_t ind_x(Cpu6502 *c) {
  uint8_t z = fetch(c) + c->x;
  return read16(z, (uint8_t)(z + 1));
}

static uint16_t ind_y(Cpu6502 *c, int cross) {
  uint8_t z = fetch(c);
  uint16_t base = read16(z, (uint8_t)(z + 1)), addr = base + c->y;
  if(cross && (addr ^ base) & 0xFF00) {
    c->cycles++;
  }
  return addr;
}

// The operand address of the eight "group 1" instructions (ORA AND EOR ADC STA LDA CMP SBC),
// from bits 2-4 of the opcode. Immediate operands come back as the address of the byte.
static uint16_t group1_addr(Cpu6502 *c, uint8_t op) {
  int cross = (op & 0xE0) != 0x80;  // everything but STA
  switch((op >> 2) & 7) {
    case 0: return ind_x(c);
    case 1: return fetch(c);
    case 2: return c->pc++;
    case 3: return fetch16(c);
    case 4: return ind_y(c, cross);
    case 5: return zp_x(c);
    case 6: return abs_i(c, c->y, cross);
    default: return abs_i(c, c->x, cross);
  }
}

static void adc(Cpu6502 *c, uint8_t v) {
  unsigned sum = c->a + v + (c->p & kFlagC);
  c->p &= ~(kFlagC | kFlagV);
  if(sum > 0xFF) {
    c->p |= kFlagC;
  }
  if(~(c->a ^ v) & (c->a ^ sum) & 0x80) {
    c->p |= kFlagV;
  }
  c->a = (uint8_t)sum;
  set_nz(c, c->a);
}

static void compare(Cpu6502 *c, uint8_t reg, uint8_t v) {
  set_c(c, reg >= v);
  set_nz(c, (uint8_t)(reg - v));
}

static void branch(Cpu6502 *c, int taken) {
  int8_t offset = (int8_t)fetch(c);
  if(taken) {
    uint16_t target = c->pc + offset;
    c->cycles += 1 + ((target ^ c->pc) & 0xFF00 ? 1 : 0);
    c->pc = target;
  }
}

// Shifts and rotates: ASL ROL LSR ROR by bits 5-6 of the opcode.
static uint8_t shift(Cpu6502 *c, uint8_t op, uint8_t v) {
  uint8_t carry = c->p & kFlagC, r;
  switch((op >> 5) & 3) {
    case 0: set_c(c, v & 0x80); r = v << 1; break;
    case 1: set_c(c, v & 0x80); r = v << 1 | carry; break;
    case 2: set_c(c, v & 1); r = v >> 1; break;
    default: set_c(c, v & 1); r = v >> 1 | carry << 7; break;
  }
  set_nz(c, r);
  return r;
}

// Read-modify-write on memory: the shifts, INC and DEC.
static void rmw(Cpu6502 *c, uint8_t op, uint16_t addr) {
  uint8_t v = cpu_bus_read(addr);
  if(op >= 0xC0) {
    v += op >= 0xE0 ? 1 : -1;
    set_nz(c, v);
  } else {
    v = shift(c, op, v);
  }
  cpu_bus_write(addr, v);
}

static uint16_t rmw_addr(Cpu6502 *c, uint8_t op) {
  switch((op >> 3) & 3) {
    case 0: return fetch(c);
    case 1: return fetch16(c);
    case 2: return zp_x(c);
    default: return abs_i(c, c->x, 0);
  }
}

// The opcodes that read or write memory as two official instructions would, with bits 0-1 both set:
// SLO RLA SRE RRA, SAX, LAX, DCP and ISB.
static void unofficial_combined(Cpu6502 *c, uint8_t op) {
  int high = (op & 0xC0) == 0x80;  // SAX and LAX, whose indexed modes use Y, not X
  int lax = (op & 0xE0) == 0xA0;
  uint16_t addr;
  switch((op >> 2) & 7) {
    case 0: addr = ind_x(c); break;
    case 1: addr = fetch(c); break;
    case 3: addr = fetch16(c); break;
    case 4: addr = ind_y(c, lax); break;
    case 5: addr = high ? zp_y(c) : zp_x(c); break;
    case 6: addr = abs_i(c, c->y, lax); break;
    default: addr = abs_i(c, high ? c->y : c->x, lax); break;
  }
  if(high) {
    if(lax) {
      c->a = c->x = cpu_bus_read(addr);
      set_nz(c, c->a);
    } else {
      cpu_bus_write(addr, c->a & c->x);
    }
    return;
  }
  uint8_t v = cpu_bus_read(addr);
  if(op >= 0xC0) {
    v += op >= 0xE0 ? 1 : -1;
    cpu_bus_write(addr, v);
    if(op >= 0xE0) {
      adc(c, ~v);
    } else {
      compare(c, c->a, v);
    }
    return;
  }
  v = shift(c, op, v);
  cpu_bus_write(addr, v);
  switch(op >> 5) {
    case 0: c->a |= v; set_nz(c, c->a); break;
    case 1: c->a &= v; set_nz(c, c->a); break;
    case 2: c->a ^= v; set_nz(c, c->a); break;
    default: adc(c, v); break;
  }
}

// SHA, SHX and SHY: `v` AND the base address's high byte + 1, which on a page cross replaces the
// high byte of the address too.
static void unofficial_store_high(uint16_t base, uint8_t index, uint8_t v) {
  uint16_t addr = base + index;
  v &= (uint8_t)((base >> 8) + 1);
  if((addr ^ base) & 0xFF00) {
    addr = (uint16_t)(v << 8 | (addr & 0xFF));
  }
  cpu_bus_write(addr, v);
}

// The rest of the unofficial opcodes. Returns 1 if op is one that freezes the 6502.
static int unofficial(Cpu6502 *c, uint8_t op) {
  switch(op) {
    case 0x02: case 0x12: case 0x22: case 0x32: case 0x42: case 0x52:
    case 0x62: case 0x72: case 0x92: case 0xB2: case 0xD2: case 0xF2:
      c->pc--;
      return 1;

    // NOPs of each size, which make their reads all the same.
    case 0x1A: case 0x3A: case 0x5A: case 0x7A: case 0xDA: case 0xFA: break;
    case 0x80: case 0x82: case 0x89: case 0xC2: case 0xE2: fetch(c); break;
    case 0x04: case 0x44: case 0x64: cpu_bus_read(fetch(c)); break;
    case 0x14: case 0x34: case 0x54: case 0x74: case 0xD4: case 0xF4: cpu_bus_read(zp_x(c)); break;
    case 0x0C: cpu_bus_read(fetch16(c)); break;
    case 0x1C: case 0x3C: case 0x5C: case 0x7C: case 0xDC: case 0xFC: cpu_bus_read(abs_i(c, c->x, 1)); break;

    // Immediate operands. $8B and $AB are unstable on the chip: these are as Mesen2 has them.
    case 0x0B: case 0x2B: c->a &= fetch(c); set_nz(c, c->a); set_c(c, c->a & 0x80); break;
    case 0x4B: c->a &= fetch(c); set_c(c, c->a & 1); c->a >>= 1; set_nz(c, c->a); break;
    case 0x6B: {
      c->a = (c->a & fetch(c)) >> 1 | (c->p & kFlagC) << 7;
      set_nz(c, c->a);
      set_c(c, c->a & 0x40);
      c->p = (c->a >> 6 ^ c->a >> 5) & 1 ? c->p | kFlagV : c->p & ~kFlagV;
      break;
    }
    case 0x8B: c->a = (c->a | 0xEE) & c->x & fetch(c); set_nz(c, c->a); break;
    case 0xAB: c->a = c->x = fetch(c); set_nz(c, c->a); break;
    case 0xCB: {
      uint8_t v = fetch(c), t = c->a & c->x;
      set_c(c, t >= v);
      c->x = t - v;
      set_nz(c, c->x);
      break;
    }
    case 0xEB: adc(c, ~fetch(c)); break;

    // Stores of a value ANDed with the address, and LAS.
    case 0x93: {
      uint8_t z = fetch(c);
      unofficial_store_high(read16(z, (uint8_t)(z + 1)), c->y, c->a & c->x);
      break;
    }
    case 0x9B: c->s = c->a & c->x; unofficial_store_high(fetch16(c), c->y, c->s); break;
    case 0x9C: unofficial_store_high(fetch16(c), c->x, c->y); break;
    case 0x9E: unofficial_store_high(fetch16(c), c->y, c->x); break;
    case 0x9F: unofficial_store_high(fetch16(c), c->y, c->a & c->x); break;
    case 0xBB: c->a = c->x = c->s = cpu_bus_read(abs_i(c, c->y, 1)) & c->s; set_nz(c, c->a); break;

    default: unofficial_combined(c, op); break;
  }
  return 0;
}

int cpu_step(Cpu6502 *c) {
  uint8_t op = fetch(c);
  c->cycles += kCycles[op];

  if((op & 3) == 1) {  // ORA AND EOR ADC STA LDA CMP SBC
    uint16_t addr = group1_addr(c, op);
    if((op & 0xE0) == 0x80) {
      cpu_bus_write(addr, c->a);
      return 0;
    }
    uint8_t v = cpu_bus_read(addr);
    switch(op >> 5) {
      case 0: c->a |= v; set_nz(c, c->a); break;
      case 1: c->a &= v; set_nz(c, c->a); break;
      case 2: c->a ^= v; set_nz(c, c->a); break;
      case 3: adc(c, v); break;
      case 5: c->a = v; set_nz(c, c->a); break;
      case 6: compare(c, c->a, v); break;
      case 7: adc(c, ~v); break;
    }
    return 0;
  }

  switch(op) {
    // Shifts on A, and read-modify-write on memory.
    case 0x0A: case 0x2A: case 0x4A: case 0x6A: c->a = shift(c, op, c->a); break;
    case 0x06: case 0x0E: case 0x16: case 0x1E:
    case 0x26: case 0x2E: case 0x36: case 0x3E:
    case 0x46: case 0x4E: case 0x56: case 0x5E:
    case 0x66: case 0x6E: case 0x76: case 0x7E:
    case 0xC6: case 0xCE: case 0xD6: case 0xDE:
    case 0xE6: case 0xEE: case 0xF6: case 0xFE: rmw(c, op, rmw_addr(c, op)); break;

    // Loads and stores of X and Y.
    case 0xA2: c->x = fetch(c); set_nz(c, c->x); break;
    case 0xA6: c->x = cpu_bus_read(fetch(c)); set_nz(c, c->x); break;
    case 0xB6: c->x = cpu_bus_read(zp_y(c)); set_nz(c, c->x); break;
    case 0xAE: c->x = cpu_bus_read(fetch16(c)); set_nz(c, c->x); break;
    case 0xBE: c->x = cpu_bus_read(abs_i(c, c->y, 1)); set_nz(c, c->x); break;
    case 0xA0: c->y = fetch(c); set_nz(c, c->y); break;
    case 0xA4: c->y = cpu_bus_read(fetch(c)); set_nz(c, c->y); break;
    case 0xB4: c->y = cpu_bus_read(zp_x(c)); set_nz(c, c->y); break;
    case 0xAC: c->y = cpu_bus_read(fetch16(c)); set_nz(c, c->y); break;
    case 0xBC: c->y = cpu_bus_read(abs_i(c, c->x, 1)); set_nz(c, c->y); break;
    case 0x86: cpu_bus_write(fetch(c), c->x); break;
    case 0x96: cpu_bus_write(zp_y(c), c->x); break;
    case 0x8E: cpu_bus_write(fetch16(c), c->x); break;
    case 0x84: cpu_bus_write(fetch(c), c->y); break;
    case 0x94: cpu_bus_write(zp_x(c), c->y); break;
    case 0x8C: cpu_bus_write(fetch16(c), c->y); break;

    // Compares and BIT.
    case 0xE0: compare(c, c->x, fetch(c)); break;
    case 0xE4: compare(c, c->x, cpu_bus_read(fetch(c))); break;
    case 0xEC: compare(c, c->x, cpu_bus_read(fetch16(c))); break;
    case 0xC0: compare(c, c->y, fetch(c)); break;
    case 0xC4: compare(c, c->y, cpu_bus_read(fetch(c))); break;
    case 0xCC: compare(c, c->y, cpu_bus_read(fetch16(c))); break;
    case 0x24: case 0x2C: {
      uint8_t v = cpu_bus_read(op == 0x24 ? fetch(c) : fetch16(c));
      c->p = (c->p & ~(kFlagN | kFlagV | kFlagZ)) | (v & (kFlagN | kFlagV)) | (c->a & v ? 0 : kFlagZ);
      break;
    }

    // Register transfers, increments and flags.
    case 0xAA: c->x = c->a; set_nz(c, c->x); break;
    case 0x8A: c->a = c->x; set_nz(c, c->a); break;
    case 0xA8: c->y = c->a; set_nz(c, c->y); break;
    case 0x98: c->a = c->y; set_nz(c, c->a); break;
    case 0xBA: c->x = c->s; set_nz(c, c->x); break;
    case 0x9A: c->s = c->x; break;
    case 0xE8: set_nz(c, ++c->x); break;
    case 0xCA: set_nz(c, --c->x); break;
    case 0xC8: set_nz(c, ++c->y); break;
    case 0x88: set_nz(c, --c->y); break;
    case 0x18: c->p &= ~kFlagC; break;
    case 0x38: c->p |= kFlagC; break;
    case 0x58: c->p &= ~kFlagI; break;
    case 0x78: c->p |= kFlagI; break;
    case 0xB8: c->p &= ~kFlagV; break;
    case 0xD8: c->p &= ~kFlagD; break;
    case 0xF8: c->p |= kFlagD; break;
    case 0xEA: break;

    // Branches.
    case 0x10: branch(c, !(c->p & kFlagN)); break;
    case 0x30: branch(c, c->p & kFlagN); break;
    case 0x50: branch(c, !(c->p & kFlagV)); break;
    case 0x70: branch(c, c->p & kFlagV); break;
    case 0x90: branch(c, !(c->p & kFlagC)); break;
    case 0xB0: branch(c, c->p & kFlagC); break;
    case 0xD0: branch(c, !(c->p & kFlagZ)); break;
    case 0xF0: branch(c, c->p & kFlagZ); break;

    // Jumps, calls, the stack and interrupts.
    case 0x4C: c->pc = fetch16(c); break;
    case 0x6C: {
      uint16_t ptr = fetch16(c);  // the high byte comes from the same page, as on the 6502
      c->pc = read16(ptr, (ptr & 0xFF00) | ((ptr + 1) & 0xFF));
      break;
    }
    case 0x20: {
      uint16_t target = fetch16(c);
      push(c, (c->pc - 1) >> 8);
      push(c, (uint8_t)(c->pc - 1));
      c->pc = target;
      break;
    }
    case 0x60: {
      uint8_t lo = pull(c);
      c->pc = (lo | pull(c) << 8) + 1;
      break;
    }
    case 0x40: {
      c->p = pull(c) & ~(kFlagB | kFlagU);
      uint8_t lo = pull(c);
      c->pc = lo | pull(c) << 8;
      break;
    }
    case 0x48: push(c, c->a); break;
    case 0x08: push(c, c->p | kFlagB | kFlagU); break;
    case 0x68: c->a = pull(c); set_nz(c, c->a); break;
    case 0x28: c->p = pull(c) & ~(kFlagB | kFlagU); break;
    case 0x00: {
      c->pc++;
      push(c, c->pc >> 8);
      push(c, (uint8_t)c->pc);
      push(c, c->p | kFlagB | kFlagU);
      c->p |= kFlagI;
      c->pc = read16(0xFFFE, 0xFFFF);
      break;
    }
    default: return unofficial(c, op);
  }
  return 0;
}

void cpu_nmi(Cpu6502 *c) {
  push(c, c->pc >> 8);
  push(c, (uint8_t)c->pc);
  push(c, c->p | kFlagU);
  c->p |= kFlagI;
  c->pc = read16(0xFFFA, 0xFFFB);
  c->cycles += 7;
}
