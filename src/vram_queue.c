// The VRAM queue and the attribute buffers. The game puts tiles in
// vram_queue as entries (address high, low, length, then the bytes) for the NMI to write to
// the nametables: a block copied or filled row by row, or a column copied downwards. When the
// queue fills up, it asks the NMI to empty it and waits. Attribute bits go to attr_buffer_2000
// and attr_buffer_2400, which the NMI uploads whole.
//
// The scratch bytes, as the originals use them: $10/$11 the VRAM address of the current row
// (or column), $12 its length, $13 the rows left, $14 the bytes left in the row, $15/$16 the
// VRAM address of the queue entry being built, $17 its length so far, $1C/$1D the VRAM address
// being written, $1E/$1F the source. Y indexes the source through a whole block.
#include "funcs.h"
#include "variables.h"

enum {
  kAttrBitsTable = 0xF1FA,  // [16] two attribute bits moved to one of the four quadrants
  kAttrMaskTable = 0xF20A,  // [4] keeps the other three quadrants
  kAttrFillTable = 0xF243,  // [4] a whole byte of one palette
};

void add16(uint8_t *p, uint8_t n) {
  emu_set_flag(kFlagC, 0);
  p[0] = emu_adc(p[0], n);
  p[1] = emu_adc(p[1], 0);
  g_nes.cpu.a = p[1];
}

uint8_t asl_a(uint8_t a) {
  emu_set_flag(kFlagC, a >> 7);
  emu_set_a((uint8_t)(a << 1));
  return g_nes.cpu.a;
}

// The same with sec and sbc.
static void sub16(uint8_t *p, uint8_t n) {
  emu_set_flag(kFlagC, 1);
  p[0] = emu_sbc(p[0], n);
  p[1] = emu_sbc(p[1], 0);
  g_nes.cpu.a = p[1];
}

// ---- Blocks -------------------------------------------------------------------------------

// queue_vram_block (7:EEE5): queues the block at $1E (a header of row length and rows, then
// the tiles) for VRAM address $1C/$1D. Keeps X and Y; returns A = X.
void queue_vram_block(void) {
  RUN_ORIGINAL_IF_LAG(kQueueVramBlock);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t x = c->x, y = c->y;
  emu_push(x);
  emu_push(y);
  emu_set_a(y);
  vram_block_setup();
  RESUME_ORIGINAL_IF_LAG(kQueueVramBlock, 0xEEEC, 2);
  do {
    vram_copy_rows();
    RESUME_ORIGINAL_IF_LAG(kQueueVramBlock, 0xEEEF, 2);
    next_vram_row();
    RESUME_ORIGINAL_IF_LAG(kQueueVramBlock, 0xEEF2, 2);
  } while(g_ram[0x13]);
  emu_pull();
  emu_pull();
  c->y = y;
  c->x = x;
  emu_set_a(x);
}

// queue_vram_fill (7:EEF9): the same with every tile the block's first.
void queue_vram_fill(void) {
  RUN_ORIGINAL_IF_LAG(kQueueVramFill);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t x = c->x, y = c->y;
  emu_push(x);
  emu_push(y);
  emu_set_a(y);
  vram_block_setup();
  RESUME_ORIGINAL_IF_LAG(kQueueVramFill, 0xEF00, 2);
  do {
    vram_fill_rows();
    RESUME_ORIGINAL_IF_LAG(kQueueVramFill, 0xEF03, 2);
    next_vram_row();
    RESUME_ORIGINAL_IF_LAG(kQueueVramFill, 0xEF06, 2);
  } while(g_ram[0x13]);
  emu_pull();
  emu_pull();
  c->y = y;
  c->x = x;
  emu_set_a(x);
}

