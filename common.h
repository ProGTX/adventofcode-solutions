#pragma once

#include <array>
#include <charconv>
#include <concepts>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(NDEBUG)
#if defined(__assume) || defined(_MSC_VER)
#define AOC_ASSERT_HELPER(condition, message) __assume(condition)
#elif defined(__has_builtin)
#if __has_builtin(__builtin_assume)
#define AOC_ASSERT_HELPER(condition, message) __builtin_assume(condition)
#else // __builtin_assume not available
#define AOC_ASSERT_HELPER(condition, message) ((void)0)
#endif // __has_builtin(__builtin_assume)
#else  // __assume not available
#define AOC_ASSERT_HELPER(condition, message) ((void)0)
#endif // __assume  || _MSC_VER
#else
#include <cassert>
#define AOC_ASSERT_HELPER(condition, message) assert((condition) && (message))
#endif // NDEBUG

#define AOC_ASSERT(condition, message) AOC_ASSERT_HELPER((condition), (message))

template <class T>
struct inspect_t;

template <auto V>
struct inspect_v;

// https://stackoverflow.com/a/51032862
// https://open-std.org/JTC1/SC22/WG21/docs/papers/2020/p2098r1.pdf
template <class T, template <class...> class Primary>
struct is_specialization_of : std::false_type {};
template <template <class...> class Primary, class... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type {};
template <class T, template <class...> class Primary>
inline constexpr bool is_specialization_of_v =
    is_specialization_of<T, Primary>::value;

template <class T, template <class...> class Primary>
concept specialization_of = is_specialization_of_v<T, Primary>;

template <class Container>
concept insertable = requires(Container c,
                              typename Container::value_type value) {
  c.begin();
  c.end();
  c.insert(value);
};

template <class Container>
concept back_insertable = requires(Container c,
                                   typename Container::value_type value) {
  c.begin();
  c.end();
  c.push_back(value);
};

template <class Container>
concept has_value_type = requires(Container c) {
  typename Container::value_type;
};

template <class T>
struct is_array_class : std::false_type {};
template <class T, std::size_t N>
struct is_array_class<std::array<T, N>> : std::true_type {};
template <class T>
inline constexpr bool is_array_class_v = is_array_class<T>::value;

// https://stackoverflow.com/a/63050738/793006
constexpr std::string_view ltrim(std::string_view str,
                                 std::string_view whitespace) {
  const auto pos(str.find_first_not_of(whitespace));
  str.remove_prefix(std::min(pos, str.length()));
  return str;
}
constexpr std::string_view rtrim(std::string_view str,
                                 std::string_view whitespace) {
  const auto pos(str.find_last_not_of(whitespace));
  str.remove_suffix(std::min(str.length() - pos - 1, str.length()));
  return str;
}
constexpr std::string_view trim(std::string_view str,
                                std::string_view whitespace = " \t\n\r\f\v") {
  str = ltrim(str, whitespace);
  str = rtrim(str, whitespace);
  return str;
}
constexpr std::string_view trim_simple(std::string_view str) {
  return trim(str);
}

template <class return_t = std::string_view>
constexpr auto get_trimmer() {
  return [](std::string_view str) { return return_t{trim(str)}; };
};
template <class return_t = std::string_view>
constexpr auto get_trimmer_no_spaces() {
  return [](std::string_view str) { return return_t{trim(str, "\t\n\r\f\v")}; };
};

template <class trim_op_t = decltype(get_trimmer()), class OpT>
requires(
    std::invocable<OpT, std::string_view, int> ||
    std::invocable<OpT, std::string_view>) void readfile_op(const std::string&
                                                                filename,
                                                            OpT operation) {
  std::ifstream file{filename};
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file " + filename);
  }
  std::string line;
  for (int linenum = 1; std::getline(file, line); ++linenum) {
    if constexpr (std::invocable<OpT, std::string_view, int>) {
      operation(trim_op_t{}(line), linenum);
    } else {
      operation(trim_op_t{}(line));
    }
  }
  file.close();
}

template <class FirstLineOpT, class OpT>
requires(std::invocable<FirstLineOpT, std::string_view>&& std::invocable<
         OpT,
         std::string_view>) void readfile_op_header(const std::string& filename,
                                                    FirstLineOpT
                                                        first_line_operation,
                                                    OpT operation) {
  readfile_op(filename, [&](std::string_view line, int linenum) {
    if (linenum == 1) {
      first_line_operation(line);
    } else {
      operation(line);
    }
  });
}

std::vector<int> readfile_numbers(const std::string& filename) {
  std::vector<int> numbers;
  readfile_op(filename, [&](std::string_view line) {
    if (line.empty()) {
      return;
    }
    int currentVal = std::stoi(std::string{line});
    numbers.push_back(currentVal);
  });
  return numbers;
}

