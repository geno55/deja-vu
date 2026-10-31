// The slot machine (bank 6; far call $6F), and the opening logos and the
// ending (far calls $60 and $61 at $8040).
//
// The slot machine's scratch bytes: data_ptr and $57 what the three reels stop on, $58-$5A what they
// show, $84 the payout, $64 a frame count, $65 the symbol being drawn.
#include "funcs.h"
#include "variables.h"

// ---- The slot machine ----------------------------------------------------------------------

// play_slot_machine (6:AF08): the sounds, the spin, and the coins if it paid.
void play_slot_machine(void) {
  RUN_ORIGINAL_IF_LAG(kPlaySlotMachine);
  Cpu6502 *c = &g_nes.cpu;
  play_sound(0x11);
  wait_60_frames();
  RESUME_ORIGINAL_IF_LAG(kPlaySlotMachine, 0xAF10, 0);
  clear_and_upload_sprites();
  slot_machine_payout();
  play_sound(0x12);
  spin_all_reels();
  RESUME_ORIGINAL_IF_LAG(kPlaySlotMachine, 0xAF1E, 0);
  stop_reels();
  RESUME_ORIGINAL_IF_LAG(kPlaySlotMachine, 0xAF21, 0);
  emu_set_a(g_ram[0x84]);
  if(!c->a) {
    play_sound(0x16);
    emu_set_a(0x75);
    print_message_2_or_5();
    RESUME_ORIGINAL_IF_LAG(kPlaySlotMachine, 0xAF58, 0);
    return;
  }
  text_number = c->a;
  script_args[0] = c->a;
  play_sound(0x14);
  wait_180_frames();
  RESUME_ORIGINAL_IF_LAG(kPlaySlotMachine, 0xAF32, 0);
  wait_60_frames();
  RESUME_ORIGINAL_IF_LAG(kPlaySlotMachine, 0xAF35, 0);
  play_sound(0x15);
  wait_60_frames();
  RESUME_ORIGINAL_IF_LAG(kPlaySlotMachine, 0xAF3D, 0);
  wait_60_frames();
  RESUME_ORIGINAL_IF_LAG(kPlaySlotMachine, 0xAF40, 0);
  play_sound(0x00);
  add_coins_and_show();
  RESUME_ORIGINAL_IF_LAG(kPlaySlotMachine, 0xAF48, 0);
  emu_set_a(0x18);
  print_message_2_or_5();
  RESUME_ORIGINAL_IF_LAG(kPlaySlotMachine, 0xAF4D, 0);
}

// slot_machine_payout (6:AF59): with no coins, or 2 times in 8 under $46 coins, three of a kind and
// its payout (6:AF81). Otherwise $84 = 0 and three reels that aren't all the same.
void slot_machine_payout(void) {
  RUN_ORIGINAL_IF_LAG(kSlotMachinePayout);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(coins);
  int pays = !c->a;
  if(!pays) {
    emu_cmp(c->a, 0x46);
    if(c->a < 0x46) {
      next_random();
      emu_set_a(random_value & 0x07);
      emu_cmp(c->a, 0x06);
      pays = c->a >= 0x06;
    }
  }
  if(pays) {
    next_random();
    emu_set_a(random_value & 0x07);
    c->y = c->a;
    emu_set_a(cpu_bus_read((uint16_t)(kSlotPayouts + c->y)));
    g_ram[0x84] = c->a;
    data_ptr[0] = data_ptr[1] = g_ram[0x57] = c->y;
    return;
  }
  emu_set_a(0x00);
  g_ram[0x84] = 0x00;
  for(;;) {
    c->y = 0;
    do {
      next_random();
      emu_set_a(random_value & 0x07);
      data_ptr[c->y] = c->a;
      c->y++;
      emu_cmp(c->y, 0x03);
    } while(c->y != 0x03);
    emu_set_a(data_ptr[0]);
    emu_cmp(c->a, data_ptr[1]);
    if(c->a != data_ptr[1]) {
      return;
    }
    emu_cmp(c->a, g_ram[0x57]);
    if(c->a != g_ram[0x57]) {
      return;
    }
  }
}

// The end of each frame of the spin: request_vram_queue, request_attrs_2000 and wait_7_frames.
static void show_reels(void) {
  request_vram_queue();
  request_attrs_2000();
  wait_7_frames();
}

