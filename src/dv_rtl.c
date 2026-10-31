// The frame side of the port: waiting for the NMI, where the game coroutine yields, and the
// list of ported routines with the glue that lets original code call them.
#include "funcs.h"
#include "variables.h"

// wait_vblank (7:EB9C): sets bit 1 of system_flags and waits until the NMI clears it
// (sound_update does). Returns A = 0, from the AND that found the flag clear.
void wait_vblank(void) {
  RUN_ORIGINAL_IF_LAG(kWaitVblank);
  emu_take_late_nmis();
  system_flags |= 0x02;
  while(system_flags & 0x02) {
    emu_wait_for_nmi(0xEBA4);
  }
  emu_set_a(0);
}

// The spin on nmi_requests at 7:EBB4: until the NMI has done the work in mask.
static void wait_nmi_requests(uint8_t mask) {
  if(nmi_requests & mask) {
    emu_take_late_nmis();
  }
  while(nmi_requests & mask) {
    emu_wait_for_nmi(0xEBB4);
  }
  emu_set_a(0);
}

// wait_vram_queue_empty (7:EBAE): until the NMI has written out the VRAM queue.
void wait_vram_queue_empty(void) {
  RUN_ORIGINAL_IF_LAG(kWaitVramQueueEmpty);
  wait_nmi_requests(0x08);
}

// wait_nmi_work_done (7:EBB2): until the NMI has done everything asked of it.
void wait_nmi_work_done(void) {
  RUN_ORIGINAL_IF_LAG(kWaitNmiWorkDone);
  wait_nmi_requests(0xFF);
}

// ---- Called from original code -------------------------------------------------------------
// Each takes its inputs from the registers the original takes them in.

#define HOOK(name)                     \
  static uint16_t hook_##name(void) {  \
    name();                            \
    return kEmuRts;                    \
  }
#define HOOK_A(name)                   \
  static uint16_t hook_##name(void) {  \
    name(g_nes.cpu.a);                 \
    return kEmuRts;                    \
  }

static uint16_t hook_reset(void) {
  return game_reset();
}

static uint16_t hook_main(void) {
  return game_main();
}

static uint16_t hook_mmc3_set_bank(void) {
  mmc3_set_bank(g_nes.cpu.a, g_nes.cpu.x);
  return kEmuRts;
}

static uint16_t hook_set_chr_banks_inline(void) {
  return set_chr_banks_inline();
}

static uint16_t hook_jump_table(void) {
  uint8_t lo = emu_pull();
  uint16_t pushed = (uint16_t)(lo | emu_pull() << 8);
  return jump_table(pushed, g_nes.cpu.a);
}

static uint16_t hook_read_inline_byte(void) {
  read_inline_byte();
  return kEmuRts;
}

static uint16_t hook_stack_pop(void) {
  stack_pop();
  return kEmuRts;
}

HOOK(set_mirroring_vertical)
HOOK_A(switch_prg_bank)
HOOK(apply_chr_banks_r0_r1)
HOOK(apply_chr_banks_r2_r5)
HOOK(apply_chr_banks)
HOOK(advance_inline_ptr)
HOOK_A(push_and_switch_bank)
HOOK_A(set_current_bank)
HOOK(switch_to_sound_bank)
HOOK(restore_current_bank)
HOOK(pop_bank)
HOOK_A(stack_push)
HOOK(disable_nmi)
HOOK(enable_nmi)
HOOK(read_joypads)
HOOK(read_joypads_once)
HOOK(wait_vblank)
HOOK_A(request_nmi_work)
HOOK(wait_vram_queue_empty)
HOOK(wait_nmi_work_done)
HOOK(wait_vblank_and_hide_sprites)
HOOK(clear_oam_buffer)
HOOK(clear_and_upload_sprites)
HOOK(request_oam_dma)
HOOK(request_vram_queue)
HOOK(request_attrs_2000)
HOOK(request_attr_upload)
HOOK(request_black_palette)
HOOK(show_bg_and_sprites)
HOOK(hide_bg_and_sprites)
HOOK(request_palette_upload)
HOOK_A(far_call_8000)
HOOK_A(far_call_8020)
HOOK_A(far_call_8040)
HOOK_A(far_call_8400)
HOOK(sound_reset)
HOOK_A(play_sound)
HOOK(sound_check_repeat)
HOOK(start_sound)
HOOK(sound_start_by_type)
HOOK(sound_start_music)
HOOK(sound_start_channel)
HOOK(sound_start_repeating_music)
HOOK(sound_start_effect_3ch)
HOOK(sound_start_effect_2ch)
HOOK(sound_update)
HOOK(sound_update_all)
HOOK(sound_step_all_channels)
HOOK(sound_step_pulse)
HOOK(sound_pulse_envelope)
HOOK(sound_step_triangle)
HOOK(multiply_8x8)
HOOK(sound_triangle_countdown)
HOOK(sound_step_noise)
HOOK(sound_tick)
HOOK(sound_add_length)
HOOK(sound_read_command)
HOOK(sound_loop_push)
HOOK(sound_loop_pop)
HOOK(sound_note_off)
HOOK(sound_channel_done)
HOOK(sound_write_apu)
HOOK(queue_vram_block)
HOOK(queue_vram_fill)
HOOK(vram_block_setup)
HOOK(vram_fill_rows)
HOOK(vram_copy_rows)
HOOK(next_vram_address)
HOOK(vram_wrap_up)
HOOK(vram_queue_header)
HOOK(next_vram_row)
HOOK(skip_block_header)
HOOK(flush_vram_queue_if_full)
HOOK(flush_vram_queue_down_if_full)
HOOK(flush_vram_queue_down)
HOOK(queue_vram_column)
HOOK(vram_column_setup)
HOOK(vram_copy_column)
HOOK(vram_addr_down_row)
HOOK(vram_wrap_column)
HOOK(attr_position)
HOOK(set_attr_block)
HOOK(next_attr_row)
HOOK(next_attr_column)
HOOK(set_attr_bits)
HOOK_A(fill_all_attrs)
HOOK_A(pick_attr_fill)
HOOK(fill_attr_buffer_2000)
HOOK(fill_attr_buffer_2400)
HOOK(restore_xy)
HOOK_A(clear_nametables)
HOOK(rendering_on)
HOOK_A(rendering_off)
HOOK(fill_nametable_2000)
HOOK(fill_nametable_2nd)
HOOK_A(rle_write_2000)
HOOK(rle_to_ppu_2000)
HOOK(fade_in_palette)
HOOK(palette_brighten_step)
HOOK(fade_out_palette)
HOOK(palette_darken_step)
HOOK(palette_fade_wait)

static uint16_t hook_draw_rle_screen_2000(void) {
  return draw_rle_screen_2000();
}

static uint16_t hook_nmi(void) {
  nmi();
  return kEmuRti;
}

