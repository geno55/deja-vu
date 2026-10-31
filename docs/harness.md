# The comparison harness (Phase 3)

The harness checks the C port against the original game frame by frame. It has two sides: the
**reference**, the original ROM running in Mesen2's
emulation core, and the **C side**, the port. The C side runs the game as a coroutine: the
routines ported so far in C, and the rest as the original 6502 code on its own model of the
console. Run with every routine as original code ("pass-through mode"), it matches the
reference at every NMI of both TAS movies; with the routines ported so far, it matches at
every NMI it compares.

Mesen2 is GPL-3.0, so Mesen2's source is cloned at a pinned
tag into `external/Mesen2` (never committed), compiled only when the CMake option
`DV_HARNESS` is on, and harness binaries are never published. Nothing from Mesen2 goes into
`src/`; the C side (everything in `src/`) is this project's MIT code and builds without the harness.

## Building and running

```
cmake -S . -B build/harness -DDV_HARNESS=ON
cmake --build build/harness --config Release --target dv_harness
build/harness/harness/Release/dv_harness --movie build/movies/tas1260.inp --compare
```

The configure step clones Mesen2 2.1.1 (commit `137ae7c`) if `external/Mesen2` is missing,
and stops if the checkout is at any other commit. The first build compiles about 430 Mesen2
source files (a few minutes). The `.inp` movies come from `tools/movie_inputs.py`
([development.md](development.md), "Disassembly (Phase 1)").

`dv_harness` plays a movie on the reference and stops at every NMI entry. Its options are in
its header comment ([harness/dv_harness.c](../harness/dv_harness.c)). `--compare` runs the C
side alongside and checks it at every NMI. With `--log` it writes one line per NMI:

```
nmi polls return_pc a= x= y= p= sp= cyc= ram= sram= nt= pal= oam= apu=count:hash
```

`ram`, `sram`, `nt`, `pal` and `oam` are FNV-1a hashes of the 2 KB work RAM, 8 KB save RAM,
2 KB nametable RAM, 32-byte palette and 256-byte OAM. `apu` counts and hashes the APU register
writes (`$4000-$4013`, `$4015`, `$4017`) since the previous NMI. `return_pc` is where the NMI
interrupted the game.

## How the reference drives Mesen2

The code is [harness/ref_mesen.cpp](../harness/ref_mesen.cpp) behind a small C API,
[harness/ref.h](../harness/ref.h).

- **Build.** `harness/CMakeLists.txt` compiles Mesen2's `Core`, `Utilities`, `SevenZip` and
  `Lua` folders (the sets its own makefile uses) into static libraries. The UI, the renderers
  and `InteropDLL` are not built. The harness uses Mesen2's C++ classes directly, the same
  ones `InteropDLL` wraps for the UI. With MSVC, Mesen2 needs `UNICODE` defined, as its own
  projects do.
- **Threads.** `Emulator::LoadRom` always starts an emulation thread. The harness powers on paused with
  `EmulationFlags::ConsoleMode` set, as Mesen2's test runner does: the debugger attaches
  while the ROM loads and breaks before the first instruction.
- **Breaking at an address.** An execute breakpoint on the NMI handler (`7:EC9A`). The
  debugger reports a break with a `CodeBreak` notification, sent on the emulation thread. The
  harness's listener handles it there, parks the thread on a condition variable, and calls
  `Debugger::Run()` before it returns. Without that call, Mesen2 polls for the resume every
  10 ms, which is too slow to stop every frame. Execute breakpoints also fire on operand
  fetches, so the listener looks only at opcode fetches.
- **Stepping a frame.** Release the thread and wait until it parks at the next NMI entry.
- **Memory.** `Emulator::GetMemory(MemoryType)` gives pointers to the work RAM, save RAM,
  nametable RAM, palette and OAM. They stay valid and stable while the thread is parked.
- **Logged without parking:** APU register writes (write breakpoints); the cycle at which the
  NMI handler's `RTI` (`7:ED23`) starts, which the C side counts cycles from; reads of PPU
  registers other than `$2002` itself (a read breakpoint with the condition
  `address != 8194`, so the game's many `$2002` reads don't each stop Mesen); and, when
  asked, every instruction with its registers (`ref_set_trace`, for diagnosing).
- **Input.** An `IInputProvider`, the hook Mesen2's movie player uses, is asked for the buttons
  at each poll (once per frame, on scanline 241, just before the NMI). It is the same moment
  `tools/mesen/record_cdl.lua` sets buttons from the `inputPolled` event. Poll `p` (counting
  from 0) gets movie line `p - 1 + offset`, as in the Lua script.
- **Save states.** `Emulator::Serialize` and `Emulator::Deserialize`, to memory, taken at an NMI
  entry. A state is loaded while the thread is parked at another NMI entry, and the thread
  carries on from the loaded one.
- **Settings.** Mesen2's defaults, with RAM filled with zeros at power-on and two standard
  controllers (what this machine's Mesen settings, and so `run_mesen.py`, use), auto-save and
  rewind off, and maximum speed. Maximum speed has to be set **after** `LoadRom`, because
  loading a ROM resets the rewind manager, which clears that flag. Save files live in a
  private Mesen home folder (`build/harness/mesen-home`), and the ROM's `.sav` there is
  deleted before each run.

## The C side in pass-through mode

- **The console** ([src/nes_hw.h](../src/nes_hw.h)): RAM, save RAM, nametable RAM, palette and
  OAM laid out as Mesen2 keeps them, so they compare byte for byte; MMC3 PRG banking and the
  save-RAM enable/protect bits (`$A001`); the PPU registers' effects on memory (VRAM address
  and increments, mirroring, palette entries `$x0/$x4/$x8/$xC` stored in both halves, values
  masked to 6 bits, OAM DMA and the attribute byte's missing bits); the controllers with the
  undriven top bits of `$4016`/`$4017` (open bus, `$40` after `lda $4016`); the APU write
  log. It powers on with the reference's power-on memories and registers.
- **The CPU** ([src/cpu6502.c](../src/cpu6502.c)): every opcode, as a glitch can run any byte as
  code, the unofficial ones from the NESdev wiki; standard cycle counts with
  the page-crossing and branch extras, and bus accesses in the 6502's order (the last byte on
  the bus is what unmapped addresses read back). No dummy reads; the game makes no indexed or
  read-modify-write access to a register where they would matter.
- **Where each NMI goes** ([src/dv_cpu_infra.h](../src/dv_cpu_infra.h)). The C side doesn't
  decide when NMIs happen; the reference does. If the reference's NMI came while the game was
  idle in a wait loop, the C side takes it once it is in the same loop with the same
  registers and the loop's condition says it will keep waiting. Otherwise (a lag frame) it
  takes it where its 6502 is at the same instruction, with the same registers and the same
  memories as the reference had when the NMI interrupted it. (The first version placed lag
  NMIs by CPU cycle, counting from the previous NMI's `RTI`. That stops working once ported
  code runs, which takes no cycles; matching the state works either way.) The cycle count is
  still set to the reference's at each `RTI`, for the approximate PPU timing: when the vblank
  and sprite-0 flags in `$2002` change.
- **Reads only a glitch makes.** The game's own code reads no PPU register but `$2002`. The
  glitched TAS #7044 walks a pointer into `$3247-$324C` (`$2007`, `$2000`, `$2001`, `$2002`,
  `$2003` and `$2004` mirrors), where the value depends on the PPU's exact position during
  rendering and on its open-bus decay. The C side takes those values from the reference, in
  order, and the harness checks that it made the same reads.