// spin_all_reels (6:AFA9): $1D frames.
void spin_all_reels(void) {
  RUN_ORIGINAL_IF_LAG(kSpinAllReels);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(0x00);
  g_ram[0x64] = 0x00;
  do {
    spin_reel_0();
    RESUME_ORIGINAL_IF_LAG(kSpinAllReels, 0xAFB0, 0);
    spin_reel_1();
    RESUME_ORIGINAL_IF_LAG(kSpinAllReels, 0xAFB3, 0);
    spin_reel_2();
    RESUME_ORIGINAL_IF_LAG(kSpinAllReels, 0xAFB6, 0);
    show_reels();
    RESUME_ORIGINAL_IF_LAG(kSpinAllReels, 0xAFBF, 0);
    g_ram[0x64]++;
    emu_set_a(g_ram[0x64]);
    emu_cmp(c->a, 0x1D);
  } while(c->a != 0x1D);
}

// stop_reels (6:AFC8): one reel at a time stops on its result, $13 frames apart, with a sound.
void stop_reels(void) {
  RUN_ORIGINAL_IF_LAG(kStopReels);
  Cpu6502 *c = &g_nes.cpu;
  play_sound(0x13);
  emu_set_a(data_ptr[0]);
  g_ram[0x58] = c->a;
  emu_set_a(0x00);
  g_ram[0x64] = 0x00;
  do {
    draw_reel_0();
    RESUME_ORIGINAL_IF_LAG(kStopReels, 0xAFD8, 0);
    spin_reel_1();
    RESUME_ORIGINAL_IF_LAG(kStopReels, 0xAFDB, 0);
    spin_reel_2();
    RESUME_ORIGINAL_IF_LAG(kStopReels, 0xAFDE, 0);
    show_reels();
    RESUME_ORIGINAL_IF_LAG(kStopReels, 0xAFE7, 0);
    g_ram[0x64]++;
    emu_set_a(g_ram[0x64]);
    emu_cmp(c->a, 0x13);
  } while(c->a != 0x13);
  play_sound(0x13);
  emu_set_a(data_ptr[1]);
  g_ram[0x59] = c->a;
  emu_set_a(0x00);
  g_ram[0x64] = 0x00;
  do {
    draw_reel_0();
    RESUME_ORIGINAL_IF_LAG(kStopReels, 0xAFFF, 0);
    draw_reel_1();
    RESUME_ORIGINAL_IF_LAG(kStopReels, 0xB002, 0);
    spin_reel_2();
    RESUME_ORIGINAL_IF_LAG(kStopReels, 0xB005, 0);
    show_reels();
    RESUME_ORIGINAL_IF_LAG(kStopReels, 0xB00E, 0);
    g_ram[0x64]++;
    emu_set_a(g_ram[0x64]);
    emu_cmp(c->a, 0x13);
  } while(c->a != 0x13);
  play_sound(0x13);
  emu_set_a(g_ram[0x57]);
  g_ram[0x5A] = c->a;
  draw_reel_0();
  RESUME_ORIGINAL_IF_LAG(kStopReels, 0xB022, 0);
  draw_reel_1();
  RESUME_ORIGINAL_IF_LAG(kStopReels, 0xB025, 0);
  draw_reel_2();
  RESUME_ORIGINAL_IF_LAG(kStopReels, 0xB028, 0);
  show_reels();
  RESUME_ORIGINAL_IF_LAG(kStopReels, 0xB031, 0);
}

// A reel's next symbol, not the one it shows ($58 + reel): next_random until it differs.
static void next_symbol(uint8_t *shown) {
  Cpu6502 *c = &g_nes.cpu;
  do {
    next_random();
    emu_set_a(random_value & 0x07);
    emu_cmp(c->a, *shown);
  } while(c->a == *shown);
  *shown = c->a;
}

// spin_reel_0 (6:B032), spin_reel_1 (6:B043), spin_reel_2 (6:B054)
void spin_reel_0(void) {
  RUN_ORIGINAL_IF_LAG(kSpinReel0);
  next_symbol(g_ram + 0x58);
  draw_reel_0();
  RESUME_ORIGINAL_IF_LAG(kSpinReel0, 0xB042, 0);
}

void spin_reel_1(void) {
  RUN_ORIGINAL_IF_LAG(kSpinReel1);
  next_symbol(g_ram + 0x59);
  draw_reel_1();
  RESUME_ORIGINAL_IF_LAG(kSpinReel1, 0xB053, 0);
}

void spin_reel_2(void) {
  RUN_ORIGINAL_IF_LAG(kSpinReel2);
  next_symbol(g_ram + 0x5A);
  draw_reel_2();
  RESUME_ORIGINAL_IF_LAG(kSpinReel2, 0xB064, 0);
}

