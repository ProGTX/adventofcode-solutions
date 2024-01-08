// https://adventofcode.com/2023/day/13

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <bit>
#include <iostream>
#include <ostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

constexpr inline auto ash = '.';
constexpr inline auto rock = '#';

// We convert a line to a number by assuming a binary representation,
// where ash is 0 and rock is 1
// But we always add an extra 1 bit at the end to ensure a fixed end bit
constexpr unsigned transform_to_number(std::string_view line) {
  auto num = aoc::binary_to_number<rock>(line);
  num += (1 << line.size());
  return num;
}

constexpr unsigned transformed_number_size(unsigned number) {
  // Important to reduce by one because of the extra guard bit
  return std::bit_width(number) - 1;
}

constexpr bool check_index(unsigned number, unsigned index) {
  if (index == 0) {
    return true;
  }
  unsigned number_size = transformed_number_size(number);
  auto sub_size = std::ranges::min(index, number_size - index);
  unsigned mask_lhs = 1 << (index - 1);
  unsigned mask_rhs = mask_lhs << 1;
  for (unsigned offset = 0; offset < sub_size; ++offset) {
    if (static_cast<bool>(number & mask_lhs) !=
        static_cast<bool>(number & mask_rhs)) {
      return false;
    }
    mask_lhs = mask_lhs >> 1;
    mask_rhs = mask_rhs << 1;
  }
  return true;
}

static_assert(check_index(transform_to_number("#.##..##."), 0));
static_assert(check_index(transform_to_number("#.##..##."), 5));
static_assert(!check_index(transform_to_number("#.##..##."), 6));
static_assert(check_index(transform_to_number("#.##..##."), 7));
static_assert(check_index(transform_to_number("#.#..##..#.##"), 6));
static_assert(!check_index(transform_to_number("#.#..##..#.##"), 7));
static_assert(check_index(transform_to_number("#.#..##..#.##"), 12));

template <class return_t = std::vector<int>>
constexpr return_t reflection_points(unsigned number) {
  const auto number_size = transformed_number_size(number);
  AOC_ASSERT(
      number_size > 1,
      "Cannot find reflection point on an empty line or single character");
  return_t refl_points;
  if constexpr (aoc::is_specialization_of_v<return_t, std::vector>) {
    refl_points.resize(number_size);
  }
  std::ranges::fill(refl_points, 0);
  for (int index = 1; index < number_size; ++index) {
    if (check_index(number, index)) {
      ++refl_points[index];
    }
  }
  return refl_points;
}

static_assert(
    std::array{0, 0, 0, 0, 0, 1, 0, 1, 0} ==
    reflection_points<std::array<int, 9>>(transform_to_number("#.##..##.")));
static_assert(
    std::array{0, 1, 0, 0, 0, 1, 0, 0, 0} ==
    reflection_points<std::array<int, 9>>(transform_to_number("..#.##.#.")));
static_assert(
    std::array{0, 1, 0, 0, 0, 1, 0, 0, 0} ==
    reflection_points<std::array<int, 9>>(transform_to_number("##......#")));
static_assert(
    std::array{0, 1, 0, 1, 0, 1, 0, 1, 0} ==
    reflection_points<std::array<int, 9>>(transform_to_number("..##..##.")));
static_assert(
    std::array{0, 0, 0, 0, 0, 1, 0, 0, 0} ==
    reflection_points<std::array<int, 9>>(transform_to_number("#.#.##.#.")));
static_assert(std::array{0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1} ==
              reflection_points<std::array<int, 13>>(
                  transform_to_number("#.#..##..#.##")));

using pattern_t = std::vector<std::string>;
using num_pattern_t = std::vector<unsigned>;

constexpr int reflection_point(const num_pattern_t& pattern) {
  AOC_ASSERT(pattern.size() > 0, "Cannot work without lines");
  const auto number_size = transformed_number_size(pattern[0]);
  AOC_ASSERT(number_size > 0, "Cannot work with empty lines");

  num_pattern_t refl_points;
  refl_points.resize(number_size);
  for (auto number : pattern) {
    auto local_refl_points = reflection_points(number);
    for (int i = 0; i < number_size; ++i) {
      refl_points[i] += local_refl_points[i];
    }
  }

  std::vector<std::pair<unsigned, unsigned>> refl_map;
  refl_map.reserve(number_size);
  for (int index = 0; index < number_size; ++index) {
    refl_map.emplace_back(index, refl_points[index]);
  }
  std::ranges::sort(refl_map, std::greater<>{},
                    &std::pair<unsigned, unsigned>::second);
  if (refl_map[0].second < pattern.size()) {
    return 0;
  }
  // In some cases two matches can be found
  // In that case select the one that's closer to the center
  if (refl_map[0].second == refl_map[1].second) {
    const auto center = number_size / 2;
    const auto dist_lhs = aoc::abs(refl_map[0].first - center);
    const auto dist_rhs = aoc::abs(refl_map[1].first - center);
    return (dist_lhs < dist_rhs) ? refl_map[0].first : refl_map[1].first;
  }
  return refl_map[0].first;
}

