// https://adventofcode.com/2022/day/20

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using value_t = std::int64_t;

struct indexed_num_t {
  value_t value;
  int index;
};

using list_t = std::vector<value_t>;
using indexed_list_t = std::vector<indexed_num_t>;

template <int num_mixings>
list_t reorder_list(const list_t& original_list) {
  indexed_list_t original_list_indexed;
  for (int index = 0; index < original_list.size(); ++index) {
    original_list_indexed.emplace_back(original_list[index], index);
  }

  indexed_list_t reordered_list = original_list_indexed;

  for (int mixing = 0; mixing < num_mixings; ++mixing) {
    for (const auto indexed_number : original_list_indexed) {
      if (indexed_number.value == 0) {
        continue;
      }

      auto current_it =
          std::ranges::find_if(reordered_list, [&](const indexed_num_t& rnum) {
            return rnum.index == indexed_number.index;
          });
      auto next_it = (aoc::cyclic_iterator{aoc::linked_list_iterator_tag{},
                                           reordered_list, current_it} +
                      indexed_number.value)
                         .to_underlying();

      auto it_distance = std::distance(current_it, next_it);
      if (it_distance < 0) {
        std::shift_right(next_it, current_it + 1, 1);
      } else {
        std::shift_left(current_it, next_it + 1, 1);
      }
      *next_it = indexed_number;
    }
  }

  list_t reordered_simple;
  std::ranges::transform(
      reordered_list, std::back_inserter(reordered_simple),
      [](const indexed_num_t& indexed_number) { return indexed_number.value; });
  return reordered_simple;
}

value_t sum_numbers(const list_t& list) {
  auto zero_it = std::ranges::find(list, 0);
  auto cyclic_it = aoc::cyclic_iterator{list, zero_it};
  value_t sum = 0;
  std::cout << "sum_numbers" << std::endl;
  cyclic_it += 1000;
  std::cout << "  " << *cyclic_it << std::endl;
  sum += *cyclic_it;
  cyclic_it += 1000;
  std::cout << "  " << *cyclic_it << std::endl;
  sum += *cyclic_it;
  cyclic_it += 1000;
  std::cout << "  " << *cyclic_it << std::endl;
  sum += *cyclic_it;
  return sum;
}

template <int num_mixings>
value_t sum_list(const list_t& original_list) {
  list_t reordered_list = reorder_list<num_mixings>(original_list);
  return sum_numbers(reordered_list);
}

template <int num_mixings, value_t decription_key>
value_t solve_case(const std::string& filename) {
  list_t original_list;
  std::ranges::transform(aoc::views::read_numbers<int>(filename),
                         std::back_inserter(original_list),
                         [](int value) { return value * decription_key; });
  value_t sum = sum_list<num_mixings>(original_list);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
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
