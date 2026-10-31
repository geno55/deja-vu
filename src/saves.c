// The save system: three files in save RAM, each 4 copies of the game's
// state, and one more copy, the backup, that main, op_06 and op_5E write.
//
// The state is 272 bytes, $0420-$052F. Each copy, $200 bytes at $6010 + file * $800 + copy * $200,
// has the state, the signature "DEJAVU" at +$120 and the 16-bit sum of the state at +$126. The
// backup is at $7E00 with its sum at $7F36.
//
// The scratch bytes, as the originals use them: $64 which copy, data_ptr the copy, $57/$58 its
// signature, $59/$5A its sum, $5B/$5C the next copy, $65/$66 the sum so far.
//
// These take long enough for NMIs to land anywhere in them, even with no wait for one in sight, so
// they check whether to go on as original code after every call, not just after the waits.
#include "funcs.h"
#include "variables.h"

// ---- Whole files ---------------------------------------------------------------------------

// load_game (7:C800): the first good copy of the file, moving the bad ones out of the way; with
// none, a new game. Then saves it, to all 4 copies. Keeps X and Y; returns A = X.
void load_game(void) {
  RUN_ORIGINAL_IF_LAG(kLoadGame);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->x);
  emu_push(c->x);
  emu_set_a(c->y);
  emu_push(c->y);
  protect_save_ram();
  RESUME_ORIGINAL_IF_LAG(kLoadGame, 0xC807, 2);
  c->x = 0;
  for(;;) {
    emu_set_a(0x00);
    g_ram[0x64] = 0x00;
    save_copy_address();
    RESUME_ORIGINAL_IF_LAG(kLoadGame, 0xC810, 2);
    read_save_copy();
    RESUME_ORIGINAL_IF_LAG(kLoadGame, 0xC813, 2);
    check_signature();
    RESUME_ORIGINAL_IF_LAG(kLoadGame, 0xC816, 2);
    if(!c->a) {
      check_checksum();
      RESUME_ORIGINAL_IF_LAG(kLoadGame, 0xC81B, 2);
      if(!c->a) {
        break;
      }
    }
    shift_save_copies();
    RESUME_ORIGINAL_IF_LAG(kLoadGame, 0xC820, 2);
    c->x++;
    emu_cmp(c->x, 0x04);
    if(c->x == 0x04) {
      reset_game_state();
      RESUME_ORIGINAL_IF_LAG(kLoadGame, 0xC828, 2);
      break;
    }
  }
  save_game();
  RESUME_ORIGINAL_IF_LAG(kLoadGame, 0xC82B, 2);
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(c->x);
}

// save_game (7:C858): to all 4 copies of the file. Keeps X and Y; returns A = X.
void save_game(void) {
  RUN_ORIGINAL_IF_LAG(kSaveGame);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->x);
  emu_push(c->x);
  emu_set_a(c->y);
  emu_push(c->y);
  unlock_save_ram();
  RESUME_ORIGINAL_IF_LAG(kSaveGame, 0xC85F, 2);
  emu_set_a(0x00);
  g_ram[0x64] = 0x00;
  do {
    save_copy_address();
    RESUME_ORIGINAL_IF_LAG(kSaveGame, 0xC866, 2);
    write_save_copy();
    RESUME_ORIGINAL_IF_LAG(kSaveGame, 0xC869, 2);
    g_ram[0x64]++;
    emu_set_a(g_ram[0x64]);
    emu_cmp(c->a, 0x04);
  } while(c->a != 0x04);
  disable_save_ram();
  RESUME_ORIGINAL_IF_LAG(kSaveGame, 0xC874, 2);
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(c->x);
}

// erase_save_file (7:C89F)
void erase_save_file(void) {
  RUN_ORIGINAL_IF_LAG(kEraseSaveFile);
  unlock_save_ram();
  RESUME_ORIGINAL_IF_LAG(kEraseSaveFile, 0xC8A2, 0);
  erase_save_copies();
  RESUME_ORIGINAL_IF_LAG(kEraseSaveFile, 0xC8A5, 0);
  disable_save_ram();
  RESUME_ORIGINAL_IF_LAG(kEraseSaveFile, 0xC8A8, 0);
}

