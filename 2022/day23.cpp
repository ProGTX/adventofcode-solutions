// https://adventofcode.com/2022/day/23

#include "../common/assert.h"
#include "../common/common.h"
#include "../common/grid.h"
#include "../common/print.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

struct tile_t {
  char value;
  grid_neighbors<tile_t, true> neighbors;

  constexpr tile_t(char value_) : value{value_} {};

  constexpr friend bool operator==(tile_t const& lhs, tile_t const& rhs) {
    return lhs.value == rhs.value;
  }
  friend std::ostream& operator<<(std::ostream& out, tile_t const& tile) {
    out << tile.value;
    return out;
  }
};

// TODO: Must be able to expand grid
using field_t = grid<tile_t>;

enum tile_type_t : char {
  empty = '.',
  elf = '#',
};

template <int num_rounds>
void simulate(field_t& field) {
  std::vector<point> elves;
  std::vector<facing_t> proposals;
  static constexpr auto invalid_proposal = facing_t::NUM_SKY_DIRECTIONS;
  for (int index = 0; tile_t const& tile : field) {
    if (tile.value == elf) {
      elves.push_back(field.position(index));
      proposals.push_back(invalid_proposal);
    }
  }
  const auto is_empty = [&]<size_t size>(
                            point pos,
                            std::array<facing_t, size> directions) -> bool {
    auto& current = field.at(pos.y, pos.x);
    return std::ranges::any_of(
        directions | std::views::transform(
                         [](facing_t facing) { return get_diff(facing); }),
        [&](const point diff) {
          auto neighbor = current.neighbors.get(diff);
          if (neighbor == nullptr) {
            return false;
          }
          return neighbor->value == empty;
        });
  };
  const auto propose = [&](point pos) -> facing_t {
    if (is_empty(pos, std::array{north, northeast, northwest})) {
      return north;
    }
    if (is_empty(pos, std::array{south, south, southeast})) {
      return south;
    }
    if (is_empty(pos, std::array{west, southwest, northwest})) {
      return west;
    }
    if (is_empty(pos, std::array{east, northeast, southeast})) {
      return east;
    }
    return invalid_proposal;
  };
  const auto execute = [&]() {};
  for (int round = 0; round < num_rounds; ++round) {
    for (int e = 0; e < elves.size(); ++e) {
      proposals[e] = propose(elves[e]);
    }
    execute();
  }
}

template <int num_rounds>
int solve_case(std::string const& filename) {
  field_t field;
  readfile_op(filename, [&](std::string_view line) { field.add_row(line); });
  field.print_all();
  set_standard_neighbors(field);
  simulate<num_rounds>(field);

  int num_empty_tiles = std::ranges::count(field, empty);
  std::cout << filename << " -> " << num_empty_tiles << std::endl;
  return num_empty_tiles;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(110, (solve_case<10>("day23.example")));
  // AOC_EXPECT_RESULT(97356, (solve_case<10>("day23.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(5031, (solve_case<100>("day23.example")));
  // AOC_EXPECT_RESULT(3239579395609, (solve_case<100>("day23.input")));
  AOC_RETURN_CHECK_RESULT();
}