// The end of draw_reel_0 (6:B065), draw_reel_1 (6:B069) and draw_reel_2 (6:B06D), at 6:B06F with
// X = the reel: the symbol's colour in the attributes and its tiles.
static void draw_reel(void) {
  Cpu6502 *c = &g_nes.cpu;
  TAIL_ORIGINAL_IF_LAG(kDrawReel2, 0xB06F, 0);
  emu_set_a(g_ram[(uint8_t)(0x58 + c->x)]);
  g_ram[0x65] = c->a;
  c->x = asl_a(c->x);
  uint8_t lo = cpu_bus_read((uint16_t)(kReelVramAddresses + c->x));
  uint8_t hi = cpu_bus_read((uint16_t)(kReelVramAddresses + 1 + c->x));
  emu_push(lo);
  g_ram[0x1C] = lo;
  emu_push(hi);
  g_ram[0x1D] = hi;
  c->x = g_ram[0x65];
  g_ram[0x22] = cpu_bus_read((uint16_t)(kReelAttributes + c->x));
  emu_set_a(0x02);
  g_ram[0x20] = 0x02;
  g_ram[0x21] = 0x02;
  set_attr_block();
  g_ram[0x1D] = emu_pull();
  g_ram[0x1C] = emu_pull();
  c->x = asl_a(g_ram[0x65]);
  g_ram[0x1E] = cpu_bus_read((uint16_t)(kReelSymbols + c->x));
  emu_set_a(cpu_bus_read((uint16_t)(kReelSymbols + 1 + c->x)));
  g_ram[0x1F] = c->a;
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kDrawReel2, 0xB0A9, 0);
}

void draw_reel_0(void) {
  RUN_ORIGINAL_IF_LAG(kDrawReel0);
  g_nes.cpu.x = 0x00;
  draw_reel();
}

void draw_reel_1(void) {
  RUN_ORIGINAL_IF_LAG(kDrawReel1);
  g_nes.cpu.x = 0x01;
  draw_reel();
}

void draw_reel_2(void) {
  RUN_ORIGINAL_IF_LAG(kDrawReel2);
  g_nes.cpu.x = 0x02;
  draw_reel();
}

// wait_7_frames (6:B0F8)
void wait_7_frames(void) {
  RUN_ORIGINAL_IF_LAG(kWait7Frames);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait7Frames, 0xB0FB, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait7Frames, 0xB0FE, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait7Frames, 0xB101, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait7Frames, 0xB104, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait7Frames, 0xB107, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait7Frames, 0xB10A, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait7Frames, 0xB10D, 0);
}

// ---- The logos ------------------------------------------------------------------------------

// opening_logos (6:B10E, far call $60 at $8040): the logo screen with its colours coming and going.
void opening_logos(void) {
  RUN_ORIGINAL_IF_LAG(kOpeningLogos);
  black_palette_buffer();
  clear_nametables(0xFF);
  RESUME_ORIGINAL_IF_LAG(kOpeningLogos, 0xB116, 0);
  fill_all_attrs(0x00);
  request_attr_upload();
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kOpeningLogos, 0xB121, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kOpeningLogos, 0xB124, 0);
  draw_logo_screen();
  RESUME_ORIGINAL_IF_LAG(kOpeningLogos, 0xB127, 0);
  SET_CHR_BANKS_INLINE(0xB127, 0);
  wait_30_frames_b6();
  RESUME_ORIGINAL_IF_LAG(kOpeningLogos, 0xB135, 0);
  logo_palette_cycle_1();
  RESUME_ORIGINAL_IF_LAG(kOpeningLogos, 0xB138, 0);
  wait_30_frames_b6();
  RESUME_ORIGINAL_IF_LAG(kOpeningLogos, 0xB13B, 0);
  wait_30_frames_b6();
  RESUME_ORIGINAL_IF_LAG(kOpeningLogos, 0xB13E, 0);
  wait_30_frames_b6();
  RESUME_ORIGINAL_IF_LAG(kOpeningLogos, 0xB141, 0);
  logo_palette_cycle_2();
  RESUME_ORIGINAL_IF_LAG(kOpeningLogos, 0xB144, 0);
  wait_30_frames_b6();
}

// black_palette_buffer (6:B147)
void black_palette_buffer(void) {
  RUN_ORIGINAL_IF_LAG(kBlackPaletteBuffer);
  Cpu6502 *c = &g_nes.cpu;
  c->x = 0;
  do {
    emu_set_a(0x0F);
    palette_buffer[c->x] = 0x0F;
    c->x++;
    emu_cmp(c->x, 0x20);
  } while(c->x != 0x20);
  request_palette_upload();
}

