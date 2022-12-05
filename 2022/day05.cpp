// https://adventofcode.com/2022/day/5

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

#include "../common.h"

using stack = std::stack<char, std::vector<char>>;

void solve_part1(const std::string& filename) {
  // There can't be more than 9 crates because that would break the parsing
  // Initially we don't know how many stacks we have,
  // and the parsing gets the crates in reverse order
  std::array<std::vector<char>, 9> reverse_crates;
  std::vector<stack> crates;

  int num_stacks = 0;

  const auto trimmer = [](std::string_view str) {
    // Don't trim spaces, we need them in this example
    return trim_leave_spaces(str);
  };

  readfile_op<decltype(trimmer)>(filename, [&](std::string_view line) {
    if (line.empty()) {
      return;
    }
    if (line.at(0) != 'm') {
      // First parse the crates
      if (line.at(1) != '1') {
        // Parsing crates
        for (int i = 0, pos = 1; pos < line.size(); pos += 4, ++i) {
          auto crate = line.at(pos);
          if (crate == ' ') {
            continue;
          }
          reverse_crates[i].push_back(crate);
        }
      } else {
        // Done parsing crate stacks, put them in correct order
        auto stack_numbers =
            split<std::vector<std::string>>(std::string{line}, ' ');
        num_stacks = static_cast<int>(stack_numbers.back().at(0) - '0');
        crates.reserve(num_stacks);
        for (int i = 0; i < num_stacks; ++i) {
          crates.emplace_back();
          for (auto crate : reverse_crates[i] | std::views::reverse) {
            crates[i].push(crate);
          }
        }
      }
      return;
    }

    // Start moving crates

    // move N from A to B
    auto [move_str, n_str, from_str, from_pos_str, to_str, to_pos_str] =
        split<std::array<std::string, 6>>(std::string{line}, ' ');

    auto move_num_crates = std::stoi(n_str);

    // Adjust the indexing by 1
    auto move_from_pos = std::stoi(from_pos_str) - 1;
    auto move_to_pos = std::stoi(to_pos_str) - 1;

    for (int i = 0; i < move_num_crates; ++i) {
      auto crate = crates[move_from_pos].top();
      crates[move_to_pos].push(crate);
      crates[move_from_pos].pop();
    }
  });

  std::cout << "Done parsing " << std::endl;
  std::string top_stacks(static_cast<size_t>(num_stacks), ' ');

  for (int pos = 0; const auto& stack : crates) {
    top_stacks[pos] = stack.top();
    ++pos;
  }

  std::cout << filename << " -> " << top_stacks << std::endl;
}

void solve_part2(const std::string& filename) {
  int score = 0;

  readfile_op(filename, [&](std::string_view line) {});

  std::cout << filename << " -> " << score << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_part1("day05.example");
  solve_part1("day05.input");
  // std::cout << "Part 2" << std::endl;
  // solve_part2("day05.example");
  // solve_part2("day05.input");
}