// vram_block_setup (7:EF0D): waits for the queue to be empty, then reads the block's header.
// Returns Y = 0.
void vram_block_setup(void) {
  RUN_ORIGINAL_IF_LAG(kVramBlockSetup);
  Cpu6502 *c = &g_nes.cpu;
  wait_vram_queue_empty();
  RESUME_ORIGINAL_IF_LAG(kVramBlockSetup, 0xEF10, 0);
  g_ram[0x12] = emu_read_ind_y(0x1E, 0);
  c->y = 1;
  g_ram[0x13] = emu_read_ind_y(0x1E, 1);
  emu_set_a(g_ram[0x13]);
  skip_block_header();
  g_ram[0x10] = g_ram[0x1C];
  g_ram[0x11] = g_ram[0x1D];
  c->a = g_ram[0x1D];
  c->y = 0;
  emu_set_nz(0);
}

// vram_copy_rows (7:EF60) and vram_fill_rows (7:EF29): one row of the block into the queue,
// as one entry or, where the row wraps past the nametable's right edge, two. A full queue is
// emptied between the two.
void vram_copy_rows(void) {
  RUN_ORIGINAL_IF_LAG(kVramCopyRows);
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x15] = g_ram[0x10];
  g_ram[0x16] = g_ram[0x11];
  g_ram[0x14] = g_ram[0x12];
  for(;;) {
    g_ram[0x17] = 0;
    uint8_t x = (uint8_t)(vram_queue_len + 3);
    do {
      uint8_t v = emu_read_ind_y(0x1E, c->y);
      vram_queue[x] = v;
      g_ram[0x17]++;
      c->y++;
      x++;
      c->x = x;
      emu_set_a(v);
      if(--g_ram[0x14] == 0) {
        emu_set_nz(0);
        vram_queue_header();  // the original branches into it
        return;
      }
      emu_set_nz(g_ram[0x14]);
      next_vram_address();
    } while(g_ram[0x1C] & 0x1F);
    emu_set_a(0);
    vram_wrap_up();
    flush_vram_queue_if_full();
    RESUME_ORIGINAL_IF_LAG(kVramCopyRows, 0xEF93, 0);
  }
}

void vram_fill_rows(void) {
  RUN_ORIGINAL_IF_LAG(kVramFillRows);
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x15] = g_ram[0x10];
  g_ram[0x16] = g_ram[0x11];
  g_ram[0x14] = g_ram[0x12];
  for(;;) {
    g_ram[0x17] = 0;
    uint8_t x = (uint8_t)(vram_queue_len + 3);
    c->y = 0;
    do {
      uint8_t v = emu_read_ind_y(0x1E, 0);
      vram_queue[x] = v;
      x++;
      g_ram[0x17]++;
      c->x = x;
      emu_set_a(v);
      if(--g_ram[0x14] == 0) {
        emu_set_nz(0);
        vram_queue_header();
        return;
      }
      emu_set_nz(g_ram[0x14]);
      next_vram_address();
    } while(g_ram[0x1C] & 0x1F);
    emu_set_a(0);
    vram_wrap_up();
    flush_vram_queue_if_full();
    RESUME_ORIGINAL_IF_LAG(kVramFillRows, 0xEF5D, 0);
  }
}

// next_vram_address (7:EF96): $1C/$1D += 1.
void next_vram_address(void) {
  RUN_ORIGINAL_IF_LAG(kNextVramAddress);
  add16(g_ram + 0x1C, 0x01);
}

// vram_wrap_up (7:EFA4): ends the queue entry at the nametable's right edge and starts the next
// one at the left edge of the next nametable across (the same one with horizontal mirroring).
void vram_wrap_up(void) {
  RUN_ORIGINAL_IF_LAG(kVramWrapUp);
  vram_queue_header();
  sub16(g_ram + 0x1C, 0x20);
  if(!(mirroring_mode & 0x08)) {
    g_ram[0x1D] ^= 0x04;
  }
  g_ram[0x15] = g_ram[0x1C];
  g_ram[0x16] = g_ram[0x1D];
  emu_set_a(g_ram[0x1D]);
}

