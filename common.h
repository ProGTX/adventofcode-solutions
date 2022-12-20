#pragma once

#include <array>
#include <concepts>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

template <class T>
struct inspect_t;

template <auto V>
struct inspect_v;

// https://stackoverflow.com/a/51032862
template <class, template <class...> class>
inline constexpr bool is_specialization = false;
template <template <class...> class T, class... Args>
inline constexpr bool is_specialization<T<Args...>, T> = true;

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
constexpr std::string_view trim_leave_spaces(std::string_view str) {
  return trim(str, "\t\n\r\f\v");
}

template <class trim_op_t = decltype([](std::string_view str) {
            return trim(str);
          }),
          class OpT>
    requires std::invocable<OpT, std::string_view, int> ||
    std::invocable<OpT, std::string_view> void readfile_op(
        const std::string& filename, OpT operation) {
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
requires std::invocable<FirstLineOpT, std::string_view>&&
    std::invocable<OpT, std::string_view> void
    readfile_op_header(const std::string& filename,
                       FirstLineOpT first_line_operation, OpT operation) {
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
  if constexpr (is_array_class_v<output_t>) {
    return std::begin(elems);
  } else {
    return std::back_insert_iterator(elems);
  }
}

template <class output_t>
constexpr auto split_item_op() {
  if constexpr (is_array_class_v<output_t> ||
                is_specialization<output_t, std::vector>) {
    if constexpr (std::is_same_v<typename output_t::value_type, int>) {
      return [](auto&& item) { return std::stoi(item); };
    } else {
      return std::identity{};
    }
  } else {
    return std::identity{};
  }
}

template <class output_t>
output_t split(const std::string& input, char delimiter) {
  output_t elems;
  split_line_to_iterator(input, delimiter, inserter_it(elems),
                         split_item_op<output_t>());
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

template <class data_t_, class row_t_ = data_t_>
class grid {
 public:
  using row_t = row_t_;
  using data_t = data_t_;
  using value_type = typename data_t::value_type;
  using iterator = typename data_t::iterator;
  using const_iterator = typename data_t::const_iterator;

  constexpr iterator add_row(const row_t& row) {
    m_row_length = row.size();
    auto old_size = (m_row_length * m_num_rows);
    auto it = inserter_it(m_data);
    if constexpr (is_specialization<data_t, std::vector>) {
      m_data.reserve(old_size + m_row_length);
    } else if (is_array_class_v<data_t>) {
      it += old_size;
    }
    std::ranges::copy_n(std::begin(row), m_row_length, it);
    ++m_num_rows;
    return std::begin(m_data) + old_size;
  }

  constexpr auto size() const { return m_data.size(); }

  constexpr const data_t& data() const { return m_data; }

  constexpr int row_length() const { return m_row_length; }

  constexpr int num_rows() const { return m_num_rows; }

  constexpr int linear_index(int row, int column) const {
    return row * m_row_length + column;
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
  void print_all(print_single_ft print_single_f = {}) const {
    for (int i = 0; i < m_num_rows; ++i) {
      std::cout << "  ";
      for (int j = 0; j < row_length(); ++j) {
        int index = (i * row_length() + j);
        get_print_single_f(print_single_f)(std::cout, index);
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

 private:
  template <class print_single_ft>
  constexpr auto get_print_single_f(print_single_ft print_single_f) const {
    if constexpr (std::is_same_v<print_single_ft, std::identity>) {
      return [this](std::ostream& out, int index) { out << m_data[index]; };
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

std::ostream& print_range(const std::ranges::range auto range,
                          std::string_view separator = ",",
                          std::ostream& out = std::cout) {
  for (const auto& item : range) {
    out << item << separator;
  }
  return out;
}

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

  friend std::ostream& operator<<(std::ostream& out, const point& p) {
    out << "{" << p.x << "," << p.y << "}";
    return out;
  }

  constexpr point abs() const { return {std::abs(x), std::abs(y)}; }

  constexpr static long distance_squared(const point& lhs, const point& rhs) {
    auto diff = rhs - lhs;
    return (diff.x * diff.x) + (diff.y * diff.y);
  }
};

// https://stackoverflow.com/a/67687348/793006
template <std::size_t I, class... Ts>
void print_tuple(std::ostream& out, const std::tuple<Ts...>& tuple) {
  if constexpr (I == sizeof...(Ts)) {
    out << ')';
  } else {
    out << std::get<I>(tuple);
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
class sorted_flat_set {
 private:
  using data_t = std::vector<T>;

 public:
  using value_type = data_t::value_type;
  using iterator = data_t::iterator;
  using const_iterator = data_t::const_iterator;

  constexpr sorted_flat_set() : sorted_flat_set{Compare{}} {}

  constexpr explicit sorted_flat_set(const Compare& comp)
      : m_comparator{comp} {}

  constexpr iterator begin() noexcept { return m_data.begin(); }
  constexpr const_iterator begin() const noexcept { return m_data.begin(); }

  constexpr iterator end() noexcept { return m_data.end(); }
  constexpr const_iterator end() const noexcept { return m_data.end(); }

  constexpr iterator find(const value_type& value) {
    return std::ranges::find(m_data, value);
  }

  constexpr const_iterator find(const value_type& value) const {
    return std::ranges::find(m_data, value);
  }

  constexpr bool contains(const T& value) const {
    return this->find(value) != this->end();
  }

  constexpr size_t size() const noexcept { return m_data.size(); }

  [[nodiscard]] constexpr bool empty() const noexcept { return m_data.empty(); }

  constexpr size_t erase(const T& value) {
    auto it = this->find(value);
    if (it == this->end()) {
      return 0;
    }
    m_data.erase(it);
    return 1;
  }

  template <class value_t = value_type>
  constexpr std::pair<iterator, bool> insert(value_t&& value) {
    auto it = this->find(value);
    if (it != this->end()) {
      return {it, false};
    }
    m_data.push_back(value);
    std::ranges::sort(m_data, m_comparator);
    return {this->find(value), true};
  }

  template <class... Args>
  constexpr std::pair<iterator, bool> emplace(Args&&... args) {
    auto value = T{std::forward<Args>(args)...};
    return this->insert(std::move(value));
  }

 private:
  data_t m_data;
  Compare m_comparator;
};
