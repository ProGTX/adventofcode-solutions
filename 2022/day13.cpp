// https://adventofcode.com/2022/day/13

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

#include "../common.h"

struct packet_tree : public btree<int, packet_tree> {
 private:
  using base_t = btree<int, packet_tree>;

 protected:
  constexpr packet_tree(packet_tree* parent, int value, bool is_leaf)
      : base_t{parent, value, is_leaf} {}

 public:
  constexpr packet_tree() : base_t{nullptr, 0, false} {}

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
      if (lhs.get_value() < rhs.get_value()) {
        return correct;
      } else if (lhs.get_value() > rhs.get_value()) {
        return incorrect;
      } else {
        // Continue checking
        continue;
      }
    }

    // If exactly one value is an integer, convert the integer to a list
    // which contains that integer as its only value, then retry the comparison.
    if (lhs.is_leaf() && !rhs.is_leaf()) {
      tmp_list = packet_tree::list_from_value(lhs.get_value());
      first_child_list = &tmp_list;
    } else if (!lhs.is_leaf() && rhs.is_leaf()) {
      tmp_list = packet_tree::list_from_value(rhs.get_value());
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

template <bool>
void solve_case(const std::string& filename) {
  using packet_pair_t = std::array<packet_node_t, 2>;

  packet_pair_t packet_pair;
  std::vector<packet_pair_t> packets;
  std::stack<packet_tree*, std::vector<packet_tree*>> packet_stack;

  std::string current_number_str;
  const auto add_number_if_not_empty = [&]() {
    if (!current_number_str.empty()) {
      packet_stack.top()->add_value(std::stoi(current_number_str));
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
  readfile_op(filename, [&](std::string_view line) {
    if (line.empty()) {
      return;
    }
    packet_pair[index] = parse_list(line);
    index = (index + 1) % 2;
    if (index == 0) {
      packets.push_back(std::move(packet_pair));
    }
  });

  int sum = 0;

  for (int index = 1; const auto& packet_pair : packets) {
    auto result = in_right_order(packet_pair[0].get(), packet_pair[1].get());
    if (result == correct) {
      sum += index;
    }
    ++index;
  }

  std::cout << filename << " -> " << sum << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<false>("day13.example");
  solve_case<false>("day13.input");
  // std::cout << "Part 2" << std::endl;
  // solve_case<true>("day13.example");
  // solve_case<true>("day13.input");
}
