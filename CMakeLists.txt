cmake_minimum_required(VERSION 3.20)
project(dejavu C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

# A player's PC may not have Visual C++'s runtime, so it goes in the program, as in zelda3's x64 Release.
# Not with the harness: Mesen2 stays built as it always was.
if(NOT DV_HARNESS)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# The C side: the console model, the 6502 that runs the game's original code, the bridge
# between it and ported code, and the routines ported so far. MIT, like everything outside
# harness/.
add_library(dv_core STATIC src/cpu6502.c src/nes_hw.c src/coro.c src/dv_cpu_infra.c
            src/dv_rtl.c src/bank_calls.c src/input.c src/ppu_upload.c src/vram_queue.c
            src/screens.c src/palette.c src/nmi.c src/text.c src/scenes.c src/commands.c
            src/scripts.c src/flags.c src/opcodes.c src/rooms.c src/special_scenes.c
            src/saves.c src/layouts.c src/room_view.c src/room_commands.c src/inventory.c
            src/slot_machine.c src/menus.c src/sound.c src/game.c src/ppu_render.c src/apu.c
            src/assets.c src/save_file.c src/config.c src/text_fixes.c src/pointer.c)
target_include_directories(dv_core PUBLIC src)
if(NOT WIN32)
  target_link_libraries(dv_core PUBLIC m)
endif()

# The game itself. The window is SDL2 (zlib license, never in the repo): tools/fetch_sdl2.py puts it in
# external/ on Windows, and elsewhere it is a package. Without it `dejavu` has no window.
find_package(SDL2 CONFIG QUIET HINTS ${CMAKE_SOURCE_DIR}/external/SDL2-2.32.10/cmake)
add_executable(dejavu src/main.c)
target_link_libraries(dejavu PRIVATE dv_core)
if(SDL2_FOUND)
  target_sources(dejavu PRIVATE src/platform_sdl.c)
  target_link_libraries(dejavu PRIVATE SDL2::SDL2)
  if(WIN32)
    add_custom_command(TARGET dejavu POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
                       $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:dejavu>)
  endif()
else()
  message(STATUS "No SDL2, so dejavu will have no window (tools/fetch_sdl2.py, or a package)")
  target_sources(dejavu PRIVATE src/platform_none.c)
endif()
# The settings file goes next to the program, where the game looks for it.
add_custom_command(TARGET dejavu POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   ${CMAKE_SOURCE_DIR}/dejavu.ini $<TARGET_FILE_DIR:dejavu>)
add_executable(test_cpu6502 tests/test_cpu6502.c src/cpu6502.c)
target_include_directories(test_cpu6502 PRIVATE src)
add_executable(test_bridge tests/test_bridge.c)
target_link_libraries(test_bridge PRIVATE dv_core)
add_executable(test_sound tests/test_sound.c)
target_link_libraries(test_sound PRIVATE dv_core)
add_executable(test_game tests/test_game.c)
target_link_libraries(test_game PRIVATE dv_core)
add_executable(test_pointer tests/test_pointer.c)
target_link_libraries(test_pointer PRIVATE dv_core)
add_executable(test_apu tests/test_apu.c)
target_link_libraries(test_apu PRIVATE dv_core)
add_executable(test_palette tests/test_palette.c)
target_link_libraries(test_palette PRIVATE dv_core)
add_executable(test_save_file tests/test_save_file.c)
target_link_libraries(test_save_file PRIVATE dv_core)
add_executable(test_text_fixes tests/test_text_fixes.c)
target_link_libraries(test_text_fixes PRIVATE dv_core)
add_executable(test_config tests/test_config.c)
target_link_libraries(test_config PRIVATE dv_core)
add_executable(test_assets tests/test_assets.c)
target_link_libraries(test_assets PRIVATE dv_core)
foreach(target dv_core dejavu test_cpu6502 test_bridge test_sound test_game test_pointer test_apu test_palette
        test_save_file test_config test_text_fixes test_assets)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra)
  endif()
endforeach()

