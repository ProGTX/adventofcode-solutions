// https://adventofcode.com/2015/day/20

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>

template <u32 target, bool Part2>
fn solve_case() -> u32 {
  constexpr let multiplier = Part2 ? 11u : 10u;
  constexpr let size = static_cast<size_t>(target / multiplier);
  auto presents = Vec<u32>(size + 1, 0u);
  for (let elf : Range{1uz, size + 1}) {
    let last = Part2 ? std::min(elf * 50uz, size) : size;
    auto house = elf;
    while (house <= last) {
      presents[house] += static_cast<u32>(elf) * multiplier;
      house += elf;
    }
  }
  return std::get<0>(*stdr::find_if(
      presents | stdv::enumerate | stdv::drop(1),
      [](let& house_pair) { return std::get<1>(house_pair) >= target; }));
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(8, (solve_case<150, false>()));
  AOC_EXPECT_RESULT(831600, (solve_case<36'000'000, false>()));

  std::println("Part 2");
  AOC_EXPECT_RESULT(8, (solve_case<150, true>()));
  AOC_EXPECT_RESULT(884520, (solve_case<36'000'000, true>()));

  AOC_RETURN_CHECK_RESULT();
}
