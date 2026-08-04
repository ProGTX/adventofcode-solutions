cmake_minimum_required(VERSION 4.0)

# `import std` is still behind an experimental gate
# that only opens for a UUID matching the exact CMake in use.
# This has to be set before project():
# the value is read while the CXX compiler is being detected
# and that detection is what fills in CMAKE_CXX_COMPILER_IMPORT_STD.
#
# The UUID can change with a minor or even a patch version,
# so the table is keyed on the release the value first appeared in,
# and the newest entry at or below the running CMake wins.
# https://github.com/Kitware/CMake/blob/v4.4.2/Help/dev/experimental.rst
#
# Getting this wrong is not fatal.
# CMake newer than the last row here costs us `import std`
# until the table grows a line.
set(AOC_IMPORT_STD_GATE_VERSIONS
  "4.0.0" # .. 4.0.2
  "4.0.3" # .. 4.2.7
  "4.3.0" # .. 4.3.4
  "4.4.0" # .. 4.4.2, newest known
)
set(AOC_IMPORT_STD_GATE_UUIDS
  "a9e1cf81-9932-4810-974b-6eccaf14e457"
  "d0edc3af-4c50-42ea-a356-e2862fe7a444"
  "451f2fe2-a8a2-47c3-bc32-94786d8fc91b"
  "f35a9ac6-8463-4d38-8eec-5d6008153e7d"
)

list(LENGTH AOC_IMPORT_STD_GATE_VERSIONS aoc_gate_count)
math(EXPR aoc_gate_last "${aoc_gate_count} - 1")
foreach (aoc_gate_index RANGE ${aoc_gate_last})
  list(GET AOC_IMPORT_STD_GATE_VERSIONS ${aoc_gate_index} aoc_gate_version)
  if (CMAKE_VERSION VERSION_GREATER_EQUAL "${aoc_gate_version}")
    list(GET AOC_IMPORT_STD_GATE_UUIDS ${aoc_gate_index}
      CMAKE_EXPERIMENTAL_CXX_IMPORT_STD)
  endif()
endforeach()

