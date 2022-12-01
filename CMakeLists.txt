cmake_minimum_required(VERSION 3.16.0)
project(adventofcode VERSION 2022 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(AOC_INPUTS_DIR "${CMAKE_SOURCE_DIR}/inputs"
    CACHE PATH "Directory containing input files")

enable_testing()

function(copy_input_files target_basename year)
  add_custom_command(
    TARGET ${year}_${target_basename} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      ${CMAKE_CURRENT_SOURCE_DIR}/${target_basename}.example
      ${CMAKE_CURRENT_BINARY_DIR}/${target_basename}.example
  )
  add_custom_command(
    TARGET ${year}_${target_basename} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      ${AOC_INPUTS_DIR}/${year}/${target_basename}.input
      ${CMAKE_CURRENT_BINARY_DIR}/${target_basename}.input
  )
endfunction()

function(add_sample sample_name year)
  add_executable(${year}_${sample_name} "${sample_name}.cpp")
  if(NOT MSVC)
    target_compile_options(${year}_${sample_name} PRIVATE
      -Wall -Wextra -Wno-sign-compare
    )
  endif()
  copy_input_files(${sample_name} ${year})
  add_test(NAME ${year}_${sample_name} COMMAND ${year}_${sample_name})
endfunction()

add_subdirectory(2021)
add_subdirectory(2022)
