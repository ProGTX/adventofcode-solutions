// https://adventofcode.com/2025/day/2

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <span>

using range_t = Range<u64, u64>;

fn parse(String const& filename) -> Vec<range_t> {
  using Point = aoc::point_type<u64>;
  return aoc::split(aoc::trim(aoc::read_file(filename)), ",") |
         std::views::transform([](str range) {
           let point = aoc::split<Point>(range, "-");
           return range_t(point.x, (point.y + 1));
         }) |
         aoc::ranges::to<Vec<range_t>>();
}

fn solve_case1(std::span<const range_t> ranges) -> u64 {
  return aoc::ranges::accumulate(
      ranges | std::views::transform([](range_t range) {
        return aoc::ranges::accumulate(
            range | std::views::filter([](u64 id) {
              let id_str = std::to_string(id);
              let s = str{id_str};
              let half = s.size() / 2;
              return (s.size() % 2 == 0) &&
                     (s.substr(0, half) == s.substr(half));
            }),
            u64{});
      }),
      u64{});
}

fn solve_case2(std::span<const range_t> ranges) -> u64 {
  return aoc::ranges::accumulate(
      ranges | std::views::transform([](range_t range) {
        return aoc::ranges::accumulate(
            range | std::views::filter([](u64 id) {
              let id_str = std::to_string(id);
              let s = str{id_str};
              let size = s.size();
              let divisors = aoc::divisors(size);
              // Skip 1
              return std::ranges::any_of(
                  divisors | std::views::drop(1), [&](u64 divisor) {
                    let chunks = s | std::views::chunk(size / (divisor));
                    auto chunks_it = std::begin(chunks);
                    let first = *chunks_it | aoc::ranges::to<String>();
                    return std::ranges::all_of(
                        chunks_it, std::end(chunks), [&](auto chunk) {
                          return (chunk | aoc::ranges::to<String>()) == first;
                        });
                  });
            }),
            u64{});
      }),
      u64{});
}

int main() {
  std::println("Part 1");
  let example = parse("day02.example");
  AOC_EXPECT_RESULT(1227775554, solve_case1(example));
  let input = parse("day02.input");
  AOC_EXPECT_RESULT(30323879646, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(4174379265, solve_case2(example));
  AOC_EXPECT_RESULT(43872163557, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
