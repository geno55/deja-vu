; RAM variables, included at the top of every asm/bank*.s.
;
; Format: name = address ; [size] meaning   (size in bytes, 1 if left out)
; tools/name_ram.py replaces raw addresses in the bank files with these names
; (name+n inside a multi-byte variable). Addresses missing from this file are
; either not understood yet or reused for different things by different routines
; (for example $10-$12, $1C-$1F and the sound driver's $44-$4B); they stay as
; numbers until someone works them out. Sources: the code (docs/*.md) and Data
; Crystal's RAM map (story flags, inventory, cursor).

; ---- Zero page --------------------------------------------------------------
current_bank      = $0C   ; PRG bank mapped at $8000 (switch_prg_bank)
stack_index       = $0D   ; top of a small data stack growing down from $07FF (stack_push and
                          ; stack_pop): saved banks, and addresses pushed by script opcode $65
nmi_requests      = $18   ; work for the next NMI: 7 OAM DMA, 6 palette, 5 attrs $23C0, 4 attrs of
                          ; the 2nd nametable, 3 VRAM queue, 2 black palette, 1 VRAM queue going
                          ; down, 0 write ppumask_shadow
sprite0_reserved  = $24   ; 1 when sprite 0 is kept for the split (clear_oam_buffer skips it)
frame_counter     = $25   ; +1 every NMI
scroll_x          = $26   ; [2] low byte, then the nametable bit (nmi_set_scroll)
scroll_y          = $28   ; [2] low byte, then the nametable bit
split_nametable   = $2B   ; bit 0: nametable for the lower part of the split
random_value      = $35   ; latest result of next_random (state nibbles swapped)
rng_state         = $36   ; next_random: state = (state + 1) * 5
inline_ptr        = $37   ; [2] reads data placed after a JSR (set_chr_banks_inline, draw_rle_screen_*)
repeat_sound      = $3A   ; sound number to restart when music ends (0: none)
sound_table_ptr   = $3B   ; [2] address of the sound table (5:8000)
text_bits_used    = $3D   ; bits of text_bit_buffer already read
text_bit_buffer   = $3E   ; current byte of the message bit stream
text_value        = $3F   ; bits being assembled into a code or byte
data_ptr          = $55   ; [2] scene picture / screen layout pointer
flag_array_ptr    = $61   ; [2] flag array the flag opcodes work on ($0450, $0480, ...)
scene_index       = $75   ; scene number within its bank (select_scene_bank); menus reuse it
                          ; (valid save files as bits, the yes/no answer of confirm_dialog_loop)
scene_blank       = $7D   ; nonzero: draw_scene_dissolve draws $FF everywhere
printed_digit     = $8E   ; nonzero once a nonzero digit was printed (no leading zeros)
text_wait_enabled = $8F   ; nonzero: pause at the end of a page (text_delay pauses of 20 frames)
text_fast         = $90   ; set when A is pressed during printing: finish the page at once
text_delay        = $91   ; number of 20-frame pauses (text_pause) at the end of a page; main sets 2
text_window_drawn = $92   ; nonzero once the window's sides are drawn
text_no_wrap      = $94   ; nonzero: stay on the last column instead of wrapping
layout_ptr        = $95   ; [2] current screen layout command
last_room         = $97   ; room at the last redraw
same_room_redraws = $99   ; times the game screen was redrawn in the same room (main loop)
cursor_x          = $A5
cursor_y          = $A6
screen_area       = $A7   ; area the cursor clicked: 0 picture, 1 commands, 2 inventory
hotspot_ptr       = $A8   ; [2] current room hotspot record
script_table      = $AA   ; which top-level script table (bits 0-6)
object_id         = $AB   ; the object being acted on; "[ab]" flag opcodes use it
command           = $AC   ; selected command (0 Examine ... $0A Move, Data Crystal)
switch_key        = $AD   ; [2] while a script waits for a choice (event flag $80), clicks store
                          ; the chosen script table and object here instead of in $AA/$AB;
                          ; the switch opcode ($29) then compares its cases with them
saved_inventory_page = $B3 ; inventory page and entry to go back to (show_item_page_keep_cursor)
saved_inventory_entry = $B4
script_flags      = $B5   ; bit 0 hotspot hit, bit 2 script condition, bit 7 set by opcode $48
script_ptr        = $B6   ; [2] next script byte
script_args       = $B8   ; [4] operands of the current opcode
script_slot       = $BC   ; index into the last script table level
message_id        = $BF
cursor_mode       = $C2   ; low nibble: 0 picture, 1 commands, 2 inventory; high: held direction
command_cursor    = $C3   ; command the cursor sits on in the command area
key_repeat        = $C4   ; frames a direction has been held (read_input_with_repeat)
object_sprite_list = $C5  ; [40] x, y, sprite for each object shown as a sprite; $FF ends
                          ; (collect_room_sprites / add_object_sprites fill it, draw_object_sprites reads it)
special_scene     = $ED   ; special scene to show (script opcode $46, some commands): picks the
                          ; picture, flag, message and action in the special_scene_* tables
save_file         = $EE   ; save file (0-2) chosen in file_select_menu; save_game and load_game use it
game_started      = $EF   ; 0 until the opening message has been shown
system_flags      = $F0   ; bit 0 sprite-0 split, 1 waiting for vblank, 2 sound busy,
                          ; 3 bank switch in progress, 7 sound update deferred
joy_pressed       = $F5   ; [2] buttons newly pressed this frame, controllers 1 and 2
joy_held          = $F7   ; [2] buttons held
mirroring_mode    = $FA   ; $47 vertical, $4F horizontal; bit 3 picks $2800 over $2400 as the 2nd nametable
ppuscroll_x       = $FC   ; scroll values last written to PPUSCROLL
ppuscroll_y       = $FD
ppumask_shadow    = $FE
ppuctrl_shadow    = $FF

; ---- $0100-$07FF -----------------------------------------------------------
; $0420-$052F (272 bytes) is the game state that save_game writes to save RAM: 3 files
; (save_file), each kept as 4 copies with a signature and a 16-bit checksum.-
chr_bank_shadows  = $0104 ; [8] MMC3 CHR bank registers R0-R5 as last set
bank_select_shadow = $010C ; MMC3 bank select ($8000) as last written
oam_buffer        = $0200 ; [256] sprites, copied to the PPU by OAM DMA
attr_buffer_2000  = $0300 ; [64] attribute table for nametable $2000
attr_buffer_2400  = $0340 ; [64] attribute table for nametable $2400
vram_queue_len    = $0380 ; bytes waiting in vram_queue
vram_queue        = $0381 ; [127] entries: address high, low, length, then the bytes
palette_buffer    = $0400 ; [32] uploaded to $3F00 by the NMI
object_state      = $0420 ; [13] bit 0: open (desks, safe, mailbox, ...)
container_items   = $0430 ; [32] items inside openable containers, 8 per container ($B0 picks one)
story_flags       = $0450 ; [12] flag array (opcodes $07-$09, $52)
flag_array_045C   = $045C ; [12] flag array (opcodes $20-$22, $53)
item_status       = $0468 ; [18] bit 0 obtained, bit 1 open (coat, wallet, guns, ...)
flag_array_047A   = $047A ; [6] flag array (opcodes $23-$25, $54)
door_flags        = $0480 ; [16] doors, windows, manholes (opcodes $13-$15, $2E-$30)
event_flags       = $0490 ; [21] flag array (opcodes $16-$18)
text_lines        = $04B6 ; lines printed in the text window
address_book      = $04C0 ; [7] inventory page 0
coat_slots        = $04C7 ; [7] inventory page 1
goods_slots       = $04CE ; [70] the rest of the inventory, in pages of 7 like the two above
room              = $0514
scene             = $0515 ; scene picture number
inventory_page    = $0516 ; inventory page on screen (0 address book, 1 coat, then goods)
store_page        = $0517 ; where store_in_inventory_slot puts an item: page...
inventory_entry   = $0518 ; entry (0-6) on that page; the draw loops count with it too
store_entry       = $0519 ; ...and entry
addresses_found   = $051A
coins             = $051B
open_containers   = $051E ; number of containers open (open_container, at most 6)
container_stack   = $051F ; [14] script table and object of each open container
current_container = $052D ; which entry of container_stack is shown
saved_scene       = $052F ; scene to go back to after a special scene
dissolve_pattern  = $0530 ; from room_settings (high nibble): which order draw_scene_dissolve uses
scene_palette_id  = $0531 ; from scene_palette_ids; picks the entry of palette_table
text_keep_open    = $0533 ; messages that may still end without closing the text window (opcode $3F)
text_item         = $0534 ; item number that messages print with <item at $0534>
text_number       = $0535 ; number that messages print with <number at $0535>
scene_tiles       = $0554 ; [196] 14x14 tiles of the current picture
text_window_x     = $0618
text_window_y     = $0619
text_char         = $061A ; character code being printed
text_digit_mark   = $061B ; print_digit sets it to $FF; nothing reads it
text_column       = $061C
text_row          = $061D
text_tile_block   = $061E ; [2] width and height of the block at text_tile, for queue_vram_block
text_tile         = $0620 ; font tile being printed
text_window_block = $0626 ; [2] width and height of the block at text_window
text_window       = $0628 ; [208] window contents: 8 rows of 26 tiles
name_buffer       = $06F8 ; [8] name being printed by a screen layout
sound_channels    = $0700 ; [216] 9 channel slots of 24 bytes
