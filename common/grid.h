#ifndef AOC_GRID_H
#define AOC_GRID_H

#include "assert.h"
#include "compiler.h"
#include "concepts.h"
#include "point.h"
#include "range_to.h"
#include "ranges.h"
#include "static_vector.h"
#include "string.h"

#ifndef AOC_MODULE_SUPPORT
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
#endif

AOC_EXPORT_NAMESPACE(aoc) {

////////////////////////////////////////////////////////////////////////////////
// Directions

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

constexpr auto to_facing(char c) {
  switch (c) {
    case '^':
      return aoc::north;
    case 'v':
      return aoc::south;
    case '<':
      return aoc::west;
    case '>':
      return aoc::east;
    default:
      AOC_UNREACHABLE("Error parsing directions");
  }
}

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
      AOC_UNREACHABLE("Invalid sky direction");
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
      AOC_UNREACHABLE("Facing into an invalid direction");
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

////////////////////////////////////////////////////////////////////////////////
// Main grid

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

  constexpr grid(Container c, size_t num_rows, size_t num_columns)
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
    if constexpr (reservable<container_type>) {
      m_data.reserve(old_size + this->row_length());
    }

    auto it = inserter_it(m_data);
    if constexpr (is_array_class_v<container_type>) {
      it += old_size;
    }
    std::ranges::copy_n(std::begin(row), this->row_length(), it);
    ++m_num_rows;
    return std::begin(m_data) + old_size;
  }

  constexpr row_t get_row(size_t row) const {
    return this->row_view_detail<row_t>(this->begin_row(row));
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
  constexpr size_t linear_index(point pos) const {
    return this->linear_index(pos.y, pos.x);
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
        out << print_single_f(this->at(row, column));
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

  template <class Return = void>
  constexpr auto row_view(size_t row) {
    return this->row_view_detail<Return>(this->begin_row(row));
  }
  template <class Return = void>
  constexpr auto row_view(size_t row) const {
    return this->row_view_detail<Return>(this->begin_row(row));
  }

  template <class Return = void>
  constexpr auto column_view(size_t column) {
    return this->column_view_detail<Return>(begin(), column);
  }
  template <class Return = void>
  constexpr auto column_view(size_t column) const {
    return this->column_view_detail<Return>(begin(), column);
  }

  constexpr void clear() {
    m_data.clear();
    m_data.resize(this->num_rows() * this->num_columns());
  }

  constexpr bool in_bounds(size_t row, size_t column) const {
    return (row < this->num_rows()) && (column < this->row_length());
  }
  constexpr bool in_bounds(std::signed_integral auto row,
                           std::signed_integral auto column) const {
    return (row >= 0) &&
           (column >= 0) &&
           this->in_bounds(static_cast<size_t>(row),
                           static_cast<size_t>(column));
  }

  constexpr auto basic_neighbor_positions(point pos) const {
    return this->get_neighbors(pos, basic_neighbor_diffs, std::true_type{});
  }
  constexpr auto basic_neighbor_positions(size_t linear_index) const {
    return this->get_neighbors(this->position(linear_index),
                               basic_neighbor_diffs, std::true_type{});
  }
  constexpr auto all_neighbor_positions(point pos) const {
    return this->get_neighbors(pos, all_neighbor_diffs, std::true_type{});
  }
  constexpr auto all_neighbor_positions(size_t linear_index) const {
    return this->get_neighbors(this->position(linear_index), all_neighbor_diffs,
                               std::true_type{});
  }
  constexpr auto basic_neighbor_values(point pos) const {
    return this->get_neighbors(pos, basic_neighbor_diffs, std::false_type{});
  }
  constexpr auto basic_neighbor_values(size_t linear_index) const {
    return this->get_neighbors(this->position(linear_index),
                               basic_neighbor_diffs, std::false_type{});
  }
  constexpr auto all_neighbor_values(point pos) const {
    return this->get_neighbors(pos, all_neighbor_diffs, std::false_type{});
  }
  constexpr auto all_neighbor_values(size_t linear_index) const {
    return this->get_neighbors(this->position(linear_index), all_neighbor_diffs,
                               std::false_type{});
  }

 private:
  template <class Return, class Begin>
  constexpr auto row_view_detail(Begin start) const {
    auto view = std::ranges::subrange(start, start + this->row_length());
    if constexpr (std::same_as<Return, void>) {
      if constexpr (std::same_as<T, char>) {
        return std::string_view{view.begin(), view.end()};
      } else {
        return view;
      }
    } else if constexpr (is_array_class_v<Return>) {
      Return row;
      std::ranges::copy(view, std::begin(row));
      return row;
    } else {
      return view | ranges::to<Return>();
    }
  }

  template <class Return, class Begin>
  constexpr auto column_view_detail(Begin begin_zero, int column) const {
    const auto tmp_end =
        this->begin_row(this->num_rows() - 1) + column + this->row_length();
    auto start = begin_zero + column;
    auto view =
        std::ranges::subrange(start, start + std::distance(start, tmp_end)) |
        std::views::stride(this->row_length());
    if constexpr (std::same_as<Return, void>) {
      return view;
    } else {
      Return return_column;
      if constexpr (reservable<Return>) {
        return_column.reserve(this->num_columns());
      }
      std::ranges::copy(view, inserter_it(return_column));
      return return_column;
    }
  }

  template <class Self, size_t size, bool return_pos>
  constexpr auto get_neighbors(this Self&& self, point pos,
                               const std::array<point, size>& neighbor_diffs,
                               std::bool_constant<return_pos>) {
    // TODO: Support returning static_vector<std::reference_wrapper>
    // this Self&& already helps with that,
    // but it will also require improvements to static_vector.
    auto neighbors =
        static_vector<std::conditional_t<return_pos, point, value_type>,
                      size>{};
    for (const auto neighbor_diff : neighbor_diffs) {
      const auto neighbor_pos = pos + neighbor_diff;
      if (self.in_bounds(neighbor_pos.y, neighbor_pos.x)) {
        if constexpr (return_pos) {
          neighbors.push_back(neighbor_pos);
        } else {
          neighbors.emplace_back(self.at(neighbor_pos.y, neighbor_pos.x));
        }
      }
    }
    return neighbors;
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

////////////////////////////////////////////////////////////////////////////////
// sparse_grid

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
    if constexpr (reservable<row_t>) {
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

////////////////////////////////////////////////////////////////////////////////
// Read from file

struct char_grid_config_input {
  std::optional<char> padding;
  std::optional<char> start_char;
  std::optional<char> end_char;
};
struct char_grid_config_output {
  using point = point_type<int>;
  std::optional<point> start_pos;
  std::optional<point> end_pos;
};

template <class... ReadArgsTs>
std::pair<char_grid<>, char_grid_config_output> read_char_grid(
    const std::string& filename, const char_grid_config_input config,
    ReadArgsTs... read_args) {
  char_grid<> return_grid;
  char_grid_config_output config_output{};
  using point = char_grid_config_output::point;
  const bool padding = config.padding.has_value();
  const auto set_once = [padding](std::optional<point>& optional_pos,
                                  const std::optional<char>& optional_char,
                                  std::string_view line, std::size_t row_id) {
    optional_pos = optional_pos.or_else([&] {
      return optional_char.and_then([&](char input_char) {
        const auto pos = line.find(input_char);
        return (pos != std::string::npos)
                   ? std::optional{point(pos + static_cast<int>(padding),
                                         row_id)}
                   : std::nullopt;
      });
    });
  };
  for (std::string line :
       views::read_lines(filename, std::forward<ReadArgsTs>(read_args)...)) {
    if (padding && return_grid.empty()) {
      return_grid.add_row(std::views::repeat(*config.padding, line.size() + 2));
    }
    const auto row_id = return_grid.num_rows();
    set_once(config_output.start_pos, config.start_char, line, row_id);
    set_once(config_output.end_pos, config.end_char, line, row_id);
    if (padding) {
      return_grid.add_row(*config.padding + std::move(line) + *config.padding);
    } else {
      return_grid.add_row(std::move(line));
    }
  }
  if (padding) {
    return_grid.add_row(
        std::views::repeat(*config.padding, return_grid.row_length()));
  }
  return std::pair{return_grid, config_output};
}

template <class... ReadArgsTs>
auto read_char_grid(const std::string& filename, ReadArgsTs... read_args) {
  auto [grid, _] =
      read_char_grid(filename, {}, std::forward<ReadArgsTs>(read_args)...);
  return grid;
}

////////////////////////////////////////////////////////////////////////////////
// Helpers

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

} // AOC_EXPORT_NAMESPACE(aoc)

#endif // AOC_GRID_H
