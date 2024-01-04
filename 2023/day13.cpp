// https://adventofcode.com/2023/day/13

#include "../common/common.h"

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

constexpr bool check_index(std::string_view line, int index) {
  auto sub_size =
      std::ranges::min(index, static_cast<int>(line.size()) - index);
  auto lhs = line.substr(index - sub_size, sub_size);
  auto rhs = line.substr(index, sub_size);
  return std::ranges::equal(lhs | std::views::reverse, rhs);
}

template <class return_t = std::vector<int>>
constexpr return_t reflection_points(std::string_view line) {
  AOC_ASSERT(
      line.size() > 1,
      "Cannot find reflection point on an empty line or single character");
  const auto check_index = [&line](int index) {
    auto sub_size =
        std::ranges::min(index, static_cast<int>(line.size()) - index);
    auto lhs = line.substr(index - sub_size, sub_size);
    auto rhs = line.substr(index, sub_size);
    return std::ranges::equal(lhs | std::views::reverse, rhs);
  };
  return_t refl_points;
  if constexpr (is_specialization_of_v<return_t, std::vector>) {
    refl_points.resize(line.size());
  }
  std::ranges::fill(refl_points, 0);
  for (int index = 1; index < line.size(); ++index) {
    if (check_index(index)) {
      ++refl_points[index];
    }
  }
  return refl_points;
}

static_assert(std::array{0, 0, 0, 0, 0, 1, 0, 1, 0} ==
              reflection_points<std::array<int, 9>>("#.##..##."));
static_assert(std::array{0, 1, 0, 0, 0, 1, 0, 0, 0} ==
              reflection_points<std::array<int, 9>>("..#.##.#."));
static_assert(std::array{0, 1, 0, 0, 0, 1, 0, 0, 0} ==
              reflection_points<std::array<int, 9>>("##......#"));
static_assert(std::array{0, 1, 0, 1, 0, 1, 0, 1, 0} ==
              reflection_points<std::array<int, 9>>("..##..##."));
static_assert(std::array{0, 0, 0, 0, 0, 1, 0, 0, 0} ==
              reflection_points<std::array<int, 9>>("#.#.##.#."));
static_assert(std::array{0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1} ==
              reflection_points<std::array<int, 13>>("#.#..##..#.##"));

using pattern_t = std::vector<std::string>;

constexpr int reflection_point(const pattern_t& pattern) {
  AOC_ASSERT(pattern.size() > 0, "Cannot work without lines");
  const auto num_pos = static_cast<int>(pattern[0].size());
  AOC_ASSERT(num_pos > 0, "Cannot work with empty lines");

  std::vector<int> refl_points;
  refl_points.resize(num_pos);
  for (auto line : pattern) {
    auto local_refl_points = reflection_points(line);
    for (int i = 0; i < num_pos; ++i) {
      refl_points[i] += local_refl_points[i];
    }
  }

  std::vector<std::pair<int, int>> refl_map;
  refl_map.reserve(num_pos);
  for (int index = 0; index < num_pos; ++index) {
    refl_map.emplace_back(index, refl_points[index]);
  }
  std::ranges::sort(refl_map, std::less<>{}, &std::pair<int, int>::second);
  if (refl_map.back().second < pattern.size()) {
    return -1;
  }
  return refl_map.back().first;
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
static_assert(-1 == reflection_point(pattern_t{
                        "#...##..#",
                        "#....#..#",
                        "..##..###",
                        "#####.##.",
                        "#####.##.",
                        "..##..###",
                        "#....#..#",
                    }));
static_assert(4 == reflection_point(transpose(pattern_t{
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

template <int>
int solve_case(const std::string& filename) {
  pattern_t pattern;
  pattern_t transposed_pattern;

  const auto check_pattern = [&]() {
    auto p = reflection_point(pattern);
    if (p < 0) {
      // Find reflection point on transposed pattern
      transposed_pattern = transpose(pattern);
      p = reflection_point(transposed_pattern);
      if (p < 0) {
        throw std::runtime_error("Invalid value of p");
      }
      p *= 100;
    }

    pattern.clear();
    transposed_pattern.clear();

    return p;
  };

  auto sum = 0;
  auto read_values = [&](std::string&& line) {
    if (transposed_pattern.empty()) {
      transposed_pattern.resize(line.size());
    }
    if (line.empty()) {
      sum += check_pattern();
    } else {
      pattern.push_back(std::move(line));
    }
  };
  readfile_op(filename, read_values);
  sum += check_pattern();

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(405, (solve_case<1>("day13.example")));
  AOC_EXPECT_RESULT(31265, (solve_case<1>("day13.input")));
  //  std::cout << "Part 2" << std::endl;
  //  AOC_EXPECT_RESULT(525152, (solve_case<5>("day13.example")));
  //   AOC_EXPECT_RESULT(525152, (solve_case<5>("day13.input")));
  AOC_RETURN_CHECK_RESULT();
}
