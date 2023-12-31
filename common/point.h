#pragma once

#include "utility.h"

#include <cstdlib>
#include <functional>
#include <ostream>
#include <type_traits>

template <class T>
struct point_type {
  using value_type = T;
  using iterator = value_type*;
  using const_iterator = const value_type*;

  value_type x = 0;
  value_type y = 0;

  constexpr friend bool operator==(const point_type&,
                                   const point_type&) = default;

  constexpr friend bool operator<(const point_type& lhs,
                                  const point_type& rhs) {
    if (lhs.y == rhs.y) {
      return lhs.x < rhs.x;
    }
    return lhs.y < rhs.y;
  };

#define AOC_POINTWISE_OP(op, op_eq)                                            \
  constexpr point_type& operator op_eq(const point_type & other) {             \
    x op_eq other.x;                                                           \
    y op_eq other.y;                                                           \
    return *this;                                                              \
  }                                                                            \
  constexpr friend point_type operator op(point_type lhs,                      \
                                          const point_type& rhs) {             \
    lhs op_eq rhs;                                                             \
    return lhs;                                                                \
  }

  AOC_POINTWISE_OP(+, +=)
  AOC_POINTWISE_OP(-, -=)
  AOC_POINTWISE_OP(*, *=)

#undef AOC_POINTWISE_OP

  // Note that this operator allows division by zero
  // by setting the element to zero

#define AOC_POINTWISE_OP(op, op_eq)                                            \
  constexpr point_type& operator op_eq(const point_type & other) {             \
    if (other.x == 0) {                                                        \
      x = 0;                                                                   \
    } else {                                                                   \
      x op_eq other.x;                                                         \
    }                                                                          \
    if (other.y == 0) {                                                        \
      y = 0;                                                                   \
    } else {                                                                   \
      y op_eq other.y;                                                         \
    }                                                                          \
    return *this;                                                              \
  }                                                                            \
  constexpr friend point_type operator op(point_type lhs,                      \
                                          const point_type& rhs) {             \
    lhs op_eq rhs;                                                             \
    return lhs;                                                                \
  }

  AOC_POINTWISE_OP(/, /=)
  AOC_POINTWISE_OP(%, %=)

#undef AOC_POINTWISE_OP

  constexpr point_type operator-() const { return {-x, -y}; }

  friend std::ostream& operator<<(std::ostream& out, const point_type& p) {
    out << "{" << p.x << "," << p.y << "}";
    return out;
  }

  constexpr point_type abs() const { return {abs_value(x), abs_value(y)}; }

  constexpr static long distance_squared(const point_type& lhs,
                                         const point_type& rhs) {
    auto diff = rhs - lhs;
    return (static_cast<long>(diff.x) * diff.x) +
           (static_cast<long>(diff.y) * diff.y);
  }

  constexpr static value_type distance_manhattan(const point_type& lhs,
                                                 const point_type& rhs) {
    auto diff = (rhs - lhs).abs();
    return diff.x + diff.y;
  }

  constexpr iterator begin() noexcept { return &x; }
  constexpr const_iterator begin() const noexcept { return &x; }

  constexpr iterator end() noexcept { return (&y) + 1; }
  constexpr const_iterator end() const noexcept { return (&y) + 1; }

  template <binary_op_r<value_type, value_type, value_type> Op = std::plus<>>
  constexpr value_type reduce(Op op = {}) const {
    return op(x, y);
  }
};

using point = point_type<int>;

template <class T>
struct cube_t {
  using value_type = T;

  T x{0};
  T y{0};
  T z{0};

#define AOC_POINTWISE_OP(op, op_eq)                                            \
  constexpr cube_t& operator op_eq(const cube_t & other) {                     \
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

  constexpr friend bool operator==(const cube_t&, const cube_t&) = default;

  constexpr friend bool operator<(const cube_t& lhs, const cube_t& rhs) {
    if (lhs.z == rhs.z) {
      if (lhs.y == rhs.y) {
        return lhs.x < rhs.x;
      }
      return lhs.y < rhs.y;
    }
    return lhs.z < rhs.z;
  }
};

struct min_max_helper {
  point min_value{2'000'000'000, 2'000'000'000};
  point max_value{0, 0};

  constexpr min_max_helper& update(const point& p) {
    if (p.x < min_value.x) {
      min_value.x = p.x;
    }
    if (p.y < min_value.y) {
      min_value.y = p.y;
    }
    if (p.x > max_value.x) {
      max_value.x = p.x;
    }
    if (p.y > max_value.y) {
      max_value.y = p.y;
    }
    return *this;
  };

#if 0
  constexpr min_max_helper& update(const min_max_helper& other) {
    if (other.min_value.x < min_value.x) {
      min_value.x = other.min_value.x;
    }
    if (other.min_value.y < min_value.y) {
      min_value.y = other.min_value.y;
    }
    if (other.max_value.x > max_value.x) {
      max_value.x = other.max_value.x;
    }
    if (other.max_value.y > max_value.y) {
      max_value.y = other.max_value.y;
    }
    return *this;
  }
#endif

  friend std::ostream& operator<<(std::ostream& out,
                                  const min_max_helper& value) {
    out << "min{" << value.min_value << ',' << value.max_value << '}';
    return out;
  }

  constexpr point grid_size() const {
    return max_value - min_value + point{1, 1};
  }

  template <std::ranges::range R>
    requires std::convertible_to<decltype(*std::begin(std::declval<R>())),
                                 point>
  static constexpr min_max_helper get(R const& range) {
    min_max_helper helper;
    for (auto const& elem : range) {
      helper.update(static_cast<point>(elem));
    }
    return helper;
  }
};
