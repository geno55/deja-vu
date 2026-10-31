-- Records Mesen's code/data log (CDL) for PRG ROM while a bot presses buttons
-- or a converted TAS movie plays.
-- Run through tools/run_mesen.py, which passes these environment variables:
--   DV_OUT     file to write: one CDL flag byte per PRG ROM byte (bit0 code, bit1 data,
--              bit2 jump target, bit3 subroutine entry). DV_OUT..".ram" gets the 2 KB of
--              internal RAM at the last frame, to check how far the game got.
--   DV_FRAMES  number of frames to run
--   DV_SEED    seed for the button bot; 0 presses nothing
--   DV_MOVIE   optional .inp file from tools/movie_inputs.py; replaces the bot
--   DV_MOVIE_OFFSET  movie frames to skip, to line up with Mesen's first frame
--   DV_SCRIPT_LOG  optional file to log every script the game runs (see tools/scripts.py):
--              "S frame aa ab bc" when one starts, "O bank addr opcode" per opcode,
--              "E" when it ends
--   DV_SCENE_LOG   optional file to log each scene picture as drawn (see below)
--   DV_NMI_LOG     optional file to log the state at every NMI entry, in the format
--              harness/dv_harness.c writes with --log (see there), to check the harness's
--              in-process reference against Mesen run this way
local out_path = os.getenv("DV_OUT")
local frames_total = tonumber(os.getenv("DV_FRAMES") or "3600")
local seed = tonumber(os.getenv("DV_SEED") or "1")
local movie_path = os.getenv("DV_MOVIE") or ""
local movie_offset = tonumber(os.getenv("DV_MOVIE_OFFSET") or "0")

