// https://adventofcode.com/2022/day/20

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

using value_t = std::int64_t;
using list_t = std::vector<value_t>;
using indexed_num_t = std::tuple<std::ptrdiff_t, value_t>;

namespace stdv = std::views;
namespace stdr = std::ranges;

template <std::size_t num_mixings, class OGList>
list_t reorder_list(OGList&& original_list) {
  const auto numbers = original_list |
                       stdv::enumerate |
                       aoc::ranges::to<std::vector<indexed_num_t>>();
  auto mixed = numbers;

  for (auto _ : stdv::iota(0uz, num_mixings)) {
    for (const auto& indexed_number : numbers) {
      const auto [orig_index, orig_value] = indexed_number;
      if (orig_value == 0) {
        continue;
      }

      const auto current_it =
          stdr::find(mixed, orig_index,
                     [](const indexed_num_t& mix) { return std::get<0>(mix); });

      const auto next_it = [&] {
        auto it = current_it;
        const auto begin = std::begin(mixed);
        const auto size = std::ssize(mixed) - 1;
        auto pos = std::distance(begin, it);
        pos = (size + (pos + (orig_value % size))) % size;
        it = begin + pos;
        return it;
      }();

      const auto it_distance = std::distance(current_it, next_it);
      if (it_distance < 0) {
        std::shift_right(next_it, current_it + 1, 1);
      } else {
        std::shift_left(current_it, next_it + 1, 1);
      }
      *next_it = indexed_number;
    }
  }

  return mixed | stdv::values | aoc::ranges::to<list_t>();
}

value_t sum_numbers(const list_t& list) {
  return aoc::ranges::accumulate(
      stdv::iota(0, 3) | stdv::transform([&, zero_it = stdr::find(list, 0)](
                                             const auto _) mutable {
        const auto begin = std::begin(list);
        auto pos = std::distance(begin, zero_it);
        pos = (pos + 1000) % std::size(list);
        zero_it = begin + pos;
        return *zero_it;
      }),
      value_t{});
}

template <int num_mixings, value_t decryption_key>
value_t solve_case(const std::string& filename) {
  const auto mixed = reorder_list<num_mixings>(
      aoc::views::read_numbers<int>(filename) |
      stdv::transform([](int value) { return value * decryption_key; }));
  return sum_numbers(mixed);
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(3, (solve_case<1, 1>("day20.example")));
  AOC_EXPECT_RESULT(1591, (solve_case<1, 1>("day20.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(1623178306, (solve_case<10, 811589153>("day20.example")));
  AOC_EXPECT_RESULT(14579387544492, (solve_case<10, 811589153>("day20.input")));
  AOC_RETURN_CHECK_RESULT();
}
