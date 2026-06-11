// https://adventofcode.com/2023/day/14

#include "../common/common.h"
#include "../common/rust.h"

#include <print>
#include <unordered_map>

inline constexpr char round_rock = 'O';
inline constexpr char cube_rock = '#';
inline constexpr char empty = '.';

using platform_t = aoc::char_grid<>;
using platform_it = platform_t::iterator;

fn parse(String const& filename) -> platform_t {
  return aoc::read_char_grid(filename);
}

fn north_load(platform_t const& platform) -> i32 {
  auto total_load = i32{0};
  for (let row : Range{0uz, platform.num_rows()}) {
    for (let col : Range{0uz, platform.row_length()}) {
      if (platform.at(row, col) == round_rock) {
        total_load += static_cast<i32>(platform.num_rows() - row);
      }
    }
  }
  return total_load;
}

fn single_rock_fall(platform_it start_it, platform_it begin_it,
                    platform_it end_it, isize fall_stride) {
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

fn rocks_fall_vertical(platform_t& platform, point fall_diff) {
  AOC_ASSERT(fall_diff.x == 0, "Invalid fall_diff for vertical function");
  let fall_stride = static_cast<isize>(platform.row_length()) * fall_diff.y;
  auto row_it = (fall_stride < 0) ? platform.begin_row(0)
                                  : platform.begin_row(platform.num_rows() - 1);
  for (let _ : Range{0uz, platform.num_rows()}) {
    for (let col : Range{0uz, platform.row_length()}) {
      let start_it = row_it + static_cast<isize>(col);
      if (*start_it != round_rock) {
        continue;
      }
      single_rock_fall(start_it, platform.begin(), platform.end(), fall_stride);
    }
    row_it -= fall_stride;
  }
}

fn rocks_fall_horizontal(platform_t& platform, point fall_diff) {
  AOC_ASSERT(fall_diff.y == 0, "Invalid fall_diff for horizontal function");
  let fall_stride = static_cast<isize>(fall_diff.x);
  auto current_col = (fall_stride < 0)
                         ? isize{0}
                         : static_cast<isize>(platform.row_length() - 1);
  for (let _ : Range{0uz, platform.row_length()}) {
    for (let row : Range{0uz, platform.num_rows()}) {
      let row_begin_it = platform.begin_row(row);
      let start_it = row_begin_it + current_col;
      if (*start_it != round_rock) {
        continue;
      }
      single_rock_fall(start_it, row_begin_it, platform.end_row(row),
                       fall_stride);
    }
    current_col -= fall_stride;
  }
}

fn rocks_fall_inplace(platform_t& platform, aoc::facing_t direction) {
  let fall_diff = aoc::get_diff(direction);
  if (fall_diff.x == 0) {
    rocks_fall_vertical(platform, fall_diff);
  } else {
    rocks_fall_horizontal(platform, fall_diff);
  }
}

fn rocks_fall(platform_t platform, aoc::facing_t direction) -> platform_t {
  rocks_fall_inplace(platform, direction);
  return platform;
}

fn spin_cycle(platform_t& platform) {
  rocks_fall_vertical(platform, aoc::get_diff(aoc::north));
  rocks_fall_horizontal(platform, aoc::get_diff(aoc::west));
  rocks_fall_vertical(platform, aoc::get_diff(aoc::south));
  rocks_fall_horizontal(platform, aoc::get_diff(aoc::east));
}

fn test_platform() -> platform_t {
  return platform_t{String{"O....#...."
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

template <bool run_cycles>
fn solve_case(platform_t const& input) -> i32 {
  auto platform = input;
  if constexpr (!run_cycles) {
    return north_load(rocks_fall(std::move(platform), aoc::north));
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

    auto cycle_map = std::unordered_map<String, i32>{};
    constexpr auto num_iter = i32{1000000000};

    for (let i : Range{i32{}, num_iter}) {
      let platform_str = String{platform.data()};
      let it = cycle_map.find(platform_str);
      if (it != cycle_map.end()) {
        // We found a cycle, run only for the minimum amount of iterations
        let remainder = (num_iter - it->second) % (i - it->second);
        for (let _ : Range{i32{}, remainder}) {
          spin_cycle(platform);
        }
        break;
      }
      cycle_map.emplace(platform_str, i);
      spin_cycle(platform);
    }
    return north_load(platform);
  }
}

int main() {
  std::println("Asserts");
  AOC_EXPECT_RESULT("OOOO.#.O.."
                    "OO..#....#"
                    "OO..O##..O"
                    "O..#.OO..."
                    "........#."
                    "..#....#.#"
                    "..O..#.O.O"
                    "..O......."
                    "#....###.."
                    "#....#....",
                    rocks_fall(test_platform(), aoc::north).data());
  AOC_EXPECT_RESULT(136, north_load(rocks_fall(test_platform(), aoc::north)));

  std::println("Part 1");
  let example = parse("day14.example");
  AOC_EXPECT_RESULT(136, (solve_case<false>(example)));
  let input = parse("day14.input");
  AOC_EXPECT_RESULT(108857, (solve_case<false>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(64, (solve_case<true>(example)));
  AOC_EXPECT_RESULT(95273, (solve_case<true>(input)));

  AOC_RETURN_CHECK_RESULT();
}
