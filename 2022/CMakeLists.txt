add_sample(day01 2022)
add_sample(day02 2022)
add_sample(day03 2022)
add_sample(day04 2022)
add_sample(day05 2022)
add_sample(day06 2022)
add_sample(day07 2022)
add_sample(day08 2022)
add_sample(day09 2022)
add_custom_command(
  TARGET 2022_day09 PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/day09.example2
    ${CMAKE_CURRENT_BINARY_DIR}/day09.example2
)
add_sample(day10 2022)
add_sample(day11 2022)
add_sample(day12 2022)
