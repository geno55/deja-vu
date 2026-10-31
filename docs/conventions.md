# How Deja Vu's code is organized

Findings from Phase 1. Addresses are CPU addresses; `7:E800` means bank 7, `$E800`.
Names match [asm/labels.txt](../asm/labels.txt). "Guess" marks anything not yet confirmed.

## Memory map
- PRG ROM: 8 banks of 16 KB. Banks 0-6 switch into `$8000-$BFFF` (MMC3 R6/R7 as an 8 KB pair);
  bank 7 is fixed at `$C000-$FFFF`.
- `switch_prg_bank` (`7:E89B`) maps a 16 KB bank. `push_and_switch_bank` (`7:EAD0`) and
  `pop_bank` (`7:EAEA`) save and restore the current bank (in `$0C`) on a small stack.
- Shadow copies of the mapper registers live in RAM: `$010C` (bank select, mode bits kept),
  `$0104-$010B` (CHR banks R0-R5). `mmc3_set_bank` (`7:E8FF`) writes the real registers.
- The game only ever sets vertical mirroring (`set_mirroring_vertical`, twice, at reset). Nothing calls
  `set_mirroring_horizontal`.

## Where things are (so far)
| Bank | Contents |
|---|---|
| 0, 1 | Scene pictures only ([scenes.md](scenes.md)) |
| 2 | Some code (screens drawn with inline RLE data, CHR setup), 4 scene pictures, 128 messages, item and word name tables, room hotspots and item lists ([rooms.md](rooms.md)) |
| 3 | **Scripts** ([scripts.md](scripts.md)) |
| 4 | No code; 256 messages, then 2,225 bytes nothing can read (only the message printer maps bank 4) |
| 5 | Music and sound effects ([sound.md](sound.md)), then 256 messages |
| 6 | Most of the swappable code, the tables it reads, and the screen layouts at `$BC00` |
| 7 | Fixed bank: engine, interpreter, NMI, mapper helpers, sound driver |

### Screen layouts
Menus and title text are drawn from short command lists (`tools/layouts.py`): an address
table at `6:BC00` (16 layouts), drawn by `draw_layout_number` (`7:FBED`). Commands: `$00 n` prints
item name n; `$01-$05 a a` print something found through address `a`; `$06 x y` moves the
cursor; `$12 a a` prints the item named at address `a`; anything else is one font tile;
`$FF` ends.

### Unreferenced code
`tools/disasm.py` also looks for code that nothing calls: runs of unclassified bytes that
trace cleanly, reach an RTS, RTI or JMP, and call routines already known or fall or branch
into them, or that decode exactly, start to end, into self-contained code. Candidates on bytes
the recordings read as data, or of a single instruction, are rejected. It finds 23 entry
points (labelled `unreferenced_*`), including unused far-call entries in bank 6, extra entry
points into known routines, and a block of coordinate helpers at `7:F32F`. Routines named in
`asm/labels.txt` that nothing reaches (like `set_mirroring_horizontal`) are traced too.

### What's left
`tools/disasm.py` writes every unclassified run to `build/asm/unclassified.txt`. About 500
bytes (0.4%) remain, all in data areas: pieces of tables that code reads through computed
pointers, a few sound bytes no stream points to, and 168 bytes after bank 2's scene pictures.

## Calling conventions
These matter to both the disassembler (bytes after some JSRs are data) and the C port.

| Call | Meaning | Count |
|---|---|---|
| `LDA #id` / `JSR far_call_8000` (`$F3B5`) | Far call: bank `id >> 4`, entry `id & 15` of the address table at `$8000` in that bank. Saves and restores the current bank, keeps X and Y. | 21 sites |
| same with `$F3C5` / `$F3D5` / `$F3E5` | Same, tables at `$8020` / `$8040` / `$8400` | 45 / 5 / 4 |
| `JSR jump_table` (`$EBE6`) | A = index; a table of addresses follows the JSR. Jumps to the entry; never returns to after the JSR. | 22 sites |
| `JSR sound_jump_table` (`$FADA`) | Same pattern, inside the sound code | 1 |
| `JSR set_chr_banks_inline` (`$E916`) | 8 CHR bank numbers follow the JSR; execution continues after them | 5 |
| `JSR draw_rle_screen_2000` (`$E9D5`) | An RLE-compressed nametable follows the JSR. It is drawn to `$2000`, and the routine returns to the **caller's caller**, so whatever calls the JSR's routine gets a drawn screen. `$E992` does the same for `$2400` but has no callers. | 4 |

