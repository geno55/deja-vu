# The comparison harness: the original game running in Mesen2's core, driven in-process.
# Mesen2 is GPL-3.0: its source is cloned at a pinned tag into external/Mesen2,
# never copied into the repo, compiled here, and linked only into dv_harness.
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MESEN_TAG 2.1.1)
set(MESEN_COMMIT 137ae7ce3bf3f539d007e2c4ef3cb3b6c97672a1)
set(MESEN_DIR ${CMAKE_SOURCE_DIR}/external/Mesen2)

find_package(Git REQUIRED)
if(NOT EXISTS ${MESEN_DIR}/.git)
  message(STATUS "Fetching Mesen2 ${MESEN_TAG} into ${MESEN_DIR}")
  execute_process(COMMAND ${GIT_EXECUTABLE} clone --depth 1 --branch ${MESEN_TAG}
                          https://github.com/SourMesen/Mesen2.git ${MESEN_DIR}
                  RESULT_VARIABLE clone_rc)
  if(clone_rc)
    message(FATAL_ERROR "Could not clone Mesen2 ${MESEN_TAG}")
  endif()
endif()
execute_process(COMMAND ${GIT_EXECUTABLE} -C ${MESEN_DIR} rev-parse HEAD
                OUTPUT_VARIABLE mesen_head OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT mesen_head STREQUAL MESEN_COMMIT)
  message(FATAL_ERROR "external/Mesen2 is at ${mesen_head}, not ${MESEN_TAG} "
                      "(${MESEN_COMMIT}). Delete external/Mesen2 to fetch it again.")
endif()
execute_process(COMMAND ${GIT_EXECUTABLE} -C ${MESEN_DIR} status --porcelain
                OUTPUT_VARIABLE mesen_dirty OUTPUT_STRIP_TRAILING_WHITESPACE)
if(mesen_dirty)
  message(WARNING "external/Mesen2 has local changes, so the reference is not stock Mesen2 ${MESEN_TAG}")
endif()

# The same source sets as Mesen2's own makefile, minus the UI, renderers and InteropDLL.
file(GLOB_RECURSE mesen_core_src CONFIGURE_DEPENDS ${MESEN_DIR}/Core/*.cpp)
file(GLOB_RECURSE mesen_util_src CONFIGURE_DEPENDS ${MESEN_DIR}/Utilities/*.cpp ${MESEN_DIR}/Utilities/*.c)
file(GLOB mesen_7z_src CONFIGURE_DEPENDS ${MESEN_DIR}/SevenZip/*.c)
file(GLOB mesen_lua_src CONFIGURE_DEPENDS ${MESEN_DIR}/Lua/*.c)
if(WIN32)
  # Mesen2's Windows project leaves out LuaSocket's Unix and serial-port files.
  list(FILTER mesen_lua_src EXCLUDE REGEX "/(serial|unix|unixdgram|unixstream)\\.c$")
endif()

add_library(mesen_lua STATIC ${mesen_lua_src})
add_library(mesen_7z STATIC ${mesen_7z_src})
add_library(mesen_utils STATIC ${mesen_util_src})
add_library(mesen_core STATIC ${mesen_core_src})
target_precompile_headers(mesen_core PRIVATE ${MESEN_DIR}/Core/pch.h)
set_source_files_properties(${MESEN_DIR}/Core/Shared/Utilities/emu2413.cpp
                            PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

foreach(lib mesen_lua mesen_7z mesen_utils mesen_core)
  target_include_directories(${lib} PUBLIC ${MESEN_DIR} ${MESEN_DIR}/Core ${MESEN_DIR}/Utilities)
  if(MSVC)
    # Mesen's code isn't ours to fix: no warnings from it.
    target_compile_options(${lib} PRIVATE /W0 /bigobj /MP)
    # Mesen2's projects build with the Unicode character set.
    target_compile_definitions(${lib} PUBLIC WIN32 UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                               _WINSOCK_DEPRECATED_NO_WARNINGS
                               _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
  else()
    target_compile_options(${lib} PRIVATE -w)
  endif()
endforeach()

find_package(Threads REQUIRED)
target_link_libraries(mesen_utils PUBLIC mesen_7z Threads::Threads)
target_link_libraries(mesen_core PUBLIC mesen_utils mesen_lua)
if(WIN32)
  target_link_libraries(mesen_core PUBLIC ws2_32 winmm ole32 oleaut32 shell32 user32 advapi32)
else()
  target_link_libraries(mesen_core PUBLIC ${CMAKE_DL_LIBS})
endif()

add_executable(dv_harness dv_harness.c ref_mesen.cpp)
target_link_libraries(dv_harness PRIVATE mesen_core dv_core)
target_compile_definitions(dv_harness PRIVATE DV_MESEN_DIR="${MESEN_DIR}"
                           DV_DEFAULT_ROM="${DV_ROM}"
                           DV_DEFAULT_HOME="${CMAKE_BINARY_DIR}/mesen-home")
# ref_mesen.cpp includes Mesen's headers, which aren't warning-clean at the higher levels.
if(MSVC)
  set_source_files_properties(dv_harness.c PROPERTIES COMPILE_OPTIONS /W4)
  set_source_files_properties(ref_mesen.cpp PROPERTIES COMPILE_OPTIONS /W3)
else()
  set_source_files_properties(dv_harness.c PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()

# Where the ROM is present and TAS #1260 has been converted (docs/development.md), check that save states
# replay exactly: the harness rewinds with them.
set(DV_TEST_MOVIE ${CMAKE_SOURCE_DIR}/build/movies/tas1260.inp)
# Our palette's colors against Mesen2's, read from its source. It needs neither the ROM nor a movie.
add_test(NAME palette_vs_mesen COMMAND dv_harness --check-palette)

if(EXISTS "${DV_ROM}" AND EXISTS ${DV_TEST_MOVIE})
  add_test(NAME harness_rewind
           COMMAND dv_harness --movie ${DV_TEST_MOVIE} --frames 6000 --rewind-check 50)
endif()

# Saving and loading in real play, on the reference and the C side in lockstep, with the save files
# carried between the port and Mesen2 in both directions (docs/saves.md).
if(EXISTS "${DV_ROM}")
  add_test(NAME harness_saves
           COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/run_save_movies.py
           --dejavu $<TARGET_FILE:dejavu> --harness $<TARGET_FILE:dv_harness> --rom "${DV_ROM}"
           --work ${CMAKE_BINARY_DIR}/save_movies_ref)
endif()
