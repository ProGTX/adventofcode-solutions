// https://adventofcode.com/2024/day/2

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>
#include <ranges>
#include <span>

using Report = aoc::static_vector<i8, 8>;

auto parse(String const& filename) -> Vec<Report> {
  return aoc::views::read_lines(filename) |
         stdv::transform(
             [](str line) { return aoc::split<Report>(line, ' '); }) |
         aoc::ranges::to<Vec<Report>>();
}

// A report only counts as safe if both of the following are true:
//   The levels are either all increasing or all decreasing.
//   Any two adjacent levels differ by at least one and at most three.
template <stdr::forward_range R>
  requires std::convertible_to<stdr::range_value_t<R>, i8>
fn is_report_safe(R&& report) -> bool {
  let diffs =
      report | stdv::pairwise_transform([](i8 a, i8 b) { return b - a; });
  AOC_ASSERT(!stdr::empty(diffs), "There must be at least 2 levels");
  let increasing_factor = aoc::sign(*stdr::begin(diffs));
  return stdr::all_of(diffs, [=](i8 diff) {
    diff *= increasing_factor;
    return (diff >= 1) && (diff <= 3);
  });
}

static_assert(is_report_safe(std::array{7, 6, 4, 2, 1}));
static_assert(!is_report_safe(std::array{1, 2, 7, 8, 9}));
static_assert(!is_report_safe(std::array{9, 7, 6, 2, 1}));
static_assert(!is_report_safe(std::array{1, 3, 2, 4, 5}));
static_assert(!is_report_safe(std::array{8, 6, 4, 4, 1}));
static_assert(is_report_safe(std::array{1, 3, 6, 7, 9}));

fn is_dampened_safe(Report const& report) -> bool {
  return stdr::any_of(Range{0uz, report.size()}, [&](usize drop_index) {
    return is_report_safe(
        Range{0uz, report.size()} |
        aoc::views::transform_filter([=](usize index) -> Option<i8> {
          if (index == drop_index) {
            return None;
          }
          return report[index];
        }));
  });
}

template <bool dampen>
fn solve_case(Vec<Report> const& reports) -> u32 {
  return aoc::ranges::accumulate(
      reports | stdv::transform([](Report const& report) {
        return static_cast<u32>(is_report_safe(report) ||
                                (dampen && is_dampened_safe(report)));
      }),
      u32{});
}

int main() {
  std::println("Part 1");
  let example = parse("day02.example");
  AOC_EXPECT_RESULT(2, solve_case<false>(example));
  let input = parse("day02.input");
  AOC_EXPECT_RESULT(502, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(4, solve_case<true>(example));
  AOC_EXPECT_RESULT(544, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
