// https://adventofcode.com/2023/day/6

#include "../common/common.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <ostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

using int_t = std::int64_t;

// d=v*tr
// tr=t-tb
// v=tb
//
// -> d=tb*t-tb^2
constexpr int_t calculate_distance(int_t total_time, int_t button_time) {
  return button_time * total_time - button_time * button_time;
}

static_assert(6 == calculate_distance(7, 1));
static_assert(10 == calculate_distance(7, 2));
static_assert(12 == calculate_distance(7, 3));

constexpr int_t num_ways_to_win(int_t total_time, int_t record_distance) {
  return std::ranges::count_if(
      std::views::iota(1, total_time), [&](int_t button_time) {
        return calculate_distance(total_time, button_time) > record_distance;
      });
}

static_assert(4 == num_ways_to_win(7, 9));
static_assert(8 == num_ways_to_win(15, 40));
static_assert(9 == num_ways_to_win(30, 200));

#if defined(AOC_COMPILER_GCC)
static_assert(71503 == num_ways_to_win(71530, 940200));
#endif

constexpr int_t num_ways_to_win(std::span<const int_t> total_times,
                                std::span<const int_t> record_distances) {
  AOC_ASSERT(total_times.size() == record_distances.size(),
             "Number of times must match number of distances");
  int_t error_margin = 1;
  for (int i = 0; i < total_times.size(); ++i) {
    error_margin *= num_ways_to_win(total_times[i], record_distances[i]);
  }
  return error_margin;
}

template <bool single_race>
int_t solve_case(const std::string& filename) {
  std::vector<int_t> total_times;
  std::vector<int_t> record_distances;
  for (int linenum = 1;
       std::string_view line : aoc::views::read_lines(filename)) {
    auto number_str =
        line.substr((linenum == 1) ? sizeof("Time:") : sizeof("Distance:"));
    using store_t = std::conditional_t<single_race, std::string_view, int_t>;
    auto tmp_container =
        aoc::split<std::vector<store_t>, true>(number_str, ' ');
    auto& container = (linenum == 1) ? total_times : record_distances;
    if constexpr (!single_race) {
      container = std::move(tmp_container);
    } else {
      auto full_str =
          std::views::join(tmp_container) | aoc::ranges::to<std::string>();
      container.push_back(aoc::to_number<int_t>(full_str));
    }
    ++linenum;
  }

  int_t error_margin = num_ways_to_win(total_times, record_distances);
  std::cout << filename << " -> " << error_margin << std::endl;
  return error_margin;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(288, (solve_case<false>("day06.example")));
  AOC_EXPECT_RESULT(1624896, (solve_case<false>("day06.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(71503, (solve_case<true>("day06.example")));
  AOC_EXPECT_RESULT(32583852, (solve_case<true>("day06.input")));
  AOC_RETURN_CHECK_RESULT();
}