// check_all_save_files (7:C8A9): scene_index = a bit for each of the 3 files that is good. Bad
// ones are erased. Keeps save_file.
void check_all_save_files(void) {
  RUN_ORIGINAL_IF_LAG(kCheckAllSaveFiles);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(save_file);
  emu_push(c->a);
  unlock_save_ram();
  RESUME_ORIGINAL_IF_LAG(kCheckAllSaveFiles, 0xC8AF, 1);
  emu_set_a(0x00);
  save_file = 0x00;
  scene_index = 0x00;
  check_save_file();
  RESUME_ORIGINAL_IF_LAG(kCheckAllSaveFiles, 0xC8B8, 1);
  save_file++;
  check_save_file();
  RESUME_ORIGINAL_IF_LAG(kCheckAllSaveFiles, 0xC8BD, 1);
  save_file++;
  check_save_file();
  RESUME_ORIGINAL_IF_LAG(kCheckAllSaveFiles, 0xC8C2, 1);
  disable_save_ram();
  RESUME_ORIGINAL_IF_LAG(kCheckAllSaveFiles, 0xC8C5, 1);
  emu_set_a(emu_pull());
  save_file = c->a;
}

// check_save_file (7:C8C9): the first good copy goes to all 4 (repair_save_copies), and the file's
// bit into scene_index; with none, the copies are erased. The sum isn't cleared between copies, so
// a copy after a bad one only passes if its sum takes the bad ones' into account.
void check_save_file(void) {
  RUN_ORIGINAL_IF_LAG(kCheckSaveFile);
  Cpu6502 *c = &g_nes.cpu;
  clear_sum16();
  emu_set_a(0x00);
  g_ram[0x64] = 0x00;
  for(;;) {
    save_copy_address();
    RESUME_ORIGINAL_IF_LAG(kCheckSaveFile, 0xC8D3, 0);
    c->y = 0;
    do {
      c->a = emu_read_ind_y(0x55, c->y);
      add_to_sum16(c->a);
      RESUME_ORIGINAL_IF_LAG(kCheckSaveFile, 0xC8DA, 0);
      c->y++;
      emu_set_nz(c->y);
    } while(c->y);
    data_ptr[1]++;
    do {
      c->a = emu_read_ind_y(0x55, c->y);
      add_to_sum16(c->a);
      RESUME_ORIGINAL_IF_LAG(kCheckSaveFile, 0xC8E4, 0);
      c->y++;
      emu_cmp(c->y, 0x10);
    } while(c->y != 0x10);
    check_signature();
    RESUME_ORIGINAL_IF_LAG(kCheckSaveFile, 0xC8EC, 0);
    if(!c->a) {
      check_checksum();
      RESUME_ORIGINAL_IF_LAG(kCheckSaveFile, 0xC8F1, 0);
      if(!c->a) {
        break;
      }
    }
    g_ram[0x64]++;
    emu_set_a(g_ram[0x64]);
    emu_cmp(c->a, 0x04);
    if(c->a == 0x04) {
      erase_save_copies();
      return;
    }
  }
  repair_save_copies();
  RESUME_ORIGINAL_IF_LAG(kCheckSaveFile, 0xC902, 0);
  c->y = save_file;
  emu_set_a(scene_index | cpu_bus_read((uint16_t)(kSaveFileBits + c->y)));
  scene_index = c->a;
}