HOOK_A(print_message_5)
HOOK(print_message_from)
HOOK_A(print_message_4)
HOOK(print_message_2_or_5)
HOOK(print_message)
HOOK_A(print_control)
HOOK(text_read_code)
HOOK(text_read_byte)
HOOK(text_read_bit)
HOOK(print_item_name_at)
HOOK(print_item_name_arg)
HOOK(print_word_arg)
HOOK(print_name)
HOOK(print_number_at)
HOOK(print_digit_skip_zero)
HOOK_A(print_digit)
HOOK(number_to_digits)
HOOK(read_number_arg)
HOOK(skip_inline_word)
HOOK(text_wait_page)
HOOK(text_newline)
HOOK(print_clear)
HOOK(clear_text_window)
HOOK(blank_text_window)
HOOK(print_tile)
HOOK(print_char)
HOOK(put_tile)
HOOK(text_line_feed)
HOOK(text_scroll)
HOOK_A(text_scroll_rows)
HOOK(text_cursor_vram_address)
HOOK(upload_text_window)
HOOK(draw_window_rule_row)
HOOK(blank_window_text_rows)
HOOK(draw_text_window_sides)
HOOK(draw_window_side_column)
HOOK(text_pause)
HOOK(wait_page_input)
HOOK(blink_more_arrow)
HOOK(show_more_arrow)
HOOK(wait_for_more_button)
HOOK(draw_more_arrow_frame)
HOOK(text_cursor_x_pixels)
HOOK(draw_more_arrow)
HOOK(nmi_ppu_updates)
HOOK(nmi_upload_attrs_2000)
HOOK(nmi_upload_attrs_2nd)
HOOK(nmi_flush_vram_queue)
HOOK(nmi_flush_vram_queue_down)
HOOK(flush_vram_queue)
HOOK(nmi_black_palette)
HOOK(nmi_upload_palette)
HOOK(ppu_addr_leave_palette)
HOOK(ppu_addr_palette)
HOOK(nmi_set_scroll)
HOOK(change_scene)
HOOK(change_scene_instant)
HOOK(load_scene)
HOOK(select_scene_bank)
HOOK(scene_pointer)
HOOK(draw_scene_dissolve)
HOOK(dissolve_draw_tile)
HOOK(dissolve_tile_value)
HOOK(dissolve_tile_address)
HOOK(load_scene_attrs)
HOOK(blacken_palette_colors)
HOOK(draw_scene)
HOOK(copy_scene_tiles)
HOOK(apply_room_settings)
HOOK(load_dissolve_pattern)
HOOK(load_scene_chr_banks)
HOOK(load_scene_palette)
HOOK(load_palette)
HOOK(palette_pointer)
HOOK(dissolve_row_mask)
HOOK(test_hotspot)
HOOK(cursor_in_hotspot)
HOOK(click_at_cursor)
HOOK(find_screen_area)
HOOK(next_screen_rect)
HOOK(click_command_area)
HOOK(click_inventory_area)
HOOK(reset_cursor)
HOOK(handle_input)
HOOK(get_input)
HOOK(move_cursor)
HOOK(read_input)
HOOK(read_input_free)
HOOK(read_input_with_repeat)
HOOK(reset_key_repeat)
HOOK(dpad_to_delta)
HOOK(move_cursor_in_scene)
HOOK(move_cursor_in_inventory)
HOOK(snap_cursor_to_command)
HOOK(move_cursor_x)
HOOK(move_cursor_y)
HOOK(move_cursor_in_commands)
HOOK(command_grid_position)
HOOK(move_in_command_grid)
HOOK(command_at_grid)
HOOK(handle_buttons)
HOOK(press_b)
HOOK(press_a)
HOOK(draw_cursor)
HOOK(highlight_command)
HOOK(draw_command_highlight)
HOOK(unhighlight_command)
HOOK(command_vram_address)
HOOK(clear_entry_highlight)
HOOK(highlight_entry)
HOOK(inventory_entry_vram_address)
HOOK(run_command)
HOOK(script_resume)
HOOK(script_start)
HOOK(script_loop)
HOOK(script_fetch_byte)
HOOK(script_advance)
HOOK(script_fetch_2)
HOOK(script_fetch_4)
HOOK(script_op_switch)
HOOK(script_op_yield)
HOOK(op_00_message_5)
HOOK(op_01_message_4)
HOOK(op_02_message_2_or_5)
HOOK(script_op_goto_if)
HOOK(script_op_goto_if_not)
HOOK(script_goto)
HOOK(script_op_push_goto)
HOOK(script_op_pop_goto)
HOOK(script_op_goto)
HOOK(script_condition_true)
HOOK(script_condition_false)
HOOK(op_52_test_story_flag)
HOOK(op_07_test_story_flag_of_object)
HOOK_A(test_story_flag)
HOOK(op_08_set_story_flag_of_object)
HOOK_A(set_story_flag_refresh)
HOOK(op_09_clear_story_flag_of_object)
HOOK_A(clear_story_flag)
HOOK_A(select_story_flags)
HOOK(test_flag_bit)
HOOK(load_flag_byte)
HOOK(op_5D)
HOOK(op_0A)
HOOK_A(test_item_obtained)
HOOK(op_0B)
HOOK(op_0C)
HOOK_A(clear_item_obtained)
HOOK(op_0D)
HOOK(op_2B)
HOOK_A(test_object_open)
HOOK(op_0E)
HOOK(op_2C)
HOOK_A(set_object_open)
HOOK(op_0F)
HOOK(op_2D)
HOOK_A(close_object)
HOOK(op_10)
HOOK(op_11)
HOOK(op_12)
HOOK_A(clear_item_open)
HOOK(op_13_test_door_flag_of_object)
HOOK(op_2E_test_door_flag)
HOOK_A(test_door_flag)
HOOK(op_14_set_door_flag_of_object)
HOOK(op_2F_set_door_flag)
HOOK_A(set_door_flag)
HOOK(op_15_clear_door_flag_of_object)
HOOK(op_30_clear_door_flag)
HOOK_A(clear_door_flag)
HOOK_A(select_door_flags)
HOOK(op_16_test_event_flag)
HOOK_A(test_event_flag)
HOOK(op_17_set_event_flag)
HOOK_A(set_event_flag)
HOOK(op_18_clear_event_flag)
HOOK_A(clear_event_flag)
HOOK_A(select_event_flags)
HOOK(op_19)
HOOK(op_20_test_flag_045C_of_object)
HOOK_A(test_flag_045C)
HOOK(op_53_set_flag_045C)
HOOK(op_21_set_flag_045C_of_object)
HOOK_A(set_flag_045C)
HOOK(op_22_clear_flag_045C_of_object)
HOOK_A(clear_flag_045C)
HOOK_A(select_flags_045C)
HOOK(op_23_test_flag_047A_of_object)
HOOK_A(test_flag_047A)
HOOK(op_54_set_flag_047A)
HOOK(op_24_set_flag_047A_of_object)
HOOK_A(set_flag_047A)
HOOK(op_25_clear_flag_047A_of_object)
HOOK_A(select_flags_047A)
HOOK(next_hotspot)
HOOK(op_68)
HOOK(op_67)
HOOK(op_64)
HOOK(op_63)
HOOK(show_found_page_next)
HOOK(op_61)
HOOK(op_60)
HOOK(op_5F)
HOOK(op_31)
HOOK(choice_is_table_2)
HOOK(op_32)
HOOK(op_32_body)
HOOK(op_5B)
HOOK_A(close_container_entry)
HOOK(close_all_containers)
HOOK(clear_listed_story_flags)
HOOK(op_06)
HOOK_A(enter_room)
HOOK(op_33)
HOOK(op_36)
HOOK(op_1A)
HOOK(op_34)
HOOK(op_37)
HOOK(op_1B)
HOOK(op_1D)
HOOK(op_1D_body)
HOOK(take_item_from_container)
HOOK(find_item_in_container)
HOOK(select_store_slot)
HOOK_A(wait_frames)
HOOK(op_1E)
HOOK(op_1F)
HOOK(op_26)
HOOK(open_container)
HOOK(load_container_view)
HOOK(select_item_list)
HOOK(fill_coat_from_item_list)
HOOK(refresh_coat_page)
HOOK(item_list_entry_visible)
HOOK(item_list_entry_condition)
HOOK(clear_coat_slots)
HOOK(copy_container_to_coat)
HOOK(op_27)
HOOK(op_2A)
HOOK(op_35)
HOOK(op_38)
HOOK(op_39)
HOOK(add_coins)
HOOK(op_62)
HOOK(add_coins_and_show)
HOOK(op_56)
HOOK(op_3A)
HOOK(op_57)
HOOK(op_3B)
HOOK(op_3C)
HOOK(op_3D)
HOOK(op_3E)
HOOK(op_40)
HOOK(show_found_page)
HOOK(op_3F)
HOOK(op_41)
HOOK(op_44)
HOOK(op_45)
HOOK(op_43)
HOOK(op_42)
HOOK(op_46_special_scene)
HOOK(op_47)
HOOK(close_container)
HOOK(op_49)
HOOK(previous_store_entry)
HOOK(op_4A)
HOOK(op_4B)
HOOK(op_4C)
HOOK(op_4D)
HOOK(op_4E)
HOOK(op_4F)
HOOK(op_50)
HOOK(op_51)
HOOK(op_58)
HOOK(op_59)
HOOK(op_55)
HOOK_A(find_and_show_item)
HOOK(find_in_goods)
HOOK(show_item_page_keep_cursor)
HOOK(restore_inventory_cursor)
HOOK_A(show_page_then_op_49)
HOOK(op_5A)
HOOK(op_5C)
HOOK(op_5E)
HOOK(op_5E_special_check)
HOOK(op_5E_special_check_body)
HOOK(op_48_yield_b5_80)
HOOK(far_8000_6D)
HOOK(far_8000_63)
HOOK(special_command_room_1E)
HOOK(special_command_by_room)
HOOK(special_command_room_20)
HOOK(special_command_room_24)
HOOK(special_command_rooms_4A)
HOOK(special_command_other_rooms)
HOOK(room_scene_list)
HOOK(clear_selection_unless_waiting)
HOOK(cursor_in_rects_933D)
HOOK(on_page_04B0)
HOOK(far_8040_64)
HOOK(select_room_scene)
HOOK(pick_room_variant)
HOOK(far_8000_66)
HOOK(step_address_choice)
HOOK(redraw_cursor_and_sprites)
HOOK(far_8000_6F)
HOOK(far_8000_6C)
HOOK(far_8000_67)
HOOK(far_8000_6E)
HOOK(far_8020_6D)
HOOK(far_8000_64)
HOOK(previous_page)
HOOK(draw_page_strings)
HOOK(far_8020_67)
HOOK(next_page)
HOOK(draw_label_string)
HOOK(draw_room_tile_patches)
HOOK(tile_patch_hidden)
HOOK(tile_patch_hidden_alt)
HOOK(tile_patch_room_rules)
HOOK_A(tile_patch_vram_address)
HOOK_A(draw_object_sprite)
HOOK(add_object_sprites)
HOOK(next_object_sprite_record)
HOOK(object_sprite_visible)
HOOK(append_object_sprite)
HOOK(sprite_value_is_0A_or_16)
HOOK(sprite_value_is_2C)
HOOK(sprite_value_is_0_or_2)
HOOK(draw_object_sprites)
HOOK(script_slot_for_command)
HOOK(script_table_is_0)
HOOK(far_8020_64)
HOOK(special_room_response)
HOOK(script_table_is_0_b)
HOOK(store_in_inventory_slot)
HOOK(item_code_in_list_AC53)
HOOK(item_number_for_object)
HOOK(kind_is_3)
HOOK(check_inventory_slot_empty)
HOOK(inventory_slot_index)
HOOK(draw_inventory_page)
HOOK(draw_address_book)
HOOK(draw_inventory_entry)
HOOK(far_8020_6E)
HOOK(inventory_page_header)
HOOK(far_8020_69)
HOOK(delete_inventory_entry)
HOOK(far_8020_6C)
HOOK(play_slot_machine)
HOOK(slot_machine_payout)
HOOK(spin_all_reels)
HOOK(stop_reels)
HOOK(spin_reel_0)
HOOK(spin_reel_1)
HOOK(spin_reel_2)
HOOK(draw_reel_0)
HOOK(draw_reel_1)
HOOK(draw_reel_2)
HOOK(wait_7_frames)
HOOK(opening_logos)
HOOK(black_palette_buffer)
HOOK(wait_30_frames_b6)
HOOK(logo_palette_cycle_1)
HOOK(logo_palette_cycle_2)
HOOK(wait_6_frames)
HOOK(draw_logo_screen)
HOOK(ending_sequence)
HOOK(ending_wait_button)
HOOK(wait_6_frames_2)
HOOK(ending_setup)
HOOK(draw_ending_box)
HOOK(draw_ending_screen)
HOOK(title_screen)
HOOK(draw_title_picture)
HOOK(file_select_menu)
HOOK(file_select_screen)
HOOK(draw_file_list)
HOOK(draw_file_entry)
HOOK(file_entry_setup)
HOOK(draw_file_status)
HOOK(draw_file_options)
HOOK(clear_file_area)
HOOK(fill_file_area)
HOOK(draw_file_cursor)
HOOK(draw_option_cursor)
HOOK(show_file_options)
HOOK(quit_dialog)
HOOK(save_dialog)
HOOK(open_confirm_dialog)
HOOK(draw_dialog)
HOOK(draw_dialog_text)
HOOK(reset_dialog_answer)
HOOK(draw_dialog_cursor)
HOOK(confirm_dialog_loop)
HOOK(draw_dialog_screen)
HOOK(click_picture)
HOOK(find_clicked_hotspot)
HOOK(pick_default_command)
HOOK(collect_room_sprites)
HOOK(item_code_for_object)
HOOK(wait_180_frames)
HOOK(wait_60_frames)
HOOK(wait_30_frames)
HOOK(run_special_scene)
HOOK(enter_special_scene)
HOOK(show_special_scene)
HOOK(leave_special_scene)
HOOK(special_scene_action)
HOOK(special_scene_with_message)
HOOK(special_scene_final)
HOOK(set_special_scene_flag)
HOOK(clear_special_scene_flag)
HOOK(special_scene_message)
HOOK(special_scene_extras)
HOOK(load_game)
HOOK(save_game)
HOOK(erase_save_file)
HOOK(check_all_save_files)
HOOK(check_save_file)
HOOK(repair_save_copies)
HOOK(read_save_copy)
HOOK(write_save_copy)
HOOK(save_copy_address)
HOOK(shift_save_copies)
HOOK(next_copy_pointer)
HOOK(copy_save_copy)
HOOK(erase_save_copies)
HOOK(erase_save_copy)
HOOK(clear_sum16)
HOOK(store_checksum)
HOOK(check_checksum)
HOOK(check_signature)
HOOK(store_signature)
HOOK(unlock_save_ram)
HOOK(protect_save_ram)
HOOK(disable_save_ram)
HOOK(reset_game_state)
HOOK(backup_state_to_save_ram)
HOOK(restore_state_backup)
HOOK(layout_run)
HOOK(layout_reset_state)
HOOK(layout_fetch)
HOOK(layout_advance_1)
HOOK(layout_advance_2)
HOOK(layout_advance_3)
HOOK(layout_put_tile)
HOOK(layout_command)
HOOK(layout_name_by_index_command)
HOOK(layout_name_at_address_command)
HOOK(layout_name_in_ram_command)
HOOK(layout_byte_padded_command)
HOOK(layout_word_padded_command)
HOOK(layout_byte_command)
HOOK(layout_word_command)
HOOK(layout_position_command)
HOOK(layout_name_from_ram)
HOOK(layout_name_from_index)
HOOK(layout_name_from_address)
HOOK(copy_name_to_buffer)
HOOK(layout_print_name_buffer)
HOOK(layout_advance_x)
HOOK(layout_set_vram_dest)
HOOK(layout_skip_5)
HOOK(layout_skip_3)
HOOK(layout_draw_tile)
HOOK(layout_tile_position)
HOOK(tile_position_to_vram)
HOOK(layout_tile_kind)
HOOK(queue_tile)
HOOK(layout_print_byte_padded)
HOOK(layout_print_byte)
HOOK(layout_print_word_padded)
HOOK(layout_print_word)
HOOK(load_byte_value)
HOOK(load_word_value)
HOOK(decimal_digits)
HOOK(trim_leading_zeros)
HOOK(digits_to_tiles_padded)
HOOK(digits_to_tiles)
HOOK(print_decimal_digits)
HOOK(draw_metasprite)
HOOK(draw_game_screen)
HOOK(update_cursor_and_sprites)
HOOK(load_current_scene)
HOOK(show_intro_once)
HOOK(new_game)
HOOK(next_random)
HOOK_A(split_item_code)
HOOK_A(add_to_sum16)
HOOK_A(draw_layout_number)

