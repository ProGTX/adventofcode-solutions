// https://adventofcode.com/2024/day/9

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

constexpr filesystem_t compact_nonfragment(filesystem_t filesystem) {
  // 9 because that's the highest number possible
  std::array<int, 9> max_empty_range;
  std::ranges::fill(max_empty_range, empty_space);
  const auto max_empty_range_begin = max_empty_range.begin();
  using iterator = filesystem_t::iterator;
  const auto begin = filesystem.begin();
  const auto end = filesystem.end();
  const auto rend = filesystem.rend();
  iterator it = end;
  for (auto rit = filesystem.rbegin(); rit != rend; ++rit) {
    // Skip over empty spaces from the back
    while (*rit == empty_space) {
      ++rit;
      if (rit == rend) {
        break;
      }
    }
    if (rit == rend) {
      break;
    }
    // Collect current elements into a range
    const auto current = *rit;
    const auto current_range_end = rit.base();
    auto current_range_begin = current_range_end;
    while (current_range_begin != begin) {
      auto next_it = current_range_begin - 1;
      if (*next_it != current) {
        break;
      }
      current_range_begin = next_it;
    }

    // Progress the reverse iterator to skip the current range
    // Plus 1 because a reverse iterator points to the element
    // before the forward iterator
    rit = std::reverse_iterator(current_range_begin + 1);

    auto current_range =
        std::ranges::subrange(current_range_begin, current_range_end);
    const auto current_size = current_range.size();

    // Find an empty range that can fit the current range
    it = std::search(begin, current_range_begin, max_empty_range_begin,
                     max_empty_range_begin + current_size);
    if ((it == end) || (it >= current_range_begin)) {
      // Can't fit the current range in any free space to the left
      continue;
    }

    // Swap the current range with the empty space
    std::ranges::swap_ranges(std::ranges::subrange(it, it + current_size),
                             current_range);
  }
  return filesystem;
}

constexpr int_t checksum(std::span<const int> filesystem) {
  return aoc::ranges::accumulate(
      filesystem | std::views::enumerate |
          std::views::transform([](auto&& current) -> int_t {
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

template <bool nonfragment>
constexpr int_t compact_from_disk_map(std::string_view disk_map) {
  auto filesystem = expand(disk_map);
  if constexpr (!nonfragment) {
    return checksum(compact(std::move(filesystem)));
  } else {
    return checksum(compact_nonfragment(std::move(filesystem)));
  }
}

static_assert(60 == compact_from_disk_map<false>("12345"));

template <bool nonfragment>
int_t solve_case(const std::string& filename) {
  std::string disk_map;
  for (std::string line : aoc::views::read_lines(filename)) {
    disk_map = std::move(line);
    break;
  }
  auto sum = compact_from_disk_map<nonfragment>(disk_map);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(1928, solve_case<false>("day09.example"));
  AOC_EXPECT_RESULT(6432869891895, solve_case<false>("day09.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(2858, solve_case<true>("day09.example"));
  AOC_EXPECT_RESULT(6467290479134, solve_case<true>("day09.input"));
  AOC_RETURN_CHECK_RESULT();
}
