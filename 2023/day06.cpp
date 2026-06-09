// https://adventofcode.com/2023/day/6

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <functional>
#include <print>
#include <ranges>
#include <span>

struct Input {
  Vec<String> time_tokens;
  Vec<String> dist_tokens;
};

auto parse(String const& filename) -> Input {
  let lines = aoc::read_lines(filename);
  return {
      .time_tokens = aoc::split_to_vec<String, true>(
          lines[0].substr(sizeof("Time:")), ' '),
      .dist_tokens = aoc::split_to_vec<String, true>(
          lines[1].substr(sizeof("Distance:")), ' '),
  };
}

// d=v*tr
// tr=t-tb
// v=tb
//
// -> d=tb*t-tb^2
fn calculate_distance(i64 total_time, i64 button_time) -> i64 {
  return button_time * total_time - button_time * button_time;
}

static_assert(6 == calculate_distance(7, 1));
static_assert(10 == calculate_distance(7, 2));
static_assert(12 == calculate_distance(7, 3));

fn num_ways_to_win(i64 total_time, i64 record_distance) -> i64 {
  return stdr::count_if(Range{i64{1}, total_time}, [&](i64 button_time) {
    return calculate_distance(total_time, button_time) > record_distance;
  });
}

static_assert(4 == num_ways_to_win(7, 9));
static_assert(8 == num_ways_to_win(15, 40));
static_assert(9 == num_ways_to_win(30, 200));

#if defined(AOC_COMPILER_GCC)
static_assert(71503 == num_ways_to_win(71530, 940200));
#endif

fn num_ways_to_win(std::span<const i64> total_times,
                   std::span<const i64> record_distances) -> i64 {
  AOC_ASSERT(total_times.size() == record_distances.size(),
             "Number of times must match number of distances");
  return stdr::fold_left(
      Range{0uz, total_times.size()} | std::views::transform([&](usize i) {
        return num_ways_to_win(total_times[i], record_distances[i]);
      }),
      i64{1}, std::multiplies{});
}

fn solve_case1(Input const& input) -> i64 {
  let total_times = input.time_tokens |
                    aoc::views::to_number<i64>() |
                    aoc::collect_vec<i64>();
  let record_distances = input.dist_tokens |
                         aoc::views::to_number<i64>() |
                         aoc::collect_vec<i64>();
  return num_ways_to_win(total_times, record_distances);
}

fn solve_case2(Input const& input) -> i64 {
  let time_str = stdv::join(input.time_tokens) | aoc::collect_string();
  let dist_str = stdv::join(input.dist_tokens) | aoc::collect_string();
  return num_ways_to_win(aoc::to_number<i64>(time_str),
                         aoc::to_number<i64>(dist_str));
}

int main() {
  std::println("Part 1");
  let example = parse("day06.example");
  AOC_EXPECT_RESULT(288, solve_case1(example));
  let input = parse("day06.input");
  AOC_EXPECT_RESULT(1624896, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(71503, solve_case2(example));
  AOC_EXPECT_RESULT(32583852, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