// vram_queue_header (7:EFC9): writes the header of the entry being built (address $15/$16,
// length $17) in front of its bytes, and adds the entry to vram_queue_len.
void vram_queue_header(void) {
  RUN_ORIGINAL_IF_LAG(kVramQueueHeader);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t x = vram_queue_len;
  emu_set_flag(kFlagC, 0);
  vram_queue_len = emu_adc(emu_adc(x, g_ram[0x17]), 0x03);
  vram_queue[x++] = g_ram[0x16];
  vram_queue[x++] = g_ram[0x15];
  vram_queue[x] = g_ram[0x17];
  c->x = x;
  emu_set_a(g_ram[0x17]);
}

// next_vram_row (7:EFE7): empties a full queue, then moves $10/$11 (and $1C/$1D) down a row,
// from the bottom of the nametable to the top of the one below; counts the row in $13.
void next_vram_row(void) {
  RUN_ORIGINAL_IF_LAG(kNextVramRow);
  flush_vram_queue_if_full();
  RESUME_ORIGINAL_IF_LAG(kNextVramRow, 0xEFEA, 0);
  emu_set_flag(kFlagC, 0);
  g_ram[0x10] = emu_adc(0x20, g_ram[0x10]);
  g_ram[0x11] = emu_adc(0x00, g_ram[0x11]);
  emu_cmp(g_ram[0x11] & 0x03, 0x03);
  if((g_ram[0x11] & 0x03) == 0x03) {
    emu_cmp(g_ram[0x10] & 0xC0, 0xC0);
    if((g_ram[0x10] & 0xC0) == 0xC0) {
      g_ram[0x10] &= 0x1F;
      g_ram[0x11] &= 0xFC;
      if(mirroring_mode & 0x08) {
        g_ram[0x11] ^= 0x08;
      }
    }
  }
  g_ram[0x1C] = g_ram[0x10];
  g_ram[0x1D] = g_ram[0x11];
  g_ram[0x13]--;
  g_nes.cpu.a = g_ram[0x11];
  emu_set_nz(g_ram[0x13]);
}

// skip_block_header (7:F02A): $1E/$1F += 2.
void skip_block_header(void) {
  RUN_ORIGINAL_IF_LAG(kSkipBlockHeader);
  add16(g_ram + 0x1E, 0x02);
}

// flush_vram_queue_if_full (7:F038): from 30 bytes queued (and below 158: the original tests
// the sign of the difference), asks the NMI to write out the queue and waits for it.
void flush_vram_queue_if_full(void) {
  RUN_ORIGINAL_IF_LAG(kFlushVramQueueIfFull);
  uint8_t len = vram_queue_len;
  emu_set_a(len);
  emu_cmp(len, 0x1E);
  if((uint8_t)(len - 0x1E) & 0x80) {
    return;
  }
  request_vram_queue();
  wait_vram_queue_empty();
}

// ---- Columns ------------------------------------------------------------------------------

// flush_vram_queue_down_if_full (7:F046): the same for queues written downwards.
void flush_vram_queue_down_if_full(void) {
  RUN_ORIGINAL_IF_LAG(kFlushVramQueueDownIfFull);
  uint8_t len = vram_queue_len;
  emu_set_a(len);
  emu_cmp(len, 0x1E);
  if((uint8_t)(len - 0x1E) & 0x80) {
    return;
  }
  flush_vram_queue_down();
  RESUME_ORIGINAL_IF_LAG(kFlushVramQueueDownIfFull, 0xF050, 0);
  wait_nmi_work_done();
}

// flush_vram_queue_down (7:F054): asks the NMI to write the queue going down, once it has done
// everything else, and waits for it.
void flush_vram_queue_down(void) {
  RUN_ORIGINAL_IF_LAG(kFlushVramQueueDown);
  wait_nmi_work_done();
  RESUME_ORIGINAL_IF_LAG(kFlushVramQueueDown, 0xF057, 0);
  request_nmi_work(0x02);
  wait_nmi_work_done();
}