constexpr num_pattern_t to_numbers(const pattern_t& pattern) {
  num_pattern_t numbers;
  numbers.reserve(pattern.size());
  std::ranges::copy(pattern | std::views::transform(transform_to_number),
                    std::back_inserter(numbers));
  return numbers;
}

constexpr int reflection_point(const pattern_t& pattern) {
  return reflection_point(to_numbers(pattern));
}

#if defined(AOC_COMPILER_MSVC)
static_assert(5 == reflection_point(pattern_t{
                       "#.##..##.",
                       "..#.##.#.",
                       "##......#",
                       "##......#",
                       "..#.##.#.",
                       "..##..##.",
                       "#.#.##.#.",
                   }));
static_assert(0 == reflection_point(pattern_t{
                       "#...##..#",
                       "#....#..#",
                       "..##..###",
                       "#####.##.",
                       "#####.##.",
                       "..##..###",
                       "#....#..#",
                   }));
static_assert(4 == reflection_point(aoc::transpose(pattern_t{
                       "#...##..#",
                       "#....#..#",
                       "..##..###",
                       "#####.##.",
                       "#####.##.",
                       "..##..###",
                       "#....#..#",
                   })));
static_assert(12 == reflection_point(pattern_t{
                        "#.#..##..#.##",
                        ".##.#..#.##..",
                        "##..####..###",
                        ".#..#..#..#..",
                        "..#.#....#...",
                        ".#.#.##.#.#..",
                        "##..####..###",
                    }));
#endif

constexpr int unsmudge_case(aoc::contains_uncvref<num_pattern_t> auto&& pattern,
                            const int ignore_p) {
  const auto pattern_size = pattern.size();
  AOC_ASSERT(pattern.size() > 0, "Cannot work without lines");
  const auto number_size = transformed_number_size(pattern[0]);
  AOC_ASSERT(number_size > 0, "Cannot work with empty lines");
  for (int row = 0; row < pattern_size; ++row) {
    auto& number = pattern[row];
    for (int column = 0; column < number_size; ++column) {
      number = aoc::flip_bit(number, column);
      auto p = reflection_point(pattern);
      number = aoc::flip_bit(number, column);
      if ((p != ignore_p) && (p > 0)) {
        return p;
      }
    }
  }
  return 0;
}

template <bool unsmudge>
constexpr int check_pattern(const pattern_t& pattern) {
  auto pattern_numbers = to_numbers(pattern);
  auto transposed_numbers = to_numbers(aoc::transpose(pattern));

  auto p1 = reflection_point(pattern_numbers);
  auto p1t = 100 * reflection_point(transposed_numbers);

  if constexpr (!unsmudge) {
    return p1 + p1t;
  } else {
    auto p2 = unsmudge_case(pattern_numbers, p1);
    auto p2t = 100 * unsmudge_case(transposed_numbers, p1t / 100);
    return p2 + p2t;
  }
};

template <bool unsmudge>
int solve_case(const std::string& filename) {
  std::cout << filename << std::endl;
  pattern_t pattern;

  int sum = 0;
  auto read_values = [&](std::string&& line) {
    if (line.empty()) {
      sum += check_pattern<unsmudge>(pattern);
      pattern.clear();
    } else {
      pattern.push_back(std::move(line));
    }
  };
  aoc::readfile_op(filename, read_values);
  sum += check_pattern<unsmudge>(pattern);

  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(405, (solve_case<false>("day13.example")));
  AOC_EXPECT_RESULT(712, (solve_case<false>("day13.example2")));
  AOC_EXPECT_RESULT(31265, (solve_case<false>("day13.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(400, (solve_case<true>("day13.example")));
  AOC_EXPECT_RESULT(1415, (solve_case<true>("day13.example2")));
  AOC_EXPECT_RESULT(-36431, (solve_case<true>("day13.input")));
  AOC_RETURN_CHECK_RESULT();
}
