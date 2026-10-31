// The script interpreter (docs/scripts.md): the game's logic is bytecode
// in bank 3, which script_loop runs an opcode at a time. Choosing a script, fetching operands, the
// dispatch and the control flow are here, the flag opcodes in flags.c and the rest in opcodes.c.
//
// An opcode runs at the original's stack depth, as if script_run_opcode's JSR had called it, with
// that return address pushed.
#include "funcs.h"
#include "variables.h"

enum {
  kScriptAfterDispatch = 0xCBBF,  // script_run_opcode's JMP script_loop, after its JSR
  kScriptOpCount = 105,
};

// run_command (7:CB4B): the script for the command and target chosen, or the default response
// if the command has none for that target.
void run_command(void) {
  RUN_ORIGINAL_IF_LAG(kRunCommand);
  Cpu6502 *c = &g_nes.cpu;
  far_call_8020(0x63);  // script_slot_for_command
  RESUME_ORIGINAL_IF_LAG(kRunCommand, 0xCB50, 0);
  emu_set_a(script_slot);
  emu_cmp(c->a, 0xFF);
  if(c->a != 0xFF) {
    script_start();
    return;
  }
  far_call_8020(0x64);
  RESUME_ORIGINAL_IF_LAG(kRunCommand, 0xCB5B, 0);
}

// script_resume (7:CB5C): goes on with a script after a yield.
void script_resume(void) {
  RUN_ORIGINAL_IF_LAG(kScriptResume);
  push_and_switch_bank(0x03);
  script_loop();
}

// script_start (7:CB64): follows the three levels of tables in bank 3 (script_table, object_id,
// script_slot) to the script, and runs it (it runs into script_loop).
void script_start(void) {
  RUN_ORIGINAL_IF_LAG(kScriptStart);
  Cpu6502 *c = &g_nes.cpu;
  push_and_switch_bank(0x03);
  c->y = (uint8_t)((script_table & 0x7F) << 1);
  script_ptr[0] = cpu_bus_read((uint16_t)(kScriptTables + c->y));
  script_ptr[1] = cpu_bus_read((uint16_t)(kScriptTables + 1 + c->y));
  g_ram[0x65] = object_id >> 7;
  emu_set_flag(kFlagC, 0);
  script_ptr[0] = emu_adc((uint8_t)(object_id << 1), script_ptr[0]);
  script_ptr[1] = emu_adc(script_ptr[1], g_ram[0x65]);
  c->y = 0;
  g_ram[0x65] = (uint8_t)(script_slot << 1);
  emu_set_flag(kFlagC, 0);
  emu_push(emu_adc(emu_read_ind_y(0xB6, 0), g_ram[0x65]));
  c->y = 1;
  script_ptr[1] = emu_adc(emu_read_ind_y(0xB6, 1), 0x00);
  script_ptr[0] = emu_pull();
  c->y = 0;
  emu_push(emu_read_ind_y(0xB6, 0));
  c->y = 1;
  script_ptr[1] = emu_read_ind_y(0xB6, 1);
  script_ptr[0] = emu_pull();
  emu_set_a(script_ptr[0]);
  script_loop();
}

