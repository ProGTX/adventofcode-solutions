#pragma once

#include "assert.h"
#include "common.h"
#include "point.h"

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
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
    requires((static_row_length > 0) && (static_data_size > 0))
  explicit constexpr grid(const value_type& value)
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

  constexpr grid(Container&& c, int num_rows, int num_columns)
      : m_data(std::move(c)), m_row_length{num_columns}, m_num_rows(num_rows) {
    AOC_ASSERT(((num_rows * num_columns) <= m_data.size()),
               "Container is not large enough for requested number"
               "of rows and columns");
  }

  template <class Row = row_t>
    requires requires(Row row) {
      std::ranges::copy_n(std::begin(row), 0, iterator{});
    }
  constexpr iterator add_row(Row const& row) {
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

  constexpr auto size_dynamic() const { return m_data.size(); }
  constexpr auto size() const {
    if constexpr (static_data_size > 0) {
      return static_data_size;
    } else {
      return this->size_dynamic();
    }
  }

  constexpr int row_length_dynamic() const { return m_row_length; }
  constexpr int row_length() const {
    if constexpr (static_row_length > 0) {
      return static_row_length;
    } else {
      return this->row_length_dynamic();
    }
  }

  constexpr int num_rows_dynamic() const { return m_num_rows; }
  constexpr int num_rows() const {
    if constexpr ((static_data_size > 0) && (static_row_length > 0)) {
      return static_data_size / static_row_length;
    } else {
      return this->num_rows_dynamic();
    }
  }

  constexpr int linear_index(int row, int column) const {
    return row * this->row_length() + column;
  }
  constexpr point position(int linear_index) const {
    this->assert_linear_index(linear_index);
    return {linear_index % this->num_rows(), linear_index / this->num_rows()};
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

  constexpr value_type& at_index(int linear_index) {
    this->assert_linear_index(linear_index);
    return m_data[linear_index];
  }
  constexpr const value_type& at_index(int linear_index) const {
    this->assert_linear_index(linear_index);
    return m_data[linear_index];
  }

  template <class print_single_ft = std::identity>
  std::ostream& print_all(print_single_ft print_single_f = {},
                          std::ostream& out = std::cout) const {
    for (int row = 0; row < this->num_rows(); ++row) {
      out << "  ";
      for (int column = 0; column < this->row_length(); ++column) {
        this->get_print_single_f(print_single_f)(out, row, column);
      }
      out << std::endl;
    }
    out << std::endl;
    return out;
  }

  constexpr auto begin() const { return m_data.begin(); }
  constexpr auto end() const { return m_data.end(); }

  constexpr bool in_bounds(int row, int column) const {
    return (row >= 0) && (row < this->num_rows()) && (column >= 0) &&
           (column < this->row_length());
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
  constexpr void assert_linear_index([[maybe_unused]] int linear_index) const {
    AOC_ASSERT(linear_index > 0, "Index must be non-negative");
    AOC_ASSERT(linear_index < this->size(), "Index cannot be out of bounds");
    AOC_ASSERT(this->num_rows() > 0, "Cannot get position without any rows");
  };

  container_type m_data;

 private:
  int m_row_length = 0;
  int m_num_rows = 0;
};

template <class T, size_t row_length, size_t num_rows = row_length>
using array_grid =
    grid<T, std::array<T, row_length>, std::array<T, row_length * num_rows>>;

template <class Grid>
concept is_grid = std::ranges::range<Grid> && requires(Grid g) {
  g.add_row();
  g.get_row();
  g.linear_index();
  g.at();
  g.modify();
  g.print_all();
};

enum facing_t : int {
  east = 0,
  south = 1,
  west = 2,
  north = 3,
  southeast = 4,
  southwest = 5,
  northwest = 6,
  northeast = 7,
  NUM_SKY_DIRECTIONS = 8,
  right = east,
  down = south,
  left = west,
  up = north,
  NUM_FACING = 4,
};

constexpr point get_diff(facing_t facing) {
  switch (facing) {
    case east:
      return {1, 0};
    case south:
      return {0, 1};
    case west:
      return {-1, 0};
    case north:
      return {0, -1};
    case southeast:
      return {1, 1};
    case southwest:
      return {-1, 1};
    case northwest:
      return {-1, -1};
    case northeast:
      return {1, -1};
    default:
      AOC_ASSERT(false, "Facing into an invalid direction");
      return {};
  }
};

constexpr inline auto basic_neighbor_diffs = std::invoke([] {
  std::array<point, NUM_FACING> positions;
  for (int f = 0; f < NUM_FACING; ++f) {
    auto facing = static_cast<facing_t>(f);
    positions[f] = get_diff(facing);
  }
  return positions;
});

constexpr inline auto all_sky_directions = std::invoke([] {
  std::array<facing_t, NUM_SKY_DIRECTIONS> directions;
  for (int f = 0; f < NUM_SKY_DIRECTIONS; ++f) {
    directions[f] = static_cast<facing_t>(f);
  }
  return directions;
});

constexpr inline auto all_neighbor_diffs = std::invoke([] {
  std::array<point, NUM_SKY_DIRECTIONS> positions;
  for (int f = 0; f < NUM_SKY_DIRECTIONS; ++f) {
    positions[f] = get_diff(all_sky_directions[f]);
  }
  return positions;
});

template <class CRTP, bool enable_diagonal = false>
struct grid_neighbors {
  using neighborhood_type = array_grid<CRTP*, 3>;
  using pointer = neighborhood_type::value_type;
  static constexpr bool allow_diagonal = enable_diagonal;

  constexpr pointer& get(point diff) {
    this->assert_diff(diff);
    return m_neighbors.at(diff.y + 1, diff.x + 1);
  }
  constexpr pointer const& get(point diff) const {
    this->assert_diff(diff);
    return m_neighbors.at(diff.y + 1, diff.x + 1);
  }

  constexpr grid_neighbors& set(point diff, pointer neighbor) {
    this->assert_diff(diff);
    m_neighbors.modify(neighbor, diff.y + 1, diff.x + 1);
    return *this;
  }

 private:
  constexpr void assert_diff(point diff) const {
    (void)diff;
    AOC_ASSERT(!((diff.x == 0) && (diff.y == 0)), "Not valid to access itself");
    if constexpr (!enable_diagonal) {
      AOC_ASSERT(std::abs(diff.x * diff.y) != 1, "Not valid to go diagonally");
    }
  }

 private:
  neighborhood_type m_neighbors{nullptr};
};

template <class T, class row_storage_t, class Container>
  requires requires(T value) {
    value.neighbors.get(point{});
    value.neighbors.set(point{}, static_cast<T*>(nullptr));
  }
constexpr void set_standard_neighbors(grid<T, row_storage_t, Container>& grid,
                                      point offset = {0, 0},
                                      point subrange = {0, 0}) {
  constexpr bool allow_diagonal =
      decltype(std::declval<T>().neighbors)::allow_diagonal;
  if (subrange == point{0, 0}) {
    subrange = point{grid.row_length(), grid.num_rows()} - offset;
  }
  const point postmax = subrange + offset;
  const auto set_diffs = [&]<size_t size>(point pos,
                                          std::array<point, size> diffs) {
    auto current = &grid.at(pos.y, pos.x);
    for (const auto diff : diffs) {
      current->neighbors.set(diff, &grid.at(pos.y + diff.y, pos.x + diff.x));
    }
  };
  // Set top row
  for (int column = offset.x + 1; column < postmax.x - 1; ++column) {
    const point pos{column, offset.y};
    set_diffs(pos,
              std::array{basic_neighbor_diffs[west], basic_neighbor_diffs[east],
                         basic_neighbor_diffs[south]});
    if constexpr (allow_diagonal) {
      set_diffs(pos, std::array{all_neighbor_diffs[southeast],
                                all_neighbor_diffs[southwest]});
    }
  }
  // Set bottom row
  for (int column = offset.x + 1; column < postmax.x - 1; ++column) {
    const point pos{column, postmax.y - 1};
    set_diffs(pos,
              std::array{basic_neighbor_diffs[west], basic_neighbor_diffs[east],
                         basic_neighbor_diffs[north]});
    if constexpr (allow_diagonal) {
      set_diffs(pos, std::array{all_neighbor_diffs[northeast],
                                all_neighbor_diffs[northwest]});
    }
  }
  // Set leftmost column
  for (int row = offset.y + 1; row < postmax.y - 1; ++row) {
    const point pos{offset.x, row};
    set_diffs(pos, std::array{basic_neighbor_diffs[north],
                              basic_neighbor_diffs[south],
                              basic_neighbor_diffs[east]});
    if constexpr (allow_diagonal) {
      set_diffs(pos, std::array{all_neighbor_diffs[northeast],
                                all_neighbor_diffs[southeast]});
    }
  }
  // Set rightmost column
  for (int row = offset.y + 1; row < postmax.y - 1; ++row) {
    const point pos{postmax.x - 1, row};
    set_diffs(pos, std::array{basic_neighbor_diffs[north],
                              basic_neighbor_diffs[south],
                              basic_neighbor_diffs[west]});
    if constexpr (allow_diagonal) {
      set_diffs(pos, std::array{all_neighbor_diffs[northwest],
                                all_neighbor_diffs[southwest]});
    }
  }
  // Set everything in the middle
  for (int row = offset.y + 1; row < postmax.y - 1; ++row) {
    for (int column = offset.x + 1; column < postmax.x - 1; ++column) {
      constexpr auto neighbor_diffs = std::invoke([&]() {
        if constexpr (allow_diagonal) {
          return all_neighbor_diffs;
        } else {
          return basic_neighbor_diffs;
        }
      });
      set_diffs(point{column, row}, neighbor_diffs);
    }
  }
  // Set corners
  constexpr auto corner_diagonal_diff = [&](facing_t facing, facing_t backup) {
    if constexpr (allow_diagonal) {
      return all_neighbor_diffs[facing];
    } else {
      return basic_neighbor_diffs[backup];
    }
  };
  set_diffs(offset,
            std::array{basic_neighbor_diffs[south], basic_neighbor_diffs[east],
                       corner_diagonal_diff(southeast, south)});
  set_diffs(offset + point{subrange.x - 1, 0},
            std::array{basic_neighbor_diffs[south], basic_neighbor_diffs[west],
                       corner_diagonal_diff(southwest, south)});
  set_diffs(offset + point{0, subrange.y - 1},
            std::array{basic_neighbor_diffs[north], basic_neighbor_diffs[east],
                       corner_diagonal_diff(northeast, north)});
  set_diffs(postmax - point{1, 1},
            std::array{basic_neighbor_diffs[north], basic_neighbor_diffs[west],
                       corner_diagonal_diff(northwest, north)});
}

template <class T, class point_class = point,
          class row_storage_t = std::vector<T>>
class sparse_grid {
 protected:
  using key_type = point_class;

 public:
  using container_type = std::map<key_type, T>;
  using row_t = row_storage_t;
  using value_type = T;
  using iterator = map_value_iterator<typename container_type::iterator>;
  using const_iterator =
      map_value_iterator<typename container_type::const_iterator>;

  static constexpr auto static_row_length =
      is_array_class_v<row_t> ? row_t{}.size() : 0;

  constexpr iterator add_row(const row_t& row) {
    m_row_length = row.size();
    for (int column = 0; const auto& elem : row) {
      this->insert_single(point_class{column, m_num_rows}, elem);
      ++column;
    }
    ++m_num_rows;
    return m_data.find(point_class{0, m_num_rows - 1});
  }

  constexpr row_t get_row(int row) const {
    row_t return_row;
    if constexpr (is_specialization_of_v<row_t, std::vector>) {
      return_row.reserve(this->row_length());
    }
    auto it = inserter_it(return_row);
    for (int column = 0; column < this->row_length(); ++column) {
      *it = this->at(row, column);
      ++it;
    }
    return return_row;
  }

  constexpr const value_type& at(int row, int column) const {
    auto it = m_data.find(point_class{column, row});
    if (it == std::end(m_data)) {
      return empty_value;
    }
    return it->second;
  }

  constexpr void modify(value_type value, int row, int column) {
    auto coords = point_class{column, row};
    auto it = m_data.find(coords);
    if (it == std::end(m_data)) {
      this->insert_single(coords, std::move(value));
    } else {
      it->second = std::move(value);
    }
  }

  constexpr auto size() const { return m_data.size(); }

  constexpr int row_length() const {
    if constexpr (static_row_length > 0) {
      return static_row_length;
    } else {
      return m_row_length;
    }
  }

  constexpr int num_rows() const { return m_num_rows; }

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

  constexpr auto begin() const { return m_data.begin(); }
  constexpr auto end() const { return m_data.begin(); }

 protected:
  static constexpr auto empty_value = T{};

  constexpr std::pair<iterator, bool> insert_single(const key_type& key,
                                                    value_type value) {
    if (value == empty_value) {
      m_data.erase(key);
      return {std::end(m_data), false};
    } else {
      auto [it, success] = m_data.emplace(key, value);
      return {it, success};
    }
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

 private:
  container_type m_data;
  int m_row_length = 0;
  int m_num_rows = 0;
};
