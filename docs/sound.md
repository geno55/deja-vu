# Music and sound effects

The sound driver lives in the fixed bank and runs from the NMI. Its data fills bank 5 from
`$8000` to `$99FF`, before the messages. `tools/sound.py` maps every table, header, channel
stream and instrument there; every byte the game read in that area during the recordings
falls inside the map.

Names match [asm/labels.txt](../asm/labels.txt).

## Playing a sound
`play_sound` (`7:F409`) takes a sound number in A, counted from 1 (0 stops everything), and
reads entry n-1 of `sound_table` (`5:8000`, 50 entries). `room_settings` (`7:C21F`) gives
each room the number of its music (see [scenes.md](scenes.md)).

Each frame, `sound_update` (`7:F538`) switches to bank 5 and steps nine channel slots of 24
bytes each at `$0700`: four for music, three for one kind of sound effect, two for another.
The code is ported in [src/sound.c](../src/sound.c).

## Channel slots
| Slot at `$0700+` | Type | APU channel | Loop stack at `$0110+` |
|---|---|---|---|
| `$00`, `$18`, `$30`, `$48` | music | pulse 1, pulse 2, triangle, noise | `$00`, `$10`, `$20`, `$30` |
| `$60`, `$78`, `$90` | effect on 3 channels | pulse 1, pulse 2, triangle | `$40`, `$50`, `$60` |
| `$A8`, `$C0` | effect on 2 channels | pulse 2, noise | `$70`, `$80` |

Each APU channel plays the first of its slots that is on, in the order effect on 2 channels, effect
on 3 channels, music. `sound_write_apu` (`7:F9B6`) writes `$4015` every frame, and register 0 of each
channel that has a slot on, but registers 1-3 only for a new note (bit 6 of the slot's flags), since
register 3 restarts the note. When an effect's stream ends, `sound_channel_done` (`7:F975`) sets bit 6 on the slots under
it, so the music gets its registers back.

| Byte | Meaning |
|---|---|
| 0 | flags: bit 7 on, 6 write registers 1-3, 5 rest |
| 1-2 | the next command in the stream |
| 3-4 | note length: each note adds 8 times this to the time left |
| 5-6 | time left: goes down by the speed each frame, and the next command comes when it goes negative |
| 7-8 | speed |
| 9 | pulse: frames per envelope step; with bit 7 set, no envelope (never seen in `tests/test_sound.c`) |
| 10-11 | pulse: the volume envelope; noise: a table of 3 register bytes per note |
| 12 | pulse: the envelope runs; triangle: the note's own countdown runs |
| 13 | pulse: the envelope's step; triangle: the setting that countdown is made from |
| 14 | pulse: frames left in the step |
| 15 | index of the loop stack, whose entries are 4 bytes: passes done, passes, loop start (2) |
| 16-19 | what goes to the APU channel's four registers |
| 20 | triangle: the note's period, high byte |
| 21 | triangle: mode; the real data has only 1 and 2 (never another in `tests/test_sound.c`) |
| 22-23 | triangle: the note's countdown |

A triangle note stops after a time of its own, set with `$E2`: in mode 1, after `setting` frames. Mode
2 looks meant to stop it `setting - 1` frames before the note's end (a guess at the intent), but the
original writes both bytes of that time to byte 22, leaving byte 23 as it was. That looks like a slip;
the port keeps it, and the real sounds do run it.

The noise channel's note numbers start at 4 and repeat every 12.

`sound_check_repeat` (`7:F433`) runs when any stream ends. If no music slot is on, it starts
`repeat_sound` again or, with none, calls `sound_reset`. So an effect that ends while no music plays
also stops any other effect that is still going.

## Header (12 bytes)
| Byte | Meaning |
|---|---|
| 0 | type: 0 music, 1 music that repeats, 2 effect on 3 channels, 3 and 4 effects on 2 channels |
| 1-8 | four channel stream addresses (types 0-1 use all four, type 2 the first three, types 3-4 the 2nd and 4th) |
| 9-10 | unused |
| 11 | speed |

## Channel streams
A list of commands, ended by `$FF`:

| Bytes | Meaning |
|---|---|
| `$00-$7D` | play a note (period from `note_periods`, `7:FB1B`), or, on the noise channel, a table entry |
| `$7F` | rest |
| `$7E` | wait (keep the current note going) |
| `$80-$BF` | set the note length (from `note_lengths`, `7:FAF5`) |
| `$C0-$CE` | start a loop that runs `n` times (`$C0`: 256, as the count is compared after adding 1) |
| `$CF` | end of loop |
| `$D0-$DF` | leave the loop on pass `n` |
| `$E0`/`$E1` `a a` | instrument at address `a`: 4 bytes, then a volume envelope |
| `$E2 a a` | 4 bytes at `a`: registers 0 and 3, the triangle's mode and setting |
| `$E3 a a` | volume envelope at `a` |
| `$E4-$FE a a` | 3 bytes, ignored |

Envelopes are read one value per step, up to 64 steps; the instruments are laid out 68 bytes
apart (4 + 64).

## Checked against the original
`tests/test_sound.c` (the test `sound_driver`, where the ROM is present) plays every sound alone and
then a long random mix, on the original as 6502 code and on the port, and compares them after every
call; [harness.md](harness.md) has the numbers. These parts of the driver never run on the real data,
so only the fuzz compares them:
- a pulse note without an envelope (`7:F5D5`);
- a triangle mode other than 1 and 2 (`7:F680`), and mode 2 when the time left is too short for it
  (`7:F6CF`);
- commands `$E4-$FE` (`7:F843`);
- while skipping to a loop's end, crossing a page (`7:F933`) or meeting another `$Dx` (`7:F93F`);
- `$CF` or `$Dx` with no loop open (`7:F96A`).

## What's mapped
47 headers, 80 channel streams, 14 instruments, 4 sweep blocks and 3 envelopes, plus an
instrument and two short streams nothing points to (found by walking the gaps): 6,527 of the
6,656 bytes. Sound 8's table entry points into instrument data rather than a header (its "type" is
`$48`, past the list of five), so it is presumably never played. Entries `$0F`, `$10` and `$11` are the
same sound. The rest is 89 bytes of `$FF` before the messages and 40 bytes near `$8B3A` whose use
isn't known.
