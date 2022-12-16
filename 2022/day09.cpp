// https://adventofcode.com/2022/day/9

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

#include "../common.h"

struct point {
  int x = 0;
  int y = 0;
  bool operator==(const point&) const = default;

#define AOC_POINTWISE_OP(op, op_eq)                                            \
  constexpr point& operator op_eq(const point& other) {                        \
    x op_eq other.x;                                                           \
    y op_eq other.y;                                                           \
    return *this;                                                              \
  }                                                                            \
  constexpr friend point operator op(point lhs, const point& rhs) {            \
    lhs op_eq rhs;                                                             \
    return lhs;                                                                \
  }

  AOC_POINTWISE_OP(+, +=)
  AOC_POINTWISE_OP(-, -=)
  AOC_POINTWISE_OP(*, *=)

#undef AOC_POINTWISE_OP

  // Note that this operator allows division by zero
  // by setting the element to zero
  constexpr point& operator/=(const point& other) {
    if (other.x == 0) {
      x = 0;
    } else {
      x /= other.x;
    }
    if (other.y == 0) {
      y = 0;
    } else {
      y /= other.y;
    }
    return *this;
  }
  constexpr friend point operator/(point lhs, const point& rhs) {
    lhs /= rhs;
    return lhs;
  }

  constexpr point operator-() const { return {-x, -y}; }

  friend std::ostream& operator<<(std::ostream& out, const point p) {
    out << "{" << p.x << "," << p.y << "}";
    return out;
  }

  constexpr point abs() const { return {std::abs(x), std::abs(y)}; }
};

template <class T>
class unique_vector {
 public:
  using data_t = std::vector<T>;
  using value_type = typename data_t::value_type;
  using iterator = typename data_t::iterator;
  using const_iterator = typename data_t::const_iterator;

  constexpr iterator push_back(const T& value) {
    auto it = std::ranges::find(m_data, value);
    if (it != std::end(m_data)) {
      return it;
    }
    m_data.push_back(value);
    return std::begin(m_data) + m_data.size() - 1;
  }

  constexpr auto begin() { return m_data.begin(); }
  constexpr auto begin() const { return m_data.begin(); }
  constexpr auto end() { return m_data.end(); }
  constexpr auto end() const { return m_data.end(); }

  constexpr auto size() { return m_data.size(); }

 private:
  data_t m_data;
};

using rope_bridge_t = grid<std::vector<char>>;

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

void print_visited(const unique_vector<point>& visited) {
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
void solve_case(const std::string& filename) {
  std::array<point, num_knots> rope;
  auto& head = rope.front();
  auto& tail = rope.back();
  print_rope(rope);

  unique_vector<point> visited;
  visited.push_back(tail);

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
    visited.push_back(tail);
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

  readfile_op(filename, [&](std::string_view line) {
    auto [operation, value_str] =
        split<std::array<std::string, 2>>(std::string{line}, ' ');
    auto value = std::stoi(value_str);
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
        throw std::runtime_error("Invalid command " + operation);
    }
  });

  std::cout << filename << " -> " << visited.size() << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<2>("day09.example");
  solve_case<2>("day09.input");
  std::cout << "Part 2" << std::endl;
  solve_case<10>("day09.example");
  solve_case<10>("day09.example2");
  solve_case<10>("day09.input");
}
