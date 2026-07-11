// https://adventofcode.com/2024/day/1

#include "../common/common.h"
#include "../common/rust.h"

#include <print>
#include <ranges>

struct Input {
  Vec<i32> left;
  Vec<i32> right;
};

auto parse(String const& filename) -> Input {
  auto input = Input{};
  for (str line : aoc::views::read_lines(filename)) {
    let parsed = aoc::split_once<i32>(line, "   ");
    input.left.push_back(parsed[0]);
    input.right.push_back(parsed[1]);
  }
  return input;
}

fn sum_diffs(Vec<i32> const& left_list, Vec<i32> const& right_list) -> i32 {
  AOC_ASSERT(left_list.size() == right_list.size(), "Lists must be same size");
  let left = aoc::ranges::sorted(left_list);
  let right = aoc::ranges::sorted(right_list);

  return aoc::ranges::accumulate(
      stdv::zip_transform([](i32 l, i32 r) { return aoc::abs(r - l); }, //
                          left, right),
      0);
}

static_assert(sum_diffs({2}, {7}) == 5);
static_assert(sum_diffs({3, 4, 2, 1, 3, 3}, {4, 3, 5, 3, 9, 3}) == 11);

fn similarity_score(Vec<i32> const& left_list, Vec<i32> const& right_list)
    -> i32 {
  auto right_counts = aoc::flat_map<i32, i32>{};
  for (auto value : right_list) {
    ++right_counts[value];
  }

  return aoc::ranges::accumulate(
      left_list | stdv::transform([&](i32 value) {
        let it = right_counts.find(value);
        return value * (it == right_counts.end() ? 0 : it->second);
      }),
      0);
}

static_assert(similarity_score({3}, {4, 3, 5, 3, 9, 3}) == 9);
static_assert(similarity_score({3, 3}, {4, 3, 5, 3, 9, 3}) == 18);
static_assert(similarity_score({3, 4}, {4, 3, 5, 3, 9, 3}) == 13);
static_assert(similarity_score({3, 4, 3}, {4, 3, 5, 3, 9, 3}) == 22);
static_assert(similarity_score({3, 4, 2}, {4, 3, 5, 3, 9, 3}) == 13);
static_assert(similarity_score({3, 4, 2, 1}, {4, 3, 5, 3, 9, 3}) == 13);
static_assert(similarity_score({3, 4, 2, 1, 3}, {4, 3, 5, 3, 9, 3}) == 22);
static_assert(similarity_score({3, 4, 2, 1, 3, 3}, {4, 3, 5, 3, 9, 3}) == 31);

int main() {
  std::println("Part 1");
  let example = parse("day01.example");
  AOC_EXPECT_RESULT(11, sum_diffs(example.left, example.right));
  let input = parse("day01.input");
  AOC_EXPECT_RESULT(3569916, sum_diffs(input.left, input.right));

  std::println("Part 2");
  AOC_EXPECT_RESULT(31, similarity_score(example.left, example.right));
  AOC_EXPECT_RESULT(26407426, similarity_score(input.left, input.right));

  AOC_RETURN_CHECK_RESULT();
}
