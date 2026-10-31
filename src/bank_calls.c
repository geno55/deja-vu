// Banks and calls between them: mirroring, the MMC3's PRG and CHR
// banks, the bank stack, the call-by-index routines.
#include "funcs.h"
#include "variables.h"

#define bank_select_scratch (g_ram[0x10D])  // switch_prg_bank's copy of the bank

// set_mirroring_vertical (7:E87B). Also clears system_flags bit 3.
void set_mirroring_vertical(void) {
  RUN_ORIGINAL_IF_LAG(kSetMirroringVertical);
  cpu_bus_write(0xA000, 0x00);
  mirroring_mode = 0x47;
  system_flags &= 0xF7;
  emu_set_a(system_flags);
}

// switch_prg_bank (7:E89B): maps 16 KB bank `bank` at $8000. system_flags bit 3 is set while
// it runs, so an NMI that comes then leaves the banks alone (sound_update defers itself).
// Returns with A = X.
void switch_prg_bank(uint8_t bank) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(bank);
  RUN_ORIGINAL_IF_LAG(kSwitchPrgBank);
  uint8_t x = c->x;
  bank_select_scratch = bank;
  system_flags |= 0x08;
  mmc3_set_bank(6, (uint8_t)(bank << 1));
  mmc3_set_bank(7, (uint8_t)((bank << 1) + 1));
  system_flags &= 0xF7;
  c->x = x;
  emu_set_a(x);
  emu_set_flag(kFlagC, bank & 0x80);  // from ASL
}

// mmc3_set_bank (7:E8FF): sets MMC3 bank register `reg` to `bank`, keeping the PRG and CHR
// mode bits in bank_select_shadow.
void mmc3_set_bank(uint8_t reg, uint8_t bank) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(reg);
  c->x = bank;
  RUN_ORIGINAL_IF_LAG(kMmc3SetBank);
  bank_select_shadow &= 0xC0;
  bank_select_shadow |= reg;
  cpu_bus_write(0x8000, bank_select_shadow);
  cpu_bus_write(0x8001, bank);
  emu_set_a(bank_select_shadow);
}

// apply_chr_banks_r0_r1 (7:E8BF): the two 2 KB CHR banks from chr_bank_shadows[0] and [2].
void apply_chr_banks_r0_r1(void) {
  Cpu6502 *c = &g_nes.cpu;
  RUN_ORIGINAL_IF_LAG(kApplyChrBanksR0R1);
  uint8_t x = c->x;
  mmc3_set_bank(0, chr_bank_shadows[0] & 0xFE);
  mmc3_set_bank(1, chr_bank_shadows[2] & 0xFE);
  c->x = x;
  emu_set_a(x);
}

// apply_chr_banks_r2_r5 (7:E8DA): the four 1 KB CHR banks from chr_bank_shadows[4-7].
void apply_chr_banks_r2_r5(void) {
  Cpu6502 *c = &g_nes.cpu;
  RUN_ORIGINAL_IF_LAG(kApplyChrBanksR2R5);
  uint8_t x = c->x;
  for(int i = 0; i < 4; i++) {
    mmc3_set_bank((uint8_t)(2 + i), chr_bank_shadows[4 + i]);
  }
  c->x = x;
  emu_set_a(x);
}

// apply_chr_banks (7:E97D)
void apply_chr_banks(void) {
  RUN_ORIGINAL_IF_LAG(kApplyChrBanks);
  apply_chr_banks_r0_r1();
  apply_chr_banks_r2_r5();
}

// advance_inline_ptr (7:E984): inline_ptr += 1, with the flags of the ADCs.
void advance_inline_ptr(void) {
  RUN_ORIGINAL_IF_LAG(kAdvanceInlinePtr);
  unsigned lo = inline_ptr[0] + 1u;
  inline_ptr[0] = (uint8_t)lo;
  uint8_t old = inline_ptr[1];
  unsigned hi = old + (lo >> 8);
  inline_ptr[1] = (uint8_t)hi;
  emu_set_a(inline_ptr[1]);
  emu_set_flag(kFlagC, hi >> 8);
  emu_set_flag(kFlagV, ~old & hi & 0x80);
}

