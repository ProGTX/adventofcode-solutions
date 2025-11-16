// https://adventofcode.com/2022/day/9

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using visited_t = aoc::flat_set<point>;
using rope_bridge_t = aoc::grid<char>;

std::tuple<rope_bridge_t, point, point> get_rope_bridge(
    const std::ranges::range auto& rope) {
  point size_positive;
  point size_negative;
  for (const auto& p : rope) {
    if (p.x > size_positive.x) {
      size_positive.x = p.x;
    }
    if (p.y > size_positive.y) {
      size_positive.y = p.y;
    }
    if (p.x < size_negative.x) {
      size_negative.x = p.x;
    }
    if (p.y < size_negative.y) {
      size_negative.y = p.y;
    }
  }
  // We calculated size based on index, need to increase it by 1
  int num_columns = size_positive.x - size_negative.x + 1;
  int num_rows = size_positive.y - size_negative.y + 1;

  rope_bridge_t rope_bridge;
  typename rope_bridge_t::row_t row(num_columns, '.');
  for (int i = 0; i < num_rows; ++i) {
    rope_bridge.add_row(row);
  }

  return {rope_bridge, size_positive, size_negative};
}

void print_visited(const visited_t& visited) {
  auto [rope_bridge, size_positive, size_negative] = get_rope_bridge(visited);

  for (const auto& p : visited) {
    point coordinate = p - size_negative;
    // Invert the display vertically
    int row = rope_bridge.num_rows() - coordinate.y - 1;
    int column = coordinate.x;
    rope_bridge.modify('#', row, column);
  }

  rope_bridge.print_all();
}

template <size_t num_knots>
void print_rope(const std::array<point, num_knots>& rope) {
  std::array<point, num_knots + 1> rope_with_start;
  std::ranges::copy(rope, std::begin(rope_with_start));
  rope_with_start[num_knots] = {0, 0};
  auto [rope_bridge, size_positive, size_negative] =
      get_rope_bridge(rope_with_start);
  rope_bridge.modify('s', rope_bridge.num_rows() - 1, 0);

  const auto get_char = [](int i) {
    if (i == 0) {
      return 'H';
    }
    if (i == num_knots - 1) {
      return 'T';
    }
    return static_cast<char>('0' + i);
  };

  for (int i = num_knots - 1; const auto& p : rope | std::views::reverse) {
    point coordinate = p - size_negative;
    // Invert the display vertically
    int row = rope_bridge.num_rows() - coordinate.y - 1;
    int column = coordinate.x;
    rope_bridge.modify(get_char(i), row, column);
    --i;
  }

  rope_bridge.print_all();
}

template <int num_knots>
int solve_case(const std::string& filename) {
  std::array<point, num_knots> rope;
  auto& head = rope.front();
  auto& tail = rope.back();
  print_rope(rope);

  visited_t visited;
  visited.insert(tail);

  const auto move_tail = [&]() {
    for (int i = 1; i < num_knots; ++i) {
      auto in_front = rope[i - 1];
      auto& current = rope[i];
      auto diff = in_front - current;
      auto diff_abs = diff.abs();
      auto delta = diff / diff_abs;
      if (diff_abs.x >= 2) {
        current.x += delta.x;
        if (diff_abs.y >= 1) {
          current.y += delta.y;
        }
      } else if (diff_abs.y >= 2) {
        current.y += delta.y;
        if (diff_abs.x >= 1) {
          current.x += delta.x;
        }
      }
    }
    visited.insert(tail);
  };

  const auto move = [&](point diff) {
    // Can only move vertically or horizontally
    if ((diff.x * diff.y) != 0) {
      throw std::runtime_error("Diagonal movement");
    }
    auto diff_abs = diff.abs();
    auto delta = diff / diff_abs;
    if (diff.x != 0) {
      for (int i = 0; i < diff_abs.x; ++i) {
        head.x += delta.x;
        move_tail();
      }
    } else {
      for (int i = 0; i < diff_abs.y; ++i) {
        head.y += delta.y;
        move_tail();
      }
    }
  };

  for (std::string_view line : aoc::views::read_lines(filename)) {
    auto [operation, value_str] = aoc::split_once(line, ' ');
    auto value = aoc::to_number<int>(value_str);
    switch (operation[0]) {
      case 'R':
        move({value, 0});
        break;
      case 'L':
        move({-value, 0});
        break;
      case 'U':
        move({0, value});
        break;
      case 'D':
        move({0, -value});
        break;
      default:
        throw std::runtime_error("Invalid command " + std::string{operation});
    }
  }

  std::cout << filename << " -> " << visited.size() << std::endl;
  return visited.size();
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(13, solve_case<2>("day09.example"));
  AOC_EXPECT_RESULT(5695, solve_case<2>("day09.input"));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(1, solve_case<10>("day09.example"));
  AOC_EXPECT_RESULT(36, solve_case<10>("day09.example2"));
  AOC_EXPECT_RESULT(2434, solve_case<10>("day09.input"));
  AOC_RETURN_CHECK_RESULT();
}
