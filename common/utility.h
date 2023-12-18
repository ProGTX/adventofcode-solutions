#pragma once

#include "assert.h"
#include "point.h"

#include <cmath>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

// https://en.cppreference.com/w/cpp/utility/to_underlying
template <class Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept {
  return static_cast<std::underlying_type_t<Enum>>(e);
}

// https://stackoverflow.com/a/35348334
template <class ReturnT, class... Args>
std::tuple<Args...> function_args_helper(ReturnT (*)(Args...));
template <class ReturnT, class F, class... Args>
std::tuple<Args...> function_args_helper(ReturnT (F::*)(Args...));
template <class ReturnT, class F, class... Args>
std::tuple<Args...> function_args_helper(ReturnT (F::*)(Args...) const);
template <class F>
decltype(function_args_helper(&F::operator())) function_args_helper(F);

template <class T>
using function_args_t = decltype(function_args_helper(std::declval<T>()));

template <std::size_t I, class T>
using function_arg_n_t = std::tuple_element_t<I, function_args_t<T>>;

// https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp

template <class T>
struct ranged_iterator {
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using pointer = T*;
  using reference = T&;

  constexpr ranged_iterator(pointer ptr, difference_type diff)
      : m_ptr{ptr}, m_diff{diff} {}

  constexpr reference operator*() const { return *m_ptr; }

  constexpr pointer operator->() { return m_ptr; }

  constexpr ranged_iterator& operator++() {
    m_ptr += m_diff;
    return *this;
  }
  constexpr ranged_iterator operator++(int) {
    ranged_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr friend bool operator==(const ranged_iterator& lhs,
                                   const ranged_iterator& rhs) {
    return (lhs.m_diff == rhs.m_diff) && (lhs.m_ptr == rhs.m_ptr);
  };

 private:
  pointer m_ptr;
  difference_type m_diff;
};
template <class T>
ranged_iterator(T*, std::ptrdiff_t) -> ranged_iterator<T>;

struct linked_list_iterator_tag {};

template <class Container, bool linked_list = false>
struct cyclic_iterator {
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = Container::value_type;
  using pointer = Container::pointer;
  using reference = std::conditional_t<std::is_const_v<Container>,
                                       typename Container::const_reference,
                                       typename Container::reference>;

  using container_type = Container;
  using iterator = std::conditional_t<std::is_const_v<Container>,
                                      typename Container::const_iterator,
                                      typename Container::iterator>;
  using const_iterator = Container::const_iterator;

  constexpr cyclic_iterator(container_type&&, iterator it) = delete;
  constexpr cyclic_iterator(container_type& container, iterator it)
      : m_begin{std::begin(container)},
        m_end{std::end(container)},
        m_it{it},
        m_size{std::distance(m_begin, m_end)} {
    if (m_size == 0) {
      throw std::runtime_error("No point having an empty cyclic operator");
    }
  }

  constexpr cyclic_iterator(linked_list_iterator_tag, container_type&&,
                            iterator) = delete;
  constexpr cyclic_iterator(linked_list_iterator_tag, container_type& container,
                            iterator it)
      : cyclic_iterator{container, it} {}

  constexpr cyclic_iterator(container_type&& container) = delete;
  constexpr cyclic_iterator(container_type& container)
      : cyclic_iterator{container, std::begin(container)} {}

  constexpr cyclic_iterator(linked_list_iterator_tag,
                            container_type&&) = delete;
  constexpr cyclic_iterator(linked_list_iterator_tag, container_type& container)
      : cyclic_iterator{linked_list_iterator_tag{}, container,
                        std::begin(container)} {}

  constexpr reference operator*() const { return *m_it; }
  constexpr pointer operator->() { return m_it.operator->(); }

  constexpr cyclic_iterator& operator+=(difference_type diff) {
    if (diff == 0) {
      return *this;
    }
    const auto size = [this]() {
      if constexpr (linked_list) {
        return m_size - 1;
      } else {
        return m_size;
      }
    }();
    auto pos = std::distance(m_begin, m_it);
    pos = (size + (pos + (diff % size))) % size;
    m_it = m_begin + pos;

    return *this;
  }
  constexpr friend cyclic_iterator operator+(cyclic_iterator lhs,
                                             difference_type diff) {
    lhs += diff;
    return lhs;
  }

  constexpr cyclic_iterator& operator-=(difference_type diff) {
    return this->operator+=(-diff);
  }
  constexpr friend cyclic_iterator operator-(cyclic_iterator lhs,
                                             difference_type diff) {
    lhs -= diff;
    return lhs;
  }