// https://stackoverflow.com/a/236803
template <class out_it_t, class item_op_t = std::identity>
void split_line_to_iterator(const std::string& input, char delimiter,
                            out_it_t outputIt, item_op_t item_op = {}) {
  std::stringstream stream{input};
  for (std::string item; std::getline(stream, item, delimiter);) {
    if (item.empty()) {
      continue;
    }
    *outputIt = item_op(std::move(item));
    // Must increase as last step, in case an item was skipped
    ++outputIt;
  }
}

template <class output_t>
constexpr auto inserter_it(output_t& elems) {
  if constexpr (insertable<output_t>) {
    return std::inserter(elems, std::end(elems));
  } else if constexpr (back_insertable<output_t>) {
    return std::back_inserter(elems);
  } else {
    return std::begin(elems);
  }
}

template <class output_t, class string_item_op_t = std::identity>
constexpr auto split_item_op() {
  if constexpr (has_value_type<output_t>) {
    using value_type = typename output_t::value_type;
    if constexpr (std::integral<value_type>) {
      return [](auto&& item) {
        auto item_transformed = string_item_op_t{}(item);
        auto first = item_transformed.data();
        auto last = first + item_transformed.size();
        value_type value;
        auto result = std::from_chars(first, last, value);
        if (result.ec != std::errc{}) {
          throw std::runtime_error("Failed to parse " +
                                   std::string(result.ptr));
        }
        return value;
      };
    } else {
      return string_item_op_t{};
    }
  } else {
    return string_item_op_t{};
  }
}

template <class output_t, class string_item_op_t = std::identity>
output_t split(const std::string& input, char delimiter) {
  output_t elems;
  split_line_to_iterator(input, delimiter, inserter_it(elems),
                         split_item_op<output_t, string_item_op_t>());
  return elems;
}

// https://en.cppreference.com/w/cpp/utility/to_underlying
template <class Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept {
  return static_cast<std::underlying_type_t<Enum>>(e);
}

// https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
template <class T>
struct ranged_iterator {
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using pointer = T*;
  using reference = T&;

  ranged_iterator(pointer ptr, difference_type diff)
      : m_ptr{ptr}, m_diff{diff} {}

  reference operator*() const { return *m_ptr; }

  pointer operator->() { return m_ptr; }

  ranged_iterator& operator++() {
    m_ptr += m_diff;
    return *this;
  }
  ranged_iterator operator++(T) {
    ranged_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  friend bool operator==(const ranged_iterator& a, const ranged_iterator& b) {
    return a.m_ptr == b.m_ptr;
  };
  friend bool operator!=(const ranged_iterator& a, const ranged_iterator& b) {
    return !(a == b);
  };

 private:
  pointer m_ptr;
  difference_type m_diff;
};

template <class T>
ranged_iterator(T*, std::ptrdiff_t)->ranged_iterator<T>;

template <class T, class row_storage_t = std::vector<T>,
          class data_storage_t = row_storage_t>
class grid {
 public:
  using row_t = row_storage_t;
  using data_t = data_storage_t;
  using value_type = T;
  using iterator = typename data_t::iterator;
  using const_iterator = typename data_t::const_iterator;

  static constexpr auto static_row_length =
      is_array_class_v<row_t> ? row_t{}.size() : 0;

  static constexpr auto static_data_size =
      is_array_class_v<data_t> ? data_t{}.size() : 0;

  constexpr grid() {}

  template <class Container = data_t>
  requires((static_row_length > 0) &&
           (static_data_size >
            0)) explicit constexpr grid(const value_type& value)
      : m_row_length{this->row_length()}, m_num_rows(this->num_rows()) {
    std::ranges::fill(m_data, value);
  }

  template <class Container = data_t>
  requires requires(Container c, size_t count,
                    const typename Container::value_type& value) {
    Container(count, value);
  }
  constexpr grid(value_type value, int num_rows, int num_columns)
      : m_data(num_rows * num_columns, value),
        m_row_length{num_columns},
        m_num_rows(num_rows) {}

  constexpr iterator add_row(const row_t& row) {
    m_row_length = row.size();
    // Use m_num_rows instead of num_rows()
    // because we allow setting rows in a fixed size container
    auto old_size = (this->row_length() * m_num_rows);
    auto it = inserter_it(m_data);
    if constexpr (is_specialization_of_v<data_t, std::vector>) {
      m_data.reserve(old_size + this->row_length());
    } else if constexpr (is_array_class_v<data_t>) {
      it += old_size;
    }
    std::ranges::copy_n(std::begin(row), this->row_length(), it);
    ++m_num_rows;
    return std::begin(m_data) + old_size;
  }