Every far call loads its ID with `LDA #imm`, so all of them resolve statically.

### RLE nametable format (`rle_to_ppu_2000`, `7:EA18`)
- any byte except `$FF`: write it once
- `$FF c n` (c != `$FF`): write `c` n times
- `$FF $FF n` (n != 0): write `$FF` n times
- `$FF $FF $00`: end

Bytes written to the attribute area (`$23C0+`) are also copied to `$0300+`.

## Frame structure
- `reset` (`7:E800`) clears RAM, sets up the PPU and jumps to `main` (`7:E600`).
- `main` and everything it calls are ordinary sequential code that waits for frames by calling
  `wait_vblank` (`7:EB9C`): it sets bit 1 of `$F0` and spins until the NMI clears it.
  There are 57 call sites in banks 2, 6 and 7. The C port runs this code as a coroutine.
- `nmi` (`7:EC9A`): turns the NMI off; OAM DMA when asked (bit 7 of `nmi_requests`); one upload
  when asked (`nmi_ppu_updates`, `7:ED24`; the highest of bits 6-1 wins); PPUMASK when asked
  (bit 0); the scroll (`nmi_set_scroll`, `7:EE38`); `frame_counter`; `sound_update`
  (`7:F538`), which also ends `wait_vblank`. Then, when the split is on (`system_flags` bit 0)
  and both layers are showing, it waits for sprite-0 hit and switches the background pattern
  table: the screen is split between the scene and the text/UI area. Last it turns the NMI back
  on. If sprite 0 never hits, the wait ends at the next vblank. Its code is in
  [src/nmi.c](../src/nmi.c).
- `irq` (`7:EB40`) is a bare `RTI`; the MMC3 scanline IRQ is not used.
- `read_joypads` (`7:EB41`) reads both controllers (and the Famicom expansion bits) twice,
  until two reads match. The NMI doesn't read them.

### What the NMI changes
Lag frames interrupt game code, so what the handler changes matters. Recorded over every NMI of
both TAS movies (71,461 handlers), the RAM it changed was the same set on both:

| RAM | What |
|---|---|
| `$18`, `$25`, `$F0`, `$FF`, `$0380` | `nmi_requests`, `frame_counter`, `system_flags`, `ppuctrl_shadow`, `vram_queue_len` |
| `$44-$48`, `$4A-$4B`, `$51-$53` | the sound driver's scratch; all their code is in the sound driver |
| `$010C-$010D` | `bank_select_shadow` and `switch_prg_bank`'s copy of the bank |
| `$0110-$014B`, `$0700-$07D3` | the sound driver's loop stacks and channel slots |
| `$01AA-$01F9` | the 6502 stack, below where the game was |

It can also change `$08`, `$09` (used nowhere else), `split_nametable`, `ppuscroll_x` and
`ppuscroll_y`, but none of them changed in these runs. Nothing in the game's own state changes:
no flags, inventory, room, scene or text, and not the temporaries at `$10-$17` and `$1C-$1F`
that the routines lag frames interrupt most use. `frame_counter` reaches the game only through
`blink_more_arrow`, which is cosmetic.