  constexpr cyclic_iterator& operator++() {
    ++m_it;
    if (m_it == m_end) {
      m_it = m_begin;
      if constexpr (linked_list) {
        ++m_it;
      }
    }
    return *this;
  }
  constexpr cyclic_iterator operator++(int) {
    cyclic_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr cyclic_iterator& operator--() {
    if (m_it == m_begin) {
      m_it = m_end;
      if constexpr (linked_list) {
        --m_it;
      }
    }
    --m_it;
    if constexpr (linked_list) {
      if (m_it == m_begin) {
        m_it = m_end - 1;
      }
    }
    return *this;
  }
  constexpr cyclic_iterator operator--(int) {
    cyclic_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr friend bool operator==(const cyclic_iterator& lhs,
                                   const cyclic_iterator& rhs) {
    return (lhs.m_it == rhs.m_it) && (lhs.m_begin == rhs.m_begin) &&
           (lhs.m_end == rhs.m_end) && (lhs.m_size == rhs.m_size);
  };

  constexpr iterator to_underlying() const { return m_it; }
  constexpr operator iterator() const { return this->to_underlying(); }

 private:
  iterator m_begin;
  iterator m_end;
  iterator m_it;
  difference_type m_size;
};
template <class Container>
cyclic_iterator(Container&) -> cyclic_iterator<Container>;
template <class Container>
cyclic_iterator(const Container&) -> cyclic_iterator<const Container>;
template <class Container>
cyclic_iterator(Container&, typename Container::iterator)
    -> cyclic_iterator<Container>;
template <class Container>
cyclic_iterator(const Container&, typename Container::iterator)
    -> cyclic_iterator<const Container>;
template <class Container>
cyclic_iterator(Container&, typename Container::const_iterator)
    -> cyclic_iterator<Container>;
template <class Container>
cyclic_iterator(const Container&, typename Container::const_iterator)
    -> cyclic_iterator<const Container>;
template <class Container>
cyclic_iterator(linked_list_iterator_tag, Container&)
    -> cyclic_iterator<Container, true>;
template <class Container>
cyclic_iterator(linked_list_iterator_tag, const Container&)
    -> cyclic_iterator<const Container, true>;
template <class Container>
cyclic_iterator(linked_list_iterator_tag, Container&,
                typename Container::iterator)
    -> cyclic_iterator<Container, true>;
template <class Container>
cyclic_iterator(linked_list_iterator_tag, const Container&,
                typename Container::iterator)
    -> cyclic_iterator<const Container, true>;
template <class Container>
cyclic_iterator(linked_list_iterator_tag, Container&,
                typename Container::const_iterator)
    -> cyclic_iterator<Container, true>;
template <class Container>
cyclic_iterator(linked_list_iterator_tag, const Container&,
                typename Container::const_iterator)
    -> cyclic_iterator<const Container, true>;

template <class map_iterator>
struct map_value_iterator {
 protected:
  using map_iterator_t = map_iterator;
  using map_value_t = map_iterator_t::value_type;

 public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = decltype(std::declval<map_value_t>().second);
  using pointer = value_type*;
  using reference = value_type&;

  constexpr map_value_iterator(map_iterator map_it) : m_map_it{map_it} {}

  constexpr reference operator*() const { return m_map_it->second; }

  constexpr pointer operator->() { return &m_map_it->second; }

  constexpr map_value_iterator& operator++() {
    ++m_map_it;
    return *this;
  }
  constexpr map_value_iterator operator++(int) {
    map_value_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr friend bool operator==(const map_value_iterator& lhs,
                                   const map_value_iterator& rhs) {
    return lhs.m_map_it == rhs.m_map_it;
  };

 private:
  map_iterator_t m_map_it;
};
template <class map_iterator>
map_value_iterator(map_iterator) -> map_value_iterator<map_iterator>;

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

template <class T = int>
constexpr std::function<T(T, T)> get_binary_op(char op) {
  switch (op) {
    case '+':
      return std::plus{};
    case '*':
      return std::multiplies{};
    case '-':
      return std::minus{};
    case '/':
      return std::divides{};
    default:
      throw std::runtime_error("Invalid operation " + std::string{op});
  }
}

template <class T = int>
constexpr std::function<T(T, T)> get_inverse_binary_op(char op) {
  switch (op) {
    case '+':
      return std::minus{};
    case '*':
      return std::divides{};
    case '-':
      return std::plus{};
    case '/':
      return std::multiplies{};
    default:
      throw std::runtime_error("Invalid operation " + std::string{op});
  }
}

template <class T>
constexpr std::function<T(T, T)> get_constant_binary_op(T value) {
  return [=](T, T) { return value; };
};

template <std::integral T>
constexpr T sign(T value) {
  if (value == 0) {
    return 0;
  }
  return value / std::abs(value);
}
template <std::floating_point T>
constexpr T sign(T value) {
  return std::copysign(T{1.0}, value);
}

struct custom_divides {
  template <class T>
  constexpr T operator()(const T& lhs, const T& rhs) const {
    if constexpr (std::integral<T>) {
      const auto remainder = (lhs % rhs);
      if (remainder != 0) {
        std::cout << "Remainder of " << remainder << " for dividing " << lhs
                  << " by " << rhs << std::endl;
      }
    }
    return lhs / rhs;
  }
};

template <std::integral T>
struct fractional_t {
 public:
  using value_type = T;
  using difference_type = std::ptrdiff_t;

  using value_point = point_t<value_type>;

  constexpr fractional_t(value_type integral = 0)
      : fractional_t{integral, 0, 1} {}

  constexpr fractional_t(value_type numerator, value_type denominator)
      : fractional_t{numerator / denominator, numerator % denominator,
                     denominator} {}

  constexpr friend bool operator==(const fractional_t&,
                                   const fractional_t&) = default;

  constexpr fractional_t operator+() const { return *this; }

  constexpr fractional_t operator-() const {
    return {-m_integral, -m_numerator, m_denominator};
  }

  constexpr fractional_t& operator+=(const fractional_t& rhs) {
    m_integral += rhs.m_integral;
    auto lcd = std::lcm(m_denominator, rhs.m_denominator);
    auto multiplied_numerators =
        value_point{m_numerator * (lcd / m_denominator),
                    rhs.m_numerator * (lcd / rhs.m_denominator)};
    m_numerator = multiplied_numerators.x + multiplied_numerators.y;
    m_denominator = lcd;
    this->simplify();
    return *this;
  }
  constexpr friend fractional_t operator+(fractional_t lhs,
                                          const fractional_t& rhs) {
    lhs += rhs;
    return lhs;
  }

  constexpr fractional_t& operator-=(const fractional_t& rhs) {
    return this->operator+=(-rhs);
  }
  constexpr friend fractional_t operator-(fractional_t lhs,
                                          const fractional_t& rhs) {
    lhs -= rhs;
    return lhs;
  }

  constexpr fractional_t& operator*=(const fractional_t& rhs) {
    fractional_t plus_lhs = fractional_t{rhs}.multiply_integral(m_integral);
    fractional_t plus_rhs =
        (fractional_t{rhs.m_integral, m_denominator} +
         fractional_t{rhs.m_numerator, m_denominator * rhs.m_denominator})
            .multiply_integral(m_numerator);
    *this = plus_lhs + plus_rhs;
    return *this;
  }
  constexpr friend fractional_t operator*(fractional_t lhs,
                                          const fractional_t& rhs) {
    lhs *= rhs;
    return lhs;
  }

  constexpr fractional_t& operator/=(const fractional_t& rhs) {
    return this->operator*=(
        fractional_t{rhs.m_denominator,
                     rhs.m_denominator * rhs.m_integral + rhs.m_numerator});
  }
  constexpr friend fractional_t operator/(fractional_t lhs,
                                          const fractional_t& rhs) {
    lhs /= rhs;
    return lhs;
  }

  template <std::floating_point F>
  constexpr explicit operator F() const {
    return static_cast<F>(m_integral) +
           static_cast<F>(m_numerator) / static_cast<F>(m_denominator);
  }

  constexpr explicit operator value_type() const { return m_integral; }

  constexpr std::array<value_type, 3> to_array() const {
    return {m_integral, m_numerator, m_denominator};
  }

 protected:
  constexpr fractional_t& multiply_integral(value_type integral) {
    m_integral *= integral;
    m_numerator *= integral;
    this->simplify();
    return *this;
  }

  constexpr fractional_t(value_type integral, value_type numerator,
                         value_type denominator)
      : m_integral{integral},
        m_numerator{numerator},
        m_denominator{denominator} {
    AOC_ASSERT(m_denominator > 0, "Denominator must be positive");
  }

  constexpr void simplify() {
    AOC_ASSERT(m_denominator > 0, "Denominator must be positive");
    auto gcd = std::gcd(m_numerator, m_denominator);
    AOC_ASSERT(gcd != 0, "gcd cannot be zero");
    m_numerator /= gcd;
    m_denominator /= gcd;
    m_integral += m_numerator / m_denominator;
    m_numerator = m_numerator % m_denominator;
  }

 private:
  value_type m_integral{0};
  value_type m_numerator{0};
  value_type m_denominator{1};
};

template <class T, class SizeT = T>
struct range_type {
  T origin;
  SizeT size;

  constexpr T end() const { return origin + size; }

  constexpr bool contains(const range_type& other) const {
    return (other.origin >= origin) && (other.end() <= end());
  }

  constexpr bool overlaps_with(const range_type& other) const {
    return (origin < other.end()) && (end() > other.origin);
  }

  bool operator==(const range_type&) const = default;

  friend constexpr bool operator<(const range_type& lhs,
                                  const range_type& rhs) {
    return lhs.origin < rhs.origin;
  }

  friend std::ostream& operator<<(std::ostream& out, const range_type& range) {
    out << '[' << range.origin << ',' << (range.end() - 1) << ']';
    return out;
  }
};

namespace ranges {

// https://en.cppreference.com/w/cpp/algorithm/ranges/contains
struct __contains_fn {
  template <std::input_iterator I, std::sentinel_for<I> S, class T,
            class Proj = std::identity>
    requires std::indirect_binary_predicate<std::ranges::equal_to,
                                            std::projected<I, Proj>, const T*>
  constexpr bool operator()(I first, S last, const T& value,
                            Proj proj = {}) const {
    return std::ranges::find(std::move(first), last, value, proj) != last;
  }

  template <std::ranges::input_range R, class T, class Proj = std::identity>
    requires std::indirect_binary_predicate<
        std::ranges::equal_to, std::projected<std::ranges::iterator_t<R>, Proj>,
        const T*>
  constexpr bool operator()(R&& r, const T& value, Proj proj = {}) const {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(value),
                   proj);
  }
};
inline constexpr __contains_fn contains{};

// https://en.cppreference.com/w/cpp/algorithm/ranges/fold_left
struct fold_left_fn {
  template <std::input_iterator I, std::sentinel_for<I> S, class T, class F>
  constexpr auto operator()(I first, S last, T init, F f) const {
    using U =
        std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<I>>>;
    if (first == last) {
      return U(std::move(init));
    }
    U accum = std::invoke(f, std::move(init), *first);
    for (++first; first != last; ++first) {
      accum = std::invoke(f, std::move(accum), *first);
    }
    return std::move(accum);
  }

  template <std::ranges::input_range R, class T, class F>
  constexpr auto operator()(R&& r, T init, F f) const {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(init),
                   std::ref(f));
  }
};
inline constexpr fold_left_fn fold_left;

struct lcm_fn {
  template <std::input_iterator I, std::sentinel_for<I> S>
  constexpr auto operator()(I first, S last) const {
    using T = std::iter_value_t<I>;
    return fold_left(first, last, T(1), [&](T first, T second) {
      return std::lcm(first, second);
    });
  }

  template <std::ranges::input_range R>
  constexpr auto operator()(R&& r) const {
    return (*this)(std::ranges::begin(r), std::ranges::end(r));
  }
};
inline constexpr lcm_fn lcm;

struct gcd_fn {
  template <std::input_iterator I, std::sentinel_for<I> S>
  constexpr auto operator()(I first, S last) const {
    using T = std::iter_value_t<I>;
    return fold_left(first, last, T(1), [&](T first, T second) {
      return std::gcd(first, second);
    });
  }

  template <std::ranges::input_range R>
  constexpr auto operator()(R&& r) const {
    return (*this)(std::ranges::begin(r), std::ranges::end(r));
  }
};
inline constexpr gcd_fn gcd;

} // namespace ranges

template <std::ranges::random_access_range R, class Comp = std::ranges::less,
          class Proj = std::identity>
constexpr auto sorted_range(R&& r, Comp comp = {}, Proj proj = {}) {
  auto r_copy = r;
  std::ranges::sort(r_copy, comp, proj);
  return r_copy;
}

template <std::integral T>
constexpr T num_digits(T n) {
  return std::abs(static_cast<std::make_signed_t<T>>(n)) >= 10
             ? num_digits(n / 10) + 1
             : 1;
}

// https://stackoverflow.com/a/59420788/793006
template <typename T>
constexpr T pown(T x, unsigned p) {
  T result = 1;

  while (p > 0) {
    if (p & 0x1) {
      result *= x;
    }
    x *= x;
    p >>= 1;
  }

  return result;
}

constexpr bool is_number(char c) { return (c >= '0') && (c <= '9'); }
