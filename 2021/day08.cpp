// https://adventofcode.com/2021/day/8

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>
#include <ranges>

using Segment = aoc::static_vector<u32, 7>;
using Pattern = std::array<Segment, 10>;
using Output = std::array<Segment, 4>;
using Input = std::pair<Vec<Pattern>, Vec<Output>>;

fn parse_segment(str s) -> Segment {
  return s |
         stdv::transform([](char c) { return static_cast<u32>(c - 'a'); }) |
         aoc::ranges::to<Segment>();
}

auto parse(String const& filename) -> Input {
  auto patterns = Vec<Pattern>{};
  auto outputs = Vec<Output>{};
  constexpr let skip_empty = true;
  for (str line : aoc::views::read_lines(filename)) {
    let[pattern_str, output_str] = aoc::split_once(line, '|');
    auto pat = Pattern{};
    stdr::transform(aoc::split_to_array<10, str, skip_empty>(pattern_str, ' '),
                    pat.begin(), parse_segment);
    patterns.push_back(std::move(pat));
    auto out = Output{};
    stdr::transform(aoc::split_to_array<4, str, skip_empty>(output_str, ' '),
                    out.begin(), parse_segment);
    outputs.push_back(std::move(out));
  }
  return {patterns, outputs};
}

constexpr let segment_lengths = std::array{6, 2, 5, 5, 4, 5, 6, 3, 7, 6};
constexpr let unique_lengths = std::array{
    segment_lengths[1],
    segment_lengths[4],
    segment_lengths[7],
    segment_lengths[8],
};

fn solve_case1(Input const& input) -> u32 {
  let& outputs = input.second;
  return aoc::ranges::accumulate(
      outputs | stdv::transform([&](Output const& output) {
        return static_cast<u32>(stdr::count_if(output, [&](Segment const& seg) {
          return stdr::contains(unique_lengths, static_cast<u32>(seg.size()));
        }));
      }),
      0u);
}

int main() {
  std::println("Part 1");
  let example = parse("day08.example");
  AOC_EXPECT_RESULT(0, solve_case1(example));
  let example2 = parse("day08.example2");
  AOC_EXPECT_RESULT(26, solve_case1(example2));
  let input = parse("day08.input");
  AOC_EXPECT_RESULT(440, solve_case1(input));

  AOC_RETURN_CHECK_RESULT();
}
