// The C port's names for RAM (zelda3's variables.h), generated from asm/ram.inc by
// tools/make_variables.py: edit asm/ram.inc and run the tool, not this file. One-byte
// variables are lvalues; longer ones ([n] in the comment) point to their first byte.
#ifndef DV_VARIABLES_H
#define DV_VARIABLES_H

#include "nes_hw.h"

#define g_ram (g_nes.ram)

#define current_bank (g_ram[0x0C])  // PRG bank mapped at $8000 (switch_prg_bank)
#define stack_index (g_ram[0x0D])  // top of a small data stack growing down from $07FF (stack_push and
#define nmi_requests (g_ram[0x18])  // work for the next NMI: 7 OAM DMA, 6 palette, 5 attrs $23C0, 4 attrs of
#define sprite0_reserved (g_ram[0x24])  // 1 when sprite 0 is kept for the split (clear_oam_buffer skips it)
#define frame_counter (g_ram[0x25])  // +1 every NMI
#define scroll_x (g_ram + 0x26)  // [2] low byte, then the nametable bit (nmi_set_scroll)
#define scroll_y (g_ram + 0x28)  // [2] low byte, then the nametable bit
#define split_nametable (g_ram[0x2B])  // bit 0: nametable for the lower part of the split
#define random_value (g_ram[0x35])  // latest result of next_random (state nibbles swapped)
#define rng_state (g_ram[0x36])  // next_random: state = (state + 1) * 5
#define inline_ptr (g_ram + 0x37)  // [2] reads data placed after a JSR (set_chr_banks_inline, draw_rle_screen_*)
#define repeat_sound (g_ram[0x3A])  // sound number to restart when music ends (0: none)
#define sound_table_ptr (g_ram + 0x3B)  // [2] address of the sound table (5:8000)
#define text_bits_used (g_ram[0x3D])  // bits of text_bit_buffer already read
#define text_bit_buffer (g_ram[0x3E])  // current byte of the message bit stream
#define text_value (g_ram[0x3F])  // bits being assembled into a code or byte
#define data_ptr (g_ram + 0x55)  // [2] scene picture / screen layout pointer
#define flag_array_ptr (g_ram + 0x61)  // [2] flag array the flag opcodes work on ($0450, $0480, ...)
#define scene_index (g_ram[0x75])  // scene number within its bank (select_scene_bank); menus reuse it
#define scene_blank (g_ram[0x7D])  // nonzero: draw_scene_dissolve draws $FF everywhere
#define printed_digit (g_ram[0x8E])  // nonzero once a nonzero digit was printed (no leading zeros)
#define text_wait_enabled (g_ram[0x8F])  // nonzero: pause at the end of a page (text_delay pauses of 20 frames)
#define text_fast (g_ram[0x90])  // set when A is pressed during printing: finish the page at once
#define text_delay (g_ram[0x91])  // number of 20-frame pauses (text_pause) at the end of a page; main sets 2
#define text_window_drawn (g_ram[0x92])  // nonzero once the window's sides are drawn
#define text_no_wrap (g_ram[0x94])  // nonzero: stay on the last column instead of wrapping
#define layout_ptr (g_ram + 0x95)  // [2] current screen layout command
#define last_room (g_ram[0x97])  // room at the last redraw
#define same_room_redraws (g_ram[0x99])  // times the game screen was redrawn in the same room (main loop)
#define cursor_x (g_ram[0xA5])
#define cursor_y (g_ram[0xA6])
#define screen_area (g_ram[0xA7])  // area the cursor clicked: 0 picture, 1 commands, 2 inventory
#define hotspot_ptr (g_ram + 0xA8)  // [2] current room hotspot record
#define script_table (g_ram[0xAA])  // which top-level script table (bits 0-6)
#define object_id (g_ram[0xAB])  // the object being acted on; "[ab]" flag opcodes use it
#define command (g_ram[0xAC])  // selected command (0 Examine ... $0A Move, Data Crystal)
#define switch_key (g_ram + 0xAD)  // [2] while a script waits for a choice (event flag $80), clicks store
#define saved_inventory_page (g_ram[0xB3])  // inventory page and entry to go back to (show_item_page_keep_cursor)
#define saved_inventory_entry (g_ram[0xB4])
#define script_flags (g_ram[0xB5])  // bit 0 hotspot hit, bit 2 script condition, bit 7 set by opcode $48
#define script_ptr (g_ram + 0xB6)  // [2] next script byte
#define script_args (g_ram + 0xB8)  // [4] operands of the current opcode
#define script_slot (g_ram[0xBC])  // index into the last script table level
#define message_id (g_ram[0xBF])
#define cursor_mode (g_ram[0xC2])  // low nibble: 0 picture, 1 commands, 2 inventory; high: held direction
#define command_cursor (g_ram[0xC3])  // command the cursor sits on in the command area
#define key_repeat (g_ram[0xC4])  // frames a direction has been held (read_input_with_repeat)
#define object_sprite_list (g_ram + 0xC5)  // [40] x, y, sprite for each object shown as a sprite; $FF ends
#define special_scene (g_ram[0xED])  // special scene to show (script opcode $46, some commands): picks the
#define save_file (g_ram[0xEE])  // save file (0-2) chosen in file_select_menu; save_game and load_game use it
#define game_started (g_ram[0xEF])  // 0 until the opening message has been shown
#define system_flags (g_ram[0xF0])  // bit 0 sprite-0 split, 1 waiting for vblank, 2 sound busy,
#define joy_pressed (g_ram + 0xF5)  // [2] buttons newly pressed this frame, controllers 1 and 2
#define joy_held (g_ram + 0xF7)  // [2] buttons held
#define mirroring_mode (g_ram[0xFA])  // $47 vertical, $4F horizontal; bit 3 picks $2800 over $2400 as the 2nd nametable
#define ppuscroll_x (g_ram[0xFC])  // scroll values last written to PPUSCROLL
#define ppuscroll_y (g_ram[0xFD])
#define ppumask_shadow (g_ram[0xFE])
#define ppuctrl_shadow (g_ram[0xFF])
#define chr_bank_shadows (g_ram + 0x104)  // [8] MMC3 CHR bank registers R0-R5 as last set
#define bank_select_shadow (g_ram[0x10C])  // MMC3 bank select ($8000) as last written
#define oam_buffer (g_ram + 0x200)  // [256] sprites, copied to the PPU by OAM DMA
#define attr_buffer_2000 (g_ram + 0x300)  // [64] attribute table for nametable $2000
#define attr_buffer_2400 (g_ram + 0x340)  // [64] attribute table for nametable $2400
#define vram_queue_len (g_ram[0x380])  // bytes waiting in vram_queue
#define vram_queue (g_ram + 0x381)  // [127] entries: address high, low, length, then the bytes
#define palette_buffer (g_ram + 0x400)  // [32] uploaded to $3F00 by the NMI
#define object_state (g_ram + 0x420)  // [13] bit 0: open (desks, safe, mailbox, ...)
#define container_items (g_ram + 0x430)  // [32] items inside openable containers, 8 per container ($B0 picks one)
#define story_flags (g_ram + 0x450)  // [12] flag array (opcodes $07-$09, $52)
#define flag_array_045C (g_ram + 0x45C)  // [12] flag array (opcodes $20-$22, $53)
#define item_status (g_ram + 0x468)  // [18] bit 0 obtained, bit 1 open (coat, wallet, guns, ...)
#define flag_array_047A (g_ram + 0x47A)  // [6] flag array (opcodes $23-$25, $54)
#define door_flags (g_ram + 0x480)  // [16] doors, windows, manholes (opcodes $13-$15, $2E-$30)
#define event_flags (g_ram + 0x490)  // [21] flag array (opcodes $16-$18)
#define text_lines (g_ram[0x4B6])  // lines printed in the text window
#define address_book (g_ram + 0x4C0)  // [7] inventory page 0
#define coat_slots (g_ram + 0x4C7)  // [7] inventory page 1
#define goods_slots (g_ram + 0x4CE)  // [70] the rest of the inventory, in pages of 7 like the two above
#define room (g_ram[0x514])
#define scene (g_ram[0x515])  // scene picture number
#define inventory_page (g_ram[0x516])  // inventory page on screen (0 address book, 1 coat, then goods)
#define store_page (g_ram[0x517])  // where store_in_inventory_slot puts an item: page...
#define inventory_entry (g_ram[0x518])  // entry (0-6) on that page; the draw loops count with it too
#define store_entry (g_ram[0x519])  // ...and entry
#define addresses_found (g_ram[0x51A])
#define coins (g_ram[0x51B])
#define open_containers (g_ram[0x51E])  // number of containers open (open_container, at most 6)
#define container_stack (g_ram + 0x51F)  // [14] script table and object of each open container
#define current_container (g_ram[0x52D])  // which entry of container_stack is shown
#define saved_scene (g_ram[0x52F])  // scene to go back to after a special scene
#define dissolve_pattern (g_ram[0x530])  // from room_settings (high nibble): which order draw_scene_dissolve uses
#define scene_palette_id (g_ram[0x531])  // from scene_palette_ids; picks the entry of palette_table
#define text_keep_open (g_ram[0x533])  // messages that may still end without closing the text window (opcode $3F)
#define text_item (g_ram[0x534])  // item number that messages print with <item at $0534>
#define text_number (g_ram[0x535])  // number that messages print with <number at $0535>
#define scene_tiles (g_ram + 0x554)  // [196] 14x14 tiles of the current picture
#define text_window_x (g_ram[0x618])
#define text_window_y (g_ram[0x619])
#define text_char (g_ram[0x61A])  // character code being printed
#define text_digit_mark (g_ram[0x61B])  // print_digit sets it to $FF; nothing reads it
#define text_column (g_ram[0x61C])
#define text_row (g_ram[0x61D])
#define text_tile_block (g_ram + 0x61E)  // [2] width and height of the block at text_tile, for queue_vram_block
#define text_tile (g_ram[0x620])  // font tile being printed
#define text_window_block (g_ram + 0x626)  // [2] width and height of the block at text_window
#define text_window (g_ram + 0x628)  // [208] window contents: 8 rows of 26 tiles
#define name_buffer (g_ram + 0x6F8)  // [8] name being printed by a screen layout
#define sound_channels (g_ram + 0x700)  // [216] 9 channel slots of 24 bytes

#endif  // DV_VARIABLES_H