// queue_vram_column (7:F060): queues the column at $1E (a length, then the tiles) for VRAM
// address $1C/$1D, going down. Keeps X and Y; returns A = X.
void queue_vram_column(void) {
  RUN_ORIGINAL_IF_LAG(kQueueVramColumn);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t x = c->x, y = c->y;
  emu_push(x);
  emu_push(y);
  emu_set_a(y);
  vram_column_setup();
  RESUME_ORIGINAL_IF_LAG(kQueueVramColumn, 0xF067, 2);
  vram_copy_column();
  RESUME_ORIGINAL_IF_LAG(kQueueVramColumn, 0xF06A, 2);
  emu_pull();
  emu_pull();
  c->y = y;
  c->x = x;
  emu_set_a(x);
}

// vram_column_setup (7:F06F): waits for the NMI's work, then reads the column's length.
// Returns Y = 0.
void vram_column_setup(void) {
  RUN_ORIGINAL_IF_LAG(kVramColumnSetup);
  Cpu6502 *c = &g_nes.cpu;
  wait_nmi_work_done();
  RESUME_ORIGINAL_IF_LAG(kVramColumnSetup, 0xF072, 0);
  c->y = 0;
  g_ram[0x12] = emu_read_ind_y(0x1E, 0);
  g_ram[0x10] = g_ram[0x1C];
  g_ram[0x11] = g_ram[0x1D];
  add16(g_ram + 0x1E, 0x01);
}

// vram_copy_column (7:F090): the column into the queue, as one entry or, where it runs past the
// nametable's bottom, two (a full queue emptied between them).
void vram_copy_column(void) {
  RUN_ORIGINAL_IF_LAG(kVramCopyColumn);
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x15] = g_ram[0x10];
  g_ram[0x16] = g_ram[0x11];
  g_ram[0x14] = g_ram[0x12];
  for(;;) {
    g_ram[0x17] = 0;
    uint8_t x = (uint8_t)(vram_queue_len + 3);
    for(;;) {
      uint8_t v = emu_read_ind_y(0x1E, c->y);
      vram_queue[x] = v;
      c->y++;
      x++;
      g_ram[0x17]++;
      c->x = x;
      emu_set_a(v);
      if(--g_ram[0x14] == 0) {
        emu_set_nz(0);
        vram_queue_header();
        return;
      }
      emu_set_nz(g_ram[0x14]);
      vram_addr_down_row();
      if((g_ram[0x1D] & 0x03) == 0x03 && (g_ram[0x1C] & 0xC0) == 0xC0) {
        break;
      }
    }
    emu_set_a(0xC0);
    emu_cmp(0xC0, 0xC0);
    vram_wrap_column();
    flush_vram_queue_down_if_full();
    RESUME_ORIGINAL_IF_LAG(kVramCopyColumn, 0xF0D0, 0);
  }
}

// vram_addr_down_row (7:F0D3): $1C/$1D += $20.
void vram_addr_down_row(void) {
  RUN_ORIGINAL_IF_LAG(kVramAddrDownRow);
  add16(g_ram + 0x1C, 0x20);
}

// vram_wrap_column (7:F0E1): ends the queue entry at the nametable's bottom and starts the next
// one at the top.
void vram_wrap_column(void) {
  RUN_ORIGINAL_IF_LAG(kVramWrapColumn);
  vram_queue_header();
  g_ram[0x1C] &= 0x1F;
  g_ram[0x1D] &= 0xFC;
  if(mirroring_mode & 0x08) {
    g_ram[0x1C] ^= 0x08;
  }
  g_ram[0x15] = g_ram[0x1C];
  g_ram[0x16] = g_ram[0x1D];
  emu_set_a(g_ram[0x1D]);
}

// ---- Attributes ---------------------------------------------------------------------------

static int asl(uint8_t *a) {
  int carry = *a >> 7;
  *a = (uint8_t)(*a << 1);
  return carry;
}

