// Unit tests for src/cpu6502.c on a flat 64 KB bus, for CI, which has no ROM. (With the ROM,
// dv_harness --compare checks the interpreter against Mesen2 on whole TAS movies.)
#include <stdio.h>
#include <string.h>

#include "cpu6502.h"

static uint8_t mem[0x10000];
static uint16_t reads[16];  // the last reads, in order
static int nreads;
static int failures;

uint8_t cpu_bus_read(uint16_t addr) {
  reads[nreads++ & 15] = addr;
  return mem[addr];
}

void cpu_bus_write(uint16_t addr, uint8_t value) {
  mem[addr] = value;
}

#define CHECK(cond)                                                \
  do {                                                             \
    if(!(cond)) {                                                  \
      fprintf(stderr, "%s:%d: failed: %s\n", __FILE__, __LINE__, #cond); \
      failures++;                                                  \
    }                                                              \
  } while(0)

// Loads code at $8000 and returns a CPU ready to run it.
static Cpu6502 load(const uint8_t *code, size_t n) {
  memset(mem, 0, sizeof(mem));
  memcpy(mem + 0x8000, code, n);
  Cpu6502 c = {0};
  c.pc = 0x8000;
  c.s = 0xFD;
  return c;
}

// Runs one instruction and returns the cycles it took.
static int step(Cpu6502 *c) {
  uint64_t before = c->cycles;
  CHECK(cpu_step(c) == 0);
  return (int)(c->cycles - before);
}

static void test_page_crossing(void) {
  static const uint8_t code[] = {
    0xBD, 0x00, 0x02,  // lda $0200,x: no crossing, 4
    0xBD, 0xFF, 0x02,  // lda $02FF,x: crosses, 5
    0x9D, 0x00, 0x02,  // sta $0200,x: always 5
    0xB1, 0x10,        // lda ($10),y with $10 = $02FF: crosses, 6
    0xB1, 0x12,        // lda ($12),y with $12 = $0200: 5
    0x91, 0x10,        // sta ($10),y: always 6
    0x1E, 0x00, 0x02,  // asl $0200,x: always 7
  };
  Cpu6502 c = load(code, sizeof(code));
  c.x = 1;
  c.y = 1;
  mem[0x10] = 0xFF, mem[0x11] = 0x02, mem[0x12] = 0x00, mem[0x13] = 0x02;
  mem[0x0300] = 0x42;
  CHECK(step(&c) == 4);
  CHECK(step(&c) == 5);
  CHECK(c.a == 0x42);
  CHECK(step(&c) == 5);
  CHECK(step(&c) == 6);
  CHECK(step(&c) == 5);
  CHECK(step(&c) == 6);
  CHECK(step(&c) == 7);
}

static void test_branches(void) {
  static const uint8_t code[] = {
    0xD0, 0x02,  // bne +2: not taken (Z set), 2
    0xF0, 0x00,  // beq +0: taken, same page, 3
  };
  Cpu6502 c = load(code, sizeof(code));
  c.p = kFlagZ;
  CHECK(step(&c) == 2);
  CHECK(step(&c) == 3);
  CHECK(c.pc == 0x8004);
  // A taken branch onto another page takes 4.
  mem[0x80FD] = 0xF0, mem[0x80FE] = 0x10;  // beq: from $80FF to $810F
  c.pc = 0x80FD;
  CHECK(step(&c) == 4);
  CHECK(c.pc == 0x810F);
  mem[0x810F] = 0xD0, mem[0x8110] = 0xFC;  // bne back, taken? Z is set: not taken
  CHECK(step(&c) == 2);
}

static void test_adc_sbc(void) {
  static const uint8_t code[] = {
    0x69, 0x50,  // adc #$50
    0x69, 0x01,  // adc #$01
    0x38,        // sec
    0xE9, 0xF0,  // sbc #$F0
    0x38,        // sec
    0xE9, 0x70,  // sbc #$70
  };
  Cpu6502 c = load(code, sizeof(code));
  c.a = 0x50;
  step(&c);  // $50 + $50 = $A0: overflow, negative, no carry
  CHECK(c.a == 0xA0 && (c.p & (kFlagV | kFlagN | kFlagC | kFlagZ)) == (kFlagV | kFlagN));
  c.a = 0xFF;
  step(&c);  // $FF + $01 = $00 with carry
  CHECK(c.a == 0x00 && (c.p & (kFlagV | kFlagN | kFlagC | kFlagZ)) == (kFlagC | kFlagZ));
  step(&c);
  c.a = 0x50;
  step(&c);  // $50 - $F0 = $60, borrow, no overflow
  CHECK(c.a == 0x60 && (c.p & (kFlagV | kFlagC)) == 0);
  step(&c);
  c.a = 0xD0;
  step(&c);  // $D0 - $70 = $60: negative minus positive overflows
  CHECK(c.a == 0x60 && (c.p & (kFlagV | kFlagC)) == (kFlagV | kFlagC));
}

static void test_shifts_compares(void) {
  static const uint8_t code[] = {
    0x0A,              // asl a: $81 -> $02, carry
    0x6A,              // ror a: $02 with carry -> $81, carry 0
    0x26, 0x20,        // rol $20: $80 -> $00, carry, zero
    0xC6, 0x21,        // dec $21: $00 -> $FF, negative
    0xC9, 0x81,        // cmp #$81: equal, carry
    0xE0, 0x05,        // cpx #$05 with x = 4: borrow
    0x2C, 0x22, 0x00,  // bit $0022 ($C0): n, v; a & $C0 = $80, so not zero
  };
  Cpu6502 c = load(code, sizeof(code));
  c.a = 0x81;
  c.x = 4;
  mem[0x20] = 0x80, mem[0x21] = 0x00, mem[0x22] = 0xC0;
  step(&c);
  CHECK(c.a == 0x02 && (c.p & kFlagC));
  step(&c);
  CHECK(c.a == 0x81 && !(c.p & kFlagC) && (c.p & kFlagN));
  step(&c);
  CHECK(mem[0x20] == 0x00 && (c.p & kFlagC) && (c.p & kFlagZ));
  CHECK(step(&c) == 5);
  CHECK(mem[0x21] == 0xFF && (c.p & kFlagN));
  step(&c);
  CHECK((c.p & (kFlagC | kFlagZ)) == (kFlagC | kFlagZ));
  step(&c);
  CHECK(!(c.p & kFlagC) && (c.p & kFlagN));
  step(&c);
  CHECK((c.p & (kFlagN | kFlagV | kFlagZ)) == (kFlagN | kFlagV));
}

static void test_calls_and_stack(void) {
  static const uint8_t code[] = {
    0x20, 0x10, 0x80,  // jsr $8010
    0x6C, 0xFF, 0x02,  // jmp ($02FF): the high byte comes from $0200, not $0300
  };
  Cpu6502 c = load(code, sizeof(code));
  mem[0x8010] = 0x08;  // php
  mem[0x8011] = 0x28;  // plp
  mem[0x8012] = 0x60;  // rts
  mem[0x02FF] = 0x34, mem[0x0200] = 0x12, mem[0x0300] = 0x99;
  c.p = kFlagC;
  CHECK(step(&c) == 6);
  CHECK(c.pc == 0x8010 && c.s == 0xFB && mem[0x1FD] == 0x80 && mem[0x1FC] == 0x02);
  CHECK(step(&c) == 3);
  CHECK(mem[0x1FB] == (kFlagC | kFlagB | kFlagU));  // php pushes B and bit 5
  CHECK(step(&c) == 4);
  CHECK(c.p == kFlagC);  // plp drops them
  CHECK(step(&c) == 6);
  CHECK(c.pc == 0x8003 && c.s == 0xFD);
  CHECK(step(&c) == 5);
  CHECK(c.pc == 0x1234);
}

static void test_read_order_and_nmi(void) {
  static const uint8_t code[] = {0xB1, 0x40};  // lda ($40),y
  Cpu6502 c = load(code, sizeof(code));
  mem[0x40] = 0x01, mem[0x41] = 0x40;
  nreads = 0;
  step(&c);
  // Opcode, operand, pointer low, pointer high, then the target: the order sets what stays
  // on the data bus, which unmapped addresses read back.
  CHECK(nreads == 5 && reads[2] == 0x40 && reads[3] == 0x41 && reads[4] == 0x4001);

  mem[0xFFFA] = 0x9A, mem[0xFFFB] = 0xEC;
  c.p = kFlagC;
  uint64_t before = c.cycles;
  cpu_nmi(&c);
  CHECK(c.cycles - before == 7 && c.pc == 0xEC9A && (c.p & kFlagI));
  CHECK(mem[0x1FD] == 0x80 && mem[0x1FC] == 0x02 && mem[0x1FB] == (kFlagC | kFlagU));

  mem[0xEC9A] = 0x02;  // not an official opcode
  CHECK(cpu_step(&c) == -1 && c.pc == 0xEC9A);
}

int main(void) {
  test_page_crossing();
  test_branches();
  test_adc_sbc();
  test_shifts_compares();
  test_calls_and_stack();
  test_read_order_and_nmi();
  if(failures) {
    fprintf(stderr, "test_cpu6502: %d checks failed\n", failures);
    return 1;
  }
  printf("test_cpu6502: all checks passed\n");
  return 0;
}
