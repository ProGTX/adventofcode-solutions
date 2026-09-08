// https://adventofcode.com/2017/day/1

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <print>
#include <ranges>
#include <span>
#endif

auto parse(String const& filename) -> Vec<u8> {
  return aoc::read_single_line(filename) |
         stdv::transform([](char c) { return aoc::to_number<u8>(c); }) |
         aoc::collect_vec<u8>();
}

fn sum_matching(std::span<const u8> numbers, usize offset) -> u32 {
  let size = numbers.size();
  return aocr::accumulate( //
      aocv::indices(size) | stdv::transform([&](usize index) {
        let a = numbers[index];
        let b = numbers[(index + offset) % size];
        return (a == b) ? static_cast<u32>(a) : 0u;
      }),
      0u);
}

fn solve_case1(std::span<const u8> numbers) -> u32 {
  return sum_matching(numbers, 1uz);
}

fn solve_case2(std::span<const u8> numbers) -> u32 {
  return sum_matching(numbers, numbers.size() / 2);
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(3, solve_case1(Vec<u8>{1, 1, 2, 2}));
  AOC_EXPECT_RESULT(4, solve_case1(Vec<u8>{1, 1, 1, 1}));
  AOC_EXPECT_RESULT(0, solve_case1(Vec<u8>{1, 2, 3, 4}));
  let example = parse("day01.example");
  AOC_EXPECT_RESULT(9, solve_case1(example));
  let input = parse("day01.input");
  AOC_EXPECT_RESULT(1203, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(6, solve_case2(Vec<u8>{1, 2, 1, 2}));
  AOC_EXPECT_RESULT(0, solve_case2(Vec<u8>{1, 2, 2, 1}));
  AOC_EXPECT_RESULT(4, solve_case2(Vec<u8>{1, 2, 3, 4, 2, 5}));
  AOC_EXPECT_RESULT(12, solve_case2(Vec<u8>{1, 2, 3, 1, 2, 3}));
  AOC_EXPECT_RESULT(4, solve_case2(Vec<u8>{1, 2, 1, 3, 1, 4, 1, 5}));
  AOC_EXPECT_RESULT(6, solve_case2(example));
  AOC_EXPECT_RESULT(1146, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
