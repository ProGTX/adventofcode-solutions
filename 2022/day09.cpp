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

template <bool>
void solve_case(const std::string& filename) {
  point head;
  point tail = head;

  unique_vector<point> visited;
  visited.push_back(tail);

  const auto move_tail = [&]() {
    auto diff = head - tail;
    auto diff_abs = diff.abs();
    auto delta = diff / diff_abs;
    if (diff_abs.x > 1) {
      tail.x += delta.x;
      if (diff_abs.y > 0) {
        tail.y += delta.y;
      }
    }
    if (diff_abs.y > 1) {
      tail.y += delta.y;
      if (diff_abs.x > 0) {
        tail.x += delta.x;
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
  solve_case<false>("day09.example");
  solve_case<false>("day09.input");
  // std::cout << "Part 2" << std::endl;
  // solve_case<true>("day09.example");
  // solve_case<true>("day09.input");
}
