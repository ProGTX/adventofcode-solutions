// https://adventofcode.com/2022/day/23

#include "../common/common.h"

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

struct elf_sim_value_t {
  int original_index;
  point pos;

  friend std::ostream& operator<<(std::ostream& out,
                                  elf_sim_value_t const& elf) {
    out << '{' << elf.original_index << ':' << elf.pos << '}';
    return out;
  }
};

void print_field(elves_t const& elves) {
  auto bounds = aoc::min_max_helper::get(elves);
  auto field =
      aoc::grid<char>(empty_tile, bounds.grid_size().y, bounds.grid_size().x);
  for (auto elf : elves) {
    elf -= bounds.min_value;
    field.modify(elf_tile, elf.y, elf.x);
  }
  field.print_all();
}

template <int num_rounds>
int simulate(elves_t& elves) {
  using elf_simulation_t = std::map<elf_t, elf_sim_value_t>;
  using elf_sim_pair_value_t = elf_simulation_t::value_type;

  elf_simulation_t elf_simulation;
  int elf_index = -1;
  std::ranges::copy(elves | std::views::transform([&](elf_t const& elf) {
                      ++elf_index;
                      return std::pair{elf, elf_sim_value_t{elf_index, elf}};
                    }),
                    std::inserter(elf_simulation, std::begin(elf_simulation)));

  const auto update_elves = [&]() {
    for (elf_sim_pair_value_t const& elf : elf_simulation) {
      elves[elf.second.original_index] = elf.second.pos;
    }
  };

  std::array<std::array<aoc::facing_t, 3>, 4> possible_proposals{
      std::array{aoc::north, aoc::northeast, aoc::northwest},
      std::array{aoc::south, aoc::southwest, aoc::southeast},
      std::array{aoc::west, aoc::southwest, aoc::northwest},
      std::array{aoc::east, aoc::northeast, aoc::southeast},
  };
  const auto is_empty =
      [&]<size_t size>(
          point pos,
          std::array<aoc::facing_t, size> const& directions) -> bool {
    return std::ranges::all_of(
        directions | std::views::transform([&](aoc::facing_t facing) {
          return get_diff(facing);
        }),
        [&](const point diff) {
          auto neighbor_pos = pos + diff;
          return !elf_simulation.contains(neighbor_pos);
        });
  };
  const auto propose = [&](elf_sim_pair_value_t& elf) {
    elf.second.pos = elf.first;
    for (auto const& directions : possible_proposals) {
      if (is_empty(elf.first, aoc::all_sky_directions)) {
        break;
      }
      if (is_empty(elf.first, directions)) {
        elf.second.pos += get_diff(directions[0]);
        break;
      }
    }
  };
  auto execute = [&]() {
    std::map<point, int> counts;
    for (auto& [current, proposed] : elf_simulation) {
      if (current != proposed.pos) {
        if (counts.contains(proposed.pos)) {
          ++counts[proposed.pos];
        } else {
          counts[proposed.pos] = 1;
        }
      }
    }
    elf_simulation_t new_simulation;
    for (auto& [current, proposed] : elf_simulation) {
      if (counts[proposed.pos] == 1) {
        new_simulation.emplace(
            proposed.pos,
            elf_sim_value_t{proposed.original_index, proposed.pos});
      } else {
        new_simulation.emplace(
            current, elf_sim_value_t{proposed.original_index, current});
      }
    }
    elf_simulation = std::move(new_simulation);
  };
  int round = 0;
  for (; round < num_rounds; ++round) {
    std::ranges::for_each(elf_simulation, propose);
    if (std::ranges::all_of(elf_simulation,
                            [&](elf_sim_pair_value_t const& elf) {
                              return elf.first == elf.second.pos;
                            })) {
      break;
    }
    execute();
    std::ranges::rotate(possible_proposals, std::begin(possible_proposals) + 1);
  }
  update_elves();
  return (round + 1);
}

template <int num_rounds, bool execute_long>
int solve_case(std::string const& filename) {
  elves_t elves;
  aoc::readfile_op(filename, [&](std::string_view line, int linenum) {
    const int row = linenum - 1;
    for (int column = 0; char value : line) {
      if (value == elf_tile) {
        elves.push_back(point{column, row});
      }
      ++column;
    }
  });

  int round_when_none_moved = simulate<num_rounds>(elves);

  int score = 0;
  if constexpr (!execute_long) {
    point field_size = aoc::min_max_helper::get(elves).grid_size();
    int num_empty_tiles = field_size.reduce<std::multiplies<>>() - elves.size();
    score = num_empty_tiles;
  } else {
    score = round_when_none_moved;
  }
  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(25, (solve_case<3, false>("day23.example2")));
  AOC_EXPECT_RESULT(110, (solve_case<10, false>("day23.example")));
  AOC_EXPECT_RESULT(3906, (solve_case<10, false>("day23.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(4, (solve_case<4, true>("day23.example2")));
  AOC_EXPECT_RESULT(20, (solve_case<100, true>("day23.example")));
  AOC_EXPECT_RESULT(895, (solve_case<1000, true>("day23.input")));
  AOC_RETURN_CHECK_RESULT();
}