// repair_save_copies (7:C90F): does nothing if copy 0 is the good one. Otherwise it looks meant to
// copy the good one to all 4 through the backup's place, but never finishes: the BNE at 7:C928 goes
// back to the first copy loop, with Y = 1, instead of on through the second page. So it reads on
// through all of memory for good, and 7:C92A on never runs. NMIs still come, and that is all the
// game does from then on. (This is what the code says: no movie gets here.)
void repair_save_copies(void) {
  RUN_ORIGINAL_IF_LAG(kRepairSaveCopies);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(g_ram[0x64]);
  if(!c->a) {
    return;
  }
  save_copy_address();
  c->y = 0;
  for(;;) {
    TAIL_ORIGINAL_IF_LAG(kRepairSaveCopies, 0xC918, 0);
    emu_count_work(3840);  // what a pass of the original takes, in cycles
    do {
      emu_set_a(emu_read_ind_y(0x55, c->y));
      cpu_bus_write((uint16_t)(0x7E00 + c->y), c->a);
      c->y++;
      emu_set_nz(c->y);
    } while(c->y);
    data_ptr[1]++;
    emu_set_a(emu_read_ind_y(0x55, c->y));
    cpu_bus_write((uint16_t)(0x7F00 + c->y), c->a);
    c->y++;
    emu_set_nz(c->y);
  }
}

// ---- One copy ------------------------------------------------------------------------------

// read_save_copy (7:C830): the state from the copy at data_ptr, and its sum.
void read_save_copy(void) {
  RUN_ORIGINAL_IF_LAG(kReadSaveCopy);
  Cpu6502 *c = &g_nes.cpu;
  clear_sum16();
  emu_set_a(data_ptr[0]);
  flag_array_ptr[0] = data_ptr[0];
  emu_set_a(data_ptr[1]);
  flag_array_ptr[1] = data_ptr[1];
  c->y = 0;
  do {
    c->a = emu_read_ind_y(0x61, c->y);
    object_state[c->y] = c->a;
    add_to_sum16(c->a);
    RESUME_ORIGINAL_IF_LAG(kReadSaveCopy, 0xC845, 0);
    c->y++;
    emu_set_nz(c->y);
  } while(c->y);
  flag_array_ptr[1]++;
  do {
    c->a = emu_read_ind_y(0x61, c->y);
    container_stack[1 + c->y] = c->a;
    add_to_sum16(c->a);
    RESUME_ORIGINAL_IF_LAG(kReadSaveCopy, 0xC852, 0);
    c->y++;
    emu_cmp(c->y, 0x10);
  } while(c->y != 0x10);
}

// write_save_copy (7:C879): the state to the copy at data_ptr, with its sum and signature.
void write_save_copy(void) {
  RUN_ORIGINAL_IF_LAG(kWriteSaveCopy);
  Cpu6502 *c = &g_nes.cpu;
  clear_sum16();
  c->y = 0;
  do {
    c->a = object_state[c->y];
    emu_write_ind_y(0x55, c->y, c->a);
    add_to_sum16(c->a);
    RESUME_ORIGINAL_IF_LAG(kWriteSaveCopy, 0xC886, 0);
    c->y++;
    emu_set_nz(c->y);
  } while(c->y);
  data_ptr[1]++;
  do {
    c->a = container_stack[1 + c->y];
    emu_write_ind_y(0x55, c->y, c->a);
    add_to_sum16(c->a);
    RESUME_ORIGINAL_IF_LAG(kWriteSaveCopy, 0xC893, 0);
    c->y++;
    emu_cmp(c->y, 0x10);
  } while(c->y != 0x10);
  store_checksum();
  store_signature();
}

// save_copy_address (7:C972): data_ptr, $57/$58 and $59/$5A for copy $64 of the file.
void save_copy_address(void) {
  RUN_ORIGINAL_IF_LAG(kSaveCopyAddress);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t a = asl_a(asl_a(asl_a(save_file)));
  emu_set_flag(kFlagC, 0);
  data_ptr[1] = emu_adc(a, 0x60);
  data_ptr[0] = 0x10;
  a = asl_a(g_ram[0x64]);
  emu_set_flag(kFlagC, 0);
  data_ptr[1] = emu_adc(a, data_ptr[1]);
  emu_set_flag(kFlagC, 0);
  g_ram[0x57] = emu_adc(data_ptr[0], 0x20);
  g_ram[0x58] = emu_adc(data_ptr[1], 0x01);
  emu_set_flag(kFlagC, 0);
  g_ram[0x59] = emu_adc(data_ptr[0], 0x26);
  g_ram[0x5A] = emu_adc(data_ptr[1], 0x01);
  c->a = g_ram[0x5A];
}

