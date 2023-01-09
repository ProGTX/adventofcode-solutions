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

using elf_t = point;
using elves_t = std::vector<elf_t>;

enum field_type_t : char {
  empty_tile = '.',
  elf_tile = '#',
};

void print_field(const elves_t elves) {
  auto bounds = min_max_helper::get(elves);
  auto field =
      grid<char>{empty_tile, bounds.grid_size().y, bounds.grid_size().x};
  for (auto elf : elves) {
    elf -= bounds.min_value;
    field.modify(elf_tile, elf.y, elf.x);
  }
  field.print_all();
}

template <int num_rounds>
void simulate(elves_t& elves) {
  static constexpr auto invalid_proposal = facing_t::NUM_SKY_DIRECTIONS;
  std::vector<facing_t> proposals(elves.size(), invalid_proposal);

  std::array<std::array<facing_t, 3>, 4> possible_proposals{
      std::array{north, northeast, northwest},
      std::array{south, southwest, southeast},
      std::array{west, southwest, northwest},
      std::array{east, northeast, southeast},
  };
  const auto is_empty =
      [&]<size_t size>(point pos,
                       std::array<facing_t, size> const& directions) -> bool {
    return std::ranges::all_of(
        directions | std::views::transform(
                         [&](facing_t facing) { return get_diff(facing); }),
        [&](const point diff) {
          auto neighbor_pos = pos + diff;
          return !contains(elves, neighbor_pos);
        });
  };
  const auto propose = [&](point pos) -> facing_t {
    for (auto const& directions : possible_proposals) {
      if (is_empty(pos, all_sky_directions)) {
        // Same as not moving
        return invalid_proposal;
      }
      if (is_empty(pos, directions)) {
        return directions[0];
      }
    }
    return invalid_proposal;
  };
  const auto execute = [&] {
    std::vector<point> proposed_positions(elves);
    for (int e = 0; e < elves.size(); ++e) {
      if (proposals[e] != invalid_proposal) {
        proposed_positions[e] += get_diff(proposals[e]);
      }
    }
    for (int e = 0; e < elves.size(); ++e) {
      auto count =
          std::ranges::count(proposed_positions, proposed_positions[e]);
      if (count == 1) {
        elves[e] = proposed_positions[e];
      }
    }
  };
  for (int round = 0; round < num_rounds; ++round) {
    for (int e = 0; e < elves.size(); ++e) {
      proposals[e] = propose(elves[e]);
    }
    execute();
    std::ranges::rotate(possible_proposals, std::begin(possible_proposals) + 1);
  }
}

template <int num_rounds>
int solve_case(std::string const& filename) {
  elves_t elves;
  readfile_op(filename, [&](std::string_view line, int linenum) {
    const int row = linenum - 1;
    for (int column = 0; char value : line) {
      if (value == elf_tile) {
        elves.push_back(point{column, row});
      }
      ++column;
    }
  });
  simulate<num_rounds>(elves);

  point field_size = min_max_helper::get(elves).grid_size();

  int num_empty_tiles = field_size.reduce<std::multiplies<>>() - elves.size();
  std::cout << filename << " -> " << num_empty_tiles << std::endl;
  return num_empty_tiles;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(25, (solve_case<3>("day23.example2")));
  AOC_EXPECT_RESULT(110, (solve_case<10>("day23.example")));
  AOC_EXPECT_RESULT(3906, (solve_case<10>("day23.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(5031, (solve_case<100>("day23.example")));
  // AOC_EXPECT_RESULT(3239579395609, (solve_case<100>("day23.input")));
  AOC_RETURN_CHECK_RESULT();
}
