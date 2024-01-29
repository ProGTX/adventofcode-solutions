// https://adventofcode.com/2023/day/22

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <ostream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#define DEBUG_PRINT 0

using int_t = int;
using coord_t = aoc::nd_point_type<int, 3>;

template <class... Ts>
void debug([[maybe_unused]] Ts... args) {
#if DEBUG_PRINT
  aoc::println(args...);
#endif
}

struct brick_t : public aoc::closed_range<coord_t> {
  using base_t = aoc::closed_range<coord_t>;

#if DEBUG_PRINT
  // Name for debugging
  std::string name;
#endif

  constexpr brick_t([[maybe_unused]] std::string name_, coord_t first_,
                    coord_t last_)
      : base_t{std::move(first_), std::move(last_)} {
#if DEBUG_PRINT
    name = std::move(name_);
#endif
  }

  constexpr brick_t& operator+=(const coord_t& diff) {
    begin += diff;
    end += diff;
    return *this;
  }
  friend constexpr brick_t operator+(brick_t lhs, const coord_t& rhs) {
    lhs += rhs;
    return lhs;
  }
  friend constexpr brick_t operator+(const coord_t& lhs, brick_t rhs) {
    rhs.begin = lhs + rhs.begin;
    rhs.end = lhs + rhs.end;
    return rhs;
  }

  constexpr bool on_floor() const { return begin.z() <= 1; }

  friend std::ostream& operator<<(std::ostream& out, const brick_t& brick) {
#if DEBUG_PRINT
    out << brick.name;
#endif
    out << static_cast<const base_t&>(brick);
    return out;
  }
};

// We want bricks to be sorted by height
using brick_list_t = std::vector<brick_t>;
using brick_iterator = typename brick_list_t::iterator;
using const_brick_iterator = typename brick_list_t::const_iterator;

#if DEBUG_PRINT
constexpr inline auto name_chars = []() {
  // 38 is required to name all the bricks from the input
  // 38*38 > 1394
  std::array<char, 38> chars{};
  std::ranges::copy(std::views::iota('A') | std::views::take(26),
                    chars.begin());
  std::ranges::copy(std::views::iota('0') | std::views::take(10),
                    chars.begin() + 26);
  chars[36] = '$';
  chars[37] = '&';
  return chars;
}();

constexpr inline auto height_chars = []() {
  std::array<char, 10> chars{};
  std::ranges::copy(std::views::iota('0') | std::views::take(10),
                    chars.begin());
  return chars;
}();
#endif

void print_tower([[maybe_unused]] const brick_list_t& bricks) {
#if DEBUG_PRINT
  aoc::min_max_helper min_max;

  for (const auto& brick : bricks) {
    min_max.update(point{brick.begin.x(), brick.begin.z()});
    min_max.update(point{brick.end.x(), brick.end.z()});
    min_max.update(point{brick.begin.y(), brick.begin.z()});
    min_max.update(point{brick.end.y(), brick.end.z()});
  }

  // Double the width because names take two spaces
  // Add some space to print out the height at the end
  // Add 2 to the height to account for the floor and the numbers at top
  constexpr auto number_edge_size = 5;
  const auto grid_size =
      (min_max.grid_size() * point{2, 1}) + point{number_edge_size, 2};
  aoc::char_grid<> tower(' ', grid_size.y, grid_size.x);

  const auto set_inner = [&](const brick_t& brick, point& brick_pos,
                             const point& brick_end) {
    const auto diff = (brick_end - brick_pos).normal();
    while (true) {
      const auto x = 2 * brick_pos.x - min_max.min_value.x;
      const auto y = grid_size.y - brick_pos.y - 1 + min_max.min_value.y - 1;
      // The value order needs to be reversed because brick.name[0] can be empty
      char& second_value = tower.at(y, x + 1);
      const auto previous = second_value;
      second_value = (previous == ' ') ? brick.name[1] : '?';
      char& first_value = tower.at(y, x);
      first_value = (previous == ' ') ? brick.name[0] : '?';
      if (brick_pos == brick_end) {
        break;
      }
      brick_pos += diff;
    }
  };

  const auto set_edges = [&]() {
    // Floor
    for (int column = 0; column < grid_size.x - number_edge_size; ++column) {
      tower.modify('-', grid_size.y - 1, column);
    }
    // Number edge for the height
    for (int row = 0; row < grid_size.y - 1; ++row) {
      const auto height = grid_size.y - row - 1 + min_max.min_value.y - 1;
      const auto n = height_chars.size();
      tower.modify(height_chars[height % n], row, grid_size.x - 1);
      if (height >= n) {
        tower.modify(height_chars[(height / n) % n], row, grid_size.x - 2);
        if (height >= n * n) {
          tower.modify(height_chars[height / (n * n)], row, grid_size.x - 3);
        }
      }
    }
    // Number edge for the width
    // Width coordinate is always a single-digit number
    for (int column = 0; column < grid_size.x - number_edge_size; column += 2) {
      tower.modify(height_chars[column / 2], 0, column + 1);
    }
  };

  // First print the tower as xz
  for (const auto& brick : bricks) {
    auto brick_pos = point{brick.begin.x(), brick.begin.z()};
    const auto brick_end = point{brick.end.x(), brick.end.z()};
    set_inner(brick, brick_pos, brick_end);
  }
  set_edges();
  tower.print_all();

  tower.clear();
  std::ranges::fill(tower, ' ');

  // Then print the tower as yz
  for (const auto& brick : bricks) {
    auto brick_pos = point{brick.begin.y(), brick.begin.z()};
    const auto brick_end = point{brick.end.y(), brick.end.z()};
    set_inner(brick, brick_pos, brick_end);
  }
  set_edges();
  tower.print_all();
#endif
}

