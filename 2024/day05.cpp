// https://adventofcode.com/2024/day/5

#include "../common/common.h"

#include <array>
#include <iostream>
#include <string>
#include <string_view>

// The input (pages) is limited to numbers below 100
// For each page we store a list of pages that must come before it and after it
using relation_list_t = aoc::static_vector<int, 100>;
struct ordering_rule_t {
  relation_list_t before;
  relation_list_t after;
};

// The input is limited to updates of at most 70 pages
using update_t = aoc::static_vector<int, 70>;

constexpr int index_of_correct_update(const std::vector<ordering_rule_t>& rules,
                                      std::span<const int> update) {
  AOC_ASSERT(rules.size() >= 100,
             "Vector of rules used as a hashmap, "
             "so it needs to have enough space to store all pages");
  for (int u = 0; u < update.size(); ++u) {
    const int page = update[u];
    // Check that pages before are not in the rules list of pages after
    for (int ub = 0; ub < u; ++ub) {
      if (aoc::ranges::contains(rules[page].after, update[ub])) {
        return -1;
      }
    }
    // Check that pages after are not in the rules list of pages before
    for (int ua = u + 1; ua < update.size(); ++ua) {
      if (aoc::ranges::contains(rules[page].before, update[ua])) {
        return -1;
      }
    }
  }
  return (update.size() / 2);
}

template <bool>
int solve_case(const std::string& filename) {
  int sum = 0;

  // We use the vector as a hashmap, so we need to use enough memory
  // for all the pages
  std::vector<ordering_rule_t> rules;
  rules.resize(100);

  bool parsing_rules = true;
  for (std::string_view line :
       aoc::views::read_lines(filename, aoc::keep_empty{})) {
    if (line.empty()) {
      parsing_rules = false;
      continue;
    }
    if (parsing_rules) {
      auto [before, after] = aoc::split<std::array<int, 2>>(line, '|');
      rules[before].after.push_back(after);
      rules[after].before.push_back(before);
    } else {
      auto update = aoc::split<update_t>(line, ',');
      auto index = index_of_correct_update(rules, update);
      if (index > 0) {
        sum += update[index];
      }
    }
  }

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(61, solve_case<false>("day05.example2"));
  AOC_EXPECT_RESULT(0, solve_case<false>("day05.example3"));
  AOC_EXPECT_RESULT(143, solve_case<false>("day05.example"));
  AOC_EXPECT_RESULT(6034, solve_case<false>("day05.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day05.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day05.input"));
  AOC_RETURN_CHECK_RESULT();
}
