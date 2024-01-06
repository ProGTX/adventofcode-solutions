// https://adventofcode.com/2023/day/14

#include "../common/common.h"
#include "../common/grid.h"

#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

inline constexpr char round_rock = 'O';
inline constexpr char cube_rock = '#';
inline constexpr char empty = '.';

using platform_t = grid<char, std::string, std::string>;

constexpr std::pair<platform_t, int> rocks_fall_vertical(
    platform_t platform, const point fall_diff) {
  AOC_ASSERT(fall_diff.x == 0, "Invalid fall_diff for vertical function");
  const auto fall_stride =
      static_cast<int>(platform.row_length()) * fall_diff.y;
  auto row_it = (fall_stride < 0) ? platform.begin() : (platform.end() - 1);
  int total_load = 0;
  int current_load = (fall_stride < 0) ? platform.num_rows() : 1;
  for (int row = 0; row < platform.num_rows();
       ++row, row_it -= fall_stride, current_load += fall_diff.y) {
    for (int col = 0; col < platform.row_length(); ++col) {
      auto previous_it = row_it + col;
      if (*previous_it != round_rock) {
        continue;
      }
      auto rock_load = current_load;
      // Rock falls
      for (auto fall_row_it = row_it + fall_stride;
           (fall_row_it >= platform.begin()) && (fall_row_it < platform.end());
           fall_row_it += fall_stride) {
        auto current_it = fall_row_it + col;
        if (*current_it != empty) {
          break;
        }
        *current_it = round_rock;
        *previous_it = empty;
        previous_it = current_it;
        rock_load -= fall_diff.y;
      }
      total_load += rock_load;
    }
  }
  return {platform, total_load};
}

constexpr std::pair<platform_t, int> rocks_fall_horizontal(
    platform_t platform, const point fall_diff) {
  AOC_ASSERT(fall_diff.y == 0, "Invalid fall_diff for horizontal function");
  // TODO
  return {platform, 0};
}

constexpr auto rocks_fall(platform_t platform, const facing_t direction) {
  const auto fall_diff = get_diff(direction);
  if (fall_diff.x == 0) {
    return rocks_fall_vertical(std::move(platform), fall_diff);
  } else {
    return rocks_fall_horizontal(std::move(platform), fall_diff);
  }
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
                                 rocks_fall(test_platform(), north).first));
static_assert(136 == rocks_fall(test_platform(), north).second);
#endif

template <bool>
int solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  platform_t platform;

  auto read_values = [&](std::string_view line) { platform.add_row(line); };
  readfile_op(filename, read_values);

  auto [settled, sum] = rocks_fall(platform, north);

  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(136, (solve_case<false>("day14.example")));
  AOC_EXPECT_RESULT(108857, (solve_case<false>("day14.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(400, (solve_case<true>("day14.example")));
  // AOC_EXPECT_RESULT(-36431, (solve_case<true>("day14.input")));
  AOC_RETURN_CHECK_RESULT();
}