// shift_save_copies (7:C9A3): each copy from the next one, and the last erased.
void shift_save_copies(void) {
  RUN_ORIGINAL_IF_LAG(kShiftSaveCopies);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(0x00);
  g_ram[0x64] = 0x00;
  do {
    save_copy_address();
    RESUME_ORIGINAL_IF_LAG(kShiftSaveCopies, 0xC9AA, 0);
    next_copy_pointer();
    copy_save_copy();
    RESUME_ORIGINAL_IF_LAG(kShiftSaveCopies, 0xC9B0, 0);
    g_ram[0x64]++;
    emu_set_a(g_ram[0x64]);
    emu_cmp(c->a, 0x04);
  } while(c->a != 0x04);
  emu_set_a(0x03);
  g_ram[0x64] = 0x03;
  erase_save_copy();
}

// next_copy_pointer (7:C9C0): $5B/$5C = the copy after data_ptr.
void next_copy_pointer(void) {
  RUN_ORIGINAL_IF_LAG(kNextCopyPointer);
  g_ram[0x5B] = data_ptr[0];
  emu_set_flag(kFlagC, 0);
  g_ram[0x5C] = emu_adc(data_ptr[1], 0x02);
  g_nes.cpu.a = g_ram[0x5C];
}

// copy_save_copy (7:C9CC): $200 bytes from $5B/$5C to data_ptr.
void copy_save_copy(void) {
  RUN_ORIGINAL_IF_LAG(kCopySaveCopy);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  do {
    emu_set_a(emu_read_ind_y(0x5B, c->y));
    emu_write_ind_y(0x55, c->y, c->a);
    c->y++;
    emu_set_nz(c->y);
  } while(c->y);
  g_ram[0x5C]++;
  data_ptr[1]++;
  do {
    emu_set_a(emu_read_ind_y(0x5B, c->y));
    emu_write_ind_y(0x55, c->y, c->a);
    c->y++;
    emu_set_nz(c->y);
  } while(c->y);
}

// erase_save_copies (7:C9E1)
void erase_save_copies(void) {
  RUN_ORIGINAL_IF_LAG(kEraseSaveCopies);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(0x00);
  g_ram[0x64] = 0x00;
  do {
    erase_save_copy();
    RESUME_ORIGINAL_IF_LAG(kEraseSaveCopies, 0xC9E8, 0);
    g_ram[0x64]++;
    emu_set_a(g_ram[0x64]);
    emu_cmp(c->a, 0x04);
  } while(c->a != 0x04);
}

// erase_save_copy (7:C9F1): $200 zeros at copy $64.
void erase_save_copy(void) {
  RUN_ORIGINAL_IF_LAG(kEraseSaveCopy);
  Cpu6502 *c = &g_nes.cpu;
  save_copy_address();
  g_ram[0x57] = data_ptr[0];
  emu_set_flag(kFlagC, 0);
  g_ram[0x58] = emu_adc(data_ptr[1], 0x01);
  c->y = 0;
  emu_set_a(0x00);
  do {
    emu_write_ind_y(0x55, c->y, 0x00);
    emu_write_ind_y(0x57, c->y, 0x00);
    c->y++;
    emu_set_nz(c->y);
  } while(c->y);
}

// ---- The sum and the signature -------------------------------------------------------------

// clear_sum16 (7:CA0B)
void clear_sum16(void) {
  RUN_ORIGINAL_IF_LAG(kClearSum16);
  emu_set_a(0x00);
  g_ram[0x65] = 0x00;
  g_ram[0x66] = 0x00;
}

