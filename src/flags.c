// The flag opcodes (docs/scripts.md, "Flags"). Five bit arrays, each with
// routines to test (the script condition, bit 2 of script_flags), set and clear a bit, and opcodes
// that take the bit from object_id or the operand: the story flags ($0450), door flags ($0480),
// event flags ($0490), and two more ($045C, $047A). And the state bytes: item_status (bit 0
// obtained, bit 1 open) and object_state (bit 0 open).
//
// flag_array_ptr ($61) is the array and script_args[0] the bit, as the originals use them.
#include "funcs.h"
#include "variables.h"

// The array selectors, select_story_flags and its siblings: A = 4 on the way out.
static void select_array(uint8_t bit, uint8_t lo) {
  script_args[0] = bit;
  flag_array_ptr[0] = lo;
  emu_set_a(0x04);
  flag_array_ptr[1] = 0x04;
}

// STA (flag_array_ptr),Y
static void store_flag_byte(uint8_t v) {
  emu_set_a(v);
  cpu_bus_write((uint16_t)((flag_array_ptr[0] | flag_array_ptr[1] << 8) + g_nes.cpu.y), v);
}

static uint8_t bit_mask(void) {
  return cpu_bus_read((uint16_t)(kBitMasks + g_nes.cpu.x));
}

static uint8_t bit_clear_mask(void) {
  return cpu_bus_read((uint16_t)(kBitClearMasks + g_nes.cpu.x));
}

// The middle of set_story_flag_refresh, set_door_flag and set_event_flag: sets the bit, keeping X
// and Y. Returns A = Y. With `clear`, of the clear_ routines.
static void change_flag_keeping_xy(int clear) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  emu_set_a(c->x);
  emu_push(c->x);
  load_flag_byte();
  store_flag_byte(clear ? c->a & bit_clear_mask() : c->a | bit_mask());
  c->x = emu_pull();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// The middle of set_flag_045C and set_flag_047A: keeps only X. Y = the byte; returns A = X.
static void set_flag_keeping_x(void) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->x);
  emu_push(c->x);
  load_flag_byte();
  store_flag_byte(c->a | bit_mask());
  c->x = emu_pull();
  emu_set_a(c->x);
}

// The middle of clear_flag_045C and op_25: they push A rather than X (PHA, TXA where the set
// routines have TXA, PHA), so they come back with X = A = 4, select_array's A.
static void clear_flag_losing_x(void) {
  Cpu6502 *c = &g_nes.cpu;
  emu_push(c->a);
  emu_set_a(c->x);
  load_flag_byte();
  store_flag_byte(c->a & bit_clear_mask());
  c->x = emu_pull();
  emu_set_a(c->x);
}

// test_flag_bit (7:D14C): the script condition = bit script_args[0] of the array. Keeps X and Y;
// returns A = X.
void test_flag_bit(void) {
  RUN_ORIGINAL_IF_LAG(kTestFlagBit);
  Cpu6502 *c = &g_nes.cpu;
  script_condition_false();
  emu_set_a(c->x);
  emu_push(c->x);
  emu_set_a(c->y);
  emu_push(c->y);
  load_flag_byte();
  emu_set_a(c->a & bit_mask());
  if(c->a) {
    script_condition_true();
  }
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(c->x);
}

// load_flag_byte (7:D163): A = the array's byte holding bit script_args[0], Y = that byte's index,
// X = the bit in it. The carry is from the last LSR.
void load_flag_byte(void) {
  RUN_ORIGINAL_IF_LAG(kLoadFlagByte);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t bit = script_args[0];
  emu_set_flag(kFlagC, (bit >> 2) & 1);
  c->y = bit >> 3;
  c->x = bit & 0x07;
  emu_set_a(emu_read_ind_y(0x61, c->y));
}

// ---- Story flags ($0450) -------------------------------------------------------------------

// op_52_test_story_flag (7:D103), op_07_test_story_flag_of_object (7:D109)
void op_52_test_story_flag(void) {
  RUN_ORIGINAL_IF_LAG(kOp52TestStoryFlag);
  script_fetch_byte();
  test_story_flag(g_nes.cpu.a);
}

void op_07_test_story_flag_of_object(void) {
  RUN_ORIGINAL_IF_LAG(kOp07TestStoryFlagOfObject);
  emu_set_a(object_id);
  test_story_flag(object_id);
}

// test_story_flag (7:D10B)
void test_story_flag(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kTestStoryFlag);
  select_story_flags(bit);
  test_flag_bit();
}

