cmake_minimum_required(VERSION 3.16.0)
project(adventofcode VERSION 2022 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(AOC_INPUTS_DIR "${CMAKE_SOURCE_DIR}/inputs"
    CACHE PATH "Directory containing input files")

enable_testing()

function(copy_input_files target_basename)
  add_custom_command(
    TARGET 2022_${target_basename} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      ${CMAKE_CURRENT_SOURCE_DIR}/${target_basename}.example
      ${CMAKE_CURRENT_BINARY_DIR}/${target_basename}.example
  )
  add_custom_command(
    TARGET 2022_${target_basename} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      ${AOC_INPUTS_DIR}/2022/${target_basename}.input
      ${CMAKE_CURRENT_BINARY_DIR}/${target_basename}.input
  )
endfunction()

add_subdirectory(2022)