// rol on a RAM byte with the carry in; returns the carry out.
static int rol(int addr, int carry) {
  int out = g_ram[addr] >> 7;
  g_ram[addr] = (uint8_t)(g_ram[addr] << 1 | carry);
  return out;
}

// attr_position (7:F105): for VRAM address $1C/$1D, the byte in attr_buffer_2000 (both
// nametables' buffers, $40 apart) in $10, and the quadrant in it (0-3) in $11.
void attr_position(void) {
  RUN_ORIGINAL_IF_LAG(kAttrPosition);
  g_ram[0x11] = 0;
  g_ram[0x10] = g_ram[0x1D] & 0x03;
  if(g_ram[0x1D] & 0x0C) {
    g_ram[0x10] |= 0x04;
  }
  uint8_t a = g_ram[0x1C];
  int carry = rol(0x10, asl(&a));
  carry = rol(0x11, asl(&a));
  asl(&a);
  carry = rol(0x10, asl(&a));
  carry = rol(0x10, asl(&a));
  carry = rol(0x10, asl(&a));
  carry = rol(0x11, asl(&a));
  g_nes.cpu.a = a;
  emu_set_nz(g_ram[0x11]);
  emu_set_flag(kFlagC, carry);
}

// set_attr_block (7:F131): sets the attribute bits of a block of $20 by $21 tiles at $1C/$1D
// (rounded down to a 2x2 square) to palette $22. Keeps X and Y; returns A = X.
void set_attr_block(void) {
  RUN_ORIGINAL_IF_LAG(kSetAttrBlock);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t x = c->x, y = c->y;
  g_ram[0x1C] &= 0xDE;
  g_ram[0x15] = g_ram[0x1C];
  g_ram[0x16] = g_ram[0x1D];
  g_ram[0x13] = g_ram[0x21];
  do {
    g_ram[0x12] = g_ram[0x20];
    emu_set_a(g_ram[0x20]);
    do {
      set_attr_bits();
      next_attr_column();
      g_ram[0x12] -= 2;
    } while(g_ram[0x12] && !(g_ram[0x12] & 0x80));
    next_attr_row();
    g_ram[0x13] -= 2;
  } while(g_ram[0x13] && !(g_ram[0x13] & 0x80));
  c->y = y;
  c->x = x;
  emu_set_a(x);
}

// next_attr_row (7:F167): $15/$16 down two rows of tiles, from the bottom of the nametable to
// the top of the one below; $1C/$1D the same.
void next_attr_row(void) {
  RUN_ORIGINAL_IF_LAG(kNextAttrRow);
  add16(g_ram + 0x15, 0x40);
  emu_cmp(g_ram[0x16] & 0x03, 0x03);
  if((g_ram[0x16] & 0x03) == 0x03) {
    emu_cmp(g_ram[0x15] & 0xC0, 0xC0);
    if((g_ram[0x15] & 0xC0) == 0xC0) {
      g_ram[0x15] &= 0x1E;
      g_ram[0x16] &= 0xFC;
      if(mirroring_mode & 0x08) {
        g_ram[0x16] ^= 0x08;
      }
    }
  }
  g_ram[0x1C] = g_ram[0x15];
  g_ram[0x1D] = g_ram[0x16];
  emu_set_a(g_ram[0x16]);
}

// next_attr_column (7:F1A5): $1C/$1D two tiles right, from the right edge to the left edge of
// the nametable across.
void next_attr_column(void) {
  RUN_ORIGINAL_IF_LAG(kNextAttrColumn);
  add16(g_ram + 0x1C, 0x02);
  emu_set_a(g_ram[0x1C] & 0x1F);
  if(g_ram[0x1C] & 0x1F) {
    return;
  }
  sub16(g_ram + 0x1C, 0x20);
  emu_set_a(mirroring_mode & 0x08);
  if(mirroring_mode & 0x08) {
    return;
  }
  g_ram[0x1D] ^= 0x04;
  emu_set_a(g_ram[0x1D]);
}