// read_inline_byte (7:EAC6): the byte at inline_ptr, which moves past it. Y = 0.
uint8_t read_inline_byte(void) {
  Cpu6502 *c = &g_nes.cpu;
  if(emu_lag_in(kReadInlineByte)) {
    emu_call_original(kReadInlineByte);
    return c->a;
  }
  c->y = 0;
  uint8_t v = emu_read_ind_y(0x37, 0);  // inline_ptr
  advance_inline_ptr();
  emu_set_a(v);
  return v;
}

// set_chr_banks_inline (7:E916), for original code: sets the eight chr_bank_shadows from the
// bytes after its JSR, applies them, and carries on after the bytes. Returns that address.
uint16_t set_chr_banks_inline(void) {
  Cpu6502 *c = &g_nes.cpu;
  inline_ptr[0] = emu_pull();
  inline_ptr[1] = emu_pull();
  uint8_t x = c->x, y = c->y;
  for(int i = 0; i < 8; i++) {
    advance_inline_ptr();
    c->y = 0;
    chr_bank_shadows[i] = emu_read_ind_y(0x37, 0);  // inline_ptr
  }
  advance_inline_ptr();
  apply_chr_banks();
  c->y = y;
  c->x = x;
  emu_set_a(x);
  return (uint16_t)(inline_ptr[0] | inline_ptr[1] << 8);
}

// push_and_switch_bank (7:EAD0): pushes current_bank on the bank stack, then switches to
// `bank` as set_current_bank does.
void push_and_switch_bank(uint8_t bank) {
  emu_set_a(bank);
  RUN_ORIGINAL_IF_LAG(kPushAndSwitchBank);
  stack_push(current_bank);
  set_current_bank(bank);
}

// set_current_bank (7:EAD7): current_bank = bank, and its low nibble is mapped.
void set_current_bank(uint8_t bank) {
  emu_set_a(bank);
  RUN_ORIGINAL_IF_LAG(kSetCurrentBank);
  current_bank = bank;
  switch_prg_bank(bank & 0x0F);
}

// switch_to_sound_bank (7:EADE)
void switch_to_sound_bank(void) {
  RUN_ORIGINAL_IF_LAG(kSwitchToSoundBank);
  switch_prg_bank(5);
}

// restore_current_bank (7:EAE3)
void restore_current_bank(void) {
  RUN_ORIGINAL_IF_LAG(kRestoreCurrentBank);
  switch_prg_bank(current_bank & 0x0F);
}

// pop_bank (7:EAEA): back to the bank push_and_switch_bank saved. It maps the whole saved byte,
// not just its low nibble. A, X and Y are kept.
void pop_bank(void) {
  Cpu6502 *c = &g_nes.cpu;
  RUN_ORIGINAL_IF_LAG(kPopBank);
  uint8_t a = c->a, x = c->x, y = c->y;
  uint8_t bank = stack_pop();
  current_bank = bank;
  switch_prg_bank(bank);
  c->y = y;
  c->x = x;
  emu_set_a(a);
}

// stack_push (7:EAFD): the bank stack at $0700 + stack_index, growing down. A, X, Y are kept.
void stack_push(uint8_t v) {
  emu_set_a(v);
  RUN_ORIGINAL_IF_LAG(kStackPush);
  sound_channels[stack_index] = v;
  stack_index--;
}

// stack_pop (7:EB13). X and Y are kept.
uint8_t stack_pop(void) {
  if(emu_lag_in(kStackPop)) {
    emu_call_original(kStackPop);
    return g_nes.cpu.a;
  }
  stack_index++;
  uint8_t v = sound_channels[stack_index];
  emu_set_a(v);
  return v;
}

// jump_table (7:EBE6): jumps through entry A of the .addr list after the JSR that called it,
// whose pushed return address (the JSR's last byte) is `pushed`. Leaves what the original
// leaves: $10/$11 the address, $12 = Y, A = $0B, the carry from ASL A. Returns the address.
uint16_t jump_table(uint16_t pushed, uint8_t index) {
  Cpu6502 *c = &g_nes.cpu;
  uint8_t y = (uint8_t)((index << 1) + 1);
  g_ram[0x12] = c->y;
  uint8_t lo = cpu_bus_read((uint16_t)(pushed + y));
  uint8_t hi = cpu_bus_read((uint16_t)(pushed + (uint8_t)(y + 1)));
  g_ram[0x10] = lo;
  g_ram[0x11] = hi;
  emu_set_flag(kFlagC, index & 0x80);
  emu_set_a(g_ram[0x0B]);
  return (uint16_t)(lo | hi << 8);
}

