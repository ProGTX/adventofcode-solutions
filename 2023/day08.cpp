// https://adventofcode.com/2023/day/8

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <compare>
#include <functional>
#include <iostream>
#include <map>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

using int_t = std::int64_t;

using node_t = std::pair<int, int>;
using node_select_func_t = decltype(&node_t::first);

constexpr node_select_func_t get_direction(char c) {
  if (c == 'L') {
    return &node_t::first;
  } else {
    return &node_t::second;
  }
}

static_assert(std::invoke(get_direction('R'), node_t{5, 6}) == 6);
static_assert(std::invoke(get_direction('L'), node_t{5, 6}) == 5);

constexpr int_t num_steps(const std::vector<node_t>& directions,
                          const std::vector<node_select_func_t>& instructions,
                          int start_index,
                          const std::vector<int>& end_indexes) {
  auto next_index = start_index;
  int_t steps = 0;
  int inst_index = 0;
  while (true) {
    if (ranges::contains(end_indexes, start_index)) {
      break;
    }
    auto inst = instructions[inst_index];
    next_index = std::invoke(inst, directions[start_index]);
    std::swap(next_index, start_index);
    inst_index = (inst_index + 1) % instructions.size();
    ++steps;
  }
  return steps;
}

static_assert(6 ==
              num_steps(std::vector{node_t{1, 1}, node_t{0, 2}, node_t{2, 2}},
                        std::vector{get_direction('L'), get_direction('L'),
                                    get_direction('R')},
                        0, {2}));

constexpr int_t num_steps(const std::vector<node_t>& directions,
                          const std::vector<node_select_func_t>& instructions,
                          const std::vector<int>& start_indexes,
                          const std::vector<int>& end_indexes) {
  return ranges::lcm(
      start_indexes | std::views::transform([&](int start_index) {
        return num_steps(directions, instructions, start_index, end_indexes);
      }));
}

template <bool all_paths>
int_t solve_case(const std::string& filename) {
  std::vector<node_select_func_t> instructions;
  auto read_instructions = [&](std::string_view line) {
    std::ranges::transform(line, std::back_inserter(instructions),
                           &get_direction);
  };

  std::map<std::string, int> name_to_index;
  int current_index = 0;
  int max_index = 0;
  std::vector<int> start_indexes;
  std::vector<int> end_indexes;
  std::vector<node_t> directions;

  auto add_name_index = [&](const std::string& name) {
    auto [it, inserted] = name_to_index.emplace(name, current_index);
    if (inserted) {
      ++current_index;
    }
    return it;
  };

  auto nodes_match = []() {
    using node_it_t = typename decltype(name_to_index)::iterator;
    if constexpr (!all_paths) {
      return [](node_it_t node_it, const std::string& match) {
        return node_it->first == match;
      };
    } else {
      return [](node_it_t node_it, const std::string& match) {
        return node_it->first[2] == match[2];
      };
    }
  }();

  auto read_values = [&](std::string_view line) {
    if (line.empty()) {
      return;
    }

    // node = (left, right)
    auto [node, lr] = split<std::array<std::string, 2>>(line, '=', trimmer<>{});
    lr = trim(lr, " ())");
    auto [left, right] =
        split<std::array<std::string, 2>>(lr, ',', trimmer<>{});

    auto node_it = add_name_index(node);
    auto left_it = add_name_index(left);
    auto right_it = add_name_index(right);
    if (current_index > max_index) {
      directions.resize(current_index + 1);
      max_index = current_index;
    }

    if (nodes_match(node_it, "AAA")) {
      start_indexes.push_back(node_it->second);
    } else if (nodes_match(node_it, "ZZZ")) {
      end_indexes.push_back(node_it->second);
    }
    directions[node_it->second] = node_t{left_it->second, right_it->second};
  };

  readfile_op_header(filename, read_instructions, read_values);

  int_t sum = num_steps(directions, instructions, start_indexes, end_indexes);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(2, (solve_case<false>("day08.example")));
  AOC_EXPECT_RESULT(6, (solve_case<false>("day08.example2")));
  AOC_EXPECT_RESULT(12737, (solve_case<false>("day08.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(2, (solve_case<true>("day08.example")));
  AOC_EXPECT_RESULT(6, (solve_case<true>("day08.example2")));
  AOC_EXPECT_RESULT(6, (solve_case<true>("day08.example3")));
  AOC_EXPECT_RESULT(9064949303801, (solve_case<true>("day08.input")));
  AOC_RETURN_CHECK_RESULT();
}
