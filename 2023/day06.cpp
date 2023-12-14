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

// d=v*tr
// tr=t-tb
// v=tb
//
// -> d=tb*t-tb^2
constexpr int calculate_distance(int total_time, int button_time) {
  return button_time * total_time - button_time * button_time;
}

static_assert(6 == calculate_distance(7, 1));
static_assert(10 == calculate_distance(7, 2));
static_assert(12 == calculate_distance(7, 3));

constexpr int num_ways_to_win(int total_time, int record_distance) {
  return std::ranges::count_if(
      std::views::iota(1, total_time), [&](int button_time) {
        return calculate_distance(total_time, button_time) > record_distance;
      });
}

static_assert(4 == num_ways_to_win(7, 9));
static_assert(8 == num_ways_to_win(15, 40));
static_assert(9 == num_ways_to_win(30, 200));

constexpr int num_ways_to_win(std::span<const int> total_times,
                              std::span<const int> record_distances) {
  AOC_ASSERT(total_times.size() == record_distances.size(),
             "Number of times must match number of distances");
  int error_margin = 1;
  for (int i = 0; i < total_times.size(); ++i) {
    error_margin *= num_ways_to_win(total_times[i], record_distances[i]);
  }
  return error_margin;
}

template <bool>
int solve_case(const std::string& filename) {
  std::vector<int> total_times;
  std::vector<int> record_distances;
  auto read_values = [&](std::string_view line, int linenum) {
    if (linenum == 1) {
      total_times =
          split<std::vector<int>, true>(line.substr(sizeof("Time:")), ' ');
    } else if (linenum == 2) {
      record_distances =
          split<std::vector<int>, true>(line.substr(sizeof("Distance:")), ' ');
    }
  };
  readfile_op(filename, read_values);

  int error_margin = num_ways_to_win(total_times, record_distances);
  std::cout << filename << " -> " << error_margin << std::endl;
  return error_margin;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(288, (solve_case<false>("day06.example")));
  AOC_EXPECT_RESULT(1624896, (solve_case<false>("day06.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(46, (solve_case<true>("day06.example")));
  // AOC_EXPECT_RESULT(56931769, (solve_case<true>("day06.input")));
  AOC_RETURN_CHECK_RESULT();
}
