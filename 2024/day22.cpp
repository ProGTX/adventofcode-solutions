// https://adventofcode.com/2024/day/22

#include "../common/common.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using int_t = std::int64_t;

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

template <int num_repeat>
constexpr int next_secret_repeat(int secret) {
  for (int i = 0; i < num_repeat; ++i) {
    secret = next_secret(secret);
  }
  return secret;
}

static_assert(8685429 == next_secret_repeat<2000>(1));
static_assert(4700978 == next_secret_repeat<2000>(10));
static_assert(15273692 == next_secret_repeat<2000>(100));
static_assert(8667524 == next_secret_repeat<2000>(2024));

template <int num_repeat>
constexpr int_t sum_secrects(std::span<const int> secrets) {
  return aoc::ranges::accumulate(
      secrets | std::views::transform(&next_secret_repeat<num_repeat>),
      int_t{0});
}

template <bool>
int_t solve_case(const std::string& filename) {
  auto buyer_starters = aoc::views::read_numbers<int>(filename) |
                        aoc::ranges::to<std::vector<int>>();

  int_t sum = 0;
  sum = sum_secrects<2000>(buyer_starters);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(37327623, solve_case<false>("day22.example"));
  AOC_EXPECT_RESULT(20215960478, solve_case<false>("day22.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day22.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day22.input"));
  AOC_RETURN_CHECK_RESULT();
}
