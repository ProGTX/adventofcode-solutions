// https://adventofcode.com/2023/day/14

#include "../common/common.h"

#include <algorithm>
#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using namespace std::string_view_literals;

inline constexpr char round_rock = 'O';
inline constexpr char cube_rock = '#';
inline constexpr char empty = '.';

using platform_t = aoc::char_grid<>;

constexpr int north_load(const platform_t& platform) {
  int total_load = 0;
  for (int row = 0; row < platform.num_rows(); ++row) {
    for (int col = 0; col < platform.row_length(); ++col) {
      if (platform.at(row, col) == round_rock) {
        total_load += platform.num_rows() - row;
      }
    }
  }
  return total_load;
}

constexpr void single_rock_fall(const typename platform_t::iterator start_it,
                                const typename platform_t::iterator begin_it,
                                const typename platform_t::iterator end_it,
                                const int fall_stride) {
  auto previous_it = start_it;
  // Rock falls
  for (auto current_it = start_it + fall_stride;
       (current_it >= begin_it) && (current_it < end_it);
       current_it += fall_stride) {
    if (*current_it != empty) {
      break;
    }
    previous_it = current_it;
  }
  if (previous_it != start_it) {
    *previous_it = round_rock;
    *start_it = empty;
  }
}

constexpr void rocks_fall_vertical(platform_t& platform,
                                   const point fall_diff) {
  AOC_ASSERT(fall_diff.x == 0, "Invalid fall_diff for vertical function");
  const auto fall_stride =
      static_cast<int>(platform.row_length()) * fall_diff.y;
  auto row_it = (fall_stride < 0) ? platform.begin_row(0)
                                  : platform.begin_row(platform.num_rows() - 1);
  for (int row = 0; row < platform.num_rows(); ++row, row_it -= fall_stride) {
    for (int col = 0; col < platform.row_length(); ++col) {
      const auto start_it = row_it + col;
      if (*start_it != round_rock) {
        continue;
      }
      single_rock_fall(start_it, platform.begin(), platform.end(), fall_stride);
    }
  }
}

constexpr void rocks_fall_horizontal(platform_t& platform,
                                     const point fall_diff) {
  AOC_ASSERT(fall_diff.y == 0, "Invalid fall_diff for horizontal function");
  const auto fall_stride = fall_diff.x;
  auto current_col = (fall_stride < 0) ? 0 : (platform.row_length() - 1);
  for (int col = 0; col < platform.row_length();
       ++col, current_col -= fall_stride) {
    for (int row = 0; row < platform.num_rows(); ++row) {
      const auto row_begin_it = platform.begin_row(row);
      const auto start_it = row_begin_it + current_col;
      if (*start_it != round_rock) {
        continue;
      }
      single_rock_fall(start_it, row_begin_it, platform.end_row(row),
                       fall_stride);
    }
  }
}

constexpr void rocks_fall_inplace(platform_t& platform,
                                  const aoc::facing_t direction) {
  const auto fall_diff = get_diff(direction);
  if (fall_diff.x == 0) {
    rocks_fall_vertical(platform, fall_diff);
  } else {
    rocks_fall_horizontal(platform, fall_diff);
  }
}

constexpr platform_t rocks_fall(platform_t platform,
                                const aoc::facing_t direction) {
  rocks_fall_inplace(platform, direction);
  return platform;
}

constexpr void spin_cycle(platform_t& platform) {
  rocks_fall_vertical(platform, get_diff(aoc::north));
  rocks_fall_horizontal(platform, get_diff(aoc::west));
  rocks_fall_vertical(platform, get_diff(aoc::south));
  rocks_fall_horizontal(platform, get_diff(aoc::east));
}

constexpr platform_t test_platform() {
  return {std::string{"O....#...."
                      "O.OO#....#"
                      ".....##..."
                      "OO.#O....O"
                      ".O.....O#."
                      "O.#..O.#.#"
                      "..O..#O..O"
                      ".......O.."
                      "#....###.."
                      "#OO..#...."},
          10, 10};
}

#if 0
static_assert(std::ranges::equal(platform_t{std::string{"OOOO.#.O.."
                                                        "OO..#....#"
                                                        "OO..O##..O"
                                                        "O..#.OO..."
                                                        "........#."
                                                        "..#....#.#"
                                                        "..O..#.O.O"
                                                        "..O......."
                                                        "#....###.."
                                                        "#....#...."},
                                            10,10},
                                 rocks_fall(test_platform(), aoc::north)));
static_assert(136 == north_load(rocks_fall(test_platform(), aoc::north)));
#endif

template <bool run_cycles>
int solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  platform_t platform = aoc::read_char_grid(filename);

  auto sum = 0;
  if constexpr (!run_cycles) {
    sum = north_load(rocks_fall(std::move(platform), aoc::north));
  } else {
    // Imagine an iteration that looks something like this:
    // |........|.................|.................|.................|........|
    // 0        S                S+A               S+2A              S+3A      N
    //
    // S is the first iteration where the cycle with a period of A begins
    // This cycle repeats until almost the end, where we need to figure out
    // the state of the cycle at point N, which represents num_iter.
    // So the cycle map will be used to store possible values of S,
    // then when we find a cycle we can figure out A and the remainder (x)
    // and calculate the state of the cycle at point N:
    // x = (N-S) % A

    std::unordered_map<std::string, int> cycle_map;
    const auto num_iter = 1000000000;

    for (int i = 0; i < num_iter; ++i) {
      const auto str = std::string{platform.data()};
      auto it = cycle_map.find(str);
      if (it != cycle_map.end()) {
        // We found a cycle, run only for the minimum amount of iterations
        const auto remainder = (num_iter - it->second) % (i - it->second);
        for (int j = 0; j < remainder; ++j) {
          spin_cycle(platform);
        }
        break;
      }
      cycle_map.emplace(str, i);
      spin_cycle(platform);
    }
    sum = north_load(platform);
  }

  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(136, (solve_case<false>("day14.example")));
  AOC_EXPECT_RESULT(108857, (solve_case<false>("day14.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(64, (solve_case<true>("day14.example")));
  AOC_EXPECT_RESULT(95273, (solve_case<true>("day14.input")));
  AOC_RETURN_CHECK_RESULT();
}