// wait_30_frames_b6 (6:B157): like wait_30_frames, with X. Returns X = $1E.
void wait_30_frames_b6(void) {
  RUN_ORIGINAL_IF_LAG(kWait30FramesB6);
  Cpu6502 *c = &g_nes.cpu;
  c->x = 0;
  do {
    wait_vblank();
    RESUME_ORIGINAL_IF_LAG(kWait30FramesB6, 0xB15C, 0);
    c->x++;
    emu_cmp(c->x, 0x1E);
  } while(c->x != 0x1E);
}

// logo_palette_cycle_1 (6:B162), logo_palette_cycle_2 (6:B193): 5 sets of 3 colours, 6 frames each.
void logo_palette_cycle_1(void) {
  RUN_ORIGINAL_IF_LAG(kLogoPaletteCycle1);
  Cpu6502 *c = &g_nes.cpu;
  c->x = 0;
  do {
    palette_buffer[1] = cpu_bus_read((uint16_t)(kLogoColours1 + c->x));
    palette_buffer[2] = cpu_bus_read((uint16_t)(kLogoColours1 + 1 + c->x));
    palette_buffer[3] = cpu_bus_read((uint16_t)(kLogoColours1 + 2 + c->x));
    request_palette_upload();
    wait_6_frames();
    RESUME_ORIGINAL_IF_LAG(kLogoPaletteCycle1, 0xB17C, 0);
    c->x += 3;
    emu_cmp(c->x, 0x0F);
  } while(c->x != 0x0F);
}

void logo_palette_cycle_2(void) {
  RUN_ORIGINAL_IF_LAG(kLogoPaletteCycle2);
  Cpu6502 *c = &g_nes.cpu;
  c->x = 0;
  do {
    palette_buffer[1] = cpu_bus_read((uint16_t)(kLogoColours2 + c->x));
    palette_buffer[2] = cpu_bus_read((uint16_t)(kLogoColours2 + 1 + c->x));
    palette_buffer[3] = cpu_bus_read((uint16_t)(kLogoColours2 + 2 + c->x));
    request_palette_upload();
    wait_6_frames();
    RESUME_ORIGINAL_IF_LAG(kLogoPaletteCycle2, 0xB1AD, 0);
    c->x += 3;
    emu_cmp(c->x, 0x0F);
  } while(c->x != 0x0F);
}

// wait_6_frames (6:B1C4)
void wait_6_frames(void) {
  RUN_ORIGINAL_IF_LAG(kWait6Frames);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait6Frames, 0xB1C7, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait6Frames, 0xB1CA, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait6Frames, 0xB1CD, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait6Frames, 0xB1D0, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait6Frames, 0xB1D3, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait6Frames, 0xB1D6, 0);
}

// draw_logo_screen (6:B1D7): the run-length-coded screen after its JSR.
void draw_logo_screen(void) {
  rle_screen_from(kDrawLogoScreen);
}

// ---- The ending -----------------------------------------------------------------------------

// A page of the ending.
static void ending_page(uint8_t layout) {
  draw_layout_number(layout);
  request_vram_queue();
}

// ending_sequence (6:B26E, far call $61 at $8040): 7 pages, each for a press of A, then the game stops
// for good at 6:B2C6, with only NMIs running.
void ending_sequence(void) {
  RUN_ORIGINAL_IF_LAG(kEndingSequence);
  Cpu6502 *c = &g_nes.cpu;
  ending_setup();
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB271, 0);
  ending_page(0x06);
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB279, 0);
  ending_wait_button();
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB27C, 0);
  ending_page(0x07);
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB284, 0);
  ending_wait_button();
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB287, 0);
  ending_page(0x0B);
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB28F, 0);
  ending_wait_button();
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB292, 0);
  ending_page(0x0C);
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB29A, 0);
  ending_wait_button();
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB29D, 0);
  ending_page(0x0D);
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB2A5, 0);
  ending_wait_button();
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB2A8, 0);
  ending_page(0x0E);
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB2B0, 0);
  ending_wait_button();
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB2B3, 0);
  ending_page(0x0F);
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB2BB, 0);
  ending_wait_button();
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB2BE, 0);
  draw_ending_box();
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB2C1, 0);
  c->x = 0x01;
  ending_wait_button();
  RESUME_ORIGINAL_IF_LAG(kEndingSequence, 0xB2C6, 0);
  for(;;) {
    emu_count_work(100);
    emu_take_late_nmis();
    emu_wait_for_nmi(0xB2C6);
  }
}