// Each original's code runs from its address to the next routine's (the end given).
#define ENTRY(name, addr, end) {#name, addr, end, 7, hook_##name}
#define INLINE(name, addr, end) {#name, addr, end, 7, NULL}
// In a switched bank: a hook there only runs while that bank is mapped.
#define BANKED(name, addr, end, bank) {#name, addr, end, bank, hook_##name}
#define BANKED_INLINE(name, addr, end, bank) {#name, addr, end, bank, NULL}

const EmuHook kPortedRoutines[] = {
  ENTRY(change_scene, kChangeScene, kChangeSceneInstant),
  ENTRY(change_scene_instant, kChangeSceneInstant, kLoadScene),
  ENTRY(load_scene, kLoadScene, kSelectSceneBank),
  ENTRY(select_scene_bank, kSelectSceneBank, kScenePointer),
  ENTRY(scene_pointer, kScenePointer, kDrawSceneDissolve),
  ENTRY(draw_scene_dissolve, kDrawSceneDissolve, kDissolveDrawTile),
  ENTRY(dissolve_draw_tile, kDissolveDrawTile, kDissolveTileValue),
  ENTRY(dissolve_tile_value, kDissolveTileValue, kDissolveTileAddress),
  ENTRY(dissolve_tile_address, kDissolveTileAddress, kLoadSceneAttrs),
  ENTRY(load_scene_attrs, kLoadSceneAttrs, kSceneAttrPositions),
  ENTRY(blacken_palette_colors, kBlackenPaletteColors, kDrawScene),
  ENTRY(draw_scene, kDrawScene, kCopySceneTiles),
  ENTRY(copy_scene_tiles, kCopySceneTiles, kApplyRoomSettings),
  ENTRY(apply_room_settings, kApplyRoomSettings, kLoadDissolvePattern),
  ENTRY(load_dissolve_pattern, kLoadDissolvePattern, kRoomSettings),
  ENTRY(test_hotspot, kTestHotspot, kCursorInHotspot),
  ENTRY(cursor_in_hotspot, kCursorInHotspot, kClickPicture),
  ENTRY(run_command, kRunCommand, kScriptResume),
  ENTRY(script_resume, kScriptResume, kScriptStart),
  ENTRY(script_start, kScriptStart, kScriptLoop),
  ENTRY(script_loop, kScriptLoop, kScriptFetchByte),
  ENTRY(script_fetch_byte, kScriptFetchByte, kScriptAdvance),
  ENTRY(script_advance, kScriptAdvance, kScriptFetch2),
  ENTRY(script_fetch_2, kScriptFetch2, kScriptFetch3),
  ENTRY(script_fetch_4, kScriptFetch4, kScriptDispatch),
  INLINE(script_dispatch, kScriptDispatch, kScriptOpTable),
  ENTRY(script_op_switch, kScriptOpSwitch, kScriptOpYield),
  ENTRY(script_op_yield, kScriptOpYield, kOp31),
  ENTRY(op_00_message_5, kOp00Message5, kOp01Message4),
  ENTRY(op_01_message_4, kOp01Message4, kOp02Message2Or5),
  ENTRY(op_02_message_2_or_5, kOp02Message2Or5, kScriptOpGotoIf),
  ENTRY(script_op_goto_if, kScriptOpGotoIf, kScriptOpGotoIfNot),
  ENTRY(script_op_goto_if_not, kScriptOpGotoIfNot, kScriptGoto),
  ENTRY(script_goto, kScriptGoto, kScriptOpPushGoto),
  ENTRY(script_op_push_goto, kScriptOpPushGoto, kScriptOpPopGoto),
  ENTRY(script_op_pop_goto, kScriptOpPopGoto, kScriptOpGoto),
  ENTRY(script_op_goto, kScriptOpGoto, kScriptConditionTrue),
  ENTRY(script_condition_true, kScriptConditionTrue, kScriptConditionFalse),
  ENTRY(script_condition_false, kScriptConditionFalse, kCloseContainerEntry),
  ENTRY(op_52_test_story_flag, kOp52TestStoryFlag, kOp07TestStoryFlagOfObject),
  ENTRY(op_07_test_story_flag_of_object, kOp07TestStoryFlagOfObject, kTestStoryFlag),
  ENTRY(test_story_flag, kTestStoryFlag, kOp08SetStoryFlagOfObject),
  ENTRY(op_08_set_story_flag_of_object, kOp08SetStoryFlagOfObject, kSetStoryFlagRefresh),
  ENTRY(set_story_flag_refresh, kSetStoryFlagRefresh, kOp09ClearStoryFlagOfObject),
  ENTRY(op_09_clear_story_flag_of_object, kOp09ClearStoryFlagOfObject, kClearStoryFlag),
  ENTRY(clear_story_flag, kClearStoryFlag, kSelectStoryFlags),
  ENTRY(select_story_flags, kSelectStoryFlags, kTestFlagBit),
  ENTRY(test_flag_bit, kTestFlagBit, kLoadFlagByte),
  ENTRY(load_flag_byte, kLoadFlagByte, kOp5D),
  ENTRY(op_5D, kOp5D, kOp0A),
  ENTRY(op_0A, kOp0A, kTestItemObtained),
  ENTRY(test_item_obtained, kTestItemObtained, kOp0B),
  ENTRY(op_0B, kOp0B, kOp0C),
  ENTRY(op_0C, kOp0C, kClearItemObtained),
  ENTRY(clear_item_obtained, kClearItemObtained, kOp0D),
  ENTRY(op_0D, kOp0D, kOp2B),
  ENTRY(op_2B, kOp2B, kTestObjectOpen),
  ENTRY(test_object_open, kTestObjectOpen, kOp0E),
  ENTRY(op_0E, kOp0E, kOp2C),
  ENTRY(op_2C, kOp2C, kSetObjectOpen),
  ENTRY(set_object_open, kSetObjectOpen, kOp0F),
  ENTRY(op_0F, kOp0F, kOp2D),
  ENTRY(op_2D, kOp2D, kCloseObject),
  ENTRY(close_object, kCloseObject, kOp10),
  ENTRY(op_10, kOp10, kOp11),
  ENTRY(op_11, kOp11, kOp12),
  ENTRY(op_12, kOp12, kClearItemOpen),
  ENTRY(clear_item_open, kClearItemOpen, kOp13TestDoorFlagOfObject),
  ENTRY(op_13_test_door_flag_of_object, kOp13TestDoorFlagOfObject, kOp2ETestDoorFlag),
  ENTRY(op_2E_test_door_flag, kOp2ETestDoorFlag, kTestDoorFlag),
  ENTRY(test_door_flag, kTestDoorFlag, kOp14SetDoorFlagOfObject),
  ENTRY(op_14_set_door_flag_of_object, kOp14SetDoorFlagOfObject, kOp2FSetDoorFlag),
  ENTRY(op_2F_set_door_flag, kOp2FSetDoorFlag, kSetDoorFlag),
  ENTRY(set_door_flag, kSetDoorFlag, kOp15ClearDoorFlagOfObject),
  ENTRY(op_15_clear_door_flag_of_object, kOp15ClearDoorFlagOfObject, kOp30ClearDoorFlag),
  ENTRY(op_30_clear_door_flag, kOp30ClearDoorFlag, kClearDoorFlag),
  ENTRY(clear_door_flag, kClearDoorFlag, kSelectDoorFlags),
  ENTRY(select_door_flags, kSelectDoorFlags, kOp16TestEventFlag),
  ENTRY(op_16_test_event_flag, kOp16TestEventFlag, kTestEventFlag),
  ENTRY(test_event_flag, kTestEventFlag, kOp17SetEventFlag),
  ENTRY(op_17_set_event_flag, kOp17SetEventFlag, kSetEventFlag),
  ENTRY(set_event_flag, kSetEventFlag, kOp18ClearEventFlag),
  ENTRY(op_18_clear_event_flag, kOp18ClearEventFlag, kClearEventFlag),
  ENTRY(clear_event_flag, kClearEventFlag, kSelectEventFlags),
  ENTRY(select_event_flags, kSelectEventFlags, kUnreferencedD2DB),
  ENTRY(op_19, kOp19, kOp33),
  ENTRY(op_20_test_flag_045C_of_object, kOp20TestFlag045COfObject, kTestFlag045C),
  ENTRY(test_flag_045C, kTestFlag045C, kOp53SetFlag045C),
  ENTRY(op_53_set_flag_045C, kOp53SetFlag045C, kOp21SetFlag045COfObject),
  ENTRY(op_21_set_flag_045C_of_object, kOp21SetFlag045COfObject, kSetFlag045C),
  ENTRY(set_flag_045C, kSetFlag045C, kOp22ClearFlag045COfObject),
  ENTRY(op_22_clear_flag_045C_of_object, kOp22ClearFlag045COfObject, kClearFlag045C),
  ENTRY(clear_flag_045C, kClearFlag045C, kSelectFlags045C),
  ENTRY(select_flags_045C, kSelectFlags045C, kOp23TestFlag047AOfObject),
  ENTRY(op_23_test_flag_047A_of_object, kOp23TestFlag047AOfObject, kTestFlag047A),
  ENTRY(test_flag_047A, kTestFlag047A, kOp54SetFlag047A),
  ENTRY(op_54_set_flag_047A, kOp54SetFlag047A, kOp24SetFlag047AOfObject),
  ENTRY(op_24_set_flag_047A_of_object, kOp24SetFlag047AOfObject, kSetFlag047A),
  ENTRY(set_flag_047A, kSetFlag047A, kOp25ClearFlag047AOfObject),
  ENTRY(op_25_clear_flag_047A_of_object, kOp25ClearFlag047AOfObject, kSelectFlags047A),
  ENTRY(select_flags_047A, kSelectFlags047A, kOp26),
  ENTRY(op_48_yield_b5_80, kOp48YieldB580, kOp49),
  ENTRY(next_hotspot, kNextHotspot, kHotspotChecks),
  ENTRY(op_68, kOp68, kOp67),
  ENTRY(op_67, kOp67, kOp64),
  ENTRY(op_64, kOp64, kOp63),
  ENTRY(op_63, kOp63, kShowFoundPageNext),
  ENTRY(show_found_page_next, kShowFoundPageNext, kOp61),
  ENTRY(op_61, kOp61, kOp60),
  ENTRY(op_60, kOp60, kOp5F),
  ENTRY(op_5F, kOp5F, kScriptOpSwitch),
  ENTRY(op_31, kOp31, kChoiceIsTable2),
  ENTRY(choice_is_table_2, kChoiceIsTable2, kOp32),
  ENTRY(op_32, kOp32, kOp32Body),
  ENTRY(op_32_body, kOp32Body, kOp5B),
  ENTRY(op_5B, kOp5B, kOp32BodyEnd),
  INLINE(op_32_body_end, kOp32BodyEnd, kOp00Message5),
  ENTRY(close_container_entry, kCloseContainerEntry, kCloseAllContainers),
  ENTRY(close_all_containers, kCloseAllContainers, kClearListedStoryFlags),
  ENTRY(clear_listed_story_flags, kClearListedStoryFlags, kOp06),
  ENTRY(op_06, kOp06, kEnterRoom),
  ENTRY(enter_room, kEnterRoom, kOp52TestStoryFlag),
  ENTRY(op_33, kOp33, kOp36),
  ENTRY(op_36, kOp36, kOp1A),
  ENTRY(op_1A, kOp1A, kOp34),
  ENTRY(op_34, kOp34, kOp37),
  ENTRY(op_37, kOp37, kOp1B),
  ENTRY(op_1B, kOp1B, kOp1D),
  ENTRY(op_1D, kOp1D, kOp1DBody),
  ENTRY(op_1D_body, kOp1DBody, kTakeItemFromContainer),
  ENTRY(take_item_from_container, kTakeItemFromContainer, kFindItemInContainer),
  ENTRY(find_item_in_container, kFindItemInContainer, kSelectStoreSlot),
  ENTRY(select_store_slot, kSelectStoreSlot, kWaitFrames),
  ENTRY(wait_frames, kWaitFrames, kOp1E),
  ENTRY(op_1E, kOp1E, kOp1F),
  ENTRY(op_1F, kOp1F, kOp20TestFlag045COfObject),
  ENTRY(op_26, kOp26, kOpenContainer),
  ENTRY(open_container, kOpenContainer, kLoadContainerView),
  ENTRY(load_container_view, kLoadContainerView, kSelectItemList),
  ENTRY(select_item_list, kSelectItemList, kFillCoatFromItemList),
  ENTRY(fill_coat_from_item_list, kFillCoatFromItemList, kRefreshCoatPage),
  ENTRY(refresh_coat_page, kRefreshCoatPage, kItemListEntryVisible),
  ENTRY(item_list_entry_visible, kItemListEntryVisible, kItemListEntryCondition),
  ENTRY(item_list_entry_condition, kItemListEntryCondition, kClearCoatSlots),
  ENTRY(clear_coat_slots, kClearCoatSlots, kCopyContainerToCoat),
  ENTRY(copy_container_to_coat, kCopyContainerToCoat, kOp27),
  ENTRY(op_27, kOp27, kOp2A),
  ENTRY(op_2A, kOp2A, kOp35),
  ENTRY(op_35, kOp35, kOp38),
  ENTRY(op_38, kOp38, kOp39),
  ENTRY(op_39, kOp39, kAddCoins),
  ENTRY(add_coins, kAddCoins, kOp62),
  ENTRY(op_62, kOp62, kAddCoinsAndShow),
  ENTRY(add_coins_and_show, kAddCoinsAndShow, kOp56),
  ENTRY(op_56, kOp56, kOp3A),
  ENTRY(op_3A, kOp3A, kOp57),
  ENTRY(op_57, kOp57, kOp3B),
  ENTRY(op_3B, kOp3B, kOp3C),
  ENTRY(op_3C, kOp3C, kOp3D),
  ENTRY(op_3D, kOp3D, kOp3E),
  ENTRY(op_3E, kOp3E, kOp40),
  ENTRY(op_40, kOp40, kShowFoundPage),
  ENTRY(show_found_page, kShowFoundPage, kOp3F),
  ENTRY(op_3F, kOp3F, kOp41),
  ENTRY(op_41, kOp41, kOp44),
  ENTRY(op_44, kOp44, kOp45),
  ENTRY(op_45, kOp45, kOp43),
  ENTRY(op_43, kOp43, kOp42),
  ENTRY(op_42, kOp42, kOp46SpecialScene),
  ENTRY(op_46_special_scene, kOp46SpecialScene, kOp47),
  ENTRY(op_47, kOp47, kCloseContainer),
  ENTRY(close_container, kCloseContainer, kOp48YieldB580),
  ENTRY(op_49, kOp49, kPreviousStoreEntry),
  ENTRY(previous_store_entry, kPreviousStoreEntry, kOp4A),
  ENTRY(op_4A, kOp4A, kOp4B),
  ENTRY(op_4B, kOp4B, kOp4C),
  ENTRY(op_4C, kOp4C, kOp4D),
  ENTRY(op_4D, kOp4D, kOp4E),
  ENTRY(op_4E, kOp4E, kOp4F),
  ENTRY(op_4F, kOp4F, kOp50),
  ENTRY(op_50, kOp50, kOp51),
  ENTRY(op_51, kOp51, kOp58),
  ENTRY(op_58, kOp58, kOp59),
  ENTRY(op_59, kOp59, kOp55),
  ENTRY(op_55, kOp55, kFindAndShowItem),
  ENTRY(find_and_show_item, kFindAndShowItem, kFindInGoods),
  ENTRY(find_in_goods, kFindInGoods, kShowItemPageKeepCursor),
  ENTRY(show_item_page_keep_cursor, kShowItemPageKeepCursor, kRestoreInventoryCursor),
  ENTRY(restore_inventory_cursor, kRestoreInventoryCursor, kShowPageThenOp49),
  ENTRY(show_page_then_op_49, kShowPageThenOp49, kOp5A),
  ENTRY(op_5A, kOp5A, kOp5C),
  ENTRY(op_5C, kOp5C, kOp5E),
  ENTRY(op_5E, kOp5E, kOp5ESpecialCheck),
  ENTRY(op_5E_special_check, kOp5ESpecialCheck, kOp5ESpecialCheckBody),
  ENTRY(op_5E_special_check_body, kOp5ESpecialCheckBody, kEncounterRooms),
  ENTRY(print_message_5, kPrintMessage5, kPrintMessageFrom),
  ENTRY(print_message_from, kPrintMessageFrom, kPrintMessage4),
  ENTRY(print_message_4, kPrintMessage4, kPrintMessage2Or5),
  ENTRY(print_message_2_or_5, kPrintMessage2Or5, kItemCodeForObject),
  ENTRY(text_read_code, kTextReadCode, kTextReadByte),
  ENTRY(text_read_byte, kTextReadByte, kTextReadBit),
  ENTRY(text_read_bit, kTextReadBit, kPrintMessage),
  ENTRY(print_message, kPrintMessage, kPrintControl),
  ENTRY(print_control, kPrintControl, kPrintItemNameAt),
  ENTRY(print_item_name_at, kPrintItemNameAt, kPrintItemNameArg),
  ENTRY(print_item_name_arg, kPrintItemNameArg, kPrintWordArg),
  ENTRY(print_word_arg, kPrintWordArg, kPrintName),
  ENTRY(print_name, kPrintName, kPrintNumberAt),
  ENTRY(print_number_at, kPrintNumberAt, kTextWaitPage),
  ENTRY(text_wait_page, kTextWaitPage, kTextNewline),
  ENTRY(text_newline, kTextNewline, kPrintClear),
  ENTRY(print_clear, kPrintClear, kPrintDigitSkipZero),
  ENTRY(print_digit_skip_zero, kPrintDigitSkipZero, kPrintDigit),
  ENTRY(print_digit, kPrintDigit, kNumberToDigits),
  ENTRY(number_to_digits, kNumberToDigits, kReadNumberArg),
  ENTRY(read_number_arg, kReadNumberArg, kSkipInlineWord),
  ENTRY(skip_inline_word, kSkipInlineWord, kClearTextWindow),
  ENTRY(clear_text_window, kClearTextWindow, kBlankTextWindow),
  ENTRY(blank_text_window, kBlankTextWindow, kPrintTile),
  ENTRY(print_tile, kPrintTile, kPrintChar),
  ENTRY(print_char, kPrintChar, kTextTileTable),
  ENTRY(put_tile, kPutTile, kTextLineFeed),
  ENTRY(text_line_feed, kTextLineFeed, kTextScroll),
  ENTRY(text_scroll, kTextScroll, kTextScrollRows),
  ENTRY(text_scroll_rows, kTextScrollRows, kTextCursorVramAddress),
  ENTRY(text_cursor_vram_address, kTextCursorVramAddress, kUploadTextWindow),
  ENTRY(upload_text_window, kUploadTextWindow, kDrawWindowRuleRow),
  ENTRY(draw_window_rule_row, kDrawWindowRuleRow, kBlankWindowTextRows),
  ENTRY(blank_window_text_rows, kBlankWindowTextRows, kDrawTextWindowSides),
  ENTRY(draw_text_window_sides, kDrawTextWindowSides, kDrawWindowSideColumn),
  ENTRY(draw_window_side_column, kDrawWindowSideColumn, kTextPause),
  ENTRY(text_pause, kTextPause, kWaitPageInput),
  ENTRY(wait_page_input, kWaitPageInput, kBlinkMoreArrow),
  ENTRY(blink_more_arrow, kBlinkMoreArrow, kShowMoreArrow),
  ENTRY(show_more_arrow, kShowMoreArrow, kWaitForMoreButton),
  ENTRY(wait_for_more_button, kWaitForMoreButton, kDrawMoreArrowFrame),
  ENTRY(draw_more_arrow_frame, kDrawMoreArrowFrame, kTextCursorXPixels),
  ENTRY(text_cursor_x_pixels, kTextCursorXPixels, kDrawMoreArrow),
  ENTRY(draw_more_arrow, kDrawMoreArrow, kDrawMoreArrowEnd),
  ENTRY(main, kMain, kDrawGameScreen),
  ENTRY(set_mirroring_vertical, kSetMirroringVertical, kSetMirroringHorizontal),
  ENTRY(switch_prg_bank, kSwitchPrgBank, kApplyChrBanksR0R1),
  ENTRY(apply_chr_banks_r0_r1, kApplyChrBanksR0R1, kApplyChrBanksR2R5),
  ENTRY(apply_chr_banks_r2_r5, kApplyChrBanksR2R5, kMmc3SetBank),
  ENTRY(mmc3_set_bank, kMmc3SetBank, kSetChrBanksInline),
  ENTRY(set_chr_banks_inline, kSetChrBanksInline, kApplyChrBanks),
  ENTRY(apply_chr_banks, kApplyChrBanks, kAdvanceInlinePtr),
  ENTRY(advance_inline_ptr, kAdvanceInlinePtr, kDrawRleScreen2400),
  ENTRY(draw_rle_screen_2000, kDrawRleScreen2000, kRleToPpu2000),
  ENTRY(rle_to_ppu_2000, kRleToPpu2000, kRleWrite2000),
  ENTRY(rle_write_2000, kRleWrite2000, kRleToPpu2400),
  ENTRY(read_inline_byte, kReadInlineByte, kPushAndSwitchBank),
  ENTRY(push_and_switch_bank, kPushAndSwitchBank, kSetCurrentBank),
  ENTRY(set_current_bank, kSetCurrentBank, kSwitchToSoundBank),
  ENTRY(switch_to_sound_bank, kSwitchToSoundBank, kRestoreCurrentBank),
  ENTRY(restore_current_bank, kRestoreCurrentBank, kPopBank),
  ENTRY(pop_bank, kPopBank, kStackPush),
  ENTRY(stack_push, kStackPush, kStackPop),
  ENTRY(stack_pop, kStackPop, kDisableNmi),
  ENTRY(disable_nmi, kDisableNmi, kEnableNmi),
  ENTRY(enable_nmi, kEnableNmi, kIrq),
  ENTRY(read_joypads, kReadJoypads, kReadJoypadsOnce),
  ENTRY(read_joypads_once, kReadJoypadsOnce, kWaitVblank),
  ENTRY(wait_vblank, kWaitVblank, kRequestNmiWork),
  ENTRY(request_nmi_work, kRequestNmiWork, kWaitVramQueueEmpty),
  ENTRY(wait_vram_queue_empty, kWaitVramQueueEmpty, kWaitNmiWorkDone),
  ENTRY(wait_nmi_work_done, kWaitNmiWorkDone, kWaitVblankAndHideSprites),
  ENTRY(wait_vblank_and_hide_sprites, kWaitVblankAndHideSprites, kClearOamBuffer),
  ENTRY(clear_oam_buffer, kClearOamBuffer, kClearAndUploadSprites),
  ENTRY(clear_and_upload_sprites, kClearAndUploadSprites, kRequestOamDma),
  ENTRY(request_oam_dma, kRequestOamDma, kJumpTable),
  ENTRY(jump_table, kJumpTable, kFarCall8000Body),
  INLINE(far_call_8000_body, kFarCall8000Body, kFarCall8020Body),
  INLINE(far_call_8020_body, kFarCall8020Body, kFarCall8040Body),
  INLINE(far_call_8040_body, kFarCall8040Body, kFarCall8400Body),
  INLINE(far_call_8400_body, kFarCall8400Body, kFarCallJump),
  INLINE(far_call_jump, kFarCallJump, kNmi),
  ENTRY(nmi, kNmi, kNmiPpuUpdates),
  ENTRY(nmi_ppu_updates, kNmiPpuUpdates, kNmiUploadAttrs2000),
  ENTRY(nmi_upload_attrs_2000, kNmiUploadAttrs2000, kNmiUploadAttrs2nd),
  ENTRY(nmi_upload_attrs_2nd, kNmiUploadAttrs2nd, kNmiFlushVramQueue),
  ENTRY(nmi_flush_vram_queue, kNmiFlushVramQueue, kNmiFlushVramQueueDown),
  ENTRY(nmi_flush_vram_queue_down, kNmiFlushVramQueueDown, kFlushVramEntry),
  INLINE(flush_vram_entry, kFlushVramEntry, kFlushVramQueue),
  ENTRY(flush_vram_queue, kFlushVramQueue, kNmiBlackPalette),
  ENTRY(nmi_black_palette, kNmiBlackPalette, kNmiUploadPalette),
  ENTRY(nmi_upload_palette, kNmiUploadPalette, kPpuAddrLeavePalette),
  ENTRY(ppu_addr_leave_palette, kPpuAddrLeavePalette, kPpuAddrPalette),
  ENTRY(ppu_addr_palette, kPpuAddrPalette, kNmiSetScroll),
  ENTRY(nmi_set_scroll, kNmiSetScroll, kClearNametables),
  ENTRY(clear_nametables, kClearNametables, kRenderingOn),
  ENTRY(rendering_on, kRenderingOn, kClearNametable2000),
  ENTRY(rendering_off, kRenderingOff, kFillNametable2000),
  ENTRY(fill_nametable_2000, kFillNametable2000, kFillNametable2nd),
  ENTRY(fill_nametable_2nd, kFillNametable2nd, kRequestVramQueue),
  ENTRY(request_vram_queue, kRequestVramQueue, kQueueVramBlock),
  ENTRY(queue_vram_block, kQueueVramBlock, kQueueVramFill),
  ENTRY(queue_vram_fill, kQueueVramFill, kVramBlockSetup),
  ENTRY(vram_block_setup, kVramBlockSetup, kVramFillRows),
  ENTRY(vram_fill_rows, kVramFillRows, kVramCopyRows),
  ENTRY(vram_copy_rows, kVramCopyRows, kNextVramAddress),
  ENTRY(next_vram_address, kNextVramAddress, kVramWrapUp),
  ENTRY(vram_wrap_up, kVramWrapUp, kVramQueueHeader),
  ENTRY(vram_queue_header, kVramQueueHeader, kNextVramRow),
  ENTRY(next_vram_row, kNextVramRow, kSkipBlockHeader),
  ENTRY(skip_block_header, kSkipBlockHeader, kFlushVramQueueIfFull),
  ENTRY(flush_vram_queue_if_full, kFlushVramQueueIfFull, kFlushVramQueueDownIfFull),
  ENTRY(flush_vram_queue_down_if_full, kFlushVramQueueDownIfFull, kFlushVramQueueDown),
  ENTRY(flush_vram_queue_down, kFlushVramQueueDown, kQueueVramColumn),
  ENTRY(queue_vram_column, kQueueVramColumn, kVramColumnSetup),
  ENTRY(vram_column_setup, kVramColumnSetup, kVramCopyColumn),
  ENTRY(vram_copy_column, kVramCopyColumn, kVramAddrDownRow),
  ENTRY(vram_addr_down_row, kVramAddrDownRow, kVramWrapColumn),
  ENTRY(vram_wrap_column, kVramWrapColumn, kAttrPosition),
  ENTRY(attr_position, kAttrPosition, kSetAttrBlock),
  ENTRY(set_attr_block, kSetAttrBlock, kNextAttrRow),
  ENTRY(next_attr_row, kNextAttrRow, kNextAttrColumn),
  ENTRY(next_attr_column, kNextAttrColumn, kSetAttrBits),
  ENTRY(set_attr_bits, kSetAttrBits, kSetAttrBitsEnd),
  ENTRY(fill_all_attrs, kFillAllAttrs, kUnreferencedF21B),
  ENTRY(pick_attr_fill, kPickAttrFill, kPickAttrFillEnd),
  ENTRY(fill_attr_buffer_2000, kFillAttrBuffer2000, kFillAttrBuffer2400),
  ENTRY(fill_attr_buffer_2400, kFillAttrBuffer2400, kRestoreXy),
  ENTRY(restore_xy, kRestoreXy, kRequestAttrs2000),
  ENTRY(request_attrs_2000, kRequestAttrs2000, kRequestAttrs2nd),
  ENTRY(request_attr_upload, kRequestAttrUpload, kRequestBlackPalette),
  ENTRY(request_black_palette, kRequestBlackPalette, kShowBgAndSprites),
  ENTRY(show_bg_and_sprites, kShowBgAndSprites, kShowSprites),
  INLINE(show_sprites, kShowSprites, kHideBgAndSprites),
  ENTRY(hide_bg_and_sprites, kHideBgAndSprites, kHideSprites),
  INLINE(hide_sprites, kHideSprites, kRequestPaletteUpload),
  ENTRY(request_palette_upload, kRequestPaletteUpload, kFadeInPalette),
  ENTRY(fade_in_palette, kFadeInPalette, kPaletteBrightenStep),
  ENTRY(palette_brighten_step, kPaletteBrightenStep, kFadeOutPalette),
  ENTRY(fade_out_palette, kFadeOutPalette, kPaletteDarkenStep),
  ENTRY(palette_darken_step, kPaletteDarkenStep, kPaletteFadeWait),
  ENTRY(palette_fade_wait, kPaletteFadeWait, kUnreferencedF32F),
  ENTRY(far_call_8000, kFarCall8000, kFarCall8020),
  ENTRY(far_call_8020, kFarCall8020, kFarCall8040),
  ENTRY(far_call_8040, kFarCall8040, kFarCall8400),
  ENTRY(far_call_8400, kFarCall8400, kSoundReset),
  ENTRY(sound_reset, kSoundReset, kPlaySound),
  ENTRY(play_sound, kPlaySound, kSoundCheckRepeat),
  ENTRY(sound_check_repeat, kSoundCheckRepeat, kStartSound),
  ENTRY(start_sound, kStartSound, kSoundStartByType),
  ENTRY(sound_start_by_type, kSoundStartByType, kSoundStartMusic),
  ENTRY(sound_start_music, kSoundStartMusic, kSoundStartChannel),
  ENTRY(sound_start_channel, kSoundStartChannel, kSoundStartRepeatingMusic),
  ENTRY(sound_start_repeating_music, kSoundStartRepeatingMusic, kSoundStartEffect3ch),
  ENTRY(sound_start_effect_3ch, kSoundStartEffect3ch, kSoundStartEffect2ch),
  ENTRY(sound_start_effect_2ch, kSoundStartEffect2ch, kSoundUpdate),
  ENTRY(sound_update, kSoundUpdate, kSoundUpdateAll),
  ENTRY(sound_update_all, kSoundUpdateAll, kSoundStepAllChannels),
  ENTRY(sound_step_all_channels, kSoundStepAllChannels, kSoundStepPulse),
  ENTRY(sound_step_pulse, kSoundStepPulse, kSoundPulseEnvelope),
  ENTRY(sound_pulse_envelope, kSoundPulseEnvelope, kSoundStepTriangle),
  ENTRY(sound_step_triangle, kSoundStepTriangle, kMultiply8x8),
  ENTRY(multiply_8x8, kMultiply8x8, kSoundTriangleCountdown),
  ENTRY(sound_triangle_countdown, kSoundTriangleCountdown, kSoundStepNoise),
  ENTRY(sound_step_noise, kSoundStepNoise, kSoundTick),
  ENTRY(sound_tick, kSoundTick, kSoundAddLength),
  ENTRY(sound_add_length, kSoundAddLength, kSoundReadCommand),
  ENTRY(sound_read_command, kSoundReadCommand, kSoundLoopPush),
  ENTRY(sound_loop_push, kSoundLoopPush, kSoundLoopPop),
  ENTRY(sound_loop_pop, kSoundLoopPop, kSoundNoteOff),
  ENTRY(sound_note_off, kSoundNoteOff, kSoundChannelDone),
  ENTRY(sound_channel_done, kSoundChannelDone, kSoundWriteApu),
  ENTRY(sound_write_apu, kSoundWriteApu, kSoundJumpTable),
  INLINE(sound_jump_table, kSoundJumpTable, kNoteLengths),
  ENTRY(click_picture, kClickPicture, kFindClickedHotspot),
  ENTRY(find_clicked_hotspot, kFindClickedHotspot, kBitMasks),
  ENTRY(collect_room_sprites, kCollectRoomSprites, kPrintMessage5),
  ENTRY(item_code_for_object, kItemCodeForObject, kSplitItemCode),
  ENTRY(split_item_code, kSplitItemCode, kItemCodeTableNumbers),
  ENTRY(run_special_scene, kRunSpecialScene, kSpecialScenePictures),
  ENTRY(special_scene_action, kSpecialSceneAction, kSpecialSceneWithMessage),
  ENTRY(special_scene_with_message, kSpecialSceneWithMessage, kSpecialSceneFinal),
  ENTRY(special_scene_final, kSpecialSceneFinal, kSetSpecialSceneFlag),
  ENTRY(set_special_scene_flag, kSetSpecialSceneFlag, kClearSpecialSceneFlag),
  ENTRY(clear_special_scene_flag, kClearSpecialSceneFlag, kSpecialSceneFlags),
  ENTRY(special_scene_message, kSpecialSceneMessage, kSpecialSceneMessageIds),
  ENTRY(enter_special_scene, kEnterSpecialScene, kShowSpecialScene),
  ENTRY(show_special_scene, kShowSpecialScene, kLeaveSpecialScene),
  ENTRY(leave_special_scene, kLeaveSpecialScene, kSpecialSceneExtras),
  ENTRY(special_scene_extras, kSpecialSceneExtras, kWait180Frames),
  ENTRY(wait_180_frames, kWait180Frames, kWait60Frames),
  ENTRY(wait_60_frames, kWait60Frames, kWait30Frames),
  ENTRY(wait_30_frames, kWait30Frames, kLoadGame),
  ENTRY(load_game, kLoadGame, kReadSaveCopy),
  ENTRY(read_save_copy, kReadSaveCopy, kSaveGame),
  ENTRY(save_game, kSaveGame, kWriteSaveCopy),
  ENTRY(write_save_copy, kWriteSaveCopy, kEraseSaveFile),
  ENTRY(erase_save_file, kEraseSaveFile, kCheckAllSaveFiles),
  ENTRY(check_all_save_files, kCheckAllSaveFiles, kCheckSaveFile),
  ENTRY(check_save_file, kCheckSaveFile, kSaveFileBits),
  ENTRY(repair_save_copies, kRepairSaveCopies, kUnlockSaveRam),
  ENTRY(unlock_save_ram, kUnlockSaveRam, kProtectSaveRam),
  ENTRY(protect_save_ram, kProtectSaveRam, kDisableSaveRam),
  ENTRY(disable_save_ram, kDisableSaveRam, kSaveCopyAddress),
  ENTRY(save_copy_address, kSaveCopyAddress, kShiftSaveCopies),
  ENTRY(shift_save_copies, kShiftSaveCopies, kNextCopyPointer),
  ENTRY(next_copy_pointer, kNextCopyPointer, kCopySaveCopy),
  ENTRY(copy_save_copy, kCopySaveCopy, kEraseSaveCopies),
  ENTRY(erase_save_copies, kEraseSaveCopies, kEraseSaveCopy),
  ENTRY(erase_save_copy, kEraseSaveCopy, kClearSum16),
  ENTRY(clear_sum16, kClearSum16, kAddToSum16),
  ENTRY(add_to_sum16, kAddToSum16, kStoreChecksum),
  ENTRY(store_checksum, kStoreChecksum, kCheckChecksum),
  ENTRY(check_checksum, kCheckChecksum, kCheckSignature),
  ENTRY(check_signature, kCheckSignature, kSaveSignature),
  ENTRY(store_signature, kStoreSignature, kResetGameState),
  ENTRY(reset_game_state, kResetGameState, kBackupStateToSaveRam),
  ENTRY(backup_state_to_save_ram, kBackupStateToSaveRam, kRestoreStateBackup),
  ENTRY(restore_state_backup, kRestoreStateBackup, kPickDefaultCommand),
  ENTRY(pick_default_command, kPickDefaultCommand, kRunCommand),
  ENTRY(draw_game_screen, kDrawGameScreen, kUpdateCursorAndSprites),
  ENTRY(update_cursor_and_sprites, kUpdateCursorAndSprites, kLoadCurrentScene),
  ENTRY(load_current_scene, kLoadCurrentScene, kShowIntroOnce),
  ENTRY(show_intro_once, kShowIntroOnce, kNewGame),
  ENTRY(new_game, kNewGame, kNextRandom),
  ENTRY(next_random, kNextRandom, kNextRandomEnd),
  ENTRY(reset, kReset, kSetMirroringVertical),
  ENTRY(draw_layout_number, kDrawLayoutNumber, kLayoutRun),
  ENTRY(layout_run, kLayoutRun, kLayoutResetState),
  ENTRY(layout_reset_state, kLayoutResetState, kLayoutFetch),
  ENTRY(layout_fetch, kLayoutFetch, kLayoutAdvance1),
  ENTRY(layout_advance_1, kLayoutAdvance1, kLayoutAdvance2),
  ENTRY(layout_advance_2, kLayoutAdvance2, kLayoutAdvance3),
  ENTRY(layout_advance_3, kLayoutAdvance3, kLayoutPutTile),
  ENTRY(layout_put_tile, kLayoutPutTile, kLayoutCommand),
  ENTRY(layout_command, kLayoutCommand, kUnreferencedFCC8),
  ENTRY(layout_name_by_index_command, kLayoutNameByIndexCommand, kLayoutNameAtAddressCommand),
  ENTRY(layout_name_at_address_command, kLayoutNameAtAddressCommand, kLayoutNameInRamCommand),
  ENTRY(layout_name_in_ram_command, kLayoutNameInRamCommand, kLayoutNameFromRam),
  ENTRY(layout_name_from_ram, kLayoutNameFromRam, kLayoutNameFromIndex),
  ENTRY(layout_name_from_index, kLayoutNameFromIndex, kLayoutNameFromAddress),
  ENTRY(layout_name_from_address, kLayoutNameFromAddress, kCopyNameToBuffer),
  ENTRY(copy_name_to_buffer, kCopyNameToBuffer, kLayoutPrintNameBuffer),
  ENTRY(layout_print_name_buffer, kLayoutPrintNameBuffer, kLayoutBytePaddedCommand),
  ENTRY(layout_byte_padded_command, kLayoutBytePaddedCommand, kLayoutWordPaddedCommand),
  ENTRY(layout_word_padded_command, kLayoutWordPaddedCommand, kLayoutByteCommand),
  ENTRY(layout_byte_command, kLayoutByteCommand, kLayoutWordCommand),
  ENTRY(layout_word_command, kLayoutWordCommand, kLayoutAdvanceX),
  ENTRY(layout_advance_x, kLayoutAdvanceX, kLayoutSetVramDest),
  ENTRY(layout_set_vram_dest, kLayoutSetVramDest, kLayoutSkip5),
  ENTRY(layout_skip_5, kLayoutSkip5, kLayoutSkip3),
  ENTRY(layout_skip_3, kLayoutSkip3, kLayoutPositionCommand),
  ENTRY(layout_position_command, kLayoutPositionCommand, kLayoutDrawTile),
  ENTRY(layout_draw_tile, kLayoutDrawTile, kLayoutTilePosition),
  ENTRY(layout_tile_position, kLayoutTilePosition, kTilePositionToVram),
  ENTRY(tile_position_to_vram, kTilePositionToVram, kLayoutTileKind),
  ENTRY(layout_tile_kind, kLayoutTileKind, kQueueTile),
  ENTRY(queue_tile, kQueueTile, kDrawMetasprite),
  ENTRY(draw_metasprite, kDrawMetasprite, kLayoutPrintBytePadded),
  ENTRY(layout_print_byte_padded, kLayoutPrintBytePadded, kLayoutPrintByte),
  ENTRY(layout_print_byte, kLayoutPrintByte, kLayoutPrintWordPadded),
  ENTRY(layout_print_word_padded, kLayoutPrintWordPadded, kLayoutPrintWord),
  ENTRY(layout_print_word, kLayoutPrintWord, kLoadByteValue),
  ENTRY(load_byte_value, kLoadByteValue, kLoadWordValue),
  ENTRY(load_word_value, kLoadWordValue, kDecimalDigits),
  ENTRY(decimal_digits, kDecimalDigits, kTrimLeadingZeros),
  ENTRY(trim_leading_zeros, kTrimLeadingZeros, kDigitsToTilesPadded),
  ENTRY(digits_to_tiles_padded, kDigitsToTilesPadded, kDigitsToTiles),
  ENTRY(digits_to_tiles, kDigitsToTiles, kPrintDecimalDigits),
  ENTRY(print_decimal_digits, kPrintDecimalDigits, kPrintDecimalDigitsEnd),
  BANKED(load_scene_chr_banks, kLoadSceneChrBanks, kSceneChrBankTable, 6),
  BANKED(load_scene_palette, kLoadScenePalette, kLoadPalette, 6),
  BANKED(load_palette, kLoadPalette, kFar8000_63, 6),
  BANKED(palette_pointer, kPalettePointer, kScenePaletteIds, 6),
  BANKED(dissolve_row_mask, kDissolveRowMask, kDissolvePatterns, 6),
  BANKED(click_at_cursor, kClickAtCursor, kFindScreenArea, 6),
  BANKED(find_screen_area, kFindScreenArea, kNextScreenRect, 6),
  BANKED(next_screen_rect, kNextScreenRect, kScreenAreaRects, 6),
  BANKED(click_command_area, kClickCommandArea, kCommandRects, 6),
  BANKED(click_inventory_area, kClickInventoryArea, kEntryRects, 6),
  BANKED(reset_cursor, kResetCursor, kHandleInput, 6),
  BANKED(handle_input, kHandleInput, kGetInput, 6),
  BANKED(get_input, kGetInput, kMoveCursor, 6),
  BANKED(move_cursor, kMoveCursor, kReadInput, 6),
  BANKED(read_input, kReadInput, kReadInputFree, 6),
  BANKED(read_input_free, kReadInputFree, kReadInputWithRepeat, 6),
  BANKED(read_input_with_repeat, kReadInputWithRepeat, kResetKeyRepeat, 6),
  BANKED(reset_key_repeat, kResetKeyRepeat, kDpadToDelta, 6),
  BANKED(dpad_to_delta, kDpadToDelta, kDpadDx, 6),
  BANKED(move_cursor_in_scene, kMoveCursorInScene, kMoveCursorInInventory, 6),
  BANKED(move_cursor_in_inventory, kMoveCursorInInventory, kSnapCursorToCommand, 6),
  BANKED(snap_cursor_to_command, kSnapCursorToCommand, kCommandCursorXs, 6),
  BANKED(move_cursor_x, kMoveCursorX, kMoveCursorY, 6),
  BANKED(move_cursor_y, kMoveCursorY, kMoveCursorInCommands, 6),
  BANKED(move_cursor_in_commands, kMoveCursorInCommands, kCommandGridPosition, 6),
  BANKED(command_grid_position, kCommandGridPosition, kCommandGridXs, 6),
  BANKED(move_in_command_grid, kMoveInCommandGrid, kCommandAtGrid, 6),
  BANKED(command_at_grid, kCommandAtGrid, kCommandGrid, 6),
  BANKED(handle_buttons, kHandleButtons, kUnreferenced9847, 6),
  BANKED(press_b, kPressB, kPressA, 6),
  BANKED(press_a, kPressA, kDrawCursor, 6),
  BANKED(draw_cursor, kDrawCursor, kCursorSprite, 6),
  BANKED(highlight_command, kHighlightCommand, kDrawCommandHighlight, 6),
  BANKED(draw_command_highlight, kDrawCommandHighlight, kUnhighlightCommand, 6),
  BANKED(unhighlight_command, kUnhighlightCommand, kCommandVramAddress, 6),
  BANKED(command_vram_address, kCommandVramAddress, kClearEntryHighlight, 6),
  BANKED(clear_entry_highlight, kClearEntryHighlight, kHighlightEntry, 6),
  BANKED(highlight_entry, kHighlightEntry, kInventoryEntryVramAddress, 6),
  BANKED(inventory_entry_vram_address, kInventoryEntryVramAddress, kHighlightBlocks, 6),
  BANKED(far_8000_6D, kFar8000_6D, kSplitSprite, 6),
  BANKED(far_8000_63, kFar8000_63, kUnreferenced81FE, 6),
  BANKED(special_command_room_1E, kSpecialCommandRoom1E, kSpecialCommandByRoom, 6),
  BANKED(special_command_by_room, kSpecialCommandByRoom, kSpecialCommandRoom20, 6),
  BANKED(special_command_room_20, kSpecialCommandRoom20, kSpecialCommandRoom24, 6),
  BANKED(special_command_room_24, kSpecialCommandRoom24, kSpecialCommandRooms4A, 6),
  BANKED(special_command_rooms_4A, kSpecialCommandRooms4A, kSpecialCommandOtherRooms, 6),
  BANKED(special_command_other_rooms, kSpecialCommandOtherRooms, kRoomSceneList, 6),
  BANKED(room_scene_list, kRoomSceneList, kOtherRoomsRect, 6),
  BANKED(clear_selection_unless_waiting, kClearSelectionUnlessWaiting, kCursorInRects933D, 6),
  BANKED(cursor_in_rects_933D, kCursorInRects933D, kOnPage04B0, 6),
  BANKED(on_page_04B0, kOnPage04B0, kFar8040_64, 6),
  BANKED(far_8040_64, kFar8040_64, kArrowRects, 6),
  BANKED(select_room_scene, kSelectRoomScene, kPickRoomVariant, 6),
  BANKED(pick_room_variant, kPickRoomVariant, kRoomScenes, 6),
  BANKED(far_8000_66, kFar8000_66, kStepAddressChoice, 6),
  BANKED(step_address_choice, kStepAddressChoice, kRedrawCursorAndSprites, 6),
  BANKED(redraw_cursor_and_sprites, kRedrawCursorAndSprites, kPromptMessage2B, 6),
  BANKED(far_8000_6F, kFar8000_6F, kFar8000_6C, 6),
  BANKED(far_8000_6C, kFar8000_6C, kFar8000_67, 6),
  BANKED(far_8000_67, kFar8000_67, kFar8000_6E, 6),
  BANKED(far_8000_6E, kFar8000_6E, kFar8020_6D, 6),
  BANKED(far_8020_6D, kFar8020_6D, kFar8000_64, 6),
  BANKED(far_8000_64, kFar8000_64, kPreviousPage, 6),
  BANKED(previous_page, kPreviousPage, kDrawPageStrings, 6),
  BANKED(draw_page_strings, kDrawPageStrings, kPageLabelsBack, 6),
  BANKED(far_8020_67, kFar8020_67, kNextPage, 6),
  BANKED(next_page, kNextPage, kPageLabelsOn, 6),
  BANKED(draw_label_string, kDrawLabelString, kFixedLabels, 6),
  BANKED(draw_room_tile_patches, kDrawRoomTilePatches, kTilePatchHidden, 6),
  BANKED(tile_patch_hidden, kTilePatchHidden, kTilePatchHiddenAlt, 6),
  BANKED(tile_patch_hidden_alt, kTilePatchHiddenAlt, kTilePatchRoomRules, 6),
  BANKED(tile_patch_room_rules, kTilePatchRoomRules, kTilePatchVramAddress, 6),
  BANKED(tile_patch_vram_address, kTilePatchVramAddress, kTilePatchBlock, 6),
  BANKED(draw_object_sprite, kDrawObjectSprite, kObjectSpritesLow, 6),
  BANKED(add_object_sprites, kAddObjectSprites, kNextObjectSpriteRecord, 6),
  BANKED(next_object_sprite_record, kNextObjectSpriteRecord, kObjectSpriteTables, 6),
  BANKED(object_sprite_visible, kObjectSpriteVisible, kObjectSpriteCondition, 6),
  BANKED(append_object_sprite, kAppendObjectSprite, kSpriteValueIs0AOr16, 6),
  BANKED(sprite_value_is_0A_or_16, kSpriteValueIs0AOr16, kSpriteKind1, 6),
  BANKED(sprite_value_is_2C, kSpriteValueIs2C, kSpriteKind4, 6),
  BANKED(sprite_value_is_0_or_2, kSpriteValueIs0Or2, kSpriteKind2, 6),
  BANKED(draw_object_sprites, kDrawObjectSprites, kObjectSpriteData, 6),
  BANKED(script_slot_for_command, kScriptSlotForCommand, kScriptSlots, 6),
  BANKED(script_table_is_0, kScriptTableIs0, kFar8020_64, 6),
  BANKED(far_8020_64, kFar8020_64, kSpecialRoomResponse, 6),
  BANKED(special_room_response, kSpecialRoomResponse, kScriptTableIs0B, 6),
  BANKED(script_table_is_0_b, kScriptTableIs0B, kDefaultResponses, 6),
  BANKED(store_in_inventory_slot, kStoreInInventorySlot, kItemCodeInListAC53, 6),
  BANKED(item_code_in_list_AC53, kItemCodeInListAC53, kItemNumberForObject, 6),
  BANKED(item_number_for_object, kItemNumberForObject, kKindIs3, 6),
  BANKED(kind_is_3, kKindIs3, kGroupItems, 6),
  BANKED(check_inventory_slot_empty, kCheckInventorySlotEmpty, kInventorySlotIndex, 6),
  BANKED(inventory_slot_index, kInventorySlotIndex, kItemNumberBases, 6),
  BANKED(draw_inventory_page, kDrawInventoryPage, kDrawAddressBook, 6),
  BANKED(draw_address_book, kDrawAddressBook, kDrawInventoryEntry, 6),
  BANKED(draw_inventory_entry, kDrawInventoryEntry, kFar8020_6E, 6),
  BANKED(far_8020_6E, kFar8020_6E, kInventoryPageHeader, 6),
  BANKED(inventory_page_header, kInventoryPageHeader, kEntryRows, 6),
  BANKED(far_8020_69, kFar8020_69, kDoorFlagsToSet, 6),
  BANKED(delete_inventory_entry, kDeleteInventoryEntry, kFar8020_6C, 6),
  BANKED(far_8020_6C, kFar8020_6C, kUnreferencedAEF0, 6),
  BANKED(play_slot_machine, kPlaySlotMachine, kSlotMachinePayout, 6),
  BANKED(slot_machine_payout, kSlotMachinePayout, kSpinAllReels, 6),
  BANKED(spin_all_reels, kSpinAllReels, kStopReels, 6),
  BANKED(stop_reels, kStopReels, kSpinReel0, 6),
  BANKED(spin_reel_0, kSpinReel0, kSpinReel1, 6),
  BANKED(spin_reel_1, kSpinReel1, kSpinReel2, 6),
  BANKED(spin_reel_2, kSpinReel2, kDrawReel0, 6),
  BANKED(draw_reel_0, kDrawReel0, kDrawReel1, 6),
  BANKED(draw_reel_1, kDrawReel1, kDrawReel2, 6),
  BANKED(draw_reel_2, kDrawReel2, kReelVramAddresses, 6),
  BANKED(wait_7_frames, kWait7Frames, kOpeningLogos, 6),
  BANKED(opening_logos, kOpeningLogos, kBlackPaletteBuffer, 6),
  BANKED(black_palette_buffer, kBlackPaletteBuffer, kWait30FramesB6, 6),
  BANKED(wait_30_frames_b6, kWait30FramesB6, kLogoPaletteCycle1, 6),
  BANKED(logo_palette_cycle_1, kLogoPaletteCycle1, kLogoColours1, 6),
  BANKED(logo_palette_cycle_2, kLogoPaletteCycle2, kLogoColours2, 6),
  BANKED(wait_6_frames, kWait6Frames, kDrawLogoScreen, 6),
  BANKED(draw_logo_screen, kDrawLogoScreen, kEndingSequence, 6),
  BANKED(ending_sequence, kEndingSequence, kEndingWaitButton, 6),
  BANKED(ending_wait_button, kEndingWaitButton, kEndingPromptColours, 6),
  BANKED(wait_6_frames_2, kWait6Frames2, kEndingSetup, 6),
  BANKED(ending_setup, kEndingSetup, kEndingSprite0, 6),
  BANKED(draw_ending_box, kDrawEndingBox, kEndingBoxBlock, 6),
  BANKED(draw_ending_screen, kDrawEndingScreen, kUnused6B5FD, 6),
  BANKED_INLINE(object_sprite_condition, kObjectSpriteCondition, kDrawObjectSprites, 6),
  BANKED(title_screen, kTitleScreen, kTitlePalette, 2),
  BANKED(draw_title_picture, kDrawTitlePicture, kFileSelectMenu, 2),
  BANKED(file_select_menu, kFileSelectMenu, kFileSelectScreen, 2),
  BANKED(file_select_screen, kFileSelectScreen, kMenuPalette, 2),
  BANKED(draw_file_list, kDrawFileList, kDrawFileEntry, 2),
  BANKED(draw_file_entry, kDrawFileEntry, kFileEntrySetup, 2),
  BANKED(file_entry_setup, kFileEntrySetup, kDrawFileStatus, 2),
  BANKED(draw_file_status, kDrawFileStatus, kFileEntryTiles, 2),
  BANKED(draw_file_options, kDrawFileOptions, kFileOptionBlocks, 2),
  BANKED(clear_file_area, kClearFileArea, kFillFileArea, 2),
  BANKED(fill_file_area, kFillFileArea, kClearFileBlock, 2),
  BANKED(draw_file_cursor, kDrawFileCursor, kFileCursorYs, 2),
  BANKED(draw_option_cursor, kDrawOptionCursor, kOptionCursorYs, 2),
  BANKED(show_file_options, kShowFileOptions, kYesNoVram, 2),
  BANKED(quit_dialog, kQuitDialog, kSaveDialog, 2),
  BANKED(save_dialog, kSaveDialog, kOpenConfirmDialog, 2),
  BANKED(open_confirm_dialog, kOpenConfirmDialog, kDrawDialog, 2),
  BANKED(draw_dialog, kDrawDialog, kDrawDialogText, 2),
  BANKED(draw_dialog_text, kDrawDialogText, kResetDialogAnswer, 2),
  BANKED(reset_dialog_answer, kResetDialogAnswer, kDrawDialogCursor, 2),
  BANKED(draw_dialog_cursor, kDrawDialogCursor, kDialogCursors, 2),
  BANKED(confirm_dialog_loop, kConfirmDialogLoop, kDrawDialogScreen, 2),
  BANKED(draw_dialog_screen, kDrawDialogScreen, kUnused2_8C01, 2),
};
const size_t kPortedRoutineCount = sizeof(kPortedRoutines) / sizeof(kPortedRoutines[0]);

int dv_boot(const uint8_t *prg, const uint8_t *chr) {
  nes_init(prg, chr);
  emu_set_hooks(kPortedRoutines, kPortedRoutineCount);
  nes_power_on(0);
  g_nes.cpu.pc = (uint16_t)(cpu_bus_read(0xFFFC) | cpu_bus_read(0xFFFD) << 8);
  return emu_start();
}