- **Comparing** (`dv_harness --compare`). At every NMI entry: registers, the five memories and
  the exact list of APU writes. A difference is reported with each differing address, both
  values and the value at the previous NMI, and the C side's last instructions. Then the
  harness **replays the frame**: it loads both sides' states from the previous NMI, traces
  every instruction on both, and prints where the two first part (from the `RTI`, since the
  NMI handler's sprite-0 wait spins a different number of times). `--keep-going` copies the
  reference's memories to the C side after a difference and carries on.

## Ported code: the bridge

Porting goes routine by routine, so ported C and original code run side by
side on the C side ([src/dv_cpu_infra.h](../src/dv_cpu_infra.h)):

- **The game is a coroutine** ([src/coro.c](../src/coro.c): Windows fibers, or `ucontext`
  elsewhere). It starts at the reset vector as 6502 code. When it waits for the NMI, it
  yields to `emu_run_to_nmi`, which takes the NMI, and the next call runs the NMI handler
  and resumes the game where it was. Waiting deep in the call stack, as the game does from 57
  places, is just a yield.
- **Original code calls ported code** through hooks: [src/dv_rtl.c](../src/dv_rtl.c) lists
  each ported routine with its bank and the range of the original's code. When the 6502
  reaches the routine's address (by `JSR`, `JMP` or falling into it), the C version runs, then
  the 6502 carries on as after the routine's `RTS`.
- **Registers.** While original code still runs, a ported routine leaves the 6502's
  registers as the original does, however it is called. It loads its C arguments into the
  registers the original takes them in, and at the end sets what the original returns
  (`A = X` after a `PLA`/`TAX` ending, the carry from a last `LSR`, ...). Original code
  after it, or in its place (below), then sees what it would have seen.
- **Ported code calls original code** with `emu_call(addr)` (zelda3's `RunEmulatedFunc`): it
  pushes a return address no code uses and runs the 6502 until the routine's `RTS` comes back
  to it. The registers carry the inputs and results. Original code called this way may wait
  for the NMI, and may call ported code again.
- **Waits in C.** A ported wait calls `emu_wait_for_nmi(spin)`, where `spin` names the
  original loop (`$EBA4` for `wait_vblank`, `$EBB4` for the `nmi_requests` spin). The NMI
  goes there if the reference waited in the same loop.
- **Names.** `src/variables.h` (generated from `asm/ram.inc` by `tools/make_variables.py`)
  names RAM for C as zelda3's does: `system_flags |= 2`.

What changes for the comparison once ported code runs:

- **Registers** mean the same only where the C side stops in 6502 code, and the stack pointer
  only if no ported routine is on the 6502 stack. Ported code doesn't push what the original
  pushed, so the stack page from `$0180` (above the sound driver's loop stacks) up to where the
  outermost ported routine was entered isn't compared either. The stack below the stack
  pointer is dead and never compared. Without the stack pointer, X counts as matching if it
  is as far from the stack pointer on both sides (it holds the stack pointer after `TSX`).
- **Lag frames.** An NMI that interrupted original code is still placed exactly, by state.
  One that interrupted code now ported can't be stopped at inside C, so:
  1. **The original runs instead, for that call.** Every ported routine starts with a guard
     (`RUN_ORIGINAL_IF_LAG`, or the hook itself). If the reference's next NMI is a lag frame
     inside the routine's range (or the range of original code it does inline), the call runs
     the original code (`emu_call_original`, which steps past the routine's own hook), and the
     NMI is placed exactly by state. Lag NMIs nearly always land in short routines without waits
     (`read_joypads_once`, `mmc3_set_bank`, the bank stack), and this places them all.
  2. **A call already under way.** A ported routine that waits may be partway through in C
     when the previous NMI comes, and the next one may land in its own code after the wait.
     So after each call that may wait, the routine checks again
     (`RESUME_ORIGINAL_IF_LAG`) and, if the NMI is in its code, runs the rest of itself as
     original code from the matching address (`emu_resume_original`). For that the 6502
     stack must be as the original has it there: routines that push (`queue_vram_block`
     saves X and Y) push the same bytes with `emu_push`, and the resume puts a return
     address under them, as if the routine had been called with `JSR`. The far calls instead
     end with `emu_lag_behind`, which takes the NMI there if the C side's memories are the
     reference's ("the reference's memories, in ported code"; the harness then compares
     memories, not registers). The end of the call alone isn't enough: a nested far call
     ends first.
  3. **Late.** Otherwise the C side takes the NMI on the way into the next wait, before the
     wait changes anything: compared if the memories match there, else late and not
     compared. The next NMI that lines up is compared as usual; a difference there says the
     late NMI may be the cause, and why it couldn't be placed (did the 6502 run the
     instruction, and with which registers?). In pure original code a missed state is an
     error, not a late NMI.

  Memories are matched only at those points, where the NMI's place may be behind. Matching
  them earlier is wrong: code that writes no memory looks like the right place. Example:
  `read_joypads_once` reading zero bits, which would take the NMI before the controllers'
  second read rather than in it.
- **Replays** need the C side's whole state, and the coroutine's C stack can't be saved. So a
  frame is replayed only when no ported routine is on the game's call stack at either NMI.
  With the waits ported that is never (the game waits in C); `--pass-through` gives the
  replay back. `--emulate NAME,...` runs single ported routines as original code, to find
  which port causes a difference. It only changes what original code calls: a ported routine
  that calls another one in C still gets the C version, so emulate the callers too.
- **Checking each port against its original** (`--check-ports`). Registers and the stack
  aren't compared under ported code, so a port that returns a wrong register shows only
  when it changes memory later, or when a lag NMI can't be placed. With `--check-ports`,
  every call of a ported routine from original code runs twice. The original runs first,
  on a copy of the state, with the ported routines it calls in C. Then the port runs for
  real. The two results are compared: registers (N, Z and C of the flags), memories (the
  stack below the stack pointer excepted), mapper and PPU registers, controllers and APU
  writes. Calls whose original waits for the NMI can't run twice and aren't checked, nor are
  routines that don't end in `RTS` (`main`, `jump_table`, `set_chr_banks_inline`). Calls from
  C aren't checked either, and the game now runs in C, so on its own this sees only what original
  code run for a lag NMI calls. To check the rest, run with `--emulate "$(python
  tools/nonleaf_routines.py)"`: every routine that calls another then runs as original code, and the
  145 leaves get checked at every call.
- **Fuzzing ports against their originals** (`--fuzz-ports N`, with `--compare` and
  `--frames 0` to do only that). Movies take only the paths the game takes: 12 ported
  routines never run in either movie. The fuzz runs every port and its original on N made-up
  states (random RAM and registers) and compares them as the port checks do. Samples are
  skipped where the original waits or doesn't return, or where a made-up pointer reads the
  stack below the stack pointer (in RAM or its mirrors), where the original reads back the
  return addresses its own `JSR`s pushed, which a port doesn't push, or reads `$2002`, which
  depends on the cycle count, which C doesn't keep. Skipped too: samples where original code
  the port calls returns past the port. A made-up sound header can send `play_sound` through the
  sound driver's jump table at `$FADA` into the weeds, and an `RTS` further up then ends at the
  port's caller, skipping the rest of the original; the port can't follow it there. The fuzz
  found that once in `draw_scene_dissolve` (in 5,000 samples) before this rule. A routine outside
  bank 7 runs as the far call that got there
  leaves things: its bank mapped, `current_bank` saying so, and the bank stack where it lives
  (`stack_index` from `$E0`, above the sound channels). With a made-up `current_bank`, the next
  `pop_bank` (`play_sound`'s, or `sound_update`'s in the NMI) mapped another bank under the
  original while the port went on in C: `press_a` differed in 172 of 173 samples so. With a
  made-up `stack_index` or sound pointers, `play_sound`'s sound data overwrote the bank it had
  pushed, a few times, so these routines also start with sound quiet. Also skipped: samples
  whose stack goes below `$0180`, into the sound driver's loop stacks, which are compared as
  data (a made-up script full of opcodes past the table nests that deep). The yields are always
  skipped so: they return past their caller, into made-up stack bytes; `script_loop`'s
  samples cover them. Skipped as well: samples that take a jump table past its end
  (`emu_note_bad_index`), as code then runs from whatever the bytes there say, on a stack that
  differs under C frames; 40 differences came from made-up `$BD` and switch keys that way, and
  none from anything else. So does a sample whose port returned past its caller, and one where a
  made-up pointer reaches the stack page or a mirror of it, in a port or in original code a port
  calls (`LDA (zp),Y` and the like), or writes to the mapper: the original has return addresses
  there, which a port doesn't push, so what lies where differs. `layout_byte_padded_command` read
  the original's return address as a number that way, where the port had X. Some routines start
  with a state that makes sense (`fuzz_help_routine`): a real hotspot under the cursor, a real
  special scene or layout, a real save file with a good copy half the time. Each routine's samples
  are seeded by its name, so adding a routine doesn't change the others' samples.
  `--fuzz-trace NAME:SAMPLE` prints every instruction of one sample under both.
  The far calls and `jump_table` aren't fuzzed: a made-up index calls whatever its table
  entry points to. Made-up states never set PRG mode 1 (bit 6 of `bank_select_shadow`): the
  game never does, and it swaps out the code at
  `$C000-$DFFF` under the original while the port carries on. A limit on 6502 cycles, and on
  calls of ported routines (C code takes no cycles), stops runs that wouldn't end. The summary
  at the end of a run also lists the ported routines that never ran; it counts a routine as
  run if another's guard asks about it, as `print_item_name_at` does for `print_item_name_arg`.
- **Fuzzing through waits.** Skipping every sample that waits would leave the routines that
  always wait with nothing compared, `print_message` among them. A second pass (N / 10 samples
  for each routine that had a sample skipped) doesn't skip them: at each wait the handler runs
  at once, on both the original and the port, and the A button changes so that loops waiting
  for a press and a release end. Both see the same NMIs if they wait alike; if they don't,
  `frame_counter` and much else differ. What isn't under test starts sane (`fuzz_quiet_nmis`):
  no sound playing, an empty VRAM queue, an empty sprite list. So `print_message_5` with a
  random id prints a real message from a made-up state to its end, and the registers it returns
  get compared. It only works through ported waits, so `wait_vblank` and its siblings
  themselves aren't compared; the movies run them at nearly every NMI.
- **Addresses** ([tools/check_ports.py](../tools/check_ports.py), the test `port_addresses`).
  A resume address is used only if a lag NMI lands there, so most never run. This checks each
  against the ROM without running anything: it must follow a `JSR` inside the routine it names.
  It also checks each routine's address and those in comments against the labels. When it was
  first run it found three resumes that went to the target of the `jmp` after the `JSR` rather
  than to the `jmp`: right in effect, except that a lag NMI on the `jmp` itself would have been
  taken late. A tail's address must be a label in that routine's bank (taken from the labels
  its `asm/bank*.s` defines, as the label file has no banks: a check against all labels missed
  a planted `$98AB`, a sound's label in bank 5), a jump table's must be a `JSR jump_table`, and
  each `jump_table(0xADDR, ...)` must give that JSR's last byte. All of these work in any bank
  (`BANKED` entries).
- **The NMI handler in C** ([src/nmi.c](../src/nmi.c)). It runs outside the game coroutine, as
  the original handler always has. Its hook returns `kEmuRti`, and the cycle count ends as the
  reference's, as with the original handler. No lag NMI can land in it, because it turns the
  NMI off first; none did in the movies. Two things differ from other ports:
  - **No sprite-0 wait.** C takes no cycles, so a spin on `$2002` would never end, and the
    wait only times a write against the picture, which the renderer will handle. The write itself stays.
    Nothing compared depends on the wait: the original
    reads `$2002` and only tests the result. In pass-through it still runs, on the model's
    approximate timing.
  - **Checks run to the RTI.** `--check-ports` and `--fuzz-ports` know the handler ends with
    `RTI` and compare it like any other routine.

  A replay's trace starts at the handler's RTI whichever way it ran: a ported handler leaves a
  marker there.
- **What the port check can't see.** It only checks calls from original code, and the
  handler's routines are called only by the handler. So one that's called from C isn't checked,
  whatever its registers. A planted wrong Y in `nmi_upload_palette` passed `--check-ports
  --emulate nmi`, because `nmi_ppu_updates` still called it from C. It takes the callers as
  original code too, level by level:
  1. `--emulate nmi` checks `nmi_ppu_updates` and `nmi_set_scroll`;
  2. `--emulate nmi,nmi_ppu_updates` checks the uploads;
  3. adding the uploads to the list checks `flush_vram_queue`, `ppu_addr_palette` and
     `ppu_addr_leave_palette`.

  The fuzz calls every port directly, so it needs no such help.

Ported ([src/funcs.h](../src/funcs.h)): 602 routines, 454 in bank 7, 125 in bank 6 and 23 in bank 2,
plus the far calls' bodies and `far_call_jump`, which the C far calls do inline, `flush_vram_entry`,
the loop in `flush_vram_queue`, `script_dispatch` and `sound_jump_table`. They make up all seven steps
of Phase 4's list. The fixed-bank infrastructure:
- the waits for the NMI (`wait_vblank`, `wait_vram_queue_empty`, `wait_nmi_work_done`);
- `main`, so C sits at the top of the call stack;
- bank switching: mirroring, `switch_prg_bank`, `mmc3_set_bank`, the CHR bank routines, the
  bank stack (`push_and_switch_bank`, `pop_bank`, `stack_push`, `stack_pop`, ...);
- the call-by-index routines: the four far calls, with their tables as C functions, and
  `jump_table`;
- the controllers (`read_joypads`, `read_joypads_once`);
- the work the game asks of the NMI: `request_nmi_work` and the routines that call it, the
  PPUMASK routines (`show_bg_and_sprites`, ...), `enable_nmi`/`disable_nmi`, and the sprite
  buffer (`clear_oam_buffer`, `wait_vblank_and_hide_sprites`, `clear_and_upload_sprites`);
- the VRAM queue ([src/vram_queue.c](../src/vram_queue.c)): blocks copied or filled row by
  row (`queue_vram_block`, `queue_vram_fill`), columns (`queue_vram_column`), the row and
  column steps, entry headers and the flushes of a full queue. Lag NMIs land in these more
  than anywhere else (`vram_copy_rows` alone has 184 of #1260's 890);
- the attribute buffers: `set_attr_block`, `set_attr_bits`, `fill_all_attrs` and their
  helpers;
- whole nametables, written to the PPU directly with rendering and the NMI off
  ([src/screens.c](../src/screens.c)): `clear_nametables`, `rendering_on`/`rendering_off`,
  the fills, and the run-length-coded screens (`draw_rle_screen_2000`, whose data follows
  its `JSR`: the title, dialog, logo and ending screens);
- the palette fades ([src/palette.c](../src/palette.c)).

And the NMI handler ([src/nmi.c](../src/nmi.c)): `nmi`, the uploads it dispatches to
(`nmi_ppu_updates`, `nmi_upload_palette`, `nmi_upload_attrs_2000`, `nmi_upload_attrs_2nd`,
`nmi_flush_vram_queue`, `nmi_flush_vram_queue_down`, `nmi_black_palette`), their helpers
(`flush_vram_queue`, `ppu_addr_palette`, `ppu_addr_leave_palette`) and `nmi_set_scroll`.

And the text decoder and printer ([src/text.c](../src/text.c); the format and the window are in
[text.md](text.md)): 44 routines, from `print_message_5` to `draw_more_arrow`.

And the scene pictures ([src/scenes.c](../src/scenes.c); the format is in
[scenes.md](scenes.md)): the 15 routines at the start of bank 7, from `change_scene` to
`load_dissolve_pattern`, and the 5 in bank 6 they far-call: `load_scene_chr_banks` (`$61`),
`load_scene_palette` (`$60`) with `load_palette` (`$62`) and `palette_pointer`, and
`dissolve_row_mask` (`$6B`). These are the first ports outside the fixed bank. A hook names
its bank (`BANKED` in [src/dv_rtl.c](../src/dv_rtl.c)) and runs only while that bank is
mapped, matched by ROM offset as every hook is. Not in this step:
`select_room_scene` and `draw_room_tile_patches` (bank 6, `far_call_8020` `$61` and `$62`),
which pick a room's scene and draw its patches over the picture; the script opcode that
enters a room calls them, so they go with the game logic.

And the cursor and the commands ([src/commands.c](../src/commands.c)): 36 routines, from
`handle_input` (far call `$69`) down: reading the controller (with key repeat in the command
area), moving the cursor in the picture, the command grid and the inventory and drawing it
(`$6A`), `reset_cursor` (`$68`), the A and B buttons, which area, command or inventory entry a
click is in (`test_hotspot` and `cursor_in_hotspot` in bank 7, the rest in bank 6), and the
markers on the chosen command and entry. What a click leads to came later: the scripts,
`click_picture` with the room's hotspots, `special_command_by_room`,
`clear_selection_unless_waiting`, `check_inventory_slot_empty` and `play_sound`. Three things are
new here:
- **A loop without a wait.** `get_input` reads the controller until a button is down and
  never waits for the NMI, so between NMIs the game spins in it, and the reference's NMI can
  land on any instruction of the loop. The C loop can't stop there, so every routine it runs
  checks its guard on every pass (`get_input`'s own at the loop's head, which is its start).
  One unguarded piece of the loop and such an NMI would hang the C side, not just come late.
- **Jumps into other code.** Two more guards in [src/funcs.h](../src/funcs.h):
  `TAIL_ORIGINAL_IF_LAG`, where the original jumps or branches into code that belongs to
  another routine (`press_a` into the end of `press_b`, `move_cursor_in_scene` into the end of
  `move_cursor_in_inventory`), and `JUMP_TABLE_OR_ORIGINAL` at each `JSR jump_table`, since C
  calls `jump_table` without a guard of its own and a made-up index jumps to whatever the
  bytes after the list say. Both run the rest of the routine as original code from that very
  place (`emu_resume_original`), which needn't follow a JSR.
- **Jumps out to original code.** `click_command_area` ends with `JMP special_command_by_room`
  and `click_at_cursor` goes to `click_picture` through `jump_table`: the original code runs on
  the port's stack frame, not a frame of its own. `emu_call` would push a return address and
  run it two bytes lower, and the fuzz found `special_command_by_room` storing a value that
  depends on the stack pointer (`$5E` off by 2, in 2 of 5,000 samples). `emu_jump` runs it
  without a return address until its `RTS` takes the stack pointer above where it started,
  then puts the stack pointer back for the port's own return.

And the script interpreter ([src/scripts.c](../src/scripts.c); the format is in
[scripts.md](scripts.md)) with all 105 opcodes, 177 routines: `run_command`, `script_start`,
`script_resume`, `script_loop`, the operand fetchers, the control flow (goto, switch, the yields)
and the message opcodes; every opcode that only tests or sets the five flag arrays, `item_status`
or `object_state` ([src/flags.c](../src/flags.c)); and the rest, with what only they use
([src/opcodes.c](../src/opcodes.c)). How an opcode runs:
- **At the original's stack depth.** `script_run_opcode`'s `JSR script_dispatch` pushes a return
  address and `jump_table` drops its own, so a handler runs two bytes down. It runs with those two
  bytes pushed (`run_opcode`).
- **Handlers that end the script.** Five (`script_op_yield`, `op_48`, `op_06`, `op_32`, `op_5E`)
  do `PLA`, `PLA`, `JMP script_end`: they drop that return address, and `script_end`'s `RTS`
  returns from the whole script. In C, `end_script` pulls those two bytes, and `run_opcode` sees
  from the stack pointer that the script is over; called from original code, the same pull takes
  the real bytes. They have no guard at the top, as `emu_call_original` would give their `PLA`s
  the wrong bytes. A lag NMI in one goes through `emu_call_may_unwind`, which runs it until it
  returns or the stack pointer rises above where it was. `emu_resume_original` stops the same way,
  for a resume inside one, leaving the stack pointer to show it.
- **A bug kept.** `clear_flag_045C` and `op_25` do `PHA`, `TXA` where their siblings do `TXA`,
  `PHA`, so they return with X = 4 instead of X kept. The port does too (`clear_flag_losing_x`).

And the rest of bank 7's game logic, 96 routines: the hotspots, `pick_default_command` and the item
codes ([src/rooms.c](../src/rooms.c)), the special scenes and their waits
([src/special_scenes.c](../src/special_scenes.c)), the save system
([src/saves.c](../src/saves.c)), screen layouts, the numbers they print and `draw_metasprite`
([src/layouts.c](../src/layouts.c)), and `reset`, `new_game`, `next_random` and the game screen
([src/game.c](../src/game.c)). What is new about them:
- **Guards in long loops.** A lag NMI can land in a routine a C loop calls; the callee runs as
  original code and takes it, and the C loop goes on. If the next NMI then lands in the loop's own
  code, a guard at the routine's top no longer helps. In TAS #7044 a glitched room sends
  `find_clicked_hotspot` walking through memory for 45 frames, and without a check on each pass all
  45 went late, and the next difference showed at NMI 25642. So these loops, and the save code,
  which can take more than a frame, check after every call.
- **A power-on in C.** `reset` throws the stack away (`emu_set_stack` tells the bridge), and it
  spins on `PPUSTATUS`, adding the loop's 7 cycles per pass, as C keeps no count of its own. The
  bridge runs the original rather than the hook for a lag NMI inside, so it has no guard at the top.
- **A hang kept.** `repair_save_copies` never ends unless copy 0 is the good one
  ([conventions.md](conventions.md), "Saving"). The C loop does the same, reaching a guard on each
  pass.
- **The jump tables are switches.** A wrong `case` is easy to write and only a run shows it:
  `special_scene_action` had the final scene at 25, not 23, which stopped both movies at about NMI
  3,100. `check_ports.py` checks a table's JSR but not what each entry does.

And the game logic in banks 2 and 6, 110 routines: which scene a room shows, its tile patches and
object sprites ([src/room_view.c](../src/room_view.c)), the script slot for a command, the default
responses, the rooms with their own ending and paging ([src/room_commands.c](../src/room_commands.c)),
the inventory ([src/inventory.c](../src/inventory.c)), the slot machine, logos and ending
([src/slot_machine.c](../src/slot_machine.c)), and the title, file select and dialogs
([src/menus.c](../src/menus.c)). What is new about them:
- **Data after a JSR.** Original code keeps a `JSR draw_rle_screen_2000` or `JSR set_chr_banks_inline`
  with the data right behind it, and those routines read it through the return address. C has no
  return address, so `rle_screen_from` and `SET_CHR_BANKS_INLINE` push what the JSR would.
- **The last wait.** The game ends in `JMP` to itself at `6:B2C6`. `ending_sequence` waits there in C,
  so all of both movies' waits now happen in ported code.
- **Two copies of a bug left alone.** The check for sprite kind 2 unbalances the stack
  ([rooms.md](rooms.md)), so `object_sprite_visible` runs that as original code. And
  `delete_inventory_entry` ends `PLA`, `TXA`, `PLA`, `TYA`, not restoring X and Y, which the port
  keeps.

And the sound driver, 27 routines ([src/sound.c](../src/sound.c); the format and the channel slots are
in [sound.md](sound.md)), from `sound_reset` to `sound_write_apu`. What is new about it:
- **Nothing waits.** So no routine here needs a resume: a guard at the top of each is enough. No NMI
  lands in what only the handler runs, and one that lands in what `play_sound` runs finds
  `system_flags` bit 2 set, and only asks for a double step next time.
- **A check of its own, with no emulator.** `tests/test_sound.c` runs the original as 6502 code on the
  ROM's real sound data against the port, comparing after every call. The movies play only some of
  the sounds (no effect of type 2), and the fuzz's made-up streams rarely get far; this runs them all.
- **Its loop stacks are in the stack page.** Slots 7 and 8 have theirs at `$0180-$019F`, above
  `kStackFloor`, so a comparison at an NMI doesn't see them while the stack pointer is above them;
  `tests/test_sound.c` does. A made-up index past them reaches the real stack, so a check skips that
  sample (`emu_note_loop_stack`).
- **A slip kept.** Triangle mode 2 writes both bytes of its time to one place ([sound.md](sound.md)).

Left unported, the code nothing calls: in bank 7 `unreferenced_*`, `item_in_any_container` and
`select_flags_0430`; in bank 6 `unreferenced_*` and what only `unreferenced_9847` reaches
(`print_04B4_message`, `prompt_message_2B`, `wait_choice_input`, `play_click`, `draw_choice_marker`),
which looks like a tool for viewing the messages (a guess); `far_8400_25` in bank 2 and `far_8000_65`
in bank 6, in far-call tables but with no call; and in bank 7 `script_fetch_3`,
`set_mirroring_horizontal`, `request_attrs_2nd`, `show_sprites`, `hide_sprites` and
`draw_rle_screen_2400`, with `rle_to_ppu_2400` and `rle_write_2400`, which only it calls. Those nine were
ported at first and taken out later. `show_sprites` and `hide_sprites` stay in `kPortedRoutines` as
`INLINE`: their code holds the end that `show_bg_and_sprites` and `hide_bg_and_sprites` share.

**Ported, though the game's own data never gets there.** Code jumps to these, through a table or a
test, but nothing in the ROM selects them. None ran in any of the five movies. They stay because they
are what the script, layout, text and sound code can do, and the game's data can be edited
([formats.md](formats.md)):
- 11 script opcodes no script uses: `op_0C`, `op_14`, `op_15`, `op_20`, `op_23`, `op_24`, `op_2A`, `op_2B`,
  `op_2D`, `script_op_push_goto` and `script_op_pop_goto`. Three more opcodes are unused, but other code
  reaches their routines: `$03` shares `$04`'s, and `take_item_from_container` calls `$22`'s and `$25`'s.
- Layout commands `$00`, `$01`, `$02`, `$03` and `$05`, which no layout uses, and the helpers only they
  call: 14 routines.
- `print_item_name_arg`, `print_word_arg` and `text_read_byte`, for text codes the bit stream can't
  produce ([text.md](text.md)), and `text_pause`, whose setting is only ever cleared.
- `sound_start_effect_3ch`: no code, room or script starts a sound of type 2 (5, 6, 7, 9 or 10).

This comes from tracing the ROM from its vectors, with the opcode and layout tables cut down to the
entries the scripts and layouts use. It is one-off work, not a test. Another 37 ported routines didn't
run in a movie, but code or real data can reach them. Four of those look unreachable in normal play
(a guess): `shift_save_copies`, `copy_save_copy`, `next_copy_pointer` and `reset_game_state` run only
if a first copy that passed the title's check fails when it's loaded.

**The far calls** go straight to C: `far_call` looks the target up in tables of C functions
([src/bank_calls.c](../src/bank_calls.c)), by what is mapped at the table, as the 6502 would find it.
[tools/check_ports.py](../tools/check_ports.py) checks the tables against the ROM's. They hold 41 of
the 43 real entries; `far_8000_65` and `far_8400_25` have no call, and an id with no routine runs as
original code.

**What still runs as 6502 code** in the movies: only what a lag NMI needs. Counted over a whole run
(a one-off, with a temporary counter in the bridge), every 6502 instruction came in a call run as
original code for a lag NMI, with a lag NMI pending, or after one in the routine it interrupted:
508,500 in #1260 and 2,836,027 in #7044, with none left over. The code that runs as original only for
made-up or glitched data never ran in either movie, TAS #7044's glitches included (a second one-off):
a far call with no routine, a jump table's index or a script opcode past its end, a sound type past
the list, and sprite kind 2. It stays in the source, as the checks need it: a made-up state gets there.

## What is verified

- **The in-process reference matches Mesen2 run from Mesen.exe.** `run_mesen.py --log-nmi`
  makes `record_cdl.lua` write the same per-NMI line from Lua, in Mesen2's own test runner.
  For TAS movie #1260, all 37,367 NMIs that run logged are identical to `dv_harness`'s
  lines: registers, cycle counts, all five memories and the APU writes. Both runs end with the
  same progress (417 flag bits, 21 address book and inventory slots). For #7044 (glitched,
  `--offset 1`), all 34,071 lines are identical too.
- **Runs repeat exactly:** a second run of #1260 writes the same log.
- **Save states replay exactly:** `--rewind-check 100` over all of #1260 rewound 372 times,
  and every replayed NMI matched the first pass. The CMake test `harness_rewind` runs the first
  6,000 NMIs with a rewind every 50, where the ROM and `build/movies/tas1260.inp` exist.
- **Pass-through matches:** `--compare --pass-through` finds no difference at any of the
  37,380 NMIs of #1260 or the 34,083 of #7044. Every lag NMI was found by state: 890 in
  #1260 and 3,575 in #7044 (the first version found the same ones by cycle count). A
  deliberately wrong palette mask was caught at the first NMI after the write.
- **With the 145 ported routines** (`--compare --check-ports`) there is no difference either,
  at any NMI of either movie, and every NMI is compared. Every lag NMI was placed exactly by
  state: 890 in #1260 and 3,575 in #7044. None was placed by memories and none was late. For
  that, 5,485 calls of ported routines in #1260 (5,170 in #7044) ran as original code, and
  35 (10) went on as original code after a wait inside them. In #1260, 36,224 NMIs came
  while the game waited in C and 266 in the halt after the ending; in #7044, 30,508 in C
  waits. The port checks ran 190,481 calls in #1260 and 156,366 in #7044 against their
  originals, with no difference; those include every NMI, the C handler against the original
  handler. 12 ported routines never ran, among them the new `text_read_byte`, `print_word_arg`
  and `text_pause`. With the NMI handler's callers as original code, each level above also found
  no difference: 250,935 and 212,749 calls with `nmi` emulated, 241,579 and 199,726 with
  `nmi_ppu_updates` too, 241,504 and 199,644 with the uploads too (#1260 and #7044, at 101
  routines). So did three levels of the printer with real text, at 145: 216,429 and 173,767
  calls with `print_message` and `print_control` and the routines above them emulated, 216,836
  and 174,068 with what those call too, and 299,896 and 241,772 with all but the leaves. These
  runs matched at every NMI as well, with no lag NMI placed late.
- **With the 165 ported routines** (the scene pictures added), the same: every NMI of both
  movies matches, every lag NMI is placed exactly by state
  (890 and 3,575), none late. 5,516 calls (5,198 in #7044) ran as original code for a lag NMI,
  and 35 (10) went on as original code after a wait. All 20 new ports ran in both movies. The
  port checks ran 153,482 and 131,988 calls with no difference: fewer than at 145, because the
  scene routines now call the VRAM queue and the rest from C, where the check doesn't see them.
  With `change_scene`, `change_scene_instant` and `load_scene` emulated, 154,867 and 132,947;
  with every scene routine that calls another emulated too (`draw_scene_dissolve`,
  `draw_scene`, `dissolve_draw_tile`, `load_scene_attrs`, `load_scene_palette`,
  `load_palette`, `blacken_palette_colors`, `apply_room_settings`), 292,985 and 222,489. None
  differed, and those runs matched at every NMI too. Pass-through on #1260 still matches.
- **With the 201 ported routines** (the cursor and commands added), the same again: every NMI
  of both movies matches, every lag NMI is placed exactly (890 and 3,575), none late, and the
  `get_input` loop never hangs. The port checks ran 155,860 and 221,161 calls with no
  difference; with `handle_input` and `get_input` emulated, 159,742 and 224,374; with every
  routine of the step that calls another emulated too, 176,993 and 226,064. In #7044, 62,746
  calls ran as original code for a lag NMI (5,198 at 165): most likely its stretches in
  `get_input`'s loop, where each pass runs the routine the NMI is in as original code until the
  pass it came in (each pass strobes the controller, so they differ); #1260 stays at 5,516.
  Pass-through on #1260 still matches.
- **With the 286 ported routines** (the interpreter and flag opcodes added), the same again:
  every NMI of both movies matches, every lag NMI is placed exactly (890 and 3,575), none late.
  The port checks ran 159,162 and 223,698 calls with no difference. Emulating has to start at
  the top now, as `press_a` calls `run_command` in C. With `handle_input` down to `script_loop`
  emulated, 167,167 and 229,831; with every routine of steps 5 and 6 that calls another emulated
  too, 192,923 and 237,990. None differed, and all of those runs matched at every NMI with no
  lag NMI late. Pass-through on #1260 still matches. Fifteen of the new opcodes and helpers never
  run in either movie (`script_op_push_goto`, `op_0C`, `op_53_set_flag_045C`, ...); the fuzz
  compares them all.
- **With the 378 ported routines** (all 105 opcodes), the same again: every NMI of both movies
  matches, every lag NMI is placed exactly (890 and 3,575), none late. The port checks ran 158,157
  and 301,228 calls with no difference. With `handle_input` down to `script_loop` emulated,
  166,317 and 307,496; with every routine of steps 5 and 6 that calls another emulated (171 of
  them), 194,009 and 316,800. None differed, and all of those runs matched at every NMI with no lag
  NMI late. Pass-through on #1260 still matches. 21 of the 92 new routines never run in either
  movie (`op_63`, `op_47`, `close_container`, `op_3D`, ...); the fuzz compares them all, though some
  on few samples (`op_3A` on 503, `close_container` on 84).
- **With the 474 ported routines** (all of bank 7's game logic), the same again: every NMI of both
  movies matches, every lag NMI is placed exactly (890 and 3,575), none late. The port checks ran
  187,765 and 317,661 calls with no difference. With `main`, `reset` and every routine of steps 5
  and 6 that calls another emulated (242 of them), 247,623 and 351,751; none differed, and those
  runs too matched at every NMI with no lag NMI late. Pass-through on #1260 still matches. 25 of the
  96 new routines never run in either movie, which never save: `load_game`, `save_game`,
  `read_save_copy`, `write_save_copy`, `reset_game_state`, `repair_save_copies` and the like, and some
  of the layout commands. The fuzz compares them all, in one pass or the other.
- **With the 584 ported routines** (all the game logic), the same again: every NMI of both movies
  matches, every lag NMI is placed exactly (890 and 3,575), none late, and every wait is in ported
  code, the end of the game's included. The port checks ran 183,941 and 314,580 calls with no
  difference. With `main`, `reset` and every routine of steps 5 and 6 that calls another emulated
  (338 of them), 255,328 and 358,741; none differed, and those runs too matched at every NMI with no
  lag NMI late. Pass-through on #1260 still matches. 9 of the 110 new routines never run in either
  movie: `save_dialog`, `show_file_options`, `draw_file_options`, `draw_option_cursor`,
  `far_8000_65` and the four rooms with their own ending; the fuzz compares them all, most through
  waits on a few hundred samples. Four get no fuzz sample, as made-up states don't get through them,
  and only the movies check them: `title_screen`, `stop_reels`, `ending_sequence`,
  `ending_wait_button`.
- **With the 611 ported routines** (the sound driver added), the same again: every NMI of both movies
  matches, including its APU writes, every lag NMI is placed exactly (890 and 3,575), none late. The
  port checks ran 107,476 and 243,683 calls with no difference: fewer than at 584, because the handler
  now calls the driver from C, where the check doesn't see it. With `main`, `reset` and every routine
  of steps 5 to 7 that calls another emulated (353 of them), 180,333 and 289,049; none differed, and
  those runs too matched at every NMI with no lag NMI late. Pass-through on #1260 still matches.
  `sound_start_effect_3ch` never runs in either movie, so sounds 5, 6, 7, 9 and 10 don't play there;
  the next bullet does play them.
- **With the 602 ported routines** (the nine nothing can call taken out), nothing moved: every NMI of
  both movies matches, as they are and in pass-through, the same 890 and 3,575 lag NMIs are placed
  exactly, none late, and the port checks ran the same 76,299 and 219,339 calls with no difference.
  With `main`, `reset` and every routine that calls another emulated (463 of them), 1,902,064 and
  1,685,879 calls; none differed. The fuzz compared 1,771,375 samples, and 151,835 through waits, with
  no difference.
- **Saving and loading in play** (`tests/run_save_movies.py`, the test `harness_saves`): the TAS movies
  never save, so three scripted movies do, on the reference and the C side in lockstep. Every NMI of
  all of them matches, none late, and the save files are byte for byte the same on both sides, each
  run starting from the file the other side wrote. `--sav` and `--sav-out` start from a save file and
  keep the one Mesen2 writes. [saves.md](saves.md) has the details.
- **The sound driver on the real data** (`tests/test_sound.c`, the test `sound_driver`, about 7
  seconds): 49 sounds played alone (entry 8 isn't a sound) for 434,774 frames, the ten that are
  repeating music until they had started again, which takes sound 2 263,500 frames, then 400,000 frames
  of random sounds over each other, stops, and updates deferred by a busy game. 837,982 calls compared,
  3,651,041 APU writes, none different. 778 of the driver's 798 instructions ran; [sound.md](sound.md)
  lists the 20 that didn't, which are 7 short pieces. A note
  value planted wrong was caught on the first frame it mattered. The 7 pieces rest on the fuzz, which
  was tried with one planted bug in each: it caught 6, and the seventh after it was given real triangle
  modes.
- **With the far calls in C**, the same again: every NMI of both movies matches, every lag NMI is placed
  exactly (890 and 3,575), none late. The port checks ran 76,299 and 219,339 calls with no difference,
  fewer again since far calls no longer pass through a hook. With every routine that calls another
  emulated (466 of them, from `tools/nonleaf_routines.py`, which covers all of Phase 4 where the list
  above had steps 5 to 7), 1,902,064 and 1,685,879; none differed, and those runs too matched at every
  NMI with no lag NMI late. Pass-through on #1260 still matches. Two mistakes planted in the far-call
  tables, two entries swapped and one too many, were named by `check_ports.py`.
- **The fuzz** (`--fuzz-ports 5000`): 1,791,335 made-up samples compared, none different, and 153,831
  more through waits, none different (1,663,305 and 144,343 at 584), in about 4 minutes. The original's
  run decides what is too long, and the port gets twice that: its own guards count as work where the
  original's own code doesn't. With C sound in the NMIs, `print_message` on one made-up state ended at
  996,728 under the original and passed the limit of 1,000,000 under the port, which read as "the port
  didn't return". Random
  states almost never get anywhere in `close_container`, `op_47`, `op_32` and `op_32_body`, nor
  in the hotspots, special scenes, save code and sound driver, so those start with just enough sense
  for their code to run (`fuzz_help_routine`).
  Every new routine but `handle_input` and `get_input` is compared: with made-up controllers
  nothing is pressed, so `get_input` spins on, and it never waits, so the second pass's button
  presses don't reach it either. The movies run both at nearly every frame. Five of the 20 scene routines always
  wait (`change_scene`, `change_scene_instant`, `load_scene`, `draw_scene`,
  `draw_scene_dissolve`), so only the second pass compares them: 386 to 500 samples each. The
  first pass compares nothing of 19
  of the 44 printer routines, because they always wait; through waits all 19 are compared, and
  `print_message_5`, `print_message_4` and `print_message_2_or_5` print 1,362 real messages to
  their ends. It covers ten of the twelve ports the movies never run (`set_mirroring_horizontal`,
  `vram_wrap_up`, `vram_wrap_column`, `flush_vram_queue_down_if_full`, `rle_write_2400`,
  `request_attrs_2nd`, `show_sprites`, `text_read_byte`, `print_word_arg`, `text_pause`). Not
  covered by either: `draw_rle_screen_2400`, which nothing calls and which always waits, and
  `rle_to_ppu_2400`, whose made-up data never ends cleanly; each shares its code with a sibling
  the movies run. (Those nothing can call have since been taken out.) Before it was clean the fuzz found only artifacts of made-up states, all now
  handled: open bus after a pointer read (the ports now read pointers through the bus as the
  6502 does, `emu_read_ind_y`), stack reads through the RAM mirrors, far-call and `jump_table`
  indexes, a C loop over endless made-up RLE data that the 6502 cycle limit didn't see, PRG mode
  1, and a source pointer aimed at `$2002`. It also found one thing to fix in the ports: a
  made-up state with A and its flags disagreeing showed that `print_digit_skip_zero` takes the
  caller's flags as an input. So does `print_message_2_or_5`, found by reading it after that,
  and its callers aren't all a plain `LDA`: `op_02_message_2_or_5` gets A from
  `script_fetch_byte`.
- **Detection:** a deliberately wrong value in the C `clear_oam_buffer` was caught at the
  first NMI after it ran (NMI 226 of #1260). A deliberately wrong A from the far calls was
  named by the port checks in the first frames ("far_call_8000 ... a=00/FE"). A palette upload
  one byte short was caught at NMI 6 ("palette $3F1F: C side 08, reference 0F"), and a wrong Y
  from it was named by the port checks with its callers emulated, and by the fuzz
  ("nmi_upload_palette ... y=1F/20"). A wrong tile in `text_window` was caught at NMI 590 of
  #1260 ("ram $06DE: C side CA, reference C9"), and a wrong A from `print_message_from`, which
  always waits, only by the fuzz through waits ("print_message_from ... a=A6/9D"): the first
  pass compared none of its samples.
- **What the bridge found on the way**, each a bug in the ports or in the bridge:
  - `clear_oam_buffer` and the far calls end with `PLA`/`TAX`, so they return A = X. The C
    versions first returned `$F8` and the target's result. Each showed only as a lag NMI that
    couldn't be placed, frames later (`$EBE1` in #7044; `draw_scene_dissolve` at `$C0FD`
    in #1260, which uses A after a far call). That is why `--check-ports` exists.
  - `stack_pop` and `stack_push` use `TSX`, so under ported code X differs as the stack
    pointers do. The state match now counts X as matching if it is as far from the stack
    pointer on both sides.
  - Running the original instead of a far call whose lag NMI is in its body (ported inline)
    first landed on the far call's own hook again, which recursed without end (#7044, NMI
    19,484). `emu_call_original` runs the first instruction as original code. A nesting
    limit (500 ported routines deep) now stops such a loop with a message instead of a crash.
  - Matching memories at every guard took NMIs too early (`read_joypads_once` reading zero
    bits), and taking the NMI at the end of any far call took it in a nested one. Memories
    are now matched only at the end of a routine that can wait, and at waits.
- **Without the ROM** (CTests run in CI): `tests/test_cpu6502.c` (`cpu6502`) checks cycle
  counts, page crossings, flags, the stack, `JMP ($xxFF)` and the order of bus reads.
  `tests/test_bridge.c` (`bridge`) runs a small made-up program with and without ported
  routines. It checks each way an NMI is placed: running the original instead (also for
  code done inline), going on as original code after a wait inside a routine that pushed,
  by memories at a wait and at a routine's end, and late. It also checks the errors,
  original code called from C that waits, a replay after a failure, a handler in C (its RTI,
  the cycle count after it, and that the port check and the fuzz tell a right handler from a
  wrong one), and the fuzz through waits telling a right routine that always waits from a wrong
  one.
  `variables_current` checks that `src/variables.h` matches `asm/ram.inc`.
- **Speed:** a movie runs at about 330 NMIs a second on the reference alone, about 300 with
  `--compare --pass-through` (the harness saves both sides' states every NMI, for replays) and
  about 280 with the ported routines and port checks (most calls are from C, which aren't
  checked). Four runs at once take about 230 each.

## The picture

[src/ppu_render.c](../src/ppu_render.c) draws a frame from the PPU's state, a line at a time. With
`--picture`, the harness compares it with the frame Mesen2 has just finished, at every NMI. Both are
palette colors (6 bits) with the emphasis bits above them, so RGB doesn't come into it, and the port's
palette can't hide or cause a difference.

- **When it draws.** Right after the NMI handler, from how things are as the PPU starts on the frame.
  That frame is the one the reference finishes at the next NMI. A frame that follows frames with no NMI
  (the game turns it off to draw a whole screen) counts apart.
- **The split.** C has no time inside a frame, so the handler doesn't wait for sprite 0. It leaves
  PPUCTRL as it is below the split and says what it was above it (`split_on`, `split_upper_ctrl`), and
  the renderer finds the hit itself and changes over on the next line. As every frame matches, the
  split's exact dot doesn't show: the game keeps it where the two screens look alike.
- **Results.** TAS #1260: all 37,373 frames the same, and the 6 after frames with no NMI. TAS #7044:
  34,065 of 34,076, and its 6. The 11 that differ all have reads of `$2007` in them, from the stray
  pointer described above, and only 12 frames in the movie do. A read of `$2007` while the PPU draws
  knocks its scroll counters off, which a line-at-a-time renderer doesn't model and no ordinary play
  does. Nothing else the renderer uses changed during those frames. That the read is the cause rests on
  this match and on the PPU's documented behavior; the effect hasn't been reproduced in the renderer.
- **What that doesn't check: the colors.** It compares palette entries, so a wrong palette passes it on
  every frame, and one did. The first `ppu_make_palette` had the hue phase wrong by one of the 12 steps,
  so every color was 30 degrees round the wheel: the title's blue sky purple, its yellow letters green.
  Only comparing the two windows by eye showed it. Now `--check-palette` compares the RGB with Mesen2's
  default palette, read from its source in `external/` as it runs: off by 0.52 of 255 on average, by 3
  at worst (the test `palette_vs_mesen` in harness builds). The colors are the NTSC signal's, from the
  wiki's levels, at the saturation (1.56) that fits best, so nothing is copied from that table.
  `tests/test_palette.c` (the test `palette`, in CI) says what the colors should be without any
  emulator: grays, the 12 hues round the wheel, how strong the title's blue and yellow are, and emphasis.
- **Looking at a frame.** `--picture-dump DIR` writes `diffs.txt` and both sides of the first frame of
  each run of differences, and [tools/frame_png.py](../tools/frame_png.py) makes a PNG of them.

## The sound

[src/apu.c](../src/apu.c) is the port's own APU, from the NESdev wiki's description: the frame counter, the
length counters, envelopes, sweeps, and the pulse, triangle and noise channels, stepped once per CPU
cycle. The game doesn't use the DMC, so there is none. `--apu` runs it on the reference's own register
writes at their CPU cycles and compares its state with the reference's at every NMI. That tests the chip
apart from the game's timing.

- **State.** All of it: duty and position, period, timer, sweep settings, enable, length, halt, envelope,
  triangle step and linear counter, noise shift register and mode. **No difference at any of the 37,380
  NMIs of #1260 or the 34,083 of #7044.** What getting there took: the reference's cycle count is one
  behind, the pulse timer's period is 0 at power-on and not 1, and the parity of the `$4017` write's
  delay. Before the last of those, 9 and 37 NMIs differed by a single clock, where the frame counter
  fell beside the comparison.
- **Sound.** `--audio-wav PREFIX` writes both sides' sound, and
  [tools/compare_audio.py](../tools/compare_audio.py) compares them band by band. On the first 200
  seconds of #1260: the reference's level is within 2% of this APU's from 80 Hz to 5 kHz (0.992 to
  0.998 below 2.5 kHz, where nearly all the power is), with coherence 0.997 to 1.0000. Below 80 Hz and
  above 5 kHz the filters differ, by design and little power: the reference has more above 16 kHz, where
  this one has a low-pass. Overall -21.9 dB is left over, mostly from there; the worst second has -7.8.
  The 1.7 samples between them are the filters' delays.
- **A choice.** The NESdev wiki has the chip read its duty table in the order 0, 7, 6, ...; Mesen2 goes
  through it the other way, which starts each pulse at another point in its cycle. No ear can tell, as
  it only moves pulses against the other channels by a fraction of a cycle, but the waveforms correlated
  at about 0.8 until they agreed (0.99 after). This follows the reference.
- **The timing in the game itself.** C has no time inside a frame, so in the game on its own all of a
  frame's writes go in at its start, where the original has them a few thousand cycles later. So each
  note starts at another point in its cycle, and the two can't be compared sample by sample, though the
  ear can't tell. Comparing spectrograms instead (`dejavu --audio-wav`, against the reference with no
  input, both idle at the title for 13 seconds): the same notes at the same times, the reference exactly
  8 frames later (its frames before the first NMI and with the NMI off, which take C no time), overall
  loudness the same (ratio 1.000), magnitudes correlated at 0.98, and at 0.96 to 0.996 up to 1 kHz. On
  the machine it was tried on, the sound's queue stayed within about 300 samples of its target of 2,404
  over 20 seconds, with frames within 0.1% of their nominal length.
- `tests/test_apu.c` (the test `apu`, in CI) checks what can be told from the docs alone, without the
  ROM: the frame counter's clocks, length, envelope, sweep and muting, timer rates, the noise register
  and the output's rate.

It has also been compared by ear with Mesen2, the exit of Phase 5, and it matches.

## The game on its own

`emu_run_frame` ([src/dv_cpu_infra.h](../src/dv_cpu_infra.h)) runs the game with no reference: the NMI
handler of the frame before, then the game until it next waits. No 6502 code runs in normal play.
[src/main.c](../src/main.c) does that in a window ([src/platform_sdl.c](../src/platform_sdl.c), SDL2), at
the NES's 60.0988 frames a second by the clock, with the save RAM kept in a file.

- **No lag frames.** C takes no time, so every NMI comes at a wait. Work that takes the original more
  than a frame (saving: `backup_state_to_save_ram` has 109 of #1260's lag NMIs) finishes at once. Only
  that work gets faster: what the game paces with waits keeps its pace.
- **Loops that never wait.** On the console the NMI comes anyway; here something has to give it a
  chance. `get_input`, `title_screen` and `ending_wait_button` spin on the controller, and call
  `emu_spin`. Any other such loop gets a frame after 200,000 calls of ported routines with no wait,
  which real work never reaches (at most about 2,500 routines fit in a frame on the console).
- **Glitches run as 6502 code.** The capsule warp can send the game running through memory, where no
  port is. Such code gets a frame's worth of cycles and then the frame passes, so a loop in it can't hang
  the window, and the NMI comes where it is, as on the console. An opcode that freezes a real 6502 freezes
  the game for good: frames still pass, with no NMI, as on Mesen2. Closing the window still works.
- **Key repeat counts passes, not frames.** In the command area `key_repeat` counts passes of
  `get_input`'s loop while a direction is held, and the cursor moves on every 128th. So the loop has to
  run at the original's rate: 9 passes a frame. A pass of the original takes about 1,000 cycles
  (`read_input` is 990 in that mode), and the frame leaves about 9,000 after the split; the one frame
  in the movies that sits in the loop throughout has 9. That makes about 14 frames a step.
- **Checked** (`--compare --free-run`): the C side on its own matches the reference at every NMI of
  both movies up to the first real lag NMI, at NMI 272 of #1260 and 270 of #7044 (`$CA1B`, in the save
  code), where it can't follow. NMIs in the spins aren't compared, as the reference is partway through
  reading the controller there; the two stay in step through them. That is only the title screen and
  the file menu. Past it, the movies' inputs no longer line up, and the reference-driven runs check the
  same code. `tests/test_game.c` (the test `game_alone`, where the ROM is present) has a monkey press
  random buttons: over 200,000 frames, about an hour of play, it saw 10,299 different pictures and 7
  rooms and ran 391 of the routines, with no call of original code and no hang.

## What the reference shows about frames

The game waits for the NMI in two loops, and nearly every NMI lands in one of them:

| Wait | Where the NMI returns | NMIs in TAS #1260 |
|---|---|---|
| `wait_vblank` (`7:EB9C`): sets bit 1 of `system_flags`, spins until the NMI clears it | `$EBA4`/`$EBA6` | 11,013 |
| `wait_nmi_work_done` / `wait_vram_queue_empty`: spin until the NMI clears bits of `nmi_requests` | `$EBB4`/`$EBB6` | 25,211 |
| The `jmp *` at `6:B2C6`, where `ending_sequence` stops after the epilogue | `$B2C6` | 266 |
| Anything else: the NMI interrupted game code | | 890 |

So the C port has **two** kinds of yield point, not one: `wait_vblank` and the
`nmi_requests` spin. `dv_harness` counts NMIs outside
both loops as "interrupted game code"; that count includes the 266 at the end of the game.
The 890 real interruptions are mostly in `vram_copy_rows` (184), `read_joypads_once` (165),
`backup_state_to_save_ram` (109), `text_scroll_rows` (69) and `mmc3_set_bank` (68). These are
the lag frames the comparison has to line up. The NMI leaves `read_joypads_once`'s
bytes alone; [conventions.md](conventions.md), "What the NMI changes", has the list of what it
does change.

## Next

- **Replays with ported code** need the coroutine's stack saved with the rest (a coroutine
  whose stack the harness owns, copied with the state).
- A second emulator as a check on the reference.
- CI has no ROM, so movies run in lockstep only where the ROM is present.
