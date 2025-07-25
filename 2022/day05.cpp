// https://adventofcode.com/2022/day/5

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <ranges>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

template <bool grab_multiple>
std::string solve_case(const std::string& filename) {
  // There can't be more than 9 crates because that would break the parsing
  // Initially we don't know how many stacks we have,
  // and the parsing gets the crates in reverse order
  std::array<std::string, 9> crates;

  // Actual number of stacks
  int num_stacks = 0;

  // Don't trim spaces, we need them in this example
  for (std::string_view line :
       aoc::views::read_lines(filename, aoc::keep_spaces{})) {
    if (line.at(0) != 'm') {
      // First parse the crates
      if (line.at(1) != '1') {
        // Parsing crates
        for (int i = 0, pos = 1; pos < line.size(); pos += 4, ++i) {
          auto crate = line.at(pos);
          if (crate == ' ') {
            continue;
          }
          crates[i].push_back(crate);
        }
      } else {
        // Done parsing crate stacks, put them in correct order
        auto stack_numbers =
            aoc::split<std::vector<std::string_view>>(line, ' ');
        // -2 because there are multiple spaces between numbers,
        // and there's always a space after the last number
        num_stacks =
            aoc::to_number<int>(stack_numbers[stack_numbers.size() - 2].at(0));
        for (int i = 0; i < num_stacks; ++i) {
          std::ranges::reverse(crates[i]);
        }
      }
      continue;
    }

    // Start moving crates

    // move N from A to B
    auto [move_str, n_str, from_str, from_pos_str, to_str, to_pos_str] =
        aoc::split<std::array<std::string_view, 6>>(line, ' ');

    auto move_num_crates = aoc::to_number<int>(n_str);

    // Adjust the indexing by 1
    auto& from_crate = crates[aoc::to_number<int>(from_pos_str) - 1];
    auto& to_crate = crates[aoc::to_number<int>(to_pos_str) - 1];

    if constexpr (!grab_multiple) {
      // Part 1
      for (int i = 0; i < move_num_crates; ++i) {
        auto crate = from_crate.back();
        to_crate.push_back(crate);
        from_crate.resize(from_crate.size() - 1);
      }
    } else {
      // Part 2
      auto from_crate_new_size = from_crate.size() - move_num_crates;
      std::string_view crate_bunch{from_crate.data() + from_crate_new_size};
      to_crate += crate_bunch;
      from_crate.resize(from_crate_new_size);
    }
  }

  std::cout << "Done parsing " << std::endl;
  std::string top_stacks(static_cast<size_t>(num_stacks), ' ');

  for (int pos = 0; const auto& stack : crates) {
    if (!stack.empty()) {
      top_stacks[pos] = stack.back();
      ++pos;
    }
  }

  std::cout << filename << " -> " << top_stacks << std::endl;
  return top_stacks;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT("CMZ", solve_case<false>("day05.example"));
  AOC_EXPECT_RESULT("QGTHFZBHV", solve_case<false>("day05.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT("MCD", solve_case<true>("day05.example"));
  AOC_EXPECT_RESULT("MGDMPSZTM", solve_case<true>("day05.input"));
  AOC_RETURN_CHECK_RESULT();
}