static void (*const kOpcodes[kScriptOpCount])(void) = {
  [0x00] = op_00_message_5,
  [0x01] = op_01_message_4,
  [0x02] = op_02_message_2_or_5,
  [0x03] = script_op_goto_if,
  [0x04] = script_op_goto_if,
  [0x05] = script_op_goto_if_not,
  [0x06] = op_06,
  [0x07] = op_07_test_story_flag_of_object,
  [0x08] = op_08_set_story_flag_of_object,
  [0x09] = op_09_clear_story_flag_of_object,
  [0x0A] = op_0A,
  [0x0B] = op_0B,
  [0x0C] = op_0C,
  [0x0D] = op_0D,
  [0x0E] = op_0E,
  [0x0F] = op_0F,
  [0x10] = op_10,
  [0x11] = op_11,
  [0x12] = op_12,
  [0x13] = op_13_test_door_flag_of_object,
  [0x14] = op_14_set_door_flag_of_object,
  [0x15] = op_15_clear_door_flag_of_object,
  [0x16] = op_16_test_event_flag,
  [0x17] = op_17_set_event_flag,
  [0x18] = op_18_clear_event_flag,
  [0x19] = op_19,
  [0x1A] = op_1A,
  [0x1B] = op_1B,
  [0x1C] = script_op_goto,
  [0x1D] = op_1D,
  [0x1E] = op_1E,
  [0x1F] = op_1F,
  [0x20] = op_20_test_flag_045C_of_object,
  [0x21] = op_21_set_flag_045C_of_object,
  [0x22] = op_22_clear_flag_045C_of_object,
  [0x23] = op_23_test_flag_047A_of_object,
  [0x24] = op_24_set_flag_047A_of_object,
  [0x25] = op_25_clear_flag_047A_of_object,
  [0x26] = op_26,
  [0x27] = op_27,
  [0x28] = script_op_yield,
  [0x29] = script_op_switch,
  [0x2A] = op_2A,
  [0x2B] = op_2B,
  [0x2C] = op_2C,
  [0x2D] = op_2D,
  [0x2E] = op_2E_test_door_flag,
  [0x2F] = op_2F_set_door_flag,
  [0x30] = op_30_clear_door_flag,
  [0x31] = op_31,
  [0x32] = op_32,
  [0x33] = op_33,
  [0x34] = op_34,
  [0x35] = op_35,
  [0x36] = op_36,
  [0x37] = op_37,
  [0x38] = op_38,
  [0x39] = op_39,
  [0x3A] = op_3A,
  [0x3B] = op_3B,
  [0x3C] = op_3C,
  [0x3D] = op_3D,
  [0x3E] = op_3E,
  [0x3F] = op_3F,
  [0x40] = op_40,
  [0x41] = op_41,
  [0x42] = op_42,
  [0x43] = op_43,
  [0x44] = op_44,
  [0x45] = op_45,
  [0x46] = op_46_special_scene,
  [0x47] = op_47,
  [0x48] = op_48_yield_b5_80,
  [0x49] = op_49,
  [0x4A] = op_4A,
  [0x4B] = op_4B,
  [0x4C] = op_4C,
  [0x4D] = op_4D,
  [0x4E] = op_4E,
  [0x4F] = op_4F,
  [0x50] = op_50,
  [0x51] = op_51,
  [0x52] = op_52_test_story_flag,
  [0x53] = op_53_set_flag_045C,
  [0x54] = op_54_set_flag_047A,
  [0x55] = op_55,
  [0x56] = op_56,
  [0x57] = op_57,
  [0x58] = op_58,
  [0x59] = op_59,
  [0x5A] = op_5A,
  [0x5B] = op_5B,
  [0x5C] = op_5C,
  [0x5D] = op_5D,
  [0x5E] = op_5E,
  [0x5F] = op_5F,
  [0x60] = op_60,
  [0x61] = op_61,
  [0x62] = op_62,
  [0x63] = op_63,
  [0x64] = op_64,
  [0x65] = script_op_push_goto,
  [0x66] = script_op_pop_goto,
  [0x67] = op_67,
  [0x68] = op_68,
};

// Runs opcode `op`, whose handler is at `target`. Returns 1 if it ended the script: such a
// handler has taken the dispatch's return address off the stack (end_script). A lag NMI in a
// handler's own code goes through emu_call_may_unwind, as those that end the script can't run
// with emu_call_original, whose return address their PLAs would take.
static int run_opcode(uint8_t op, uint16_t target) {
  Cpu6502 *c = &g_nes.cpu;
  if(emu_lag_in(target)) {
    return emu_call_may_unwind(target, kScriptAfterDispatch);
  }
  uint8_t s0 = c->s;
  emu_push((kScriptAfterDispatch - 1) >> 8);
  emu_push((kScriptAfterDispatch - 1) & 0xFF);
  kOpcodes[op]();
  if(c->s == s0) {
    return 1;
  }
  emu_pull();
  emu_pull();
  return 0;
}