// op_08_set_story_flag_of_object (7:D112)
void op_08_set_story_flag_of_object(void) {
  RUN_ORIGINAL_IF_LAG(kOp08SetStoryFlagOfObject);
  set_story_flag_refresh(object_id);
}

// set_story_flag_refresh (7:D114): sets the flag, then the room's sprites may change.
void set_story_flag_refresh(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kSetStoryFlagRefresh);
  select_story_flags(bit);
  change_flag_keeping_xy(0);
  collect_room_sprites();
  RESUME_ORIGINAL_IF_LAG(kSetStoryFlagRefresh, 0xD12A, 0);
}

// op_09_clear_story_flag_of_object (7:D12B)
void op_09_clear_story_flag_of_object(void) {
  RUN_ORIGINAL_IF_LAG(kOp09ClearStoryFlagOfObject);
  clear_story_flag(object_id);
}

// clear_story_flag (7:D12D)
void clear_story_flag(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kClearStoryFlag);
  select_story_flags(bit);
  change_flag_keeping_xy(1);
}

// select_story_flags (7:D141)
void select_story_flags(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kSelectStoryFlags);
  select_array(bit, 0x50);
}

// ---- Door flags ($0480) --------------------------------------------------------------------

// op_13_test_door_flag_of_object (7:D246), op_2E_test_door_flag (7:D24B)
void op_13_test_door_flag_of_object(void) {
  RUN_ORIGINAL_IF_LAG(kOp13TestDoorFlagOfObject);
  emu_set_a(object_id);
  test_door_flag(object_id);
}

void op_2E_test_door_flag(void) {
  RUN_ORIGINAL_IF_LAG(kOp2ETestDoorFlag);
  script_fetch_byte();
  test_door_flag(g_nes.cpu.a);
}

// test_door_flag (7:D24E)
void test_door_flag(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kTestDoorFlag);
  select_door_flags(bit);
  test_flag_bit();
}

// op_14_set_door_flag_of_object (7:D255), op_2F_set_door_flag (7:D25A)
void op_14_set_door_flag_of_object(void) {
  RUN_ORIGINAL_IF_LAG(kOp14SetDoorFlagOfObject);
  set_door_flag(object_id);
}

void op_2F_set_door_flag(void) {
  RUN_ORIGINAL_IF_LAG(kOp2FSetDoorFlag);
  script_fetch_byte();
  set_door_flag(g_nes.cpu.a);
}

// set_door_flag (7:D25D)
void set_door_flag(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kSetDoorFlag);
  select_door_flags(bit);
  change_flag_keeping_xy(0);
}

// op_15_clear_door_flag_of_object (7:D271), op_30_clear_door_flag (7:D276)
void op_15_clear_door_flag_of_object(void) {
  RUN_ORIGINAL_IF_LAG(kOp15ClearDoorFlagOfObject);
  clear_door_flag(object_id);
}

void op_30_clear_door_flag(void) {
  RUN_ORIGINAL_IF_LAG(kOp30ClearDoorFlag);
  script_fetch_byte();
  clear_door_flag(g_nes.cpu.a);
}

// clear_door_flag (7:D279)
void clear_door_flag(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kClearDoorFlag);
  select_door_flags(bit);
  change_flag_keeping_xy(1);
}

// select_door_flags (7:D28D)
void select_door_flags(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kSelectDoorFlags);
  select_array(bit, 0x80);
}

// ---- Event flags ($0490) -------------------------------------------------------------------

// op_16_test_event_flag (7:D298)
void op_16_test_event_flag(void) {
  RUN_ORIGINAL_IF_LAG(kOp16TestEventFlag);
  script_fetch_byte();
  test_event_flag(g_nes.cpu.a);
}

// test_event_flag (7:D29B)
void test_event_flag(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kTestEventFlag);
  select_event_flags(bit);
  test_flag_bit();
}

// op_17_set_event_flag (7:D2A2)
void op_17_set_event_flag(void) {
  RUN_ORIGINAL_IF_LAG(kOp17SetEventFlag);
  script_fetch_byte();
  set_event_flag(g_nes.cpu.a);
}

// set_event_flag (7:D2A5)
void set_event_flag(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kSetEventFlag);
  select_event_flags(bit);
  change_flag_keeping_xy(0);
}

// op_18_clear_event_flag (7:D2B9)
void op_18_clear_event_flag(void) {
  RUN_ORIGINAL_IF_LAG(kOp18ClearEventFlag);
  script_fetch_byte();
  clear_event_flag(g_nes.cpu.a);
}

// clear_event_flag (7:D2BC)
void clear_event_flag(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kClearEventFlag);
  select_event_flags(bit);
  change_flag_keeping_xy(1);
}