constexpr inline auto down_diff = coord_t{0, 0, -1};

brick_list_t fall_down(brick_list_t bricks) {
  std::ranges::sort(bricks);

  const auto bricks_end = bricks.end();
  // Move the lower bricks first
  for (auto brick_it = bricks.begin(); brick_it != bricks_end; ++brick_it) {
    auto& brick = *brick_it;
    auto current = brick;
    while (!current.on_floor()) {
      auto new_brick = current + down_diff;
      bool overlaps = false;
      for (auto other_it = std::reverse_iterator(brick_it);
           other_it != bricks.rend(); ++other_it) {
        const auto& other = *other_it;
        if (&other == &brick) {
          // Don't check the brick we're trying to move
          continue;
        }
        if (new_brick.overlaps_with(other)) {
          overlaps = true;
        }
        if (overlaps) {
          break;
        }
      }
      if (overlaps) {
        break;
      }
      current = new_brick;
    }
    brick = current;
  }

  std::ranges::sort(bricks);
  return bricks;
}

bool is_supported(const_brick_iterator check_it, const brick_list_t& bricks,
                  const_brick_iterator remove_it) {
  const auto& check = *check_it;
  if (check.on_floor()) {
    // Floor supports this brick
    return true;
  }
  // Otherwise check there's at least one brick underneath
  const auto below = check + down_diff;
  const auto height_below_checked = below.begin.z();
  bool has_below = false;
  // Don't use any_of because it goes through all the bricks
  // Bricks are sorted, so we only need to check the lower ones
  for (auto other_it = std::reverse_iterator(check_it);
       other_it != bricks.rend(); ++other_it) {
    const auto& other = *other_it;
    if (other.end.z() > height_below_checked) {
      // Only interested in bricks below the checked one
      continue;
    }
    if (other.end.z() < height_below_checked) {
      // Stop iteration when we're too low
      break;
    }
    if (&*other_it == &*remove_it) {
      // Don't check the brick we're trying to remove
      continue;
    }
    has_below = other.overlaps_with(below);
    if (has_below) {
      break;
    }
  }
  return has_below;
}

bool all_supported(const brick_list_t& bricks, const_brick_iterator remove_it) {
  // Check that each brick above still has bricks supporting it
  // Use iterators instead of all_of because we need to reverse them later
  bool all_supported = true;
  const auto height_above_removed = remove_it->end.z() + 1;
  for (auto check_it = remove_it; check_it != bricks.end(); ++check_it) {
    if (check_it->begin.z() < height_above_removed) {
      // Only interested in bricks above the removed one
      continue;
    }
    if (check_it->begin.z() > height_above_removed) {
      // Stop iteration when we're too high
      break;
    }
    // Don't check the brick we're trying to remove
    if ((check_it != remove_it) && !is_supported(check_it, bricks, remove_it)) {
      all_supported = false;
      break;
    }
  }
  return all_supported;
}

int num_single_disintegrations(const brick_list_t& bricks) {
  // Try to remove each brick
  int count = 0;
  for (auto remove_it = bricks.begin(); remove_it != bricks.end();
       ++remove_it) {
    if (all_supported(bricks, remove_it)) {
      ++count;
      debug("  remove", *remove_it);
    }
  }
  return count;
}

template <bool>
int_t solve_case(const std::string& filename) {
  std::cout << filename << std::endl;

  brick_list_t bricks;
  std::string name = "AA";

  for (int id = 0; std::string_view line : aoc::views::read_lines(filename)) {
    auto [first, last] = aoc::split<std::array<std::string_view, 2>>(line, '~');
    {
#if DEBUG_PRINT
      const auto n = name_chars.size();
      auto counter0 = (id / n) % n;
      name[0] = (counter0 == 0) ? ' ' : name_chars[counter0];
      auto counter1 = id % n;
      name[1] = name_chars[counter1];
#endif
    }
    bricks.emplace_back(name, aoc::split<coord_t>(first, ','),
                        aoc::split<coord_t>(last, ','));
    ++id;
  }

  // print_tower(bricks);
  bricks = fall_down(bricks);
  if (!all_supported(bricks, std::end(bricks))) {
    aoc::println("NOT SUPPORTED");
  }
  print_tower(bricks);

  int_t sum = 0;
  sum = num_single_disintegrations(bricks);
  std::cout << "  -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(5, (solve_case<false>("day22.example")));
  AOC_EXPECT_RESULT(3, (solve_case<false>("day22.example2")));
  AOC_EXPECT_RESULT(2, (solve_case<false>("day22.example3")));
  AOC_EXPECT_RESULT(465, (solve_case<false>("day22.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(16, (solve_case<true>("day22.example")));
  // AOC_EXPECT_RESULT(16733044, (solve_case<true>("day22.input")));
  AOC_RETURN_CHECK_RESULT();
}