  constexpr auto size() const {
    if constexpr (static_data_size > 0) {
      return static_data_size;
    } else {
      return m_data.size();
    }
  }

  constexpr const data_t& data() const { return m_data; }

  constexpr int row_length() const {
    if constexpr (static_row_length > 0) {
      return static_row_length;
    } else {
      return m_row_length;
    }
  }

  constexpr int num_rows() const {
    if constexpr ((static_data_size > 0) && (static_row_length > 0)) {
      return static_data_size / static_row_length;
    } else {
      return m_num_rows;
    }
  }

  constexpr int linear_index(int row, int column) const {
    return row * this->row_length() + column;
  }

  constexpr value_type& at(int row, int column) {
    return m_data[this->linear_index(row, column)];
  }

  constexpr const value_type& at(int row, int column) const {
    return m_data[this->linear_index(row, column)];
  }

  constexpr void modify(value_type value, int linear_index) {
    m_data[linear_index] = std::move(value);
  }

  constexpr void modify(value_type value, int row, int column) {
    this->modify(std::move(value), this->linear_index(row, column));
  }

  template <class print_single_ft = std::identity>
  void print_all(print_single_ft print_single_f = {},
                 std::ostream& out = std::cout) const {
    for (int row = 0; row < this->num_rows(); ++row) {
      out << "  ";
      for (int column = 0; column < row_length(); ++column) {
        get_print_single_f(print_single_f)(out, row, column);
      }
      out << std::endl;
    }
    out << std::endl;
  }

 private:
  template <class print_single_ft>
  constexpr auto get_print_single_f(print_single_ft print_single_f) const {
    if constexpr (std::is_same_v<print_single_ft, std::identity>) {
      return [this](std::ostream& out, int row, int column) {
        out << this->at(row, column);
      };
    } else {
      return print_single_f;
    }
  }

 protected:
  data_t m_data;

 private:
  int m_row_length = 0;
  int m_num_rows = 0;
};

template <class T, size_t row_length, size_t num_rows = row_length>
using array_grid =
    grid<T, std::array<T, row_length>, std::array<T, row_length * num_rows>>;

std::ostream& print_range(const std::ranges::range auto range,
                          std::string_view separator = ",",
                          std::ostream& out = std::cout) {
  for (const auto& item : range) {
    out << item << separator;
  }
  return out;
}

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

// https://stackoverflow.com/a/67687348/793006
template <std::size_t I, class... Ts>
void print_tuple(std::ostream& out, const std::tuple<Ts...>& tuple) {
  if constexpr (I == sizeof...(Ts)) {
    out << ')';
  } else {
    using current_t = decltype(std::get<I>(tuple));
    if constexpr (std::ranges::range<current_t> &&
                  !std::convertible_to<current_t, std::string>) {
      out << '{';
      print_range(std::get<I>(tuple), ",", out) << '}';
    } else {
      out << std::get<I>(tuple);
    }
    if constexpr (I + 1 != sizeof...(Ts)) {
      out << ",";
    }
    print_tuple<I + 1>(out, tuple);
  }
}

template <class... Types>
struct printable_tuple : public std::tuple<Types...> {
 private:
  using base_t = std::tuple<Types...>;

 public:
  using base_t::base_t;