// select_event_flags (7:D2D0)
void select_event_flags(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kSelectEventFlags);
  select_array(bit, 0x90);
}

// ---- The flags at $045C --------------------------------------------------------------------

// op_20_test_flag_045C_of_object (7:D42A)
void op_20_test_flag_045C_of_object(void) {
  RUN_ORIGINAL_IF_LAG(kOp20TestFlag045COfObject);
  emu_set_a(object_id);
  test_flag_045C(object_id);
}

// test_flag_045C (7:D42C)
void test_flag_045C(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kTestFlag045C);
  select_flags_045C(bit);
  test_flag_bit();
}

// op_53_set_flag_045C (7:D433), op_21_set_flag_045C_of_object (7:D439)
void op_53_set_flag_045C(void) {
  RUN_ORIGINAL_IF_LAG(kOp53SetFlag045C);
  script_fetch_byte();
  set_flag_045C(g_nes.cpu.a);
}

void op_21_set_flag_045C_of_object(void) {
  RUN_ORIGINAL_IF_LAG(kOp21SetFlag045COfObject);
  set_flag_045C(object_id);
}

// set_flag_045C (7:D43B)
void set_flag_045C(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kSetFlag045C);
  select_flags_045C(bit);
  set_flag_keeping_x();
}

// op_22_clear_flag_045C_of_object (7:D44B)
void op_22_clear_flag_045C_of_object(void) {
  RUN_ORIGINAL_IF_LAG(kOp22ClearFlag045COfObject);
  clear_flag_045C(object_id);
}

// clear_flag_045C (7:D44D)
void clear_flag_045C(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kClearFlag045C);
  select_flags_045C(bit);
  clear_flag_losing_x();
}

// select_flags_045C (7:D45D)
void select_flags_045C(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kSelectFlags045C);
  select_array(bit, 0x5C);
}

// ---- The flags at $047A --------------------------------------------------------------------

// op_23_test_flag_047A_of_object (7:D468)
void op_23_test_flag_047A_of_object(void) {
  RUN_ORIGINAL_IF_LAG(kOp23TestFlag047AOfObject);
  emu_set_a(object_id);
  test_flag_047A(object_id);
}

// test_flag_047A (7:D46A)
void test_flag_047A(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kTestFlag047A);
  select_flags_047A(bit);
  test_flag_bit();
}

// op_54_set_flag_047A (7:D471), op_24_set_flag_047A_of_object (7:D477)
void op_54_set_flag_047A(void) {
  RUN_ORIGINAL_IF_LAG(kOp54SetFlag047A);
  script_fetch_byte();
  set_flag_047A(g_nes.cpu.a);
}

void op_24_set_flag_047A_of_object(void) {
  RUN_ORIGINAL_IF_LAG(kOp24SetFlag047AOfObject);
  set_flag_047A(object_id);
}

// set_flag_047A (7:D479)
void set_flag_047A(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kSetFlag047A);
  select_flags_047A(bit);
  set_flag_keeping_x();
}

// op_25_clear_flag_047A_of_object (7:D489)
void op_25_clear_flag_047A_of_object(void) {
  RUN_ORIGINAL_IF_LAG(kOp25ClearFlag047AOfObject);
  select_flags_047A(object_id);
  clear_flag_losing_x();
}

// select_flags_047A (7:D49B)
void select_flags_047A(uint8_t bit) {
  emu_set_a(bit);
  RUN_ORIGINAL_IF_LAG(kSelectFlags047A);
  select_array(bit, 0x7A);
}

// ---- Item status and object state ----------------------------------------------------------

// The state-byte routines: the operand is in script_args[0] while Y, which they keep, indexes the
// array. Each returns A = Y.
static void test_state_bit(uint8_t *array, uint8_t id, uint8_t mask) {
  Cpu6502 *c = &g_nes.cpu;
  script_args[0] = id;
  emu_set_a(c->y);
  emu_push(c->y);
  c->y = script_args[0];
  script_condition_false();
  emu_set_a(array[c->y] & mask);
  if(c->a) {
    script_condition_true();
  }
  c->y = emu_pull();
  emu_set_a(c->y);
}

static void set_state_bit(uint8_t *array, uint8_t id, uint8_t mask) {
  Cpu6502 *c = &g_nes.cpu;
  script_args[0] = id;
  emu_set_a(c->y);
  emu_push(c->y);
  c->y = script_args[0];
  array[c->y] |= mask;
  c->y = emu_pull();
  emu_set_a(c->y);
}

