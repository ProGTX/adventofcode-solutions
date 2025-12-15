// https://adventofcode.com/2025/day/5

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

using range_t = aoc::point_type<u64>;
using Input = std::pair<Vec<range_t>, Vec<u64>>;

auto parse(String const& filename) -> Input {
  using Point = aoc::point_type<u64>;
  auto lines = aoc::views::read_lines(filename, aoc::keep_empty{});

  auto ranges = lines |
                std::views::take_while([](str line) { return !line.empty(); }) |
                std::views::transform([&](str range) {
                  let point = aoc::split<Point>(range, "-");
                  return range_t{
                      .x = point.x,
                      .y = (point.y + 1),
                  };
                }) |
                aoc::ranges::to<Vec<range_t>>();
  std::ranges::sort(ranges, {}, &range_t::x);

  // Remove overlap from ranges
  // Needed by part 2, but also helps part 1 be faster
  ranges = [&] {
    auto ranges2 = Vec<range_t>{};
    auto last_end = u64{};
    for (auto& r : ranges) {
      if (last_end >= r.y) {
        continue;
      }
      if (last_end > r.x) {
        r.x = last_end;
      }
      last_end = r.y;
      ranges2.push_back(std::move(r));
    }
    return ranges2;
  }();

  auto ids = lines | aoc::views::to_number<u64>() | aoc::ranges::to<Vec<u64>>();
  std::ranges::sort(ids);

  return {std::move(ranges), std::move(ids)};
}

fn solve_case1(Input const& input) -> usize {
  let & [ ranges, ids ] = input;
  return std::ranges::count_if(ids, [&](u64 id) {
    return std::ranges::any_of(ranges, [&](range_t range) {
      return (id >= range.x) && (id < range.y);
    });
  });
}

fn solve_case2(Input const& input) -> usize {
  let & [ ranges, _ ] = input;
  return aoc::ranges::accumulate( //
      ranges |
          std::views::transform([](let& range) { return (range.y - range.x); }),
      usize{});
}

int main() {
  std::println("Part 1");
  let example = parse("day05.example");
  AOC_EXPECT_RESULT(3, solve_case1(example));
  let input = parse("day05.input");
  AOC_EXPECT_RESULT(821, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(14, solve_case2(example));
  AOC_EXPECT_RESULT(344771884978261, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
