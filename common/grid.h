#ifndef AOC_GRID_H
#define AOC_GRID_H

#include "assert.h"
#include "point.h"

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace aoc {

template <class T, class row_storage_t = std::vector<T>,
          class Container = row_storage_t>
class grid {
 public:
  using row_t = row_storage_t;
  using container_type = Container;
  using value_type = T;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;

  using point = point_type<int>;

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
  constexpr grid(value_type value, size_t num_rows, size_t num_columns)
      : m_data(num_rows * num_columns, value),
        m_row_length{num_columns},
        m_num_rows(num_rows) {}

  constexpr grid(Container&& c, size_t num_rows, size_t num_columns)
      : m_data(std::move(c)), m_row_length{num_columns}, m_num_rows(num_rows) {
    AOC_ASSERT(((num_rows * num_columns) <= m_data.size()),
               "Container is not large enough for requested number"
               "of rows and columns");
  }

  template <class Row = row_t>
    requires requires(Row row) {
      std::ranges::copy_n(std::begin(row), 0, iterator{});
    }
  constexpr iterator add_row(Row&& row) {
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

  constexpr row_t get_row(size_t row) const {
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

  [[nodiscard]] constexpr auto empty_dynamic() const { return m_data.empty(); }
  [[nodiscard]] constexpr bool empty() const {
    if constexpr (static_data_size > 0) {
      return false;
    } else {
      return this->empty_dynamic();
    }
  }

  constexpr size_t row_length_dynamic() const { return m_row_length; }
  constexpr size_t row_length() const {
    if constexpr (static_row_length > 0) {
      return static_row_length;
    } else {
      return this->row_length_dynamic();
    }
  }
  constexpr size_t num_columns() const { return this->row_length(); }

  constexpr size_t num_rows_dynamic() const { return m_num_rows; }
  constexpr size_t num_rows() const {
    if constexpr ((static_data_size > 0) && (static_row_length > 0)) {
      return static_data_size / static_row_length;
    } else {
      return this->num_rows_dynamic();
    }
  }

  constexpr size_t linear_index(size_t row, size_t column) const {
    return row * this->row_length() + column;
  }
  constexpr point position(size_t linear_index) const {
    this->assert_linear_index(linear_index);
    return point(linear_index % this->row_length(),
                 linear_index / this->row_length());
  }

  constexpr value_type& at(size_t row, size_t column) {
    return m_data[this->linear_index(row, column)];
  }
  constexpr const value_type& at(size_t row, size_t column) const {
    return m_data[this->linear_index(row, column)];
  }

  constexpr void modify(value_type value, size_t linear_index) {
    m_data[linear_index] = std::move(value);
  }
  constexpr void modify(value_type value, size_t row, size_t column) {
    this->modify(std::move(value), this->linear_index(row, column));
  }

  constexpr value_type& at_index(size_t linear_index) {
    this->assert_linear_index(linear_index);
    return m_data[linear_index];
  }
  constexpr const value_type& at_index(size_t linear_index) const {
    this->assert_linear_index(linear_index);
    return m_data[linear_index];
  }

  template <class print_single_ft = std::identity>
  std::ostream& print_all(print_single_ft print_single_f = {},
                          std::ostream& out = std::cout) const {
    for (size_t row = 0; row < this->num_rows(); ++row) {
      out << "  ";
      for (size_t column = 0; column < this->row_length(); ++column) {
        this->get_print_single_f(print_single_f)(out, row, column);
      }
      out << std::endl;
    }
    out << std::endl;
    return out;
  }

  constexpr auto begin() { return m_data.begin(); }
  constexpr auto begin() const { return m_data.begin(); }
  constexpr auto end() { return m_data.end(); }
  constexpr auto end() const { return m_data.end(); }

  constexpr auto begin_row(size_t row) {
    return m_data.begin() + this->linear_index(row, 0);
  }
  constexpr auto begin_row(size_t row) const {
    return m_data.begin() + this->linear_index(row, 0);
  }
  constexpr auto end_row(size_t row) {
    return this->begin_row(row) + this->row_length();
  }
  constexpr auto end_row(size_t row) const {
    return this->begin_row(row) + this->row_length();
  }

  constexpr void clear() {
    m_data.clear();
    m_data.resize(this->num_rows() * this->num_columns());
  }

  constexpr bool in_bounds(size_t row, size_t column) const {
    return (row < this->num_rows()) && (column < this->row_length());
  }

 private:
  template <class print_single_ft>
  constexpr auto get_print_single_f(print_single_ft print_single_f) const {
    if constexpr (std::is_same_v<print_single_ft, std::identity>) {
      return [this](std::ostream& out, size_t row, size_t column) {
        out << this->at(row, column);
      };
    } else {
      return print_single_f;
    }
  }

 protected:
  constexpr void assert_linear_index(
      [[maybe_unused]] size_t linear_index) const {
    AOC_ASSERT(linear_index < this->size(), "Index cannot be out of bounds");
    AOC_ASSERT(this->num_rows() > 0, "Cannot get position without any rows");
  };

  constexpr void set_size(size_t num_rows_, size_t row_length_) {
    m_num_rows = num_rows_;
    m_row_length = row_length_;
  }

  container_type m_data;

 private:
  size_t m_row_length = 0;
  size_t m_num_rows = 0;
};

template <class T, size_t row_length, size_t num_rows = row_length>
using array_grid =
    grid<T, std::array<T, row_length>, std::array<T, row_length * num_rows>>;

template <class row_storage_t = std::string, class Container = std::string>
using char_grid = grid<char, row_storage_t, Container>;

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

std::ostream& operator<<(std::ostream& out, const facing_t& facing) {
  switch (facing) {
    case east:
      out << "east";
      break;
    case south:
      out << "south";
      break;
    case west:
      out << "west";
      break;
    case north:
      out << "north";
      break;
    case southeast:
      out << "southeast";
      break;
    case southwest:
      out << "southwest";
      break;
    case northwest:
      out << "northwest";
      break;
    case northeast:
      out << "northeast";
      break;
    default:
      AOC_ASSERT(false, "Invalid sky direction");
  }
  return out;
}

template <class T = int>
constexpr point_type<T> get_diff(facing_t facing) {
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

constexpr inline auto basic_sky_directions = std::invoke([] {
  std::array<facing_t, NUM_FACING> directions;
  for (int f = 0; f < NUM_FACING; ++f) {
    auto facing = static_cast<facing_t>(f);
    directions[f] = facing;
  }
  return directions;
});
constexpr inline auto all_sky_directions = std::invoke([] {
  std::array<facing_t, NUM_SKY_DIRECTIONS> directions;
  for (int f = 0; f < NUM_SKY_DIRECTIONS; ++f) {
    directions[f] = static_cast<facing_t>(f);
  }
  return directions;
});

constexpr facing_t clockwise_basic(facing_t current) {
  AOC_ASSERT(current >= 0, "Invalid sky direction");
  AOC_ASSERT(current < NUM_FACING, "Not a basic sky direction");
  return static_cast<facing_t>((static_cast<int>(current) + 1) % NUM_FACING);
}

constexpr facing_t anticlockwise_basic(facing_t current) {
  AOC_ASSERT(current >= 0, "Invalid sky direction");
  AOC_ASSERT(current < NUM_FACING, "Not a basic sky direction");
  return static_cast<facing_t>((static_cast<int>(current) + NUM_FACING - 1) %
                               NUM_FACING);
}

constexpr inline auto basic_neighbor_diffs = std::invoke([] {
  std::array<point_type<int>, NUM_FACING> positions;
  std::ranges::transform(basic_sky_directions, std::begin(positions),
                         &get_diff<int>);
  return positions;
});
constexpr inline auto all_neighbor_diffs = std::invoke([] {
  std::array<point_type<int>, NUM_SKY_DIRECTIONS> positions;
  std::ranges::transform(all_sky_directions, std::begin(positions),
                         &get_diff<int>);
  return positions;
});

template <class CRTP, bool enable_diagonal = false>
struct grid_neighbors {
  using neighborhood_type = array_grid<CRTP*, 3>;
  using pointer = neighborhood_type::value_type;
  using point = point_type<int>;

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
    value.neighbors.get(point_type<int>{});
    value.neighbors.set(point_type<int>{}, static_cast<T*>(nullptr));
  }
constexpr void set_standard_neighbors(grid<T, row_storage_t, Container>& grid,
                                      point_type<int> offset = {0, 0},
                                      point_type<int> subrange = {0, 0}) {
  using point = point_type<int>;
  constexpr bool allow_diagonal =
      decltype(std::declval<T>().neighbors)::allow_diagonal;
  if (subrange == point{0, 0}) {
    subrange = point(grid.row_length(), grid.num_rows()) - offset;
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

template <class T, T empty_value_param = T{},
          class point_class = point_type<int>,
          class row_storage_t = std::vector<T>>
class sparse_grid : protected grid<T, row_storage_t, std::map<point_class, T>> {
 private:
  using base_t = grid<T, row_storage_t, std::map<point_class, T>>;

 protected:
  using key_type = point_class;

 public:
  using typename base_t::container_type;
  using typename base_t::row_t;
  using typename base_t::value_type;
  using iterator = map_value_iterator<typename container_type::iterator>;
  using const_iterator =
      map_value_iterator<typename container_type::const_iterator>;

  using base_t::static_data_size;
  using base_t::static_row_length;

  using base_t::data;

  using base_t::size;
  using base_t::size_dynamic;

  using base_t::empty;
  using base_t::empty_dynamic;

  using base_t::row_length;
  using base_t::row_length_dynamic;

  using base_t::num_rows;
  using base_t::num_rows_dynamic;

  using base_t::begin;
  using base_t::end;

  // Note that sparse_grid doesn't allow linear index calculations

  constexpr sparse_grid() = default;

  constexpr sparse_grid(size_t num_rows, size_t num_columns) {
    this->set_size(num_rows, num_columns);
  }

  constexpr iterator add_row(const row_t& row) {
    this->set_size(this->num_rows(), row.size());
    for (size_t column = 0; const auto& elem : row) {
      this->insert_single(point_class(column, this->num_rows()), elem);
      ++column;
    }
    this->set_size(this->num_rows() + 1, this->row_length());
    return base_t::m_data.find(point_class(0, this->num_rows() - 1));
  }

  constexpr iterator add_empty_row() {
    this->set_size(this->num_rows() + 1, this->row_length());
  }

  constexpr row_t get_row(size_t row) const {
    row_t return_row;
    if constexpr (is_specialization_of_v<row_t, std::vector>) {
      return_row.reserve(this->row_length());
    }
    auto it = inserter_it(return_row);
    for (size_t column = 0; column < this->row_length(); ++column) {
      *it = this->at(row, column);
      ++it;
    }
    return return_row;
  }

  constexpr const value_type& at(size_t row, size_t column) const {
    auto it = base_t::m_data.find(point_class(column, row));
    if (it == std::end(base_t::m_data)) {
      return empty_value;
    }
    return it->second;
  }

  constexpr void modify(value_type value, size_t row, size_t column) {
    auto coords = point_class(column, row);
    auto it = base_t::m_data.find(coords);
    if (it == std::end(base_t::m_data)) {
      this->insert_single(coords, std::move(value));
    } else {
      it->second = std::move(value);
    }
  }

  constexpr void clear() { base_t::m_data.clear(); }

  template <class print_single_ft = std::identity>
  void print_all(print_single_ft print_single_f = {},
                 std::ostream& out = std::cout) const {
    for (size_t row = 0; row < this->num_rows(); ++row) {
      out << "  ";
      for (size_t column = 0; column < this->row_length(); ++column) {
        this->get_print_single_f(print_single_f)(out, row, column);
      }
      out << std::endl;
    }
    out << std::endl;
  }

 protected:
  static constexpr auto empty_value = empty_value_param;

  constexpr std::pair<iterator, bool> insert_single(const key_type& key,
                                                    value_type value) {
    if (value == empty_value) {
      base_t::m_data.erase(key);
      return {std::end(base_t::m_data), false};
    } else {
      auto [it, success] = base_t::m_data.emplace(key, value);
      return {it, success};
    }
  }

 private:
  template <class print_single_ft>
  constexpr auto get_print_single_f(print_single_ft print_single_f) const {
    if constexpr (std::is_same_v<print_single_ft, std::identity>) {
      return [this](std::ostream& out, size_t row, size_t column) {
        out << this->at(row, column);
      };
    } else {
      return print_single_f;
    }
  }
};

template <class Grid>
concept is_grid = std::ranges::range<Grid> &&
                  requires(Grid g, typename Grid::value_type value,
                           typename Grid::row_t row) {
                    g.data();
                    g.size();
                    g.empty();
                    g.row_length();
                    g.num_rows();
                    g.add_row(row);
                    g.get_row(0);
                    g.at(0, 0);
                    g.modify(value, 0, 0);
                    g.print_all();
                    g.clear();
                  };

static_assert(is_grid<grid<int>>);
static_assert(is_grid<array_grid<int, 7, 5>>);
static_assert(is_grid<sparse_grid<int>>);
static_assert(is_grid<char_grid<>>);

// An "arrow" has a position and direction
// Often used in search algorithms (see Dijkstra)
template <class T>
struct arrow_type {
  point_type<T> position;
  facing_t direction;

  constexpr bool operator==(const arrow_type&) const = default;
  constexpr std::weak_ordering operator<=>(const arrow_type& other) const {
    if (position == other.position) {
      return direction <=> other.direction;
    } else {
      return position <=> other.position;
    }
  }

  friend std::ostream& operator<<(std::ostream& out, const arrow_type& arrow) {
    out << "(" << arrow.position << "," << arrow.direction << ")";
    return out;
  }
};

} // namespace aoc

#endif // AOC_GRID_H