// ending_wait_button (6:B2C9): the prompt's colour comes up; unless X = 1, it waits for A, the colour
// goes down, and the page is cleared.
void ending_wait_button(void) {
  RUN_ORIGINAL_IF_LAG(kEndingWaitButton);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0x01;
  do {
    palette_buffer[15] = cpu_bus_read((uint16_t)(kEndingPromptColours + c->y));
    request_palette_upload();
    wait_6_frames_2();
    RESUME_ORIGINAL_IF_LAG(kEndingWaitButton, 0xB2D7, 0);
    c->y++;
    emu_cmp(c->y, 0x04);
  } while(c->y != 0x04);
  emu_cmp(c->x, 0x01);
  if(c->x == 0x01) {
    return;
  }
  do {
    read_joypads();
    RESUME_ORIGINAL_IF_LAG(kEndingWaitButton, 0xB2E3, 0);
    emu_set_a(joy_pressed[0]);
    if(!(c->a & 0x80)) {
      emu_spin(10);
    }
  } while(!(c->a & 0x80));
  c->y = 0x03;
  do {
    palette_buffer[15] = cpu_bus_read((uint16_t)(kEndingPromptColours + c->y));
    request_palette_upload();
    wait_6_frames_2();
    RESUME_ORIGINAL_IF_LAG(kEndingWaitButton, 0xB2F5, 0);
    c->y--;
    emu_set_nz(c->y);
  } while(!(c->y & 0x80));
  g_ram[0x1E] = 0x16;
  g_ram[0x1F] = 0xB3;
  g_ram[0x1C] = 0xA3;
  emu_set_a(0x22);
  g_ram[0x1D] = 0x22;
  queue_vram_fill();
  RESUME_ORIGINAL_IF_LAG(kEndingWaitButton, 0xB30B, 0);
  request_vram_queue();
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kEndingWaitButton, 0xB311, 0);
}

// wait_6_frames_2 (6:B319)
void wait_6_frames_2(void) {
  RUN_ORIGINAL_IF_LAG(kWait6Frames2);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait6Frames2, 0xB31C, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait6Frames2, 0xB31F, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait6Frames2, 0xB322, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait6Frames2, 0xB325, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait6Frames2, 0xB328, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kWait6Frames2, 0xB32B, 0);
}

// ending_setup (6:B32C): the ending's screen, with sprite 0 for the split.
void ending_setup(void) {
  RUN_ORIGINAL_IF_LAG(kEndingSetup);
  Cpu6502 *c = &g_nes.cpu;
  play_sound(0x0E);
  fade_out_palette();
  RESUME_ORIGINAL_IF_LAG(kEndingSetup, 0xB334, 0);
  system_flags &= 0xFE;  // LSR, ASL
  emu_set_flag(kFlagC, 0);
  emu_set_nz(system_flags);
  clear_and_upload_sprites();
  RESUME_ORIGINAL_IF_LAG(kEndingSetup, 0xB33B, 0);
  draw_ending_screen();
  RESUME_ORIGINAL_IF_LAG(kEndingSetup, 0xB33E, 0);
  SET_CHR_BANKS_INLINE(0xB33E, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kEndingSetup, 0xB34C, 0);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kEndingSetup, 0xB34F, 0);
  c->x = 0;
  do {
    emu_set_a(cpu_bus_read((uint16_t)(kEndingSprite0 + c->x)));
    oam_buffer[c->x] = c->a;
    c->x++;
    emu_cmp(c->x, 0x04);
  } while(c->x != 0x04);
  request_oam_dma();
  system_flags |= 0x01;
  g_ram[0x1E] = 0x75;
  emu_set_a(0xB3);
  g_ram[0x1F] = 0xB3;
  fade_in_palette();
  RESUME_ORIGINAL_IF_LAG(kEndingSetup, 0xB370, 0);
}

// draw_ending_box (6:B395)
void draw_ending_box(void) {
  RUN_ORIGINAL_IF_LAG(kDrawEndingBox);
  g_ram[0x1E] = 0xAC;
  g_ram[0x1F] = 0xB3;
  g_ram[0x1C] = 0xEB;
  emu_set_a(0x22);
  g_ram[0x1D] = 0x22;
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kDrawEndingBox, 0xB3A8, 0);
  request_vram_queue();
}

// draw_ending_screen (6:B3C2): the run-length-coded screen after its JSR.
void draw_ending_screen(void) {
  rle_screen_from(kDrawEndingScreen);
}