// add_to_sum16 (7:CA12): returns A = the sum's high byte.
void add_to_sum16(uint8_t v) {
  emu_set_a(v);
  RUN_ORIGINAL_IF_LAG(kAddToSum16);
  emu_set_flag(kFlagC, 0);
  g_ram[0x65] = emu_adc(v, g_ram[0x65]);
  g_ram[0x66] = emu_adc(0x00, g_ram[0x66]);
  g_nes.cpu.a = g_ram[0x66];
}

// store_checksum (7:CA1E)
void store_checksum(void) {
  RUN_ORIGINAL_IF_LAG(kStoreChecksum);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  emu_set_a(g_ram[0x65]);
  emu_write_ind_y(0x59, 0, c->a);
  c->y = 1;
  emu_set_a(g_ram[0x66]);
  emu_write_ind_y(0x59, 1, c->a);
}

// check_checksum (7:CA2A): returns A = 0 if the copy's sum is $65/$66, otherwise $FF.
void check_checksum(void) {
  RUN_ORIGINAL_IF_LAG(kCheckChecksum);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  emu_set_a(emu_read_ind_y(0x59, 0));
  emu_cmp(c->a, g_ram[0x65]);
  if(c->a != g_ram[0x65]) {
    emu_set_a(0xFF);
    return;
  }
  c->y = 1;
  emu_set_a(emu_read_ind_y(0x59, 1));
  emu_cmp(c->a, g_ram[0x66]);
  emu_set_a(c->a == g_ram[0x66] ? 0x00 : 0xFF);
}

// check_signature (7:CA3F): returns A = 0 if the copy has it, otherwise $FF.
void check_signature(void) {
  RUN_ORIGINAL_IF_LAG(kCheckSignature);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  do {
    emu_set_a(emu_read_ind_y(0x57, c->y));
    uint8_t expected = cpu_bus_read((uint16_t)(kSaveSignature + c->y));
    emu_cmp(c->a, expected);
    if(c->a != expected) {
      emu_set_a(0xFF);
      return;
    }
    c->y++;
    emu_cmp(c->y, 0x06);
  } while(c->y != 0x06);
  emu_set_a(0x00);
}

// store_signature (7:CA59)
void store_signature(void) {
  RUN_ORIGINAL_IF_LAG(kStoreSignature);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  do {
    emu_set_a(cpu_bus_read((uint16_t)(kSaveSignature + c->y)));
    emu_write_ind_y(0x57, c->y, c->a);
    c->y++;
    emu_cmp(c->y, 0x06);
  } while(c->y != 0x06);
}

// ---- Save RAM on and off, and the game's state ---------------------------------------------

// unlock_save_ram (7:C94E), protect_save_ram (7:C95A), disable_save_ram (7:C966): the MMC3's save
// RAM setting, between two frames: on, read-only, off.
void unlock_save_ram(void) {
  RUN_ORIGINAL_IF_LAG(kUnlockSaveRam);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kUnlockSaveRam, 0xC951, 0);
  emu_set_a(0x80);
  cpu_bus_write(0xA001, 0x80);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kUnlockSaveRam, 0xC959, 0);
}

void protect_save_ram(void) {
  RUN_ORIGINAL_IF_LAG(kProtectSaveRam);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kProtectSaveRam, 0xC95D, 0);
  emu_set_a(0xC0);
  cpu_bus_write(0xA001, 0xC0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kProtectSaveRam, 0xC965, 0);
}

void disable_save_ram(void) {
  RUN_ORIGINAL_IF_LAG(kDisableSaveRam);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kDisableSaveRam, 0xC969, 0);
  emu_set_a(0x40);
  cpu_bus_write(0xA001, 0x40);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kDisableSaveRam, 0xC971, 0);
}

// reset_game_state (7:CA66): the state zero, and a new game (which zeroes it again).
void reset_game_state(void) {
  RUN_ORIGINAL_IF_LAG(kResetGameState);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(0x00);
  c->y = 0;
  do {
    object_state[c->y] = 0x00;
    c->y++;
    emu_set_nz(c->y);
  } while(c->y);
  do {
    container_stack[1 + c->y] = 0x00;
    c->y++;
    emu_cmp(c->y, 0x10);
  } while(c->y != 0x10);
  new_game();
  RESUME_ORIGINAL_IF_LAG(kResetGameState, 0xCA7B, 0);
}

