# Room hotspots and item lists

Both live in bank 2 and are mapped by `tools/rooms.py`. Every byte the recordings read in
either area falls inside the map. Names match [asm/labels.txt](../asm/labels.txt).

## Hotspots (`2:9C00`)
The clickable areas of each room. An address table indexed by the room number (`$0514`,
78 rooms) points to a list of 6-byte records, ended by a byte `$FF` (288 hotspots in all):

| Byte | Meaning |
|---|---|
| 0, 1 | x range: the cursor's x (`$A5`) must be at least byte 0 and less than byte 1 |
| 2, 3 | y range, the same way with `$A6` |
| 4 | kind: bits 0-6 are compared with `$AA` (which script table applies); bit 7 makes `7:C462` also run far call `$60` with `$BD` = kind & `$7F`, `$BE` = id |
| 5 | object id, compared with `$AB` (the current object) |

`7:C283` tests the cursor against a record (it sets bit 0 of `$B5` on a hit). `7:C462` and
`7:CE13` walk a room's list. In the data, the kinds with bit 7 are `$80`, `$81`, `$83` and `$84`.
Kind 2 has a table of sprites (`6:A88F`) but no record: just as well, as its check in
`object_sprite_condition` ends `PLA`, `TXA`, `RTS`, which would return into zero page (from the
code; nothing gets there).

A room number past the table's end reads whatever follows: see "The capsule warp" below.

## The capsule warp
Using the capsules on medicine not yet taken makes the room `$FF`. The script's search of the inventory
(`$5F`/`$60`) runs past the last page to page 12, entry 0, without finding it, and `delete_inventory_entry`
(`6:AEC2`) then takes that as slot `$54`. It checks neither that the slot holds the item nor that the slot
is on the list, so it writes the list's `$FF` end mark at `address_book + $54`, which is `room`. In that
room the tables read out of bounds, and clicks in the picture walk through memory, reaching code in RAM and
elsewhere. That is the original's bug, kept as it is, and TAS #7044 uses it; found by others, on TASVideos.

## Item lists (`2:A400`)
`7:D4FD` fills the coat's item slots (`$04C7`) from lists of item numbers ended by `$FF`.
`2:A400` holds two addresses: entry 0 when `$AF` is 1, entry 1 otherwise. Each points to a
table indexed by `$B0` whose entries point to the lists: 3 tables and 31 lists, 148 bytes.