enable_testing()
add_test(NAME smoke COMMAND dejavu --help)
add_test(NAME cpu6502 COMMAND test_cpu6502)
add_test(NAME bridge COMMAND test_bridge)
add_test(NAME apu COMMAND test_apu)
add_test(NAME palette COMMAND test_palette)
add_test(NAME save_file COMMAND test_save_file ${CMAKE_BINARY_DIR})
add_test(NAME config COMMAND test_config ${CMAKE_BINARY_DIR})
add_test(NAME variables_current
         COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/make_variables.py --check)
add_test(NAME tools_unit
         COMMAND ${Python3_EXECUTABLE} -m unittest discover -s ${CMAKE_SOURCE_DIR}/tests -v)

# The ROM is never in the repo, so these only exist where the user has one.
set(DV_ROM "${CMAKE_SOURCE_DIR}/Deja Vu (USA).nes" CACHE FILEPATH "Path to your Deja Vu (USA) ROM")
find_program(CA65 ca65 HINTS "$ENV{CC65_HOME}/bin" "$ENV{LOCALAPPDATA}/Programs/cc65/bin")
set(DV_BUILD_ROM ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/build_rom.py
    --rom "${DV_ROM}" --out ${CMAKE_BINARY_DIR}/rom/dejavu.nes)
if(EXISTS "${DV_ROM}")
  # The game's data next to the program, so that it runs from anywhere (src/assets.h). A wrong ROM there
  # shouldn't stop the build; the game and the test rom_hash say so when they run.
  add_custom_command(TARGET dejavu POST_BUILD
                     COMMAND dejavu --extract-assets "${DV_ROM}" || (exit 0))
  add_test(NAME rom_hash COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_rom.py "${DV_ROM}")
  add_test(NAME assets COMMAND test_assets ${CMAKE_BINARY_DIR} "${DV_ROM}")
  # Saving and loading in real play, which the TAS movies never do.
  add_test(NAME save_movies COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/run_save_movies.py
           --dejavu $<TARGET_FILE:dejavu> --rom "${DV_ROM}" --work ${CMAKE_BINARY_DIR}/save_movies)
  # The program itself on scripted movies, which also fixes how fast key repeat is.
  add_test(NAME play_movies COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/run_play_movies.py
           --dejavu $<TARGET_FILE:dejavu> --rom "${DV_ROM}" --work ${CMAKE_BINARY_DIR}/play_movies)
  add_test(NAME sound_driver COMMAND test_sound "${DV_ROM}")
  add_test(NAME text_fixes COMMAND test_text_fixes "${DV_ROM}")
  add_test(NAME pointer COMMAND test_pointer "${DV_ROM}")
  set_tests_properties(pointer PROPERTIES TIMEOUT 120)
  add_test(NAME game_alone COMMAND test_game "${DV_ROM}")
  set_tests_properties(game_alone PROPERTIES TIMEOUT 300)  # a loop that never waits shows as this
  # Every data block -> editable text/PNG -> bytes again, checked against the ROM.
  add_test(NAME data_roundtrip COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/roundtrip.py
           --rom "${DV_ROM}" --out ${CMAKE_BINARY_DIR}/src)
  if(CA65)
    # Assemble asm/*.s (plus data extracted from the ROM) and check it's byte-identical.
    add_custom_target(rom COMMAND ${DV_BUILD_ROM} USES_TERMINAL)
    add_test(NAME rom_rebuild COMMAND ${DV_BUILD_ROM})
    # The addresses src/ names, most of which no movie ever exercises, against that build.
    add_test(NAME port_addresses COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_ports.py
             --rom "${DV_ROM}" --labels ${CMAKE_BINARY_DIR}/rom/dejavu.labels)
    set_tests_properties(port_addresses PROPERTIES DEPENDS rom_rebuild)
  else()
    message(STATUS "cc65 not found; skipping the rom target and rom_rebuild test")
  endif()
else()
  message(STATUS "No ROM at ${DV_ROM}; skipping the rom_hash and rom_rebuild tests")
  add_test(NAME assets COMMAND test_assets ${CMAKE_BINARY_DIR})
endif()

# The comparison harness links Mesen2, which is GPL-3.0. It is off by
# default, release builds never turn it on, and its binaries are never published.
option(DV_HARNESS "Build the comparison harness against Mesen2 (fetched into external/)" OFF)
if(DV_HARNESS)
  add_subdirectory(harness)
endif()
