// https://adventofcode.com/2015/day/10

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>

void look_and_say(Vec<u32> const& input, Vec<u32>& output) {
  output.clear();
  auto index = 0u;
  while (index < input.size()) {
    let current = input[index];
    let count = static_cast<u32>(
        stdr::distance(input |
                       stdv::drop(index) |
                       stdv::take_while(aoc::equal_to_value{current})));
    output.push_back(count);
    output.push_back(current);
    index += count;
  }
}

template <int iterations>
int solve_case(const std::string& filename) {
  auto input = aoc::read_single_line(filename) |
               aoc::views::to_number<u32>() |
               aoc::collect_vec<u32>();
  auto output = decltype(input){};
  for ([[maybe_unused]] auto index : stdv::iota(0, iterations)) {
    look_and_say(input, output);
    std::swap(input, output);
  }
  return input.size();
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(237746, solve_case<40>("day10.example"));
  AOC_EXPECT_RESULT(360154, solve_case<40>("day10.input"));
  std::println("Part 2");
  AOC_EXPECT_RESULT(3369156, solve_case<50>("day10.example"));
  AOC_EXPECT_RESULT(5103798, solve_case<50>("day10.input"));
  AOC_RETURN_CHECK_RESULT();
}
