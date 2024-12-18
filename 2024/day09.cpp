// https://adventofcode.com/2024/day/9

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

constexpr const int empty_space = -1;

using int_t = std::uint64_t;
using filesystem_t = std::vector<int>;

constexpr filesystem_t expand(std::string_view disk_map) {
  filesystem_t filesystem;
  int id = 0;
  bool is_empty = false;
  for (char current_char : disk_map) {
    auto num = aoc::to_number<int>(current_char);
    int current = empty_space;
    if (!is_empty) {
      current = id;
      ++id;
    }
    auto repeated =
        aoc::views::repeat(current, num) | aoc::ranges::to<filesystem_t>();
    filesystem.insert(filesystem.end(), repeated.begin(), repeated.end());
    is_empty = !is_empty;
  }
  return filesystem;
}

static_assert(std::ranges::equal( // "0..111....22222"
    std::array{0, -1, -1, 1, 1, 1, -1, -1, -1, -1, 2, 2, 2, 2, 2},
    expand("12345")));

constexpr filesystem_t compact(filesystem_t filesystem) {
  auto it = filesystem.begin();
  const auto end = filesystem.end();
  const auto rend = filesystem.rend();
  for (auto rit = filesystem.rbegin(); rit != rend; ++rit) {
    while (*rit == empty_space) {
      ++rit;
      if (rit == rend) {
        break;
      }
    }
    if (rit == rend) {
      break;
    }
    it = std::ranges::find(it, end, empty_space);
    if ((it == end) || (it >= rit.base())) {
      break;
    }
    AOC_ASSERT(*it == empty_space, "");
    AOC_ASSERT(*rit != empty_space, "");
    std::swap(*rit, *it);
    ++it;
  }
  return filesystem;
}

constexpr int_t checksum(std::span<const int> filesystem) {
  return aoc::ranges::accumulate(filesystem | aoc::views::enumerate() |
                                     std::views::transform([](auto&& current) {
                                       auto [position, id] = current;
                                       if (id == empty_space) {
                                         return 0;
                                       }
                                       return position * id;
                                     }),
                                 int_t{0});
}

static_assert(60 ==
              // "022111222......"
              checksum(std::array{0, 2, 2, 1, 1, 1, 2, 2, 2}));
static_assert(1928 ==
              // "0099811188827773336446555566.............."
              checksum(std::array{0, 0, 9, 9, 8, 1, 1, 1, 8, 8, 8, 2, 7, 7,
                                  7, 3, 3, 3, 6, 4, 4, 6, 5, 5, 5, 5, 6, 6}));
static_assert(1928 ==
              // "0099811188827773336446555566.............."
              checksum(std::array{0, 0, 9, 9, 8, 1, 1, 1, 8, 8, 8, 2, 7, 7, 7,
                                  3, 3, 3, 6, 4, 4, 6, 5, 5, 5, 5, 6, 6, -1}));

constexpr int_t compact_from_disk_map(std::string_view disk_map) {
  return checksum(compact(expand(disk_map)));
}

static_assert(60 == compact_from_disk_map("12345"));

template <bool>
int_t solve_case(const std::string& filename) {
  std::string disk_map;
  for (std::string line : aoc::views::read_lines(filename)) {
    disk_map = std::move(line);
    break;
  }
  auto sum = compact_from_disk_map(disk_map);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(1928, solve_case<false>("day09.example"));
  AOC_EXPECT_RESULT(6432869891895, solve_case<false>("day09.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day09.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day09.input"));
  AOC_RETURN_CHECK_RESULT();
}
