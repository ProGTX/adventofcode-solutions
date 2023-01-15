// https://adventofcode.com/2021/day/6

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using int_t = int;

template <int production_length>
using school = std::array<int_t, production_length>;

template <int num_days, int production_length, int maturing_length>
int_t simulate_fish(school<production_length>& fish) {
  school<maturing_length> brood;
  std::ranges::fill(brood, 0);

  for (int day = 0; day < num_days; ++day) {
    int_t num_new_brood = fish.front();
    std::ranges::rotate(fish, fish.begin() + 1);

    int_t num_new_matures = brood.front();
    std::shift_left(brood.begin(), brood.end(), 1);

    fish.back() += num_new_matures;
    // Important to overwrite brood in the back
    // because shift_left did not necessarily clear them
    brood.back() = num_new_brood;
  }

  return std::accumulate(fish.begin(), fish.end(), int_t{0}) +
         std::accumulate(brood.begin(), brood.end(), int_t{0});
}

template <int num_days, int production_length, int maturing_length>
int_t solve_case(const std::string& filename) {
  using lanternfish = school<production_length>;
  lanternfish fish;
  std::ranges::fill(fish, 0);

  readfile_op(filename, [&](std::string_view line) {
    auto stages = split<std::vector<int>>(line, ',');
    for (const int fish_stage : stages) {
      ++fish[fish_stage];
    }
  });

  int_t num_fish =
      simulate_fish<num_days, production_length, maturing_length>(fish);
  std::cout << filename << " -> " << num_fish << std::endl;
  return num_fish;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(26, (solve_case<18, 7, 2>("day06.example")));
  AOC_EXPECT_RESULT(5934, (solve_case<80, 7, 2>("day06.example")));
  AOC_EXPECT_RESULT(371379, (solve_case<80, 7, 2>("day06.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(12, (solve_case<true>("day06.example")));
  // AOC_EXPECT_RESULT(18065, (solve_case<true>("day06.input")));
  AOC_RETURN_CHECK_RESULT();
}
