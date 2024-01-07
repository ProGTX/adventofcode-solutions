// https://adventofcode.com/2021/day/8

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::int64_t;

struct input_line_t {
  using signal_patterns_t = std::array<std::string, 10>;
  using output_t = std::array<std::string, 4>;

  signal_patterns_t signal_patterns;
  output_t output;
};

using input_t = std::vector<input_line_t>;

constexpr inline auto digit_wires = std::invoke([] {
  std::array<std::string_view, 10> wires;
  wires[0] = "abcefg";
  wires[1] = "cf";
  wires[2] = "acdeg";
  wires[3] = "acdfg";
  wires[4] = "bcdf";
  wires[5] = "abdfg";
  wires[6] = "abdefg";
  wires[7] = "acf";
  wires[8] = "abcdefg";
  wires[9] = "abcdfg";
  return wires;
});
// 8 includes all segments
constexpr inline auto original_map = digit_wires[8];

int count_easy(input_t const& input) {
  int count = 0;
  constexpr auto easy_digits = std::array{1, 4, 7, 8};
  for (input_line_t const& line : input) {
    count +=
        std::ranges::count_if(line.output, [&](std::string_view digit_str) {
          return std::ranges::any_of(easy_digits, [&](int digit) {
            return digit_str.size() == digit_wires[digit].size();
          });
        });
  }
  return count;
}

using digit_possibility_t = static_vector<int, digit_wires.size()>;
using digit_combinations_t =
    std::array<digit_possibility_t, digit_wires.size()>;

digit_combinations_t get_digit_combinations(
    input_line_t::signal_patterns_t const& patterns) {
  digit_combinations_t digit_combinations;
  for (int p = 0; p < patterns.size(); ++p) {
    for (int w = 0; w < digit_wires.size(); ++w) {
      if (patterns[p].size() == digit_wires[w].size()) {
        digit_combinations[p].push_back(w);
      }
    }
  }
  return digit_combinations;
}

using wire_set_t = static_vector<char, original_map.size()>;
using segment_set_t = std::array<wire_set_t, original_map.size()>;

constexpr wire_set_t pattern_to_set(std::string_view wire) {
  wire_set_t set;
  std::ranges::transform(wire, std::back_inserter(set),
                         [](char c) { return c; });
  return set;
}

std::string get_mapping(input_line_t::signal_patterns_t const& patterns) {
  digit_combinations_t digit_combinations = get_digit_combinations(patterns);

  constexpr auto max_set_size = original_map.size();
  const auto full_wire_set = std::invoke([&]() {
    wire_set_t wire_set{};
    std::ranges::copy(std::views::iota('a') | std::views::take(max_set_size),
                      std::back_inserter(wire_set));
    return wire_set;
  });

  auto segment_set = segment_set_t{};
  std::ranges::fill(segment_set, full_wire_set);
  wire_set_t invalid_set;
  wire_set_t new_set;
  wire_set_t union_set;
  wire_set_t new_union_set;

  for (int p = 0; p < patterns.size(); ++p) {
    auto combination = digit_combinations[p];

    wire_set_t valid_set = pattern_to_set(patterns[p]);
    // Sorting is required for the set algorithms
    std::ranges::sort(valid_set);

    invalid_set.clear();
    std::ranges::set_difference(full_wire_set, valid_set,
                                std::back_inserter(invalid_set));

    for (int w = 0; w < max_set_size; ++w) {
      union_set.clear();
      for (auto digit : combination) {
        auto digit_wire = digit_wires[digit];
        const auto& intersecting_set =
            ranges::contains(digit_wire, static_cast<char>(w + 'a'))
                ? valid_set
                : invalid_set;

        new_set.clear();
        std::ranges::set_intersection(segment_set[w], intersecting_set,
                                      std::back_inserter(new_set));

        new_union_set.clear();
        std::ranges::set_union(union_set, new_set,
                               std::back_inserter(new_union_set));
        union_set = new_union_set;
      }
      segment_set[w] = union_set;
    }
  }

  auto new_map = std::string{original_map};
  for (int w = 0; w < max_set_size; ++w) {
    if (segment_set[w].size() == 1) {
      for (int w2 = 0; w2 < max_set_size; ++w2) {
        if (w == w2) {
          continue;
        }
        new_set.clear();
        std::ranges::set_difference(segment_set[w2], segment_set[w],
                                    std::back_inserter(new_set));
        segment_set[w2] = new_set;
      }
    }
  }
  for (int w = 0; w < max_set_size; ++w) {
    new_map[w] = segment_set[w][0];
  }
  return new_map;
}

int output_to_number(std::string_view out, std::string_view mapping) {
  wire_set_t wire_set;
  for (char c : out) {
    wire_set.push_back(original_map[mapping.find(c)]);
  }
  std::ranges::sort(wire_set);

  for (int w = 0; w < digit_wires.size(); ++w) {
    if (std::ranges::equal(wire_set, pattern_to_set(digit_wires[w]))) {
      return w;
    }
  }
  std::cout << "Error: Invalid wire set: " << print_range(wire_set)
            << std::endl;
  throw std::runtime_error("Invalid wire set given");
}

int solve_line(input_line_t const& line) {
  auto new_map = get_mapping(line.signal_patterns);

  int multiplier = 1;
  int number = 0;
  for (auto const& output_digit : line.output | std::views::reverse) {
    number += output_to_number(output_digit, new_map) * multiplier;
    multiplier *= 10;
  }
  return number;
}

int_t sum_outputs(input_t const& input) {
  int_t sum = 0;
  for (input_line_t const& line : input) {
    sum += solve_line(line);
  }
  return sum;
}

template <bool deduce>
int_t solve_case(const std::string& filename) {
  input_t input;

  readfile_op(filename, [&](std::string_view line) {
    auto [signal_patterns_str, output_str] =
        split<std::array<std::string_view, 2>>(line, '|');
    input.emplace_back(split<typename input_line_t::signal_patterns_t>(
                           signal_patterns_str, ' '),
                       split<typename input_line_t::output_t>(output_str, ' '));
  });

  int_t count = 0;
  if constexpr (!deduce) {
    count = count_easy(input);
  } else {
    count = sum_outputs(input);
  }
  std::cout << filename << " -> " << count << std::endl;
  return count;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(0, (solve_case<false>("day08.example")));
  AOC_EXPECT_RESULT(26, (solve_case<false>("day08.example2")));
  AOC_EXPECT_RESULT(440, (solve_case<false>("day08.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(5353, (solve_case<true>("day08.example")));
  AOC_EXPECT_RESULT(61229, (solve_case<true>("day08.example2")));
  AOC_EXPECT_RESULT(1046281, (solve_case<true>("day08.input")));
  AOC_RETURN_CHECK_RESULT();
}
