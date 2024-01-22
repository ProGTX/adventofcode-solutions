#ifndef AOC_POINT_H
#define AOC_POINT_H

#include "assert.h"
#include "math.h"
#include "utility.h"

#include <array>
#include <compare>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <ostream>
#include <ranges>
#include <span>
#include <type_traits>

namespace aoc {

struct fill_tag {};

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

  constexpr friend value_type distance_manhattan(const point_type& lhs,
                                                 const point_type& rhs) {
    auto diff = (rhs - lhs).abs();
    return diff.x + diff.y;
  }

  constexpr iterator begin() noexcept { return &x; }
  constexpr const_iterator begin() const noexcept { return &x; }

  constexpr iterator end() noexcept { return (&y) + 1; }
  constexpr const_iterator end() const noexcept { return (&y) + 1; }

  template <size_t I>
  constexpr friend value_type get(const point_type& p) {
    if constexpr (I == 0) {
      return p.x;
    } else if constexpr (I == 1) {
      return p.y;
    } else {
      static_assert(false);
    }
  }
};

template <class T>
struct arity<point_type<T>>
    : public std::integral_constant<int, arity_v<T> * 2> {};

template <class T, int dims>
class nd_point_type {
 public:
  static_assert(dims > 0);

  using value_type = T;
  using data_t = std::array<value_type, dims>;
  static constexpr auto dimensions = dims;
  using reference = typename data_t::reference;
  using const_reference = typename data_t::const_reference;
  using iterator = typename data_t::iterator;
  using const_iterator = typename data_t::const_iterator;
  using size_type = typename data_t::size_type;

  constexpr nd_point_type() = default;
  constexpr nd_point_type(const data_t& data) : m_data{data} {}

  template <std::convertible_to<value_type>... Us>
    requires(sizeof...(Us) == dims)
  constexpr nd_point_type(Us... values)
      : m_data{static_cast<value_type>(values)...} {}

  constexpr nd_point_type(fill_tag,
                          std::convertible_to<value_type> auto value) {
    std::ranges::fill(m_data, static_cast<value_type>(value));
  }

  constexpr bool operator==(const nd_point_type&) const = default;

  constexpr std::strong_ordering operator<=>(const nd_point_type& other) const {
    // Inverse the order
    for (int i = dims - 1; i >= 0; --i) {
      auto order = (m_data[i] <=> other[i]);
      if (order != std::strong_ordering::equal) {
        return order;
      }
    }
    return std::strong_ordering::equal;
  }

  constexpr reference operator[](size_type pos) { return m_data[pos]; }
  constexpr const_reference operator[](size_type pos) const {
    return m_data[pos];
  }

  template <class F>
    requires(std::invocable<F, value_type> ||
             std::invocable<F, value_type, int>)
  constexpr void transform(F&& f) {
    for (int i = 0; i < dims; ++i) {
      if constexpr (std::invocable<F, value_type, int>) {
        m_data[i] = f(m_data[i], i);
      } else {
        m_data[i] = f(m_data[i]);
      }
    }
  }

  template <class U>
    requires(!std::same_as<std::remove_cvref_t<U>, std::remove_cvref_t<T>> &&
             std::convertible_to<T, U>)
  constexpr operator nd_point_type<U, dims>() const {
    nd_point_type<U, dims> other;
    other.transform(
        [this](value_type, int i) { return static_cast<U>(m_data[i]); });
    return other;
  }

  constexpr reference x() { return m_data[0]; }
  constexpr const_reference x() const { return m_data[0]; }

  constexpr reference y()
    requires(dims > 1)
  {
    return m_data[1];
  }
  constexpr const_reference y() const
    requires(dims > 1)
  {
    return m_data[1];
  }

  constexpr reference z()
    requires(dims > 2)
  {
    return m_data[2];
  }
  constexpr const_reference z() const
    requires(dims > 2)
  {
    return m_data[2];
  }

  constexpr reference w()
    requires(dims > 3)
  {
    return m_data[3];
  }
  constexpr const_reference w() const
    requires(dims > 3)
  {
    return m_data[3];
  }

#define AOC_POINTWISE_OP(op, op_eq)                                            \
  constexpr nd_point_type& operator op_eq(const nd_point_type & other) {       \
    transform([&](value_type val, int i) { return val op other[i]; });         \
    return *this;                                                              \
  }                                                                            \
  constexpr friend nd_point_type operator op(nd_point_type lhs,                \
                                             const nd_point_type& rhs) {       \
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
  constexpr nd_point_type& operator op_eq(const nd_point_type & other) {       \
    transform([&](value_type val, int i) {                                     \
      return (other[i] == 0) ? 0 : (val op other[i]);                          \
    });                                                                        \
    return *this;                                                              \
  }                                                                            \
  constexpr friend nd_point_type operator op(nd_point_type lhs,                \
                                             const nd_point_type& rhs) {       \
    lhs op_eq rhs;                                                             \
    return lhs;                                                                \
  }

  AOC_POINTWISE_OP(/, /=)
  AOC_POINTWISE_OP(%, %=)

#undef AOC_POINTWISE_OP

#define AOC_POINTWISE_OP(op, op_eq)                                            \
  constexpr nd_point_type& operator op_eq(value_type value) {                  \
    return *this op_eq nd_point_type{fill_tag{}, value};                       \
  }                                                                            \
  constexpr friend nd_point_type operator op(nd_point_type lhs,                \
                                             value_type value) {               \
    return lhs op_eq value;                                                    \
  }                                                                            \
  constexpr friend nd_point_type operator op(value_type value,                 \
                                             const nd_point_type& rhs) {       \
    return nd_point_type{fill_tag{}, value} op rhs;                            \
  }

  AOC_POINTWISE_OP(*, *=)
  AOC_POINTWISE_OP(/, /=)
  AOC_POINTWISE_OP(%, %=)

#undef AOC_POINTWISE_OP

  constexpr nd_point_type operator-() const {
    nd_point_type other;
    other.transform([this](value_type, int i) { return -m_data[i]; });
    return other;
  }

  constexpr nd_point_type abs() const {
    nd_point_type other;
    other.transform([this](value_type, int i) { return aoc::abs(m_data[i]); });
    return other;
  }

  constexpr nd_point_type normal() const { return *this / this->abs(); }

  constexpr friend value_type distance_manhattan(const nd_point_type& lhs,
                                                 const nd_point_type& rhs) {
    auto diff = (rhs - lhs).abs();
    return diff.x + diff.y;
  }

  constexpr value_type* data() noexcept { m_data.data(); }
  constexpr const value_type* data() const noexcept { return m_data.data(); }

  constexpr iterator begin() noexcept { return m_data.begin(); }
  constexpr const_iterator begin() const noexcept { return m_data.begin(); }

  constexpr iterator end() noexcept { return m_data.end(); }
  constexpr const_iterator end() const noexcept { return m_data.end(); }

  friend std::ostream& operator<<(std::ostream& out, const nd_point_type& np) {
    out << '{';
    for (int i = 0; i < dims - 1; ++i) {
      out << np.m_data[i] << ',';
    }
    out << np.m_data[dims - 1] << '}';
    return out;
  }

  template <size_t I>
  constexpr friend value_type get(const nd_point_type& np) {
    static_assert(I < dims);
    return np[I];
  }

 private:
  data_t m_data;
};

template <class T, int dims>
struct arity<nd_point_type<T, dims>>
    : public std::integral_constant<int, arity_v<T> * dims> {};

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
