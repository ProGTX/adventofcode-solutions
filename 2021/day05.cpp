// https://adventofcode.com/2021/day/5

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

struct line_t {
  point start;
  point end;

  friend std::ostream& operator<<(std::ostream& out, line_t const& line) {
    out << line.start << " -> " << line.end;
    return out;
  }
};

using lines_t = std::vector<line_t>;

template <bool>
int solve_case(const std::string& filename) {
  lines_t lines;

  readfile_op(filename, [&](std::string_view line) {
    auto [start_str, end_str] = split<std::array<std::string, 2>>(line, '-');
    auto start = split<point>(start_str, ',');
    auto end = split<point>(std::string_view{end_str}.substr(sizeof(">")), ',');
    lines.emplace_back(start, end);
  });

  std::map<point, int> point_count;
  const auto increment_count = [&](point const& p) {
    auto it = point_count.find(p);
    if (it == std::end(point_count)) {
      point_count[p] = 1;
    } else {
      ++(it->second);
    }
  };

  for (line_t const& line : lines) {
    if (line.start.x == line.end.x) {
      int max_y = std::max(line.start.y, line.end.y);
      for (int y = std::min(line.start.y, line.end.y); y <= max_y; ++y) {
        increment_count(point{line.start.x, y});
      }
    } else if (line.start.y == line.end.y) {
      int max_x = std::max(line.start.x, line.end.x);
      for (int x = std::min(line.start.x, line.end.x); x <= max_x; ++x) {
        increment_count(point{x, line.start.y});
      }
    }
  }

  int score = std::ranges::count_if(
      point_count,
      [](typename decltype(point_count)::value_type const& count_pair) {
        return count_pair.second > 1;
      });
  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(5, (solve_case<false>("day05.example")));
  AOC_EXPECT_RESULT(6461, (solve_case<false>("day05.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(11, (solve_case<true>("day05.example")));
  // AOC_EXPECT_RESULT(11, (solve_case<true>("day05.input")));
  AOC_RETURN_CHECK_RESULT();
}
