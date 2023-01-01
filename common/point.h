#pragma once

#include <cstdlib>
#include <ostream>
#include <type_traits>

struct point {
  using value_type = int;
  using iterator = value_type*;
  using const_iterator = const value_type*;

  value_type x = 0;
  value_type y = 0;
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

  friend std::ostream& operator<<(std::ostream& out, const point& p) {
    out << "{" << p.x << "," << p.y << "}";
    return out;
  }

  constexpr point abs() const {
    if (std::is_constant_evaluated()) {
      return {(x < 0) ? -x : x, (y < 0) ? -y : y};
    } else {
      return {std::abs(x), std::abs(y)};
    }
  }

  constexpr static long distance_squared(const point& lhs, const point& rhs) {
    auto diff = rhs - lhs;
    return (static_cast<long>(diff.x) * diff.x) +
           (static_cast<long>(diff.y) * diff.y);
  }

  constexpr static int distance_manhattan(const point& lhs, const point& rhs) {
    auto diff = (rhs - lhs).abs();
    return diff.x + diff.y;
  }

  constexpr iterator begin() noexcept { return &x; }
  constexpr const_iterator begin() const noexcept { return &x; }

  constexpr iterator end() noexcept { return (&y) + 1; }
  constexpr const_iterator end() const noexcept { return (&y) + 1; }
};
