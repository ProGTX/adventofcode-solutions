cmake_minimum_required(VERSION 3.28)
project(adventofcode VERSION 2022 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(AOC_INPUTS_DIR "${CMAKE_SOURCE_DIR}/inputs"
    CACHE PATH "Directory containing input files")

enable_testing()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_compile_definitions(AOC_CXX_STANDARD=${CMAKE_CXX_STANDARD})

message("CMAKE_CXX_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION}")

if (((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND
      (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "16.0")) OR
    ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") AND
      (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "14.1")) OR
    ((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") AND
      (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "17.4")))
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_definitions(AOC_MODULE_SUPPORT_AOC_STD)
  endif()
  set(AOC_MODULE_SUPPORT ON)
  add_compile_definitions(AOC_MODULE_SUPPORT)
else()
  set(AOC_MODULE_SUPPORT OFF)
endif()

if (AOC_MODULE_SUPPORT)
  # Add module library aoc
  add_library(aoc)
  target_sources(aoc
    PUBLIC
      FILE_SET CXX_MODULES FILES
        common/module-aoc.cpp
  )
  add_library(aoc.std)
  target_sources(aoc.std
    PUBLIC
      FILE_SET CXX_MODULES FILES
        common/module-aoc-std.cpp
  )
endif()

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
      ${AOC_INPUTS_DIR}/${year}/${target_basename}.input
      ${CMAKE_CURRENT_BINARY_DIR}/${target_basename}.input
  )
endfunction()

add_custom_target(year_2021)
add_custom_target(year_2022)
add_custom_target(year_2023)

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

  if (AOC_MODULE_SUPPORT)
    target_link_libraries(${target} aoc aoc.std)
  endif()

  add_test(NAME ${target} COMMAND ${target})
endfunction()

add_subdirectory(2021)
add_subdirectory(2022)
add_subdirectory(2023)