The bank switching is guarded both ways. `sound_update` skips a frame (and catches up by stepping
twice) when `system_flags` says a bank switch (bit 3) or `play_sound` (bit 2) is under way. So
mirroring and save-RAM protection never changed across an NMI, and the MMC3 registers rarely did:
bank select 4 times on each movie, bank registers 2 and 4 times. That is what an NMI landing in
the middle of an unguarded CHR bank change, or just after `set_current_bank` stored
`current_bank`, would do (guess: those cases weren't looked at one by one). Later NMIs and
comparisons stayed clean.

## The script interpreter: game logic is data
Deja Vu's game logic runs as **bytecode scripts**: the C port
needs the interpreter plus extracted scripts, not hand-ported code for every room.

- Bank 3 starts with a table of 6 addresses, one per top-level script table.
- Start-up (`7:CB64`): switch to bank 3, then follow three levels of address tables:
  `bank3[$8000 + 2 * ($AA & $7F)]`, then `+ 2 * $AB`, then `+ 2 * $BC`. The result is the
  script pointer in `$B6/$B7`. What `$AA`, `$AB` and `$BC` mean is still to confirm; Data
  Crystal lists `$AC` as the selected command.
- `script_loop` (`7:CBB1`): fetch an opcode with `script_fetch_byte`; `$FF` ends the script;
  anything else goes to `script_dispatch` (`7:CC17`), a jump table with **105 entries**.
- Handlers read 1-4 operand bytes from `($B6)` into `$B8-$BB` and advance the pointer with
  `script_advance` (`7:CBCA`).

The script tables, the 105 opcodes and their sizes are in [scripts.md](scripts.md).

## Special scenes
Some events cut away to a picture of their own: script opcode `$46` (and a few commands,
`special_command_by_room`) set `special_scene` (`$ED`) and call `run_special_scene`. It
remembers the current scene in `saved_scene`, shows the picture from
`special_scene_pictures`, runs that scene's action from `special_scene_action` (a jump
table with 27 entries; entry 0 is the slot machine, several others set a flag from
`special_scene_flags`, print a message and clear the flag), then goes back to the saved
scene and restores the music.

## Saving
The game state is RAM `$0420-$052F` (272 bytes: object states, flags, item status,
inventory, room and scene). `save_game` (`7:C858`) writes it to save RAM for the file the
player picked on the file select screen (`save_file`, 0-2), as **4 copies**, $200 bytes each
at `$6010 + file * $800 + copy * $200`, with a 6-byte signature (`save_signature`) at +`$120` and a
16-bit sum at +`$126`. `load_game` checks the signature and sum and falls back to the other copies
when one is bad, then saves the file again.
`check_all_save_files` runs at the title to find which files are usable. Save RAM is
write-protected (`protect_save_ram`) or switched off (`disable_save_ram`) except while saving
(`unlock_save_ram`).

Two quirks in `check_save_file`. It doesn't clear the sum between copies, so a later copy only passes
if the copies before it sum to 0. So when copy 0 is bad and doesn't sum to 0, all 4 fail and the file
is erased, good copies and all: one wrong byte is enough. That is checked in play, on Mesen2 and the
port alike ([saves.md](saves.md)). And when a later copy does pass (copy 0 all zeros, say),
`repair_save_copies` never ends: the `BNE` at `7:C928` goes back to the first copy loop instead of on
through the second page, so the game hangs with NMIs still running. That one is read from the code,
and no recording gets there. The fuzz agrees: its samples with `$64` not 0 never return. The game fixes both
by default ([saves.md](saves.md), "The fix").

The menus around it live in bank 2: `title_screen`, `file_select_screen` and
`file_select_menu`, and a yes/no dialog (`open_confirm_dialog`, `confirm_dialog_loop`)
used by `save_dialog` and `quit_dialog`; the main loop goes back to `reset` when
`quit_dialog` answers yes.

## Sequences
- `opening_logos` (bank 6, far call from `main`): black palette, a logo screen drawn from
  inline RLE data (`draw_logo_screen`), and two palette cycles.
- `ending_sequence` (bank 6): shows the epilogue as screen layouts `$06`, `$07` and
  `$0B-$0F`, waiting for a button between pages (`ending_wait_button`), then stops for good at
  `6:B2C6`, with only NMIs running.
- `play_slot_machine` (bank 6): the casino's slot machine; three reels picked with
  `next_random` (`spin_reel_*`, `draw_reel_*`), and winnings shown through `text_number`.

## Text
Compressed messages fill most of banks 4 and 5 and part of bank 2; the format is in
[text.md](text.md).

## Open questions
- What the rest of banks 0, 1, 3, 4 and 5 hold (scene data, scripts, music).
- The meaning of `$AA`, `$AB`, `$BC` and the 105 opcodes.
