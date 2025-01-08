// https://adventofcode.com/2024/day/22

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::int64_t;
constexpr const int num_repeat = 2000;
constexpr const int pattern_size = 4;
constexpr const int max_diff_abs = 9;
constexpr const int diff_opts = 2 * max_diff_abs + 1;

constexpr int_t mix(int_t lhs, int_t rhs) { return lhs ^ rhs; }
constexpr int prune(int_t secret) {
  return static_cast<int>(secret % 16777216);
}

constexpr int next_secret(int secret) {
  secret = prune(mix(secret, secret * 64));
  secret = prune(mix(secret, secret / 32));
  secret = prune(mix(secret, secret * int_t{2048}));
  return secret;
}

static_assert(15887950 == next_secret(123));
static_assert(16495136 == next_secret(15887950));
static_assert(527345 == next_secret(16495136));
static_assert(704524 == next_secret(527345));
static_assert(1553684 == next_secret(704524));
static_assert(12683156 == next_secret(1553684));
static_assert(11100544 == next_secret(12683156));
static_assert(12249484 == next_secret(11100544));
static_assert(7753432 == next_secret(12249484));
static_assert(5908254 == next_secret(7753432));

constexpr int next_secret_repeat(int secret) {
  for (int i = 0; i < num_repeat; ++i) {
    secret = next_secret(secret);
  }
  return secret;
}

static_assert(8685429 == next_secret_repeat(1));
static_assert(4700978 == next_secret_repeat(10));
static_assert(15273692 == next_secret_repeat(100));
static_assert(8667524 == next_secret_repeat(2024));

constexpr int_t sum_secrets(std::span<const int> secrets) {
  return aoc::ranges::accumulate(
      secrets | std::views::transform(&next_secret_repeat), int_t{0});
}

constexpr int get_pattern(const std::array<int, 4>& diffs) {
  return (diffs[0] + max_diff_abs) * (diff_opts * diff_opts * diff_opts) +
         (diffs[1] + max_diff_abs) * (diff_opts * diff_opts) +
         (diffs[2] + max_diff_abs) * diff_opts + //
         (diffs[3] + max_diff_abs);
}

constexpr auto get_value_total() {
  constexpr const int max_pattern = get_pattern({9, 9, 9, 9});
  std::vector<int> value_total(max_pattern + 1, 0);
  return value_total;
}

// See https://www.reddit.com/r/adventofcode/comments/1hk15et/comment/m3asuqa/
// for inspiration
constexpr int most_bananas(std::span<const int> secrets) {
  aoc::flat_set<int> buyer_has_pattern;
  auto value_total = get_value_total();

  for (int s = 0; s < secrets.size(); ++s) {
    buyer_has_pattern.clear();
    std::array<int, pattern_size> diffs;
    auto secret = secrets[s];
    int previous = secret % 10;
    for (int i = 0; i < num_repeat; ++i) {
      secret = next_secret(secret);
      const int current = secret % 10;
      diffs[pattern_size - 1] = current - previous;
      if (i >= (pattern_size - 1)) {
        const auto pattern = get_pattern(diffs);
        auto it = buyer_has_pattern.find(pattern);
        if (it == std::end(buyer_has_pattern)) {
          buyer_has_pattern.emplace(pattern);
          value_total[pattern] += current;
        }
      }
      aoc::ranges::rotate_left(diffs);
      previous = current;
    }
  }
  return std::ranges::max(value_total);
}

template <bool change_sequence>
int_t solve_case(const std::string& filename) {
  auto buyer_starters = aoc::views::read_numbers<int>(filename) |
                        aoc::ranges::to<std::vector<int>>();

  int_t sum = 0;
  if constexpr (!change_sequence) {
    sum = sum_secrets(buyer_starters);
  } else {
    sum = most_bananas(buyer_starters);
  }
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(37327623, solve_case<false>("day22.example"));
  AOC_EXPECT_RESULT(20215960478, solve_case<false>("day22.input"));
  std::cout << "Part 2" << std::endl;
  // The following two calls can't be static_assert because of compiler limits
  AOC_EXPECT_RESULT(9, most_bananas(std::array{123}));
  AOC_EXPECT_RESULT(23, most_bananas(std::array{1, 2, 3, 2024}));
  AOC_EXPECT_RESULT(24, solve_case<true>("day22.example"));
  AOC_EXPECT_RESULT(2221, solve_case<true>("day22.input"));
  AOC_RETURN_CHECK_RESULT();
}