// script_loop (7:CBB1): runs opcodes until $FF or a yield, then pops the bank script_start or
// script_resume pushed. It stands for script_end (7:CBB8) and script_run_opcode (7:CBBC) too;
// script_dispatch (7:CC17) is the JSR jump_table with the opcode table after it.
void script_loop(void) {
  RUN_ORIGINAL_IF_LAG(kScriptLoop);
  Cpu6502 *c = &g_nes.cpu;
  for(;;) {
    script_fetch_byte();
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      pop_bank();
      return;
    }
    uint8_t op = c->a;
    if(op >= kScriptOpCount) {
      emu_note_bad_index();
    }
    if(op >= kScriptOpCount || emu_lag_in(kScriptDispatch) || emu_lag_in(kJumpTable)) {
      emu_resume_original(kScriptRunOpcode, 0);
      return;
    }
    uint16_t target = jump_table(kScriptDispatch + 2, op);
    if(run_opcode(op, target)) {
      return;
    }
    RESUME_ORIGINAL_IF_LAG(kScriptLoop, 0xCBBF, 0);
  }
}

// script_fetch_byte (7:CBC2): the next script byte into script_args and A (it runs into
// script_advance). Y = 0.
void script_fetch_byte(void) {
  RUN_ORIGINAL_IF_LAG(kScriptFetchByte);
  g_nes.cpu.y = 0;
  script_args[0] = emu_read_ind_y(0xB6, 0);
  emu_set_a(0x01);
  script_advance();
}

// script_advance (7:CBCA): script_ptr += A. Returns A = script_args[0], with the carry from the
// high byte's ADC.
void script_advance(void) {
  RUN_ORIGINAL_IF_LAG(kScriptAdvance);
  emu_set_flag(kFlagC, 0);
  script_ptr[0] = emu_adc(g_nes.cpu.a, script_ptr[0]);
  script_ptr[1] = emu_adc(script_ptr[1], 0x00);
  emu_set_a(script_args[0]);
}

// The next n script bytes into script_args, as script_fetch_2 and its siblings do. Y = n - 1.
static void fetch_operands(int n) {
  Cpu6502 *c = &g_nes.cpu;
  for(int i = 0; i < n; i++) {
    c->y = (uint8_t)i;
    script_args[i] = emu_read_ind_y(0xB6, (uint8_t)i);
  }
  emu_set_a((uint8_t)n);
  script_advance();
}

// script_fetch_2 (7:CBD8), script_fetch_4 (7:CBFD)
void script_fetch_2(void) {
  RUN_ORIGINAL_IF_LAG(kScriptFetch2);
  fetch_operands(2);
}

void script_fetch_4(void) {
  RUN_ORIGINAL_IF_LAG(kScriptFetch4);
  fetch_operands(4);
}

// ---- Control flow --------------------------------------------------------------------------

// script_op_switch (7:CDE0, opcode $29): cases of 4 bytes: the two compared with switch_key (a
// first byte of $FE matches anything and ends the list), then the script address to go on at.
void script_op_switch(void) {
  RUN_ORIGINAL_IF_LAG(kScriptOpSwitch);
  Cpu6502 *c = &g_nes.cpu;
  for(;;) {
    script_fetch_4();
    emu_set_a(script_args[0]);
    emu_cmp(c->a, 0xFE);
    if(c->a == 0xFE) {
      break;
    }
    emu_cmp(c->a, switch_key[0]);
    if(c->a != switch_key[0]) {
      continue;
    }
    emu_set_a(script_args[1]);
    emu_cmp(c->a, switch_key[1]);
    if(c->a == switch_key[1]) {
      break;
    }
  }
  script_ptr[0] = script_args[2];
  script_ptr[1] = script_args[3];
  clear_event_flag(0x80);
}

// PLA, PLA, JMP script_end, how an opcode ends the script: drops the return address of the
// dispatch, and pops the bank. script_end's RTS then returns from the whole script.
void end_script(void) {
  emu_pull();
  emu_set_a(emu_pull());
  pop_bank();
}

