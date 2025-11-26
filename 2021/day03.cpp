// https://adventofcode.com/2021/day/3

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <span>

using Bitset = aoc::static_vector<u8, 12>;
using Counter = aoc::static_vector<i16, 12>;

fn parse(String const& filename) -> Vec<Bitset> {
  return aoc::views::read_lines(filename) |
         std::views::transform([](str line) {
           return line |
                  aoc::views::to_number<u8>() |
                  aoc::ranges::to<Bitset>();
         }) |
         aoc::ranges::to<Vec<Bitset>>();
}

fn count_bits(std::span<const Bitset> report) -> Counter {
  let init = Counter(report[0].size(), 0);
  return std::ranges::fold_left(report, init, [](let& acc, let& bitset) {
    return std::views::zip(acc, bitset) |
           std::views::transform([](let&& zip_it) {
             let[count, bit] = zip_it;
             return count + static_cast<i16>(bit) - static_cast<i16>(bit == 0);
           }) |
           aoc::ranges::to<Counter>();
  });
}

fn to_decimal(Bitset const& report) -> u32 {
  using pair = aoc::point_type<u32>;
  return std::ranges::fold_left(
             report | std::views::reverse, pair{0, 1},
             [](let& acc_pair, let bit) {
               let[acc, multiplier] = acc_pair;
               return pair{acc + (static_cast<u32>(bit) * multiplier),
                           multiplier * 2};
             })
      .x;
}

fn solve_case1(std::span<const Bitset> report) -> u32 {
  let gamma = count_bits(report) |
              std::views::transform(
                  [](let count) { return static_cast<u8>(count > 0); }) |
              aoc::ranges::to<Bitset>();
  let epsilon = gamma |
                std::views::transform(aoc::equal_to_value{0}) |
                aoc::views::transform_cast<u8>() |
                aoc::ranges::to<Bitset>();
  return to_decimal(gamma) * to_decimal(epsilon);
}

fn solve_case2(std::span<const Bitset> report) -> u32 {
  using Report = Vec<Bitset>;
  auto oxy_gen = report | aoc::ranges::to<Report>();
  auto co2_scrubber = report | aoc::ranges::to<Report>();
  for (let index : Range{0uz, report[0].size()}) {
    if (oxy_gen.size() > 1) {
      let oxy_count = count_bits(oxy_gen);
      oxy_gen = oxy_gen |
                std::views::filter([&](let& bitset) {
                  return (oxy_count[index] >= 0) == (bitset[index] > 0);
                }) |
                aoc::ranges::to<Report>();
    }
    if (co2_scrubber.size() > 1) {
      let co2_count = count_bits(co2_scrubber);
      co2_scrubber = co2_scrubber |
                     std::views::filter([&](let& bitset) {
                       return (co2_count[index] >= 0) != (bitset[index] > 0);
                     }) |
                     aoc::ranges::to<Report>();
    }
  }
  return to_decimal(oxy_gen[0]) * to_decimal(co2_scrubber[0]);
}

int main() {
  std::cout << "Part 1" << std::endl;
  let example = parse("day03.example");
  AOC_EXPECT_RESULT(198, solve_case1(example));
  let input = parse("day03.input");
  AOC_EXPECT_RESULT(4138664, solve_case1(input));

  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(230, solve_case2(example));
  AOC_EXPECT_RESULT(4273224, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
