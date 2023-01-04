// https://adventofcode.com/2022/day/20

#include "../common/common.h"
#include "../common/print.h"

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

using list_t = std::vector<int>;

template <int num_mixings>
list_t reorder_list(const list_t& original_list) {
  list_t reordered_list = original_list;

  for (const int number : original_list) {
    if (number == 0) {
      continue;
    }

    auto current_it = std::ranges::find(reordered_list, number);
    auto next_it = static_cast<cyclic_iterator<list_t>::iterator>(
        cyclic_iterator<list_t, true>{reordered_list, current_it} + number);

    auto it_distance = std::distance(current_it, next_it);
    if (it_distance < 0) {
      std::shift_right(next_it, current_it + 1, 1);
    } else {
      std::shift_left(current_it, next_it + 1, 1);
    }
    *next_it = number;
  }

  return reordered_list;
}

int sum_numbers(const list_t& list) {
  auto zero_it = std::ranges::find(list, 0);
  auto cyclic_it = cyclic_iterator{list, zero_it};
  int sum = 0;
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
int sum_list(const list_t& original_list) {
  list_t reordered_list = reorder_list<num_mixings>(original_list);
  return sum_numbers(reordered_list);
}

template <int num_mixings>
int solve_case(const std::string& filename) {
  list_t original_list = readfile_numbers(filename);
  int sum = sum_list<num_mixings>(original_list);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(3, (solve_case<1>("day20.example")));
  AOC_EXPECT_RESULT(1591, solve_case<24>("day20.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(1623178306, solve_case<24>("day20.example"));
  // AOC_EXPECT_RESULT(14579387544492, solve_case<24>("day20.input"));
  AOC_RETURN_CHECK_RESULT();
}
