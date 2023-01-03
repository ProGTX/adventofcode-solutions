#pragma once

#include <cstdlib>
#include <ostream>
#include <type_traits>

template <class T>
struct point_t {
  using value_type = T;
  using iterator = value_type*;
  using const_iterator = const value_type*;

  value_type x = 0;
  value_type y = 0;
  bool operator==(const point_t&) const = default;

#define AOC_POINTWISE_OP(op, op_eq)                                            \
  constexpr point_t& operator op_eq(const point_t& other) {                    \
    x op_eq other.x;                                                           \
    y op_eq other.y;                                                           \
    return *this;                                                              \
  }                                                                            \
  constexpr friend point_t operator op(point_t lhs, const point_t& rhs) {      \
    lhs op_eq rhs;                                                             \
    return lhs;                                                                \
  }

  AOC_POINTWISE_OP(+, +=)
  AOC_POINTWISE_OP(-, -=)
  AOC_POINTWISE_OP(*, *=)

#undef AOC_POINTWISE_OP

  // Note that this operator allows division by zero
  // by setting the element to zero
  constexpr point_t& operator/=(const point_t& other) {
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
  constexpr friend point_t operator/(point_t lhs, const point_t& rhs) {
    lhs /= rhs;
    return lhs;
  }

  constexpr point_t operator-() const { return {-x, -y}; }

  friend std::ostream& operator<<(std::ostream& out, const point_t& p) {
    out << "{" << p.x << "," << p.y << "}";
    return out;
  }

  constexpr point_t abs() const {
    if (std::is_constant_evaluated()) {
      return {(x < 0) ? -x : x, (y < 0) ? -y : y};
    } else {
      return {std::abs(x), std::abs(y)};
    }
  }

  constexpr static long distance_squared(const point_t& lhs,
                                         const point_t& rhs) {
    auto diff = rhs - lhs;
    return (static_cast<long>(diff.x) * diff.x) +
           (static_cast<long>(diff.y) * diff.y);
  }

  constexpr static int distance_manhattan(const point_t& lhs,
                                          const point_t& rhs) {
    auto diff = (rhs - lhs).abs();
    return diff.x + diff.y;
  }

  constexpr iterator begin() noexcept { return &x; }
  constexpr const_iterator begin() const noexcept { return &x; }

  constexpr iterator end() noexcept { return (&y) + 1; }
  constexpr const_iterator end() const noexcept { return (&y) + 1; }
};

template <class T>
constexpr auto get_lex_point_sorter() {
  return [](const point_t<T>& lhs, const point_t<T>& rhs) {
    if (lhs.y == rhs.y) {
      return lhs.x < rhs.x;
    }
    return lhs.y < rhs.y;
  };
}

using point = point_t<int>;

template <class T>
struct cube_t {
  using value_type = T;

  T x{0};
  T y{0};
  T z{0};

#define AOC_POINTWISE_OP(op, op_eq)                                            \
  constexpr cube_t& operator op_eq(const cube_t& other) {                      \
    x op_eq other.x;                                                           \
    y op_eq other.y;                                                           \
    z op_eq other.z;                                                           \
    return *this;                                                              \
  }                                                                            \
  constexpr friend cube_t operator op(cube_t lhs, const cube_t& rhs) {         \
    lhs op_eq rhs;                                                             \
    return lhs;                                                                \
  }

  AOC_POINTWISE_OP(+, +=)
  AOC_POINTWISE_OP(-, -=)
  AOC_POINTWISE_OP(*, *=)

#undef AOC_POINTWISE_OP

  friend bool operator==(const cube_t& lhs, const cube_t& rhs) = default;

  friend bool operator<(const cube_t& lhs, const cube_t& rhs) {
    if (lhs.z == rhs.z) {
      if (lhs.y == rhs.y) {
        return lhs.x < rhs.x;
      }
      return lhs.y < rhs.y;
    }
    return lhs.z < rhs.z;
  }
};
