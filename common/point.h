#ifndef AOC_POINT_H
#define AOC_POINT_H

#include "assert.h"
#include "math.h"
#include "utility.h"

#include <array>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <ostream>
#include <span>
#include <type_traits>

namespace aoc {

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

  template <class U>
    requires(!std::same_as<std::remove_cvref_t<U>, std::remove_cvref_t<T>> &&
             std::convertible_to<T, U>)
  constexpr operator point_type<U>() const {
    return {static_cast<U>(x), static_cast<U>(y)};
  }

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

#define AOC_POINTWISE_OP(op, op_eq)                                            \
  constexpr point_type& operator op_eq(value_type value) {                     \
    return *this op_eq point_type{value, value};                               \
  }                                                                            \
  constexpr friend point_type operator op(point_type lhs, value_type value) {  \
    return lhs op_eq value;                                                    \
  }                                                                            \
  constexpr friend point_type operator op(value_type value,                    \
                                          const point_type& rhs) {             \
    return point_type{value, value} op rhs;                                    \
  }

  AOC_POINTWISE_OP(*, *=)
  AOC_POINTWISE_OP(/, /=)
  AOC_POINTWISE_OP(%, %=)

#undef AOC_POINTWISE_OP

  constexpr point_type operator-() const { return {-x, -y}; }

  friend std::ostream& operator<<(std::ostream& out, const point_type& p) {
    out << "{" << p.x << "," << p.y << "}";
    return out;
  }

  constexpr point_type abs() const { return {aoc::abs(x), aoc::abs(y)}; }

  constexpr point_type normal() const { return *this / this->abs(); }

  constexpr friend std::uint64_t distance_squared(const point_type& lhs,
                                                  const point_type& rhs) {
    auto diff = rhs - lhs;
    // TODO: Handle sizes better
    return static_cast<std::uint64_t>(static_cast<std::int64_t>(diff.x) *
                                      diff.x) +
           static_cast<std::uint64_t>(static_cast<std::int64_t>(diff.y) *
                                      diff.y);
  }

  constexpr friend value_type distance_manhattan(const point_type& lhs,
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

template <class T>
struct cube_type {
  using value_type = T;

  T x{0};
  T y{0};
  T z{0};

#define AOC_POINTWISE_OP(op, op_eq)                                            \
  constexpr cube_type& operator op_eq(const cube_type & other) {               \
    x op_eq other.x;                                                           \
    y op_eq other.y;                                                           \
    z op_eq other.z;                                                           \
    return *this;                                                              \
  }                                                                            \
  constexpr friend cube_type operator op(cube_type lhs,                        \
                                         const cube_type& rhs) {               \
    lhs op_eq rhs;                                                             \
    return lhs;                                                                \
  }

  AOC_POINTWISE_OP(+, +=)
  AOC_POINTWISE_OP(-, -=)
  AOC_POINTWISE_OP(*, *=)

#undef AOC_POINTWISE_OP

  constexpr friend bool operator==(const cube_type&,
                                   const cube_type&) = default;

  constexpr friend bool operator<(const cube_type& lhs, const cube_type& rhs) {
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
  using point = point_type<int>;

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

// https://en.wikipedia.org/wiki/Shoelace_formula
template <class return_t = void, class T>
constexpr auto calculate_area(std::span<const point_type<T>> polygon) {
  using actual_ret_t =
      std::conditional_t<std::same_as<return_t, void>, int, return_t>;
  const auto n = polygon.size();
  AOC_ASSERT(n >= 3, "Formula only works on triangles or higher");
  actual_ret_t area{};
  for (int i = 0; i < (n - 1); ++i) {
    area += polygon[i].x * polygon[i + 1].y - polygon[i + 1].x * polygon[i].y;
  }
  // Add the last edge
  area += polygon[n - 1].x * polygon[0].y - polygon[0].x * polygon[n - 1].y;
  return abs(area) / actual_ret_t{2};
}
template <class return_t = void, class T>
constexpr auto calculate_area(std::span<point_type<T>> polygon) {
  return calculate_area(std::span<const point_type<T>>{polygon});
}
static_assert(16 == calculate_area(std::span<const point_type<int>>{
                        std::array{point_type<int>{1, 6}, point_type<int>{3, 1},
                                   point_type<int>{7, 2}, point_type<int>{4, 4},
                                   point_type<int>{8, 5}}}));
static_assert(16.5f ==
              calculate_area<float>(std::span<const point_type<int>>{
                  std::array{point_type<int>{1, 6}, point_type<int>{3, 1},
                             point_type<int>{7, 2}, point_type<int>{4, 4},
                             point_type<int>{8, 5}}}));

} // namespace aoc

namespace std {
template <class T>
struct hash<aoc::point_type<T>> {
  // https://stackoverflow.com/a/64151007
  constexpr size_t operator()(const aoc::point_type<T>& value) const {
    size_t x_hash = std::hash<T>{}(value.x);
    size_t y_hash = std::hash<T>{}(value.y) << 1;
    return x_hash ^ y_hash;
  }
};
} // namespace std

#endif // AOC_POINT_H