local movie = nil
if movie_path ~= "" then
  movie = {}
  for line in io.lines(movie_path) do
    movie[#movie + 1] = line
  end
end

-- Column order of .inp lines.
local inp_buttons = {"up", "down", "left", "right", "start", "select", "b", "a"}

local function movie_input(line)
  local held = {}
  if line then
    for i = 1, #inp_buttons do
      if line:sub(i, i) ~= "." then
        held[inp_buttons[i]] = true
      end
    end
  end
  return held
end

-- Deterministic LCG so a seed always replays the same presses.
local rng = seed
local function rand(n)
  rng = (rng * 1103515245 + 12345) % 2147483648
  return (rng >> 16) % n
end

-- Mostly d-pad moves and A presses (cursor + select), some B, rare Start/Select.
local choices = {
  {up = true}, {down = true}, {left = true}, {right = true},
  {up = true}, {down = true}, {left = true}, {right = true},
  {a = true}, {a = true}, {a = true}, {b = true},
  {}, {}, {start = true}, {select = true},
}

local held, hold_frames, frame = {}, 0, 0
local finished, finish_hooks = false, {}  -- hooks run once, just before the run stops

local function bot_input()
  if hold_frames == 0 then
    -- Alternate presses with releases so the game sees fresh button-down edges.
    if next(held) == nil then
      held = choices[rand(#choices) + 1]
      hold_frames = 1 + rand(8)
    else
      held = {}
      hold_frames = 1 + rand(20)
    end
  end
  hold_frames = hold_frames - 1
  return held
end

local function write_file(path, bytes, count)
  local f = assert(io.open(path, "wb"))
  local chunk = {}
  for i = 0, count - 1 do
    chunk[#chunk + 1] = string.char(bytes(i))
    if #chunk == 4096 then
      f:write(table.concat(chunk))
      chunk = {}
    end
  end
  f:write(table.concat(chunk))
  f:close()
end

-- Mesen's test runner hides script errors, so write them next to DV_OUT and stop.
-- The callback's arguments (address and value for memory callbacks) are passed through;
-- its return value is not, so a memory callback never changes the value read or written.
local function guard(fn)
  return function(...)
    local ok, err = pcall(fn, ...)
    if not ok then
      local f = io.open(out_path .. ".err", "w")
      if f then
        f:write(tostring(err))
        f:close()
      end
      emu.stop(3)
    end
  end
end

emu.addEventCallback(guard(function()
  frame = frame + 1
  if movie then
    held = movie_input(movie[frame + movie_offset])
  elseif seed ~= 0 then
    held = bot_input()
  end

  if frame >= frames_total and not finished then
    finished = true
    for _, hook in ipairs(finish_hooks) do
      hook()
    end
    local cdl = emu.getCdlData(emu.memType.nesPrgRom)  -- indexed from 0
    write_file(out_path, function(i) return cdl[i] end, #cdl + 1)
    write_file(out_path .. ".ram", function(i) return emu.read(i, emu.memType.nesInternalRam) end, 0x800)
    emu.stop(0)
  end
end), emu.eventType.startFrame)

emu.addEventCallback(guard(function()
  emu.setInput(held, 0, 0)
end), emu.eventType.inputPolled)

-- DV_SCENE_LOG: once per scene picture ($0515), 2 frames after one of the drawing
-- routines returns (7:C118 draws tile by tile, 7:C1F3 all at once), log
-- "P scene tiles attrs" with the 14x14 tiles at nametable $2082 and the 16 attribute
-- bytes the picture covers, to check tools/scenes.py against what the game draws.
local scene_log_path = os.getenv("DV_SCENE_LOG") or ""
if scene_log_path ~= "" then
  local log, seen, pending = {}, {}, -1
  local attr_pos = {0x08, 0x09, 0x0A, 0x0B, 0x10, 0x11, 0x12, 0x13,
                    0x18, 0x19, 0x1A, 0x1B, 0x20, 0x21, 0x22, 0x23}  -- as the table at 7:C1A6
  for _, rts in ipairs({0xC118, 0xC1F3}) do
    emu.addMemoryCallback(guard(function() pending = 2 end), emu.callbackType.exec, rts)
  end
  emu.addEventCallback(guard(function()
    if pending < 0 then
      return
    end
    pending = pending - 1
    if pending ~= 0 then
      return
    end
    local scene = emu.read(0x0515, emu.memType.nesInternalRam)
    local t = {}
    for r = 0, 13 do
      for c = 0, 13 do
        t[#t + 1] = string.format("%02X", emu.read(0x082 + r * 32 + c, emu.memType.nesNametableRam))
      end
    end
    local tiles = table.concat(t)
    -- A dark room draws as all $FF; log the lit picture too when it shows up later.
    local key = scene .. (tiles == string.rep("FF", 196) and "dark" or "lit")
    if not seen[key] then
      seen[key] = true
      local a = {}
      for _, p in ipairs(attr_pos) do
        a[#a + 1] = string.format("%02X", emu.read(0x3C0 + p, emu.memType.nesNametableRam))
      end
      log[#log + 1] = string.format("P %02X %s %s", scene, tiles, table.concat(a))
    end
  end), emu.eventType.endFrame)
  finish_hooks[#finish_hooks + 1] = function()
    local f = assert(io.open(scene_log_path, "w"))
    f:write(table.concat(log, "\n"), "\n")
    f:close()
  end
end

local nmi_log_path = os.getenv("DV_NMI_LOG") or ""
if nmi_log_path ~= "" then
  local log, nmis, polls, apu = {}, 0, 0, {}
  local mems = {emu.memType.nesInternalRam, emu.memType.nesSaveRam, emu.memType.nesNametableRam,
                emu.memType.nesPaletteRam, emu.memType.nesSpriteRam}
  local sizes = {}
  for i, m in ipairs(mems) do
    sizes[i] = emu.getMemorySize(m)
  end
  local function fnv(h, b) return ((h ~ b) * 16777619) & 0xFFFFFFFF end

  emu.addEventCallback(guard(function() polls = polls + 1 end), emu.eventType.inputPolled)
  -- The APU registers: $4000-$4013, $4015 and $4017.
  for _, range in ipairs({{0x4000, 0x4013}, {0x4015, 0x4015}, {0x4017, 0x4017}}) do
    emu.addMemoryCallback(guard(function(address, value)
      apu[#apu + 1] = {address, value}
    end), emu.callbackType.write, range[1], range[2])
  end
  -- 7:EC9A is the NMI handler; this runs before its first instruction.
  emu.addMemoryCallback(guard(function()
    nmis = nmis + 1
    local s = emu.getState()
    local sp = s["cpu.sp"]
    local function stack(i) return emu.read(0x100 + ((sp + i) & 0xFF), emu.memType.nesInternalRam) end
    local h = {}
    for i, m in ipairs(mems) do
      local x = 2166136261
      for a = 0, sizes[i] - 1 do
        x = fnv(x, emu.read(a, m))
      end
      h[i] = x
    end
    local ah = 2166136261
    for _, w in ipairs(apu) do
      ah = fnv(fnv(fnv(ah, w[1] & 0xFF), w[1] >> 8), w[2])
    end
    log[#log + 1] = string.format(
      "%d %d %04X a=%02X x=%02X y=%02X p=%02X sp=%02X cyc=%d ram=%08X sram=%08X nt=%08X pal=%08X oam=%08X apu=%d:%08X",
      nmis, polls, stack(2) | stack(3) << 8, s["cpu.a"], s["cpu.x"], s["cpu.y"], stack(1), sp,
      s["cpu.cycleCount"], h[1], h[2], h[3], h[4], h[5], #apu, ah)
    apu = {}
  end), emu.callbackType.exec, 0xEC9A)

  finish_hooks[#finish_hooks + 1] = function()
    local f = assert(io.open(nmi_log_path, "w"))
    f:write(table.concat(log, "\n"), "\n")
    f:close()
  end
end

local script_log_path = os.getenv("DV_SCRIPT_LOG") or ""
if script_log_path ~= "" then
  local log = {}
  local function ram(a) return emu.read(a, emu.memType.nesInternalRam) end

  -- 7:CB64 starts a script chosen by $AA/$AB/$BC (docs/conventions.md).
  emu.addMemoryCallback(guard(function()
    log[#log + 1] = string.format("S %d %02X %02X %02X", frame, ram(0xAA), ram(0xAB), ram(0xBC))
  end), emu.callbackType.exec, 0xCB64)
  -- 7:CBB1 (script_loop) is about to fetch the opcode at ($B6); $0C holds the current bank.
  emu.addMemoryCallback(guard(function()
    local ptr = ram(0xB6) | ram(0xB7) << 8
    log[#log + 1] = string.format("O %d %04X %02X", ram(0x0C) & 0x0F, ptr, emu.read(ptr, emu.memType.nesMemory))
  end), emu.callbackType.exec, 0xCBB1)
  -- 7:CBB8: the script hit $FF and ends.
  emu.addMemoryCallback(guard(function()
    log[#log + 1] = "E"
  end), emu.callbackType.exec, 0xCBB8)

  finish_hooks[#finish_hooks + 1] = function()
    local f = assert(io.open(script_log_path, "w"))
    f:write(table.concat(log, "\n"), "\n")
    f:close()
  end
end
