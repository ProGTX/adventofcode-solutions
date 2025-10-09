// https://adventofcode.com/2024/day/25

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

constexpr const auto keyhole_size = point{5, 7};
constexpr const int max_height = keyhole_size.y - 2;
using keyhole_storage_t = std::array<int, keyhole_size.x>;

constexpr const char active = '#';
constexpr const char inactive = '.';
constexpr const std::string_view all_filled = "#####";

constexpr bool overlaps(const keyhole_storage_t& key,
                        const keyhole_storage_t& lock) {
  return std::ranges::any_of(key | aoc::views::enumerate(),
                             [&](auto&& current) {
                               const auto [i, key_value] = current;
                               const auto lock_value = lock[i];
                               return (key_value + lock_value) > max_height;
                             });
}

constexpr int count_fitting(const std::span<const keyhole_storage_t> keys,
                            const std::span<const keyhole_storage_t> locks) {
  return aoc::ranges::accumulate(
      keys | std::views::transform([&](const keyhole_storage_t& key) {
        return std::ranges::count_if(locks, [&](const keyhole_storage_t& lock) {
          return !overlaps(key, lock);
        });
      }),
      0);
}

template <bool>
int solve_case(const std::string& filename) {
  std::vector<keyhole_storage_t> keys;
  std::vector<keyhole_storage_t> locks;
  keyhole_storage_t* current_keyhole_ptr = nullptr;

  bool parsing_lock = false;
  for (int row = 0; std::string_view line : aoc::views::read_lines(filename)) {
    if ((row % keyhole_size.y) == 0) {
      parsing_lock = (line == all_filled);
      if (parsing_lock) {
        locks.emplace_back();
        current_keyhole_ptr = &locks.back();
        std::ranges::fill(*current_keyhole_ptr, 0);
      } else {
        keys.emplace_back();
        current_keyhole_ptr = &keys.back();
        std::ranges::fill(*current_keyhole_ptr, max_height);
      }
      row = 1;
      continue;
    }
    for (const auto [i, c] : line | aoc::views::enumerate()) {
      if (parsing_lock) {
        if (c == active) {
          ++current_keyhole_ptr->at(i);
        }
      } else {
        if (c == inactive) {
          --current_keyhole_ptr->at(i);
        }
      }
    }
    ++row;
  }

  int sum = 0;
  sum = count_fitting(keys, locks);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(3, solve_case<false>("day25.example"));
  AOC_EXPECT_RESULT(3338, solve_case<false>("day25.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day25.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day25.input"));
  AOC_RETURN_CHECK_RESULT();
}
