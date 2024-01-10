#ifndef AOC_UTILITY_H
#define AOC_UTILITY_H

#include "assert.h"
#include "compiler.h"
#include "concepts.h"

#include <algorithm>
#include <charconv>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace aoc {

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

template <class T>
struct equal_to_value {
  T value;

  template <std::equality_comparable_with<T> U>
  constexpr bool operator()(U&& other) const {
    return value == other;
  }
};

template <std::ranges::common_range Container>
constexpr Container transpose(const Container& container) {
  Container transposed_container;
  const auto num_rows = std::size(container);
  AOC_ASSERT(num_rows > 0, "Cannot transpose empty container");
  const auto num_columns = std::size(container[0]);
  AOC_ASSERT(num_columns > 0, "Cannot transpose empty container");
  transposed_container.resize(num_columns);
  for (unsigned row = 0; row < num_rows; ++row) {
    for (unsigned column = 0; column < num_columns; ++column) {
      transposed_container[column].resize(num_rows);
      transposed_container[column][row] = container[row][column];
    }
  }
  return transposed_container;
}
#if defined(AOC_COMPILER_MSVC)
static_assert(std::ranges::equal(transpose(std::vector{
                                     std::vector{1, 2, 3},
                                     std::vector{4, 5, 6},
                                     std::vector{7, 8, 9},
                                     std::vector{10, 11, 12},
                                 }),
                                 std::vector{
                                     std::vector{1, 4, 7, 10},
                                     std::vector{2, 5, 8, 11},
                                     std::vector{3, 6, 9, 12},
                                 }));
#endif
#if !defined(AOC_COMPILER_GCC)
static_assert(std::ranges::equal(transpose(std::vector<std::string>{
                                     "123",
                                     "456",
                                     "789",
                                     "ABC",
                                 }),
                                 std::vector<std::string>{
                                     "147A",
                                     "258B",
                                     "369C",
                                 }));
#endif
#if 0
// TODO
static_assert(std::ranges::equal(transpose(std::vector{
                                     std::array{1, 2, 3},
                                     std::array{4, 5, 6},
                                     std::array{7, 8, 9},
                                     std::array{10, 11, 12},
                                 }),
                                 std::vector{
                                     std::array{1, 4, 7, 10},
                                     std::array{2, 5, 8, 11},
                                     std::array{3, 6, 9, 12},
                                 }));
#endif

template <class return_t>
struct transform_cast {
  template <class T>
  constexpr return_t operator()(T&& val) {
    return static_cast<return_t>(std::forward<T>(val));
  }
};

template <class value_type>
constexpr auto to_number(std::string_view str) {
  auto first = str.data();
  auto last = first + str.size();
  value_type value;
  auto result = std::from_chars(first, last, value);
  if (result.ec != std::errc{}) [[unlikely]] {
    throw std::runtime_error("to_number failed to parse " +
                             std::string(result.ptr));
  }
  return value;
}

template <class T>
struct number_converter {
  constexpr T operator()(std::string_view str) const {
    return to_number<T>(str);
  }
};

auto pop_stack(specialization_of<std::vector> auto&& container) {
  auto elem = std::move(container.back());
  container.resize(container.size() - 1);
  return elem;
}

} // namespace aoc

#endif // AOC_UTILITY_H
