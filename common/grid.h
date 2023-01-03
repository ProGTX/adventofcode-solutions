#pragma once

#include "common.h"

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <ranges>
#include <utility>
#include <vector>

template <class T, class row_storage_t = std::vector<T>,
          class Container = row_storage_t>
class grid {
 public:
  using row_t = row_storage_t;
  using container_type = Container;
  using value_type = T;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;

  static constexpr auto static_row_length =
      is_array_class_v<row_t> ? row_t{}.size() : 0;

  static constexpr auto static_data_size =
      is_array_class_v<container_type> ? container_type{}.size() : 0;

  constexpr grid() {}

  template <class container_dependent = container_type>
  requires((static_row_length > 0) &&
           (static_data_size >
            0)) explicit constexpr grid(const value_type& value)
      : m_row_length{this->row_length()}, m_num_rows(this->num_rows()) {
    std::ranges::fill(m_data, value);
  }

  template <class container_dependent = container_type>
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
    if constexpr (is_specialization_of_v<container_type, std::vector>) {
      m_data.reserve(old_size + this->row_length());
    } else if constexpr (is_array_class_v<container_type>) {
      it += old_size;
    }
    std::ranges::copy_n(std::begin(row), this->row_length(), it);
    ++m_num_rows;
    return std::begin(m_data) + old_size;
  }

  constexpr row_t get_row(int row) const {
    row_t return_row;
    if constexpr (is_specialization_of_v<row_t, std::vector>) {
      return_row.reserve(this->row_length());
    }
    auto index = this->linear_index(row, 0);
    std::ranges::copy_n(std::begin(m_data) + index, this->row_length(),
                        inserter_it(return_row));
    return return_row;
  }

  constexpr const container_type& data() const { return m_data; }

  constexpr auto size() const {
    if constexpr (static_data_size > 0) {
      return static_data_size;
    } else {
      return m_data.size();
    }
  }

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
      for (int column = 0; column < this->row_length(); ++column) {
        this->get_print_single_f(print_single_f)(out, row, column);
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
  container_type m_data;

 private:
  int m_row_length = 0;
  int m_num_rows = 0;
};

template <class T, size_t row_length, size_t num_rows = row_length>
using array_grid =
    grid<T, std::array<T, row_length>, std::array<T, row_length * num_rows>>;