typedef void FarRoutine(void);

// The far-call tables that hold code, in ROM order: the rest of the slots are data. far_8000_65 and
// far_8400_25 are in their tables with nothing calling them, so they aren't ported.
// tools/check_ports.py checks these against the ROM.
static FarRoutine *const kFar6_8000[16] = {
  load_scene_palette, load_scene_chr_banks, load_palette, far_8000_63,
  far_8000_64, NULL, far_8000_66, far_8000_67,
  reset_cursor, handle_input, draw_cursor, dissolve_row_mask,
  far_8000_6C, far_8000_6D, far_8000_6E, far_8000_6F,
};
static FarRoutine *const kFar6_8020[16] = {
  add_object_sprites, select_room_scene, draw_room_tile_patches, script_slot_for_command,
  far_8020_64, draw_object_sprites, store_in_inventory_slot, far_8020_67,
  draw_inventory_entry, far_8020_69, draw_inventory_page, delete_inventory_entry,
  far_8020_6C, far_8020_6D, far_8020_6E, play_slot_machine,
};
static FarRoutine *const kFar6_8040[16] = {
  opening_logos, ending_sequence, item_number_for_object, draw_address_book,
  far_8040_64, highlight_entry,
};
static FarRoutine *const kFar2_8400[16] = {
  title_screen, title_screen, file_select_menu, quit_dialog,
  save_dialog,
};

// The C routine for entry `index` of `table`, with what is mapped there now, as the 6502 would find
// it. NULL for anything but the entries above.
static FarRoutine *far_routine(uint16_t table, uint8_t index) {
  uint32_t bank = nes_prg_offset(table) >> 14;
  switch(table) {
    case 0x8000: return bank == 6 ? kFar6_8000[index] : NULL;
    case 0x8020: return bank == 6 ? kFar6_8020[index] : NULL;
    case 0x8040: return bank == 6 ? kFar6_8040[index] : NULL;
    default: return bank == 2 ? kFar2_8400[index] : NULL;
  }
}

// far_call_8000 and its three siblings (7:F3B5 ...): calls entry (id & 15) of the table at
// $8000 (or $8020, $8040, $8400) in bank id >> 4 and comes back to the bank it was in. The
// routine gets A = $0B, X and Y as the caller had them; its A comes back in $0B only. X and Y
// are kept, and A = X on return (the end is PLA/TAY, PLA/TAX). An id with no routine, which takes a
// made-up state or a glitch, runs what the bytes there say as original code.
static void far_call(uint16_t entry, uint16_t body, uint16_t table, uint8_t id) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(id);
  if(emu_lag_in(entry) || emu_lag_in(body) || emu_lag_in(kFarCallJump)) {
    emu_call_original(entry);
    return;
  }
  uint8_t x = c->x, y = c->y;
  g_ram[0x76] = id;
  push_and_switch_bank(id >> 4);
  uint8_t index = (uint8_t)((id & 0x0F) << 1);
  g_ram[0x0A] = y;
  g_ram[0x10] = cpu_bus_read((uint16_t)(table + index));
  g_ram[0x11] = cpu_bus_read((uint16_t)(table + index + 1));
  // far_call_jump: lda $0B, ldy $0A, jmp ($10)
  c->x = x;
  emu_set_a(g_ram[0x0B]);
  c->y = g_ram[0x0A];
  emu_set_nz(c->y);
  FarRoutine *routine = far_routine(table, id & 0x0F);
  if(routine) {
    routine();
  } else {
    emu_call((uint16_t)(g_ram[0x10] | g_ram[0x11] << 8));
  }
  g_ram[0x0B] = c->a;
  pop_bank();
  c->y = y;
  c->x = x;
  emu_set_a(x);
  emu_lag_behind(entry);
  emu_lag_behind(body);
  emu_lag_behind(kFarCallJump);
}

void far_call_8000(uint8_t id) {
  far_call(kFarCall8000, kFarCall8000Body, 0x8000, id);
}

void far_call_8020(uint8_t id) {
  far_call(kFarCall8020, kFarCall8020Body, 0x8020, id);
}

void far_call_8040(uint8_t id) {
  far_call(kFarCall8040, kFarCall8040Body, 0x8040, id);
}

void far_call_8400(uint8_t id) {
  far_call(kFarCall8400, kFarCall8400Body, 0x8400, id);
}