// set_attr_bits (7:F1D2): sets the two attribute bits for VRAM address $1C/$1D to palette $22.
// Returns X = the byte's index, Y = the table index, $14 the bits set.
void set_attr_bits(void) {
  RUN_ORIGINAL_IF_LAG(kSetAttrBits);
  Cpu6502 *c = &g_nes.cpu;
  attr_position();
  uint8_t x = g_ram[0x10], y = g_ram[0x11];
  uint8_t keep = cpu_bus_read((uint16_t)(kAttrMaskTable + y));
  attr_buffer_2000[x] &= keep;
  g_ram[0x14] = y;
  uint8_t palette = g_ram[0x22];
  rol(0x14, palette & 1);
  int carry = rol(0x14, palette >> 1 & 1);
  uint8_t index = g_ram[0x14];
  g_ram[0x14] = cpu_bus_read((uint16_t)(kAttrBitsTable + index));
  attr_buffer_2000[x] |= g_ram[0x14];
  c->x = x;
  c->y = index;
  emu_set_a(attr_buffer_2000[x]);
  emu_set_flag(kFlagC, carry);
}

// fill_all_attrs (7:F20E): sets every attribute byte of both buffers to palette A & 3, once the
// NMI has done its work. Keeps X and Y.
void fill_all_attrs(uint8_t palette) {
  emu_set_a(palette);
  RUN_ORIGINAL_IF_LAG(kFillAllAttrs);
  pick_attr_fill(palette);
  RESUME_ORIGINAL_IF_LAG(kFillAllAttrs, 0xF211, 0);
  fill_attr_buffer_2000();
  fill_attr_buffer_2400();
  restore_xy();
}

// pick_attr_fill (7:F22F): keeps A, X, Y in $10, $11, $12, waits for the NMI's work, and puts
// the attribute byte for palette A & 3 in $13.
void pick_attr_fill(uint8_t palette) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(palette);
  RUN_ORIGINAL_IF_LAG(kPickAttrFill);
  g_ram[0x10] = palette;
  g_ram[0x11] = c->x;
  g_ram[0x12] = c->y;
  wait_nmi_work_done();
  RESUME_ORIGINAL_IF_LAG(kPickAttrFill, 0xF238, 0);
  c->y = g_ram[0x10] & 0x03;
  g_ram[0x13] = cpu_bus_read((uint16_t)(kAttrFillTable + c->y));
  emu_set_a(g_ram[0x13]);
}

// fill_attr_buffer_2000 (7:F247) and fill_attr_buffer_2400 (7:F24B): fill one buffer with $13.
// The first jumps into the second's loop.
static void fill_attr_buffer(uint8_t x) {
  Cpu6502 *c = &g_nes.cpu;
  for(int i = 0; i < 0x40; i++) {
    attr_buffer_2000[x + i] = g_ram[0x13];
  }
  c->a = g_ram[0x13];
  c->x = (uint8_t)(x + 0x40);
  c->y = 0x40;
  emu_cmp(0x40, 0x40);
}

void fill_attr_buffer_2000(void) {
  if(emu_lag_in(kFillAttrBuffer2000) || emu_lag_in(kFillAttrBuffer2400)) {
    emu_call_original(kFillAttrBuffer2000);
    return;
  }
  fill_attr_buffer(0x00);
}

void fill_attr_buffer_2400(void) {
  RUN_ORIGINAL_IF_LAG(kFillAttrBuffer2400);
  fill_attr_buffer(0x40);
}

// restore_xy (7:F25B): X and Y from where pick_attr_fill kept them.
void restore_xy(void) {
  RUN_ORIGINAL_IF_LAG(kRestoreXy);
  g_nes.cpu.x = g_ram[0x11];
  g_nes.cpu.y = g_ram[0x12];
  emu_set_nz(g_nes.cpu.y);
}