// backup_state_to_save_ram (7:CA7C): the state to the backup, unless event flag $B0 is set, which
// it clears then. Keeps X and Y; returns A = X.
void backup_state_to_save_ram(void) {
  RUN_ORIGINAL_IF_LAG(kBackupStateToSaveRam);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->x);
  emu_push(c->x);
  emu_set_a(c->y);
  emu_push(c->y);
  test_event_flag(0xB0);
  emu_set_a(script_flags & 0x04);
  if(c->a) {
    clear_event_flag(0xB0);
  } else {
    unlock_save_ram();
    RESUME_ORIGINAL_IF_LAG(kBackupStateToSaveRam, 0xCA8E, 2);
    clear_sum16();
    c->y = 0;
    do {
      c->a = object_state[c->y];
      cpu_bus_write((uint16_t)(0x7E00 + c->y), c->a);
      add_to_sum16(c->a);
      RESUME_ORIGINAL_IF_LAG(kBackupStateToSaveRam, 0xCA9C, 2);
      c->y++;
      emu_set_nz(c->y);
    } while(c->y);
    do {
      c->a = container_stack[1 + c->y];
      cpu_bus_write((uint16_t)(0x7F00 + c->y), c->a);
      add_to_sum16(c->a);
      RESUME_ORIGINAL_IF_LAG(kBackupStateToSaveRam, 0xCAA8, 2);
      c->y++;
      emu_cmp(c->y, 0x10);
    } while(c->y != 0x10);
    emu_set_a(g_ram[0x65]);
    cpu_bus_write(0x7F36, c->a);
    emu_set_a(g_ram[0x66]);
    cpu_bus_write(0x7F37, c->a);
    disable_save_ram();
    RESUME_ORIGINAL_IF_LAG(kBackupStateToSaveRam, 0xCABA, 2);
  }
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(c->x);
}

// restore_state_backup (7:CAC7): the state from the backup; if its sum is wrong, load_game. Leaves
// save RAM read-only. Keeps X and Y; returns A = X.
void restore_state_backup(void) {
  RUN_ORIGINAL_IF_LAG(kRestoreStateBackup);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->x);
  emu_push(c->x);
  emu_set_a(c->y);
  emu_push(c->y);
  protect_save_ram();
  RESUME_ORIGINAL_IF_LAG(kRestoreStateBackup, 0xCACE, 2);
  clear_sum16();
  c->y = 0;
  do {
    c->a = cpu_bus_read((uint16_t)(0x7E00 + c->y));
    object_state[c->y] = c->a;
    add_to_sum16(c->a);
    RESUME_ORIGINAL_IF_LAG(kRestoreStateBackup, 0xCADC, 2);
    c->y++;
    emu_set_nz(c->y);
  } while(c->y);
  do {
    c->a = cpu_bus_read((uint16_t)(0x7F00 + c->y));
    container_stack[1 + c->y] = c->a;
    add_to_sum16(c->a);
    RESUME_ORIGINAL_IF_LAG(kRestoreStateBackup, 0xCAE8, 2);
    c->y++;
    emu_cmp(c->y, 0x10);
  } while(c->y != 0x10);
  emu_set_a(cpu_bus_read(0x7F36));
  emu_cmp(c->a, g_ram[0x65]);
  int good = c->a == g_ram[0x65];
  if(good) {
    emu_set_a(cpu_bus_read(0x7F37));
    emu_cmp(c->a, g_ram[0x66]);
    good = c->a == g_ram[0x66];
  }
  if(!good) {
    load_game();
    RESUME_ORIGINAL_IF_LAG(kRestoreStateBackup, 0xCB03, 2);
  }
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(c->x);
}
