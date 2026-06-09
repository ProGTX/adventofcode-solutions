// https://adventofcode.com/2023/day/9

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <span>
#include <string>

using Reading = Vec<i32>;
using Input = Vec<Reading>;

fn parse(String const& filename) -> Input {
  return aoc::views::read_lines(filename) |
         stdv::transform(
             [](str line) { return aoc::split_to_vec<i32>(line, ' '); }) |
         aoc::collect_vec<Reading>();
}

template <int multiplier>
fn predict_value(Reading reading) -> i32 {
  static_assert(multiplier != 0);

  auto edge_values = Vec<i32>{};
  auto next_reading = Vec<i32>{};

  for (let _ : Range{0uz, reading.size()}) {
    if constexpr (multiplier > 0) {
      edge_values.push_back(reading.back());
    } else {
      edge_values.push_back(reading.front());
    }
    auto next_reading = //
        reading |
        stdv::adjacent<2> |
        stdv::transform([](auto const& ab) {
          let[a, b] = ab;
          return b - a;
        }) |
        aoc::collect_vec<i32>();
    if (stdr::all_of(next_reading, aoc::equal_to_value(0))) {
      break;
    }
    std::swap(next_reading, reading);
  }
  return stdr::fold_right(edge_values, i32{0}, [](i32 element, i32 acc) {
    return element + acc * multiplier;
  });
}

static_assert(18 == predict_value<1>({0, 3, 6, 9, 12, 15}));
static_assert(28 == predict_value<1>({1, 3, 6, 10, 15, 21}));
static_assert(68 == predict_value<1>({10, 13, 16, 21, 30, 45}));
static_assert(-3 == predict_value<-1>({0, 3, 6, 9, 12, 15}));
static_assert(0 == predict_value<-1>({1, 3, 6, 10, 15, 21}));
static_assert(5 == predict_value<-1>({10, 13, 16, 21, 30, 45}));

template <int multiplier>
fn solve_case(Input const& input) -> i32 {
  return aoc::ranges::accumulate(
      input | stdv::transform(predict_value<multiplier>), 0);
}

int main() {
  std::println("Part 1");
  let example = parse("day09.example");
  AOC_EXPECT_RESULT(114, solve_case<1>(example));
  let input = parse("day09.input");
  AOC_EXPECT_RESULT(1980437560, solve_case<1>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(2, solve_case<-1>(example));
  AOC_EXPECT_RESULT(977, solve_case<-1>(input));

  AOC_RETURN_CHECK_RESULT();
}