// script_op_yield (7:CE01, opcode $28): the script waits for the player to choose something
// (event flag $80), and goes on from the next opcode with script_resume.
void script_op_yield(void) {
  set_event_flag(0x80);
  end_script();
}

// op_48_yield_b5_80 (7:D8A0): stops the script, with bit 7 of script_flags set for the input loop.
void op_48_yield_b5_80(void) {
  emu_set_a(script_flags | 0x80);
  script_flags = g_nes.cpu.a;
  end_script();
}

// op_00_message_5 (7:CF87), op_01_message_4 (7:CF8E), op_02_message_2_or_5 (7:CF95): a message
// from the table each names, by the operand.
void op_00_message_5(void) {
  RUN_ORIGINAL_IF_LAG(kOp00Message5);
  script_fetch_byte();
  print_message_5(g_nes.cpu.a);
  RESUME_ORIGINAL_IF_LAG(kOp00Message5, 0xCF8D, 0);
}

void op_01_message_4(void) {
  RUN_ORIGINAL_IF_LAG(kOp01Message4);
  script_fetch_byte();
  print_message_4(g_nes.cpu.a);
  RESUME_ORIGINAL_IF_LAG(kOp01Message4, 0xCF94, 0);
}

void op_02_message_2_or_5(void) {
  RUN_ORIGINAL_IF_LAG(kOp02Message2Or5);
  script_fetch_byte();
  print_message_2_or_5();
  RESUME_ORIGINAL_IF_LAG(kOp02Message2Or5, 0xCF9B, 0);
}

// script_op_goto_if (7:CF9C, opcodes $03, $04), script_op_goto_if_not (7:CFA6, opcode $05)
void script_op_goto_if(void) {
  RUN_ORIGINAL_IF_LAG(kScriptOpGotoIf);
  script_fetch_2();
  emu_set_a(script_flags & 0x04);
  if(g_nes.cpu.a) {
    script_goto();
  }
}

void script_op_goto_if_not(void) {
  RUN_ORIGINAL_IF_LAG(kScriptOpGotoIfNot);
  script_fetch_2();
  emu_set_a(script_flags & 0x04);
  if(!g_nes.cpu.a) {
    script_goto();
  }
}

// script_goto (7:CFB0): the script goes on at script_args. Returns A = the high byte.
void script_goto(void) {
  RUN_ORIGINAL_IF_LAG(kScriptGoto);
  script_ptr[0] = script_args[0];
  emu_set_a(script_args[1]);
  script_ptr[1] = script_args[1];
}

// script_op_push_goto (7:CFB9, opcode $65), script_op_pop_goto (7:CFCA, opcode $66): goto with the
// target pushed on the bank stack, and goto a target popped from it, which make loops.
void script_op_push_goto(void) {
  RUN_ORIGINAL_IF_LAG(kScriptOpPushGoto);
  script_fetch_2();
  stack_push(script_args[0]);
  stack_push(script_args[1]);
  script_goto();
}

void script_op_pop_goto(void) {
  RUN_ORIGINAL_IF_LAG(kScriptOpPopGoto);
  script_args[1] = stack_pop();
  script_args[0] = stack_pop();
  script_goto();
}

// script_op_goto (7:CFD8, opcode $1C)
void script_op_goto(void) {
  RUN_ORIGINAL_IF_LAG(kScriptOpGoto);
  script_fetch_2();
  script_goto();
}

// script_condition_true (7:CFDF), script_condition_false (7:CFE6): bit 2 of script_flags, what the
// test opcodes leave and goto_if tests.
void script_condition_true(void) {
  RUN_ORIGINAL_IF_LAG(kScriptConditionTrue);
  emu_set_a(script_flags | 0x04);
  script_flags = g_nes.cpu.a;
}

void script_condition_false(void) {
  RUN_ORIGINAL_IF_LAG(kScriptConditionFalse);
  emu_set_a(script_flags & 0xFB);
  script_flags = g_nes.cpu.a;
}
