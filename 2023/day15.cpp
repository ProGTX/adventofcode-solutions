// https://adventofcode.com/2023/day/15

#include "../common/common.h"

#include <array>
#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

using namespace std::string_view_literals;

// Determine the ASCII code for the current character of the string.
// Increase the current value by the ASCII code you just determined.
// Set the current value to itself multiplied by 17.
// Set the current value to the remainder of dividing itself by 256.
constexpr int hash_alg(std::string_view str) {
  return aoc::ranges::fold_left(str | aoc::views::transform_cast<int>(), 0,
                                [](int current_value, int ascii) {
                                  current_value += ascii;
                                  current_value *= 17;
                                  current_value %= 256;
                                  return current_value;
                                });
}
static_assert(hash_alg("HASH") == 52);

using steps_t = std::vector<std::string>;

constexpr int sum_steps(const steps_t& steps) {
  return aoc::ranges::accumulate(steps | std::views::transform(hash_alg), 0);
}

constexpr steps_t test_case() {
  return {
      "rn=1", "cm-",  "qp=3", "cm=2", "qp-",  "pc=4",
      "ot=9", "ab=5", "pc-",  "pc=6", "ot=7",
  };
}
static_assert(1320 == sum_steps(test_case()));

using lens_t = std::pair<std::string, int>;

lens_t split_label(std::string_view str) {
  if (str.back() == '-') {
    return {std::string(str.substr(0, str.size() - 1)), -1};
  }
  auto [name, number] = aoc::split<std::array<std::string_view, 2>>(str, "="sv);
  return {std::string(name), aoc::to_number<int>(number)};
}

using instructions_t = std::vector<lens_t>;
using box_t = std::vector<lens_t>;
using boxes_t = std::array<box_t, 256>;

constexpr boxes_t sort_boxes(const instructions_t& instructions) {
  boxes_t boxes{};

  for (auto&& [name, number] : instructions) {
    auto& box = boxes[hash_alg(name)];
    auto it = std::ranges::find(box, name, &lens_t::first);
    if (it != std::end(box)) {
      if (number < 0) {
        box.erase(it);
      } else {
        *it = {name, number};
      }
    } else if (number >= 0) {
      box.emplace_back(name, number);
    } else {
      // Trying to remove from an empty box, do nothing
    }
  }

  return boxes;
}

constexpr int focusing_power(const box_t& box) {
  int multiplier = 1;
  return aoc::ranges::fold_left(box, 0, [&](int power, const lens_t& lens) {
    power += multiplier * lens.second;
    ++multiplier;
    return power;
  });
}

constexpr int sum_boxes(const boxes_t& boxes) {
  int multiplier = 1;
  return aoc::ranges::fold_left(boxes, 0, [&](int sum, const box_t& box) {
    sum += multiplier * focusing_power(box);
    ++multiplier;
    return sum;
  });
}

template <bool lens_sort>
int solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  steps_t steps;
  instructions_t instructions;

  for (std::string_view line : aoc::views::read_lines(filename)) {
    steps = aoc::split<steps_t>(line, ',');
    if constexpr (lens_sort) {
      for (const auto& step : steps) {
        instructions.push_back(split_label(step));
      }
    }
  }

  int sum = 0;
  if constexpr (!lens_sort) {
    sum = sum_steps(steps);
  } else {
    sum = sum_boxes(sort_boxes(instructions));
  }
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(1320, (solve_case<false>("day15.example")));
  AOC_EXPECT_RESULT(510273, (solve_case<false>("day15.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(145, (solve_case<true>("day15.example")));
  AOC_EXPECT_RESULT(212449, (solve_case<true>("day15.input")));
  AOC_RETURN_CHECK_RESULT();
}
