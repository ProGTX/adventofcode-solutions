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

struct buyer_t {
  std::vector<int> values;
  std::string diffs;
};

// -9 is the minimum diff, we normalize it to 0
// The largest diff can now be 18
// We store it into a char because it's easy to perform string search
constexpr char diff_to_char(const int diff) {
  return static_cast<char>('a' + 9 + diff);
}

constexpr auto get_buyers(std::span<const int> secrets) {
  std::vector<buyer_t> buyers;
  buyers.reserve(secrets.size());
  for (int s = 0; s < secrets.size(); ++s) {
    buyers.emplace_back();
    auto& buyer = buyers.back();
    buyer.values.reserve(num_repeat);
    buyer.diffs.reserve(num_repeat);
    auto secret = next_secret(secrets[s]);
    int previous = secret % 10;
    for (int i = 1; i < num_repeat; ++i) {
      secret = next_secret(secret);
      const int current = secret % 10;
      buyer.values.push_back(current);
      const int diff = current - previous;
      buyer.diffs.push_back(diff_to_char(diff));
      previous = current;
    }
  }
  return buyers;
}

constexpr const int pattern_size = 4;
constexpr const int patterns_per_buyer = num_repeat - pattern_size;

constexpr int max_pattern_value(const buyer_t& buyer,
                                std::string_view pattern) {
  const auto pos = buyer.diffs.find(pattern);
  if (pos != std::string::npos) {
    return buyer.values[pos + pattern_size - 1];
  }
  return 0;
}

constexpr std::string get_pattern(const std::array<int, 4>& diffs) {
  return diffs | std::views::transform(&diff_to_char) |
         aoc::ranges::to<std::string>();
}

static_assert(7 == max_pattern_value(get_buyers(std::array{1})[0],
                                     get_pattern({-2, 1, -1, 3})));
static_assert(7 == max_pattern_value(get_buyers(std::array{2})[0],
                                     get_pattern({-2, 1, -1, 3})));
static_assert(0 == max_pattern_value(get_buyers(std::array{3})[0],
                                     get_pattern({-2, 1, -1, 3})));
static_assert(9 == max_pattern_value(get_buyers(std::array{2024})[0],
                                     get_pattern({-2, 1, -1, 3})));

using cache_t = aoc::flat_map<std::string_view, int>;

constexpr int max_pattern_value(cache_t& cache, std::span<const buyer_t> buyers,
                                std::string_view pattern) {
  auto it = cache.find(pattern);
  if (it != std::end(cache)) {
    return it->second;
  }
  const auto value = aoc::ranges::accumulate(
      buyers | std::views::transform([pattern](const buyer_t& buyer) {
        return max_pattern_value(buyer, pattern);
      }),
      0);
  cache[pattern] = value;
  return value;
}

constexpr int most_bananas(std::span<const int> secrets) {
  const auto buyers = get_buyers(secrets);
  int max_bananas = 0;
  cache_t cache;
  for (const buyer_t& buyer : buyers) {
    AOC_ASSERT((patterns_per_buyer == (buyer.values.size() - pattern_size + 1)),
               "Wrong calculation for patterns_per_buyer");
    for (int i = 0; i < patterns_per_buyer; ++i) {
      const auto pattern =
          std::string_view{buyer.diffs.data() + i, pattern_size};
      const auto value = max_pattern_value(cache, buyers, pattern);
      max_bananas = std::ranges::max(value, max_bananas);
    }
  }
  return max_bananas;
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
  // The following call can't be static_assert because of compiler limits
  AOC_EXPECT_RESULT(23, most_bananas(std::array{1, 2, 3, 2024}));
  AOC_EXPECT_RESULT(24, solve_case<true>("day22.example"));
  // Takes over 90s on a LNL laptop
  AOC_EXPECT_RESULT(2221, solve_case<true>("day22.input"));
  AOC_RETURN_CHECK_RESULT();
}
