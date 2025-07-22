// https://adventofcode.com/2022/day/13

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <set>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

struct packet_tree : public aoc::graph<int, packet_tree, std::string_view> {
 private:
  using base_t = aoc::graph<int, packet_tree, std::string_view>;

 protected:
  constexpr packet_tree(packet_tree* parent, int value, bool is_leaf)
      : base_t{parent, "", value, is_leaf} {}

 public:
  constexpr packet_tree() : base_t{nullptr, "", 0, false} {}

  packet_tree* add_value(int value) {
    return this->add_child(child_ptr_t{new packet_tree{this, value, true}});
  }

  packet_tree* add_list() {
    return this->add_child(child_ptr_t{new packet_tree{this, 0, false}});
  }

  static packet_tree list_from_value(int value) {
    packet_tree packet;
    packet.add_value(value);
    return packet;
  }
};

using packet_node_t = packet_tree::child_ptr_t;

enum ordering {
  incorrect,
  correct,
  inconclusive,
};

ordering in_right_order(packet_tree* first, packet_tree* second) {
  packet_tree tmp_list;
  packet_tree* first_child_list;
  packet_tree* second_child_list;
  for (auto lhs_it = first->begin(), rhs_it = second->begin();;
       ++lhs_it, ++rhs_it) {
    if (lhs_it == first->end()) {
      if (rhs_it != second->end()) {
        // If the left list runs out of items first,
        // the inputs are in the right order
        return correct;
      } else {
        return inconclusive;
      }
    }
    if (rhs_it == second->end()) {
      // If the right list runs out of items first,
      // the inputs are not in the right order.
      return incorrect;
    }

    first_child_list = lhs_it->get();
    second_child_list = rhs_it->get();
    const auto& lhs = *first_child_list;
    const auto& rhs = *second_child_list;

    // Check integers
    if (lhs.is_leaf() && rhs.is_leaf()) {
      if (lhs.value() < rhs.value()) {
        return correct;
      } else if (lhs.value() > rhs.value()) {
        return incorrect;
      } else {
        // Continue checking
        continue;
      }
    }

    // If exactly one value is an integer, convert the integer to a list
    // which contains that integer as its only value, then retry the comparison.
    if (lhs.is_leaf() && !rhs.is_leaf()) {
      tmp_list = packet_tree::list_from_value(lhs.value());
      first_child_list = &tmp_list;
    } else if (!lhs.is_leaf() && rhs.is_leaf()) {
      tmp_list = packet_tree::list_from_value(rhs.value());
      second_child_list = &tmp_list;
    }

    auto list_in_right_order =
        in_right_order(first_child_list, second_child_list);
    if (list_in_right_order != inconclusive) {
      return list_in_right_order;
    }
  }
  return inconclusive;
}

template <bool sort_all>
int solve_case(const std::string& filename) {
  using packet_pair_t = std::array<packet_node_t, 2>;

  using packets_t = std::conditional_t<
      !sort_all, std::vector<packet_pair_t>,
      std::set<packet_node_t,
               decltype([](const packet_node_t& lhs, const packet_node_t& rhs) {
                 return (in_right_order(lhs.get(), rhs.get()) == correct);
               })>>;

  packet_pair_t packet_pair;
  packets_t packets;
  std::stack<packet_tree*, std::vector<packet_tree*>> packet_stack;

  std::string current_number_str;
  const auto add_number_if_not_empty = [&]() {
    if (!current_number_str.empty()) {
      packet_stack.top()->add_value(aoc::to_number<int>(current_number_str));
      current_number_str.clear();
    }
  };

  const auto parse_list = [&](std::string_view line) -> packet_node_t {
    auto packet = packet_node_t(new packet_tree{});
    packet_stack.push(packet.get());
    // Ignore first bracket, handled by the creation above
    for (auto current_char : line | std::views::drop(1)) {
      if (current_char == '[') {
        packet_stack.push(packet_stack.top()->add_list());
      } else if (current_char == ']') {
        add_number_if_not_empty();
        packet_stack.pop();
      } else if (current_char == ',') {
        add_number_if_not_empty();
      } else {
        current_number_str.push_back(current_char);
      }
    }
    return packet;
  };

  int index = 0;
  for (std::string line : aoc::views::read_lines(filename)) {
    packet_pair[index] = parse_list(line);
    index = (index + 1) % 2;
    if (index == 0) {
      if constexpr (!sort_all) {
        packets.push_back(std::move(packet_pair));
      } else {
        packets.insert(std::move(packet_pair[0]));
        packets.insert(std::move(packet_pair[1]));
      }
    }
  }

  int score = 0;

  if constexpr (!sort_all) {
    for (int index = 1; const auto& packet_pair : packets) {
      auto result = in_right_order(packet_pair[0].get(), packet_pair[1].get());
      if (result == correct) {
        score += index;
      }
      ++index;
    }
  } else {
    auto divider1_str = "[[2]]";
    auto divider2_str = "[[6]]";
    packets.insert(parse_list(divider1_str));
    packets.insert(parse_list(divider2_str));
    auto divider1 = packets.find(parse_list(divider1_str));
    auto divider2 = packets.find(parse_list(divider2_str));
    auto begin = std::begin(packets);
    score = (std::distance(begin, divider1) + 1) *
            (std::distance(begin, divider2) + 1);
  }

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(13, solve_case<false>("day13.example"));
  AOC_EXPECT_RESULT(5198, solve_case<false>("day13.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(140, solve_case<true>("day13.example"));
  AOC_EXPECT_RESULT(22344, solve_case<true>("day13.input"));
  AOC_RETURN_CHECK_RESULT();
}
