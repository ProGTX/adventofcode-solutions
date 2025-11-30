// https://adventofcode.com/2021/day/6

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <ostream>
#include <print>
#include <ranges>
#include <string>
#include <string_view>

using int_t = std::int64_t;

template <int production_length>
using school = std::array<int_t, production_length>;

template <int num_days, int production_length, int maturing_length>
int_t simulate_fish(school<production_length>& fish) {
  school<maturing_length> brood{};

  for (int day = 0; day < num_days; ++day) {
    const auto num_new_brood = fish.front();
    aoc::ranges::rotate_left(fish);

    const auto num_new_matures = brood.front();
    std::shift_left(brood.begin(), brood.end(), 1);

    fish.back() += num_new_matures;
    brood.back() = num_new_brood;
  }

  return aoc::ranges::accumulate(fish, int_t{0}) +
         aoc::ranges::accumulate(brood, int_t{0});
}

template <int num_days, int production_length, int maturing_length>
int_t solve_case(const std::string& filename) {
  using lanternfish = school<production_length>;
  lanternfish fish{};

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto stages = aoc::split_to_vec<int>(line, ',');
    for (const int fish_stage : stages) {
      ++fish[fish_stage];
    }
  }

  int_t num_fish =
      simulate_fish<num_days, production_length, maturing_length>(fish);
  std::cout << filename << " -> " << num_fish << std::endl;
  return num_fish;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(26, (solve_case<18, 7, 2>("day06.example")));
  AOC_EXPECT_RESULT(5934, (solve_case<80, 7, 2>("day06.example")));
  AOC_EXPECT_RESULT(371379, (solve_case<80, 7, 2>("day06.input")));

  std::println("Part 2");
  AOC_EXPECT_RESULT(26984457539, (solve_case<256, 7, 2>("day06.example")));
  AOC_EXPECT_RESULT(1674303997472, (solve_case<256, 7, 2>("day06.input")));

  AOC_RETURN_CHECK_RESULT();
}