  friend std::ostream& operator<<(std::ostream& out,
                                  const printable_tuple& tuple) {
    out << '(';
    print_tuple<0>(out, tuple);
    return out;
  }
};
template <class... Types>
printable_tuple(Types...)->printable_tuple<Types...>;

template <class T, class Compare = std::less<T>>
requires std::equality_comparable<T> class sorted_flat_set {
 private:
  using data_t = std::vector<T>;

 public:
  using value_type = data_t::value_type;
  using iterator = data_t::iterator;
  using const_iterator = data_t::const_iterator;

  constexpr sorted_flat_set() : sorted_flat_set{{}, Compare{}} {}

  constexpr explicit sorted_flat_set(const Compare& comp)
      : sorted_flat_set{{}, comp} {}

  constexpr sorted_flat_set(data_t init, const Compare& comp = Compare{})
      : m_data{init}, m_comparator{comp} {
    this->sort();
  }

  constexpr iterator begin() noexcept { return m_data.begin(); }
  constexpr const_iterator begin() const noexcept { return m_data.begin(); }

  constexpr iterator end() noexcept { return m_data.end(); }
  constexpr const_iterator end() const noexcept { return m_data.end(); }

  constexpr iterator find(
      const std::equality_comparable_with<value_type> auto& value) {
    return std::ranges::find_if(
        m_data, [&](const value_type& v) { return v == value; });
  }
  constexpr const_iterator find(
      const std::equality_comparable_with<value_type> auto& value) const {
    return std::ranges::find_if(
        m_data, [&](const value_type& v) { return v == value; });
  }

  constexpr bool contains(
      const std::equality_comparable_with<value_type> auto& value) const {
    return this->find(value) != this->end();
  }

  constexpr size_t size() const noexcept { return m_data.size(); }

  [[nodiscard]] constexpr bool empty() const noexcept { return m_data.empty(); }

  constexpr std::pair<iterator, bool> insert(
      std::equality_comparable_with<value_type> auto&& value) {
    auto it = this->find(value);
    if (it != this->end()) {
      return {it, false};
    }
    m_data.push_back(value_type{value});
    this->sort();
    return {this->find(value), true};
  }

  template <std::equality_comparable_with<value_type> value_t = value_type>
  constexpr iterator insert(const_iterator, value_t&& value) {
    auto [it, success] = this->insert(std::forward<value_t>(value));
    return it;
  }

  template <class... Args>
  constexpr std::pair<iterator, bool> emplace(Args&&... args) {
    auto value = T{std::forward<Args>(args)...};
    return this->insert(std::move(value));
  }

  constexpr std::pair<iterator, bool> update(
      std::equality_comparable_with<value_type> auto&& value) {
    auto it = this->find(value);
    if (it == this->end()) {
      m_data.push_back(value_type{value});
    } else {
      *it = value;
    }
    this->sort();
    return {this->find(value), true};
  }

  constexpr size_t erase(
      const std::equality_comparable_with<value_type> auto& value) {
    auto it = this->find(value);
    if (it == this->end()) {
      return 0;
    }
    m_data.erase(it);
    this->sort();
    return 1;
  }

  constexpr void sort() { std::ranges::sort(m_data, m_comparator); }

 private:
  data_t m_data;
  Compare m_comparator;
};

template <class T, class CRTP, class String = std::string>
class graph {
 public:
  using child_ptr_t = std::unique_ptr<CRTP>;
  using children_t = std::vector<child_ptr_t>;
  using name_t = String;

  using value_type = T;
  using iterator = typename children_t::iterator;
  using const_iterator = typename children_t::const_iterator;

  static_assert(
      !std::same_as<T, name_t>,
      "This class assumes strings are only used for names, not for values");

 protected:
  constexpr graph(CRTP* parent, name_t name, T value, bool is_leaf)
      : m_parent{parent},
        m_name{std::move(name)},
        m_value{std::move(value)},
        m_is_leaf{is_leaf} {}

 public:
  constexpr CRTP* get_parent() const { return m_parent; }

  constexpr CRTP* get_child(
      const std::equality_comparable_with<value_type> auto& value) const {
    auto it = std::ranges::find_if(m_children, [&](auto& child_ptr) {
      return (child_ptr->m_value == value);
    });
    if (it == std::end(m_children)) {
      return nullptr;
    }
    return it->get();
  }

  constexpr CRTP* get_child(
      const std::equality_comparable_with<name_t> auto& name) const {
    auto it = std::ranges::find_if(m_children, [&](auto& child_ptr) {
      return (child_ptr->m_name == name);
    });
    if (it == std::end(m_children)) {
      return nullptr;
    }
    return it->get();
  }

  constexpr CRTP* add_child(child_ptr_t child) {
    if (m_is_leaf) {
      throw std::runtime_error("Cannot add nodes to leaf");
    }
    m_children.push_back(std::move(child));
    return m_children.back().get();
  }

  constexpr iterator begin() { return std::begin(m_children); }
  constexpr const_iterator begin() const { return std::begin(m_children); }

  constexpr iterator end() { return std::end(m_children); }
  constexpr const_iterator end() const { return std::end(m_children); }

  constexpr bool is_leaf() const { return m_is_leaf; }

  std::string_view name() const { return m_name; }

  constexpr T& value() { return m_value; }
  constexpr const T& value() const { return m_value; }

  std::ostream& print(std::ostream& out = std::cout) const {
    if (this->is_leaf()) {
      out << m_value;
      return out;
    }
    out << '[';
    for (const auto& child : m_children) {
      child->print(out) << ',';
    }
    out << ']';
    return out;
  }

 private:
  children_t m_children;
  CRTP* m_parent{nullptr};
  name_t m_name;
  T m_value{};
  bool m_is_leaf{false};
};

struct min_max_helper {
  point min_value{2'000'000'000, 2'000'000'000};
  point max_value{0, 0};

  constexpr void update(const point& p) {
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
  };
};