static void clear_state_bit(uint8_t *array, uint8_t id, uint8_t mask) {
  Cpu6502 *c = &g_nes.cpu;
  script_args[0] = id;
  emu_set_a(c->y);
  emu_push(c->y);
  c->y = script_args[0];
  array[c->y] &= (uint8_t)~mask;
  c->y = emu_pull();
  emu_set_a(c->y);
}

// op_5D (7:D171), op_0A (7:D177)
void op_5D(void) {
  RUN_ORIGINAL_IF_LAG(kOp5D);
  script_fetch_byte();
  test_item_obtained(g_nes.cpu.a);
}

void op_0A(void) {
  RUN_ORIGINAL_IF_LAG(kOp0A);
  test_item_obtained(object_id);
}

// test_item_obtained (7:D179)
void test_item_obtained(uint8_t id) {
  emu_set_a(id);
  RUN_ORIGINAL_IF_LAG(kTestItemObtained);
  test_state_bit(item_status, id, 0x01);
}

// op_0B (7:D18F): the item is obtained, so the room's sprites may change.
void op_0B(void) {
  RUN_ORIGINAL_IF_LAG(kOp0B);
  set_state_bit(item_status, object_id, 0x01);
  collect_room_sprites();
  RESUME_ORIGINAL_IF_LAG(kOp0B, 0xD1A4, 0);
}

// op_0C (7:D1A5)
void op_0C(void) {
  RUN_ORIGINAL_IF_LAG(kOp0C);
  clear_item_obtained(object_id);
}

// clear_item_obtained (7:D1A7)
void clear_item_obtained(uint8_t id) {
  emu_set_a(id);
  RUN_ORIGINAL_IF_LAG(kClearItemObtained);
  clear_state_bit(item_status, id, 0x01);
}

// op_0D (7:D1B8), op_2B (7:D1BD)
void op_0D(void) {
  RUN_ORIGINAL_IF_LAG(kOp0D);
  test_object_open(object_id);
}

void op_2B(void) {
  RUN_ORIGINAL_IF_LAG(kOp2B);
  script_fetch_byte();
  test_object_open(g_nes.cpu.a);
}

// test_object_open (7:D1C0)
void test_object_open(uint8_t id) {
  emu_set_a(id);
  RUN_ORIGINAL_IF_LAG(kTestObjectOpen);
  test_state_bit(object_state, id, 0x01);
}

// op_0E (7:D1D6), op_2C (7:D1DB)
void op_0E(void) {
  RUN_ORIGINAL_IF_LAG(kOp0E);
  set_object_open(object_id);
}

void op_2C(void) {
  RUN_ORIGINAL_IF_LAG(kOp2C);
  script_fetch_byte();
  set_object_open(g_nes.cpu.a);
}

// set_object_open (7:D1DE)
void set_object_open(uint8_t id) {
  emu_set_a(id);
  RUN_ORIGINAL_IF_LAG(kSetObjectOpen);
  set_state_bit(object_state, id, 0x01);
}

// op_0F (7:D1EF), op_2D (7:D1F4)
void op_0F(void) {
  RUN_ORIGINAL_IF_LAG(kOp0F);
  close_object(object_id);
}

void op_2D(void) {
  RUN_ORIGINAL_IF_LAG(kOp2D);
  script_fetch_byte();
  close_object(g_nes.cpu.a);
}

// close_object (7:D1F7)
void close_object(uint8_t id) {
  emu_set_a(id);
  RUN_ORIGINAL_IF_LAG(kCloseObject);
  clear_state_bit(object_state, id, 0x01);
}

// op_10 (7:D208): is the item open?
void op_10(void) {
  RUN_ORIGINAL_IF_LAG(kOp10);
  test_state_bit(item_status, object_id, 0x02);
}

// op_11 (7:D220)
void op_11(void) {
  RUN_ORIGINAL_IF_LAG(kOp11);
  set_state_bit(item_status, object_id, 0x02);
}

// op_12 (7:D233)
void op_12(void) {
  RUN_ORIGINAL_IF_LAG(kOp12);
  clear_item_open(object_id);
}

// clear_item_open (7:D235)
void clear_item_open(uint8_t id) {
  emu_set_a(id);
  RUN_ORIGINAL_IF_LAG(kClearItemOpen);
  clear_state_bit(item_status, id, 0x02);
}

// op_19 (7:D305): the script condition = whether the operand is the room.
void op_19(void) {
  RUN_ORIGINAL_IF_LAG(kOp19);
  script_condition_false();
  script_fetch_byte();
  emu_cmp(g_nes.cpu.a, room);
  if(g_nes.cpu.a == room) {
    script_condition_true();
  }
}
