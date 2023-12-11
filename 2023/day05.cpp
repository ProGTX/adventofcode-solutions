// https://adventofcode.com/2023/day/5

#include "../common/common.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

using int_t = long;

struct single_mapping_t {
  int_t dest_start;
  int_t source_start;
  int_t range;

  constexpr bool in_source_range(int_t value) const {
    return (value >= source_start) && (value < (source_start + range));
  }
};

template <size_t num_seeds>
using seeds_type = std::array<int_t, num_seeds>;

template <size_t num_seeds>
constexpr seeds_type<num_seeds> apply_mapping(seeds_type<num_seeds> input,
                                              seeds_type<num_seeds> output,
                                              single_mapping_t mapping) {
  const auto diff = mapping.dest_start - mapping.source_start;
  for (int i = 0; i < num_seeds; ++i) {
    const auto seed = input[i];
    if (!mapping.in_source_range(seed)) {
      continue;
    }
    output[i] = seed + diff;
  }
  return output;
}

inline constexpr int_t invalid = -1;

static_assert(seeds_type<4>{81, invalid, 57, invalid} ==
              apply_mapping(seeds_type<4>{79, 14, 55, 13},
                            seeds_type<4>{invalid, invalid, invalid, invalid},
                            single_mapping_t{52, 50, 48}));

template <size_t num_seeds, bool>
int_t solve_case(const std::string& filename) {
  using seeds_t = seeds_type<num_seeds>;
  seeds_t current_seeds;
  seeds_t next_seeds;

  auto read_first_line = [&](std::string_view line) {
    // Using next_seeds instead of current_seeds as a way
    // to handle the first non-mapping line
    next_seeds = split<seeds_t>(line.substr(sizeof("seeds:")), ' ');
  };

  auto default_mapping = [&]() {
    for (int i = 0; i < num_seeds; ++i) {
      if (next_seeds[i] == invalid) {
        next_seeds[i] = current_seeds[i];
      }
    }
    std::swap(current_seeds, next_seeds);
    std::ranges::fill(next_seeds, invalid);
  };

  auto solver = [&](std::string_view line) {
    if (line.empty()) {
      return;
    }
    if (!is_number(line[0])) {
      default_mapping();
      return;
    }
    auto [dest_start, source_start, range] =
        split<std::array<int_t, 3>>(line, ' ');
    next_seeds =
        apply_mapping(current_seeds, next_seeds,
                      single_mapping_t{dest_start, source_start, range});
  };

  readfile_op_header(filename, read_first_line, solver);

  // Have to run this one last time
  default_mapping();

  int_t lowest_location = std::ranges::min(current_seeds);
  std::cout << filename << " -> " << lowest_location << std::endl;
  return lowest_location;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(35, (solve_case<4, false>("day05.example")));
  AOC_EXPECT_RESULT(486613012, (solve_case<20, false>("day05.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(30, (solve_case<4, true>("day05.example")));
  // AOC_EXPECT_RESULT(14624680, (solve_case<20, true>("day05.input")));
  AOC_RETURN_CHECK_RESULT();
}
