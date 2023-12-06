cmake_minimum_required(VERSION 3.16.0)
project(adventofcode VERSION 2022 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(AOC_INPUTS_DIR "${CMAKE_SOURCE_DIR}/inputs"
    CACHE PATH "Directory containing input files")

enable_testing()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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
  add_test(NAME ${target} COMMAND ${target})
endfunction()

add_subdirectory(2021)
add_subdirectory(2022)
add_subdirectory(2023)
