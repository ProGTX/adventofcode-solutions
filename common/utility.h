#ifndef AOC_UTILITY_H
#define AOC_UTILITY_H

#include "assert.h"
#include "compiler.h"
#include "concepts.h"

#ifndef AOC_MODULE_SUPPORT
#include <algorithm>
#include <charconv>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <optional>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#endif

AOC_EXPORT_NAMESPACE(aoc) {

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

template <int V>
using int_constant = std::integral_constant<int, V>;

struct sorted_unique_t {
  explicit constexpr sorted_unique_t() = default;
};
inline constexpr sorted_unique_t sorted_unique{};

template <class output_t>
constexpr size_t max_container_elems() {
  if constexpr (requires {
                  std::tuple_size<std::remove_cvref_t<output_t>>::value;
                }) {
    return std::tuple_size<std::remove_cvref_t<output_t>>::value;
  } else {
    return std::string::npos;
  }
}
static_assert(4 == max_container_elems<std::array<int, 4>>());

template <class output_t>
constexpr auto inserter_it(output_t& elems) {
  if constexpr (insertable<output_t>) {
    return std::inserter(elems, std::end(elems));
  } else if constexpr (back_insertable<output_t>) {
    return std::back_inserter(elems);
  } else if constexpr (specialization_of<output_t, std::optional>) {
    return std::addressof(elems);
  } else {
    return std::begin(elems);
  }
}

template <class output_t>
constexpr auto insertion_end_it(output_t& elems) {
  if constexpr (specialization_of<output_t, std::optional>) {
    return std::addressof(elems) + 1;
  } else if constexpr (const auto N = max_container_elems<output_t>();
                       N != std::string::npos) {
    return std::begin(elems) + N;
  } else {
    return std::unreachable_sentinel;
  }
}

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
    return (lhs.m_it == rhs.m_it) &&
           (lhs.m_begin == rhs.m_begin) &&
           (lhs.m_end == rhs.m_end) &&
           (lhs.m_size == rhs.m_size);
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

template <class T>
struct arity : public std::integral_constant<int, 1> {};

template <class T>
constexpr inline auto arity_v = arity<T>::value;

template <class T>
struct closed_range {
  T begin;
  T end;

  constexpr closed_range(T first_, T last_)
      : begin{std::move(first_)}, end{std::move(last_)} {
    if (end < begin) {
      // Ensure the range is ordered
      std::swap(begin, end);
    }
  }

  constexpr T direction() const { return end - begin; }

  constexpr bool contains(const closed_range& other) const
    requires(arity_v<T> == 1)
  {
    return (other.begin >= begin) && (other.end <= end);
  }

  // Given three collinear points p, q, r, the function checks if
  // point q lies on line segment 'pr'
  constexpr bool contains(const T& q) const
    requires(arity_v<T> >= 2)
  {
    bool contains = get<0>(q) <= std::ranges::max(get<0>(begin), get<0>(end)) &&
                    get<0>(q) >= std::ranges::min(get<0>(begin), get<0>(end)) &&
                    get<1>(q) <= std::ranges::max(get<1>(begin), get<1>(end)) &&
                    get<1>(q) >= std::ranges::min(get<1>(begin), get<1>(end));
    if constexpr (arity_v<T> > 2) {
      contains = contains &&
                 get<2>(q) <= std::ranges::max(get<2>(begin), get<2>(end)) &&
                 get<2>(q) >= std::ranges::min(get<2>(begin), get<2>(end));
    }
    return contains;
  }

  constexpr bool overlaps_with(const closed_range& other) const {
    const bool overlaps = (begin <= other.end) && (end >= other.begin);
    if constexpr (arity_v<T> == 2) {
      return overlaps && overlaps_with_2d(other);
    } else if constexpr (arity_v<T> == 3) {
      return overlaps && overlaps_with_3d(other);
    } else {
      return overlaps;
    }
  }

  // https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
  constexpr bool overlaps_with_2d(const closed_range& other) const
    requires(arity_v<T> == 2)
  {
    const auto& p1 = begin;
    const auto& q1 = end;
    const auto& p2 = other.begin;
    const auto& q2 = other.end;

    // Find the four orientations needed for general and special cases
    auto o1 = orientation(p1, q1, p2);
    auto o2 = orientation(p1, q1, q2);
    auto o3 = orientation(p2, q2, p1);
    auto o4 = orientation(p2, q2, q1);

    // General case
    if ((o1 != o2) && (o3 != o4)) {
      return true;
    }

    // Special Cases

    // p1, q1 and p2 are collinear and p2 lies on segment p1q1
    if ((o1 == 0) && contains(p2)) {
      return true;
    }
    // p1, q1 and q2 are collinear and q2 lies on segment p1q1
    if ((o2 == 0) && contains(q2)) {
      return true;
    }
    // p2, q2 and p1 are collinear and p1 lies on segment p2q2
    if ((o3 == 0) && other.contains(p1)) {
      return true;
    }
    // p2, q2 and q1 are collinear and q1 lies on segment p2q2
    if ((o4 == 0) && other.contains(q1)) {
      return true;
    }

    return false; // Doesn't fall in any of the above cases
  }

  // https://stackoverflow.com/a/63288956
  // https://forum.unity.com/threads/line-intersection.17384/
  constexpr bool overlaps_with_3d(const closed_range& other) const
    requires(arity_v<T> == 3)
  {
    const auto dir = direction();
    const auto other_dir = other.direction();

    // Handle single points
    if (dir == T{}) {
      return other.contains(begin);
    } else if (other_dir == T{}) {
      return contains(other.begin);
    }

    const auto begin_diff = other.begin - begin;
    const auto directions_cross = dir.cross(other_dir);
    const auto planar_factor = begin_diff.dot(directions_cross);
    const auto dirs_cross_sqr_magnitude = directions_cross.sqr_magnitude();

    const bool coplanar = (abs(planar_factor) == 0);
    const bool parallel = (dirs_cross_sqr_magnitude == 0);
    const bool lines_intersect = coplanar && !parallel;
    if (!lines_intersect) {
      return false;
    }

    const auto begin_cross_dir_other = begin_diff.cross(other_dir);
    const auto crossing_point_factor =
        begin_cross_dir_other.dot(directions_cross) /
        static_cast<float>(dirs_cross_sqr_magnitude);
    return (crossing_point_factor >= 0) && (crossing_point_factor <= 1);
  }

  constexpr bool operator==(const closed_range&) const = default;
  constexpr auto operator<=>(const closed_range&) const = default;

  friend std::ostream& operator<<(std::ostream& out,
                                  const closed_range& range) {
    out << '[' << range.begin << ',' << range.end << ']';
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

template <class T>
struct not_equal_to_value {
  T value;

  template <std::equality_comparable_with<T> U>
  constexpr bool operator()(U&& other) const {
    return value != other;
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

template <class value_type = int>
constexpr auto to_number_with_rest(std::string_view str, int base = 10) {
  auto first = str.data();
  auto last = first + str.size();
  value_type value;
  auto result = std::from_chars(first, last, value, base);
  if (result.ec != std::errc{}) [[unlikely]] {
    throw std::runtime_error("to_number failed to parse " + std::string(str));
  }
  return std::pair<value_type, std::string_view>{
      value, str.substr(std::distance(&str[0], result.ptr))};
}

template <class value_type = int>
constexpr auto to_number(std::string_view str, int base = 10) {
  return to_number_with_rest<value_type>(str, base).first;
}
template <class value_type = int>
constexpr auto to_number(char c) {
  return static_cast<value_type>(c - '0');
}
template <class value_type = int>
constexpr auto to_number(char c, int base) {
  return to_number<value_type>(std::string_view{&c, 1}, base);
}

template <class T>
struct number_converter {
  int base = 10;

  constexpr number_converter() = default;
  constexpr number_converter(int base) : base{base} {}

  constexpr T operator()(std::string_view str) const {
    return to_number<T>(str, base);
  }
  constexpr T operator()(char c) const {
    //
    return to_number<T>(c, base);
  }
  template <std::integral U = T>
  constexpr U operator()(std::integral auto value) const {
    return static_cast<U>(value);
  }
};

constexpr auto pop_stack(specialization_of<std::vector> auto&& container) {
  auto elem = std::move(container.back());
  container.resize(container.size() - 1);
  return elem;
}

template <class T>
struct constant_value {
  using value_type = T;
  value_type value;

  template <class... Args>
  constexpr value_type& operator()(Args&&...) {
    return value;
  }
  template <class... Args>
  constexpr const value_type& operator()(Args&&...) const {
    return value;
  }
};

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_UTILITY_H