project(adventofcode VERSION 2022 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(AOC_DISABLE_MODULES "Explicitly disables modules, `import std` included" OFF)
option(AOC_DISABLE_IMPORT_STD "Explicitly disables `import std`" OFF)

# `import std` is only known good on MSVC so far,
# so that is the only place it turns itself on.
# Clang and GCC can have it enabled, but not everything works yet,
# so it takes an explicit opt-in.
option(AOC_ENABLE_IMPORT_STD
  "Opts in to `import std` on compilers where it is not on by default" OFF)

add_compile_definitions(AOC_CXX_STANDARD=${CMAKE_CXX_STANDARD})
set(AOC_RELEASE_CHECK_EXPR
  $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>
)

if (MSVC)
  # MSVC Debug defaults to /Ob0, so every STL and range adaptor template
  # stays an out-of-line call.
  # These solutions are template heavy,
  # which is why Debug here runs far slower than a Clang -O0 build.
  # /Ob1 inlines what is marked inline while leaving the code debuggable,
  # but /RTC1 disables inlining outright, so it has to go for /Ob1 to do anything.
  string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  add_compile_options($<$<CONFIG:Debug>:/Ob1>)

  # The memoized searches recurse once per step of the state they consume,
  # and a Debug frame holds every range adaptor temporary that Release folds away -
  # deep enough that 2023/day12 overflows the default 1 MB reserve.
  # It only needs a little over 1 MB, so the margin was always thin.
  # 8 MB is reserved address space rather than committed memory,
  # and buys room to stop rediscovering this.
  # This also sets the default for the std::async threads,
  # which is where it actually overflows:
  # they are created with a zero stack size,
  # meaning "use the reserve recorded in the image header".
  add_link_options($<$<CONFIG:Debug>:/STACK:8388608>)

  # _ITERATOR_DEBUG_LEVEL defaults to 2 in Debug,
  # which tracks iterator invalidation by threading every live iterator
  # onto a heap allocated container proxy.
  # That bookkeeping dominates Debug runtime here.
  # Level 1 keeps the range and validity checks -
  # the ones that catch real bugs - without the proxy.
  add_compile_definitions($<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=1>)
endif()

if ((CMAKE_BUILD_TYPE STREQUAL "Debug") OR
    (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
  add_compile_definitions(_GLIBCXX_ASSERTIONS)
endif()

# Minimum compiler versions for the aoc module:
#  - Clang 16 is the first release the module builds on.
#  - GCC gives module linkage to std declarations
#    coming in through the global module fragment,
#    which clashes with headers included alongside `import aoc`.
#    Verified still broken in 15.1, so try again on 16.
#  - MSVC 19.40 is VS 2022 17.10, the first toolset shipping the STL std module.
#    Older toolsets could build the module in principle,
#    but not the way it is written now: it only stays clear of the GMF clashes
#    by taking the standard library through `import std`.
set(AOC_MIN_MODULES_Clang "16.0")
set(AOC_MIN_MODULES_GNU "16.0")
set(AOC_MIN_MODULES_MSVC "19.40")

# Minimum compiler versions for `import std`:
#  - Clang needs 18.1.2 for the modules.json key spelling CMake reads.
#  - GCC ships the std module from 15, but the module gate above binds first.
#  - MSVC as above. Verified on 19.51, 19.40 is the documented floor.
set(AOC_MIN_IMPORT_STD_Clang "18.1.2")
set(AOC_MIN_IMPORT_STD_GNU "15.0")
set(AOC_MIN_IMPORT_STD_MSVC "19.40")

set(AOC_MODULE_SUPPORT OFF)
set(AOC_IMPORT_STD OFF)

# Whether `import std` is even being asked for, as opposed to being asked for
# and then turned down by a version or capability check further on
set(aoc_want_import_std OFF)
if ((NOT AOC_DISABLE_IMPORT_STD) AND
    (AOC_ENABLE_IMPORT_STD OR (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")))
  set(aoc_want_import_std ON)
endif()

if (NOT AOC_DISABLE_MODULES)
  set(aoc_min_modules "${AOC_MIN_MODULES_${CMAKE_CXX_COMPILER_ID}}")
  if (aoc_min_modules AND
      (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "${aoc_min_modules}"))
    set(AOC_MODULE_SUPPORT ON)
  endif()

  set(aoc_min_import_std "${AOC_MIN_IMPORT_STD_${CMAKE_CXX_COMPILER_ID}}")
  if (aoc_want_import_std AND aoc_min_import_std AND
      (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "${aoc_min_import_std}"))
    if (CMAKE_GENERATOR MATCHES "^Visual Studio")
      # MSBuild finds, builds and references the STL's std module by itself.
      # CMake's own `import std` support is not wired up for this generator,
      # so the experimental gate above does nothing here.
      set(AOC_IMPORT_STD ON)
    elseif ("23" IN_LIST CMAKE_CXX_COMPILER_IMPORT_STD)
      # Everywhere else CMake builds the std module, but only once the
      # experimental gate has matched this CMake version and it has found the
      # standard library's module metadata.
      set(AOC_IMPORT_STD ON)
      set(CMAKE_CXX_MODULE_STD ON)
    endif()
  endif()

  # Clang can have either one on its own. MSVC and GCC cannot: their aoc module
  # only builds while the global module fragment stays empty, which is exactly
  # what `import std` buys, and `import std` without the aoc module leaves every
  # single target compiling its own copy of the std module.
  if ((NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND
      (NOT (AOC_MODULE_SUPPORT AND AOC_IMPORT_STD)))
    set(AOC_MODULE_SUPPORT OFF)
    set(AOC_IMPORT_STD OFF)
    set(CMAKE_CXX_MODULE_STD OFF)
  endif()
endif()

if (AOC_MODULE_SUPPORT)
  add_compile_definitions(AOC_MODULE_SUPPORT)
  add_compile_definitions(AOC_EXPORT=export)
endif()

if (AOC_IMPORT_STD)
  add_compile_definitions(AOC_IMPORT_STD)
endif()

message(STATUS "aoc: module support ${AOC_MODULE_SUPPORT}, import std ${AOC_IMPORT_STD}")
if ((NOT AOC_IMPORT_STD) AND aoc_want_import_std AND
    CMAKE_CXX_COMPILER_IMPORT_STD_ERROR_MESSAGE)
  # Says which of the gate, the generator or the standard library's module
  # metadata was missing, rather than leaving it to be guessed at
  message(STATUS
    "aoc: no import std - ${CMAKE_CXX_COMPILER_IMPORT_STD_ERROR_MESSAGE}")
endif()

# Before the aoc module below, which imports unordered_dense's module
add_subdirectory(external)

if (AOC_MODULE_SUPPORT)
  # Add module library aoc
  add_library(aoc)
  target_sources(aoc
    PUBLIC
      FILE_SET CXX_MODULES FILES
        common/module-aoc.cpp
  )
  target_link_libraries(aoc PUBLIC unordered_dense::unordered_dense_module)
endif()

# We use this to mark incomplete tests as skipped
set(AOC_SKIP_RETURN_CODE 17)
add_compile_definitions(AOC_SKIP_RETURN_CODE=${AOC_SKIP_RETURN_CODE})

function(copy_single_input_file target filename)
  add_custom_command(
    TARGET ${target} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      ${CMAKE_CURRENT_SOURCE_DIR}/${filename}
      ${CMAKE_CURRENT_BINARY_DIR}/${filename}
  )
endfunction()

function(copy_input_files target_basename year)
  copy_single_input_file(${year}_${target_basename} ${target_basename}.example)
  add_custom_command(
    TARGET ${year}_${target_basename} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      "${PROJECT_SOURCE_DIR}/${year}/${target_basename}.input"
      "${CMAKE_CURRENT_BINARY_DIR}/${target_basename}.input"
  )
endfunction()

add_custom_target(year_2015)
add_custom_target(year_2021)
add_custom_target(year_2022)
add_custom_target(year_2023)
add_custom_target(year_2024)
add_custom_target(year_2025)

function(add_sample sample_name year)
  set(target ${year}_${sample_name})
  add_executable(${target} "${sample_name}.cpp")
  if(NOT MSVC)
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wno-sign-compare
    )
  endif()
  copy_input_files(${sample_name} ${year})
  set_target_properties(${target} PROPERTIES FOLDER ${year})
  
  add_dependencies(year_${year} ${target})

  # Header-only outside module builds.
  # Inside one the module target carries the BMI
  # and the include directory comes along with it either way.
  target_link_libraries(${target} PRIVATE unordered_dense::unordered_dense)
  if (AOC_MODULE_SUPPORT)
    target_link_libraries(${target} PRIVATE aoc)
  endif()

  add_test(NAME ${target} COMMAND ${target})
  set_tests_properties(${target} PROPERTIES
      SKIP_RETURN_CODE ${AOC_SKIP_RETURN_CODE}
  )
endfunction()

find_program(CARGO_EXECUTABLE cargo)
if(NOT CARGO_EXECUTABLE)
  message(WARNING "Cargo not found; skipping Rust build")
else()
  add_subdirectory(rust)
endif()

add_subdirectory(2015)
add_subdirectory(2021)
add_subdirectory(2022)
add_subdirectory(2023)
add_subdirectory(2024)
add_subdirectory(2025)
