// https://adventofcode.com/2016/day/20

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <limits>
#include <print>
#endif

using BlockedRange = aoc::closed_range<u32>;

/// The blocked ranges of IPs, ordered by where they begin
using Input = Vec<BlockedRange>;

auto parse(String const& filename) -> Input {
  auto input = aoc::views::read_lines(filename) |
               stdv::transform([](str line) {
                 // BEGIN-END
                 let[begin, end] = aoc::split_once<u32>(line, '-');
                 return BlockedRange{begin, end};
               }) |
               aoc::collect_vec<BlockedRange>();
  stdr::sort(input);
  return input;
}

/// The lowest IP that is not blocked by any of the ranges,
/// or how many IPs in the whole u32 range the ranges leave allowed
template <bool COUNT_ALLOWED>
fn solve_case(Input const& blacklist) -> u32 {
  AOC_ASSERT((blacklist[0].begin == 0),
             "Algorithm assumes there is nothing before first range");
  auto num_allowed = u32{};
  // The lowest IP not yet known to be blocked
  auto lowest = u32{};
  for (let& range : blacklist) {
    // Sorted by where they begin, so anything between the ranges is a gap
    if (range.begin > lowest) {
      // The first gap already holds the lowest allowed IP
      if constexpr (!COUNT_ALLOWED) {
        return lowest;
      } else {
        num_allowed += range.begin - lowest;
      }
    }
    // A range reaching the top of the address space blocks all the rest
    if (range.end == std::numeric_limits<u32>::max()) {
      return num_allowed;
    }
    // Ranges already passed can reach further than this one does
    lowest = std::max(lowest, range.end + 1);
  }
  // Everything above the last blocked range is allowed
  if constexpr (!COUNT_ALLOWED) {
    return lowest;
  } else {
    return num_allowed + (std::numeric_limits<u32>::max() - lowest) + 1;
  }
}

int main() {
  std::println("Part 1");
  let example = parse("day20.example");
  AOC_EXPECT_RESULT(3, solve_case<false>(example));
  let input = parse("day20.input");
  AOC_EXPECT_RESULT(4793564, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(4294967288, solve_case<true>(example));
  AOC_EXPECT_RESULT(146, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
