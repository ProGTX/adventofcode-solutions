// https://adventofcode.com/2023/day/13

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <bit>
#include <print>
#include <ranges>

using pattern_t = Vec<String>;
using num_pattern_t = Vec<u32>;
using Input = Vec<pattern_t>;

auto parse(String const& filename) -> Input {
  auto result = Input{};
  auto pattern = pattern_t{};
  for (String& line : aoc::views::read_lines(filename, aoc::keep_empty{})) {
    if (line.empty()) {
      result.push_back(std::move(pattern));
      pattern.clear();
    } else {
      pattern.push_back(std::move(line));
    }
  }
  result.push_back(std::move(pattern));
  return result;
}

constexpr inline auto ash = '.';
constexpr inline auto rock = '#';

// We convert a line to a number by assuming a binary representation,
// where ash is 0 and rock is 1
// But we always add an extra 1 bit at the end to ensure a fixed end bit
fn transform_to_number(str line) -> u32 {
  auto num = aoc::binary_to_number<rock>(line);
  num += (1 << line.size());
  return num;
}

fn transformed_number_size(u32 number) -> u32 {
  // Important to reduce by one because of the extra guard bit
  return std::bit_width(number) - 1;
}

fn check_index(u32 number, u32 index) -> bool {
  if (index == 0) {
    return true;
  }
  let number_size = transformed_number_size(number);
  let sub_size = stdr::min(index, number_size - index);
  auto mask_lhs = 1 << (index - 1);
  auto mask_rhs = mask_lhs << 1;
  for (let _ : Range{u32{}, sub_size}) {
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

template <class return_t = Vec<i32>>
fn reflection_points(u32 number) -> return_t {
  let number_size = transformed_number_size(number);
  AOC_ASSERT(
      number_size > 1,
      "Cannot find reflection point on an empty line or single character");
  auto refl_points = return_t{};
  if constexpr (aoc::is_specialization_of_v<return_t, std::vector>) {
    refl_points.resize(number_size);
  }
  stdr::fill(refl_points, 0);
  for (let index : Range{u32{1}, number_size}) {
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

fn reflection_counts(const num_pattern_t& pattern) -> num_pattern_t {
  let number_size = transformed_number_size(pattern[0]);
  auto refl_points = num_pattern_t{};
  refl_points.resize(number_size);
  for (auto number : pattern) {
    auto local_refl_points = reflection_points(number);
    for (let i : Range{u32{}, number_size}) {
      refl_points[i] += local_refl_points[i];
    }
  }
  return refl_points;
}

fn reflection_point(const num_pattern_t& pattern) -> i32 {
  AOC_ASSERT(pattern.size() > 0, "Cannot work without lines");
  let number_size = transformed_number_size(pattern[0]);
  AOC_ASSERT(number_size > 0, "Cannot work with empty lines");

  let refl_points = reflection_counts(pattern);

  auto refl_map = Vec<std::pair<u32, u32>>{};
  refl_map.reserve(number_size);
  for (let index : Range{u32{}, number_size}) {
    refl_map.emplace_back(index, refl_points[index]);
  }
  stdr::sort(refl_map, std::greater<>{}, &std::pair<u32, u32>::second);
  if (refl_map[0].second < pattern.size()) {
    return 0;
  }
  // In some cases two matches can be found
  // In that case select the one that's closer to the center
  if (refl_map[0].second == refl_map[1].second) {
    let center = number_size / 2;
    let dist_lhs = aoc::abs(static_cast<i32>(refl_map[0].first) -
                            static_cast<i32>(center));
    let dist_rhs = aoc::abs(static_cast<i32>(refl_map[1].first) -
                            static_cast<i32>(center));
    return (dist_lhs < dist_rhs) ? refl_map[0].first : refl_map[1].first;
  }
  return refl_map[0].first;
}

fn to_numbers(const pattern_t& pattern) -> num_pattern_t {
  auto numbers = num_pattern_t{};
  numbers.reserve(pattern.size());
  stdr::copy(pattern | stdv::transform(transform_to_number),
             std::back_inserter(numbers));
  return numbers;
}

fn reflection_point(const pattern_t& pattern) -> i32 {
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

// Unlike reflection_point, this doesn't apply
// the "closer to center" tie-break:
// for unsmudging we need to know about *any* reflection point
// other than ignore_p, even if the original one is still also a full match
// and would otherwise win the tie-break.
fn find_new_reflection_point(const num_pattern_t& pattern, i32 ignore_p)
    -> i32 {
  let refl_points = reflection_counts(pattern);
  for (let index : Range{1uz, refl_points.size()}) {
    if ((refl_points[index] == pattern.size()) &&
        (static_cast<i32>(index) != ignore_p)) {
      return static_cast<i32>(index);
    }
  }
  return 0;
}

fn unsmudge_case(aoc::contains_uncvref<num_pattern_t> auto&& pattern,
                 const i32 ignore_p) -> i32 {
  AOC_ASSERT(pattern.size() > 0, "Cannot work without lines");
  let number_size = transformed_number_size(pattern[0]);
  AOC_ASSERT(number_size > 0, "Cannot work with empty lines");
  for (auto&& [row, number] : pattern | stdv::enumerate) {
    for (let column : Range{u32{}, number_size}) {
      number = aoc::flip_bit(number, column);
      auto p = find_new_reflection_point(pattern, ignore_p);
      number = aoc::flip_bit(number, column);
      if (p > 0) {
        return p;
      }
    }
  }
  return 0;
}

template <bool unsmudge>
fn check_pattern(const pattern_t& pattern) -> i32 {
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
}

template <bool unsmudge>
fn solve_case(Input const& input) -> i32 {
  return aoc::ranges::accumulate(
      input | stdv::transform(check_pattern<unsmudge>), i32{});
}

int main() {
  std::println("Part 1");
  let example = parse("day13.example");
  AOC_EXPECT_RESULT(405, (solve_case<false>(example)));
  let example2 = parse("day13.example2");
  AOC_EXPECT_RESULT(712, (solve_case<false>(example2)));
  let input = parse("day13.input");
  AOC_EXPECT_RESULT(31265, (solve_case<false>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(400, (solve_case<true>(example)));
  AOC_EXPECT_RESULT(1415, (solve_case<true>(example2)));
  AOC_EXPECT_RESULT(39359, (solve_case<true>(input)));

  AOC_RETURN_CHECK_RESULT();
}
