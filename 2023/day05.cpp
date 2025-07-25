// https://adventofcode.com/2023/day/5

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

using int_t = long;
struct range_t : public aoc::closed_range<int_t> {
  using base_t = aoc::closed_range<int_t>;
  constexpr range_t(int_t begin_, size_t size_)
      : base_t(begin_, begin_ + size_), size{size_} {}

  size_t size;
};

struct single_mapping_t {
  range_t src;
  range_t dst;

  constexpr bool in_source_range(int_t value) const {
    return (value >= src.begin) && (value < (src.begin + src.size));
  }
};

using seeds_t = std::vector<range_t>;

inline constexpr int_t invalid = -1;

constexpr range_t map_single(const range_t& seed, const single_mapping_t& map) {
  return range_t{map.dst.begin - map.src.begin + seed.begin, seed.size};
}

// Diagram for the explanation of how an individual map applies to a seed range
// (x-y) seed range
// (a-b) map range
// Depending on the order, there are 4 possible scenarios
// Hyphen indicates copying the ranges as-is, plus means map is applied
// (x,a,b,y) - (x-a-1)(a+b)(b+1-y)
// (a,x,b,y) - (x+b)(b+1-y)
// (x,a,y,b) - (x-a-1)(a+y)
// (a,x,y,b) - (x+y)
constexpr seeds_t apply_mapping(seeds_t current_seeds,
                                const std::vector<single_mapping_t>& mapping) {
  // We want all the ranges to be sorted to simplify the algorithm
  AOC_ASSERT(
      std::ranges::is_sorted(mapping, std::less<>{}, &single_mapping_t::src),
      "mapping not presorted");

  seeds_t next_seeds;
  next_seeds.reserve(current_seeds.size());
  auto seed_it = std::begin(current_seeds);
  auto map_it = std::begin(mapping);

  auto find_relevant_map = [&]() {
    // Find first seed range where the map applies
    while (true) {
      if (map_it == std::end(mapping)) {
        // End of mapping, add all the remaining seeds as-is
        for (; seed_it != std::end(current_seeds); ++seed_it) {
          next_seeds.push_back(*seed_it);
        }
        break;
      }
      if (seed_it == std::end(current_seeds)) {
        // Out of seeds, ignore any remaining mapping
        break;
      }
      if (seed_it->overlaps_with(map_it->src)) {
        // Map applies to this seed range,
        // return to perform the rest of the algorithm
        break;
      }
      if (map_it->src < *seed_it) {
        // Map behind seed range
        ++map_it;
      } else {
        // Seed range behind map, pass seed range as-is
        next_seeds.push_back(*seed_it);
        ++seed_it;
      }
    }
  };

  auto split_seed_range = [&](const range_t& update_seed,
                              const range_t& new_seed) {
    *seed_it = update_seed;
    seed_it = current_seeds.insert(seed_it + 1, new_seed);
    ++map_it;
  };

  auto apply_map_to_seeds = [&]() {
    // Find last seed range where the map applies
    // For everything in between apply the map
    for (; seed_it != std::end(current_seeds); ++seed_it) {
      if (!seed_it->overlaps_with(map_it->src)) {
        // Map doesn't apply anymore, return to outer loop
        break;
      }
      // Note that end() points to 1 beyond the last element,
      // so we decrement by one
      const auto a = map_it->src.begin;
      const auto b = map_it->src.end - 1;
      const auto x = seed_it->begin;
      const auto y = seed_it->end - 1;
      if (seed_it->contains(map_it->src)) {
        // (x-a-1)(a+b)(b+1-y)
        if ((a - x) > 1) {
          // (x-a-1)
          next_seeds.emplace_back(x, a - x);
        }
        // (a+b)
        next_seeds.push_back(map_single(map_it->src, *map_it));
        if ((y - b) > 0) {
          // (b+1-y)
          split_seed_range(range_t(x, b - x + 2), range_t(b + 1, y - b));
          // Important to break the loop after the split
          // because the map doesn't apply anymore
          break;
        }
      } else if (map_it->src.contains(*seed_it)) {
        // (a,x,y,b) - (x+y)
        next_seeds.push_back(map_single(*seed_it, *map_it));
      } else if ((x >= a) && (b <= y)) {
        // (a,x,b,y) - (x+b)(b+1-y)
        // (x+b)
        next_seeds.push_back(map_single(range_t(x, b - x + 1), *map_it));
        // (b+1-y)
        split_seed_range(range_t(x, b - x + 2), range_t(b + 1, y - b));
        // Important to break the loop after the split
        // because the map doesn't apply anymore
        break;
      } else {
        // (x,a,y,b) - (x-a-1)(a+y)
        // (x-a-1)
        next_seeds.emplace_back(x, a - x);
        // (a+y)
        next_seeds.push_back(map_single(range_t(a, y - a + 1), *map_it));
      }
    }
  };

  while (true) {
    find_relevant_map();
    if ((map_it == std::end(mapping)) || (seed_it == std::end(current_seeds))) {
      break;
    }
    apply_map_to_seeds();
  }

  std::ranges::sort(next_seeds, std::less<>{});
  return next_seeds;
}

template <bool full_ranges>
int_t solve_case(const std::string& filename) {
  seeds_t current_seeds;
  std::vector<single_mapping_t> mapping;

  std::ifstream file{filename};

  // read_first_line
  [&]() {
    auto line = aoc::read_line(file);
    const auto seeds_ints =
        aoc::split<std::vector<int_t>>(line.substr(sizeof("seeds:")), ' ');
    for (int i = 0; i < seeds_ints.size(); i += 2) {
      if constexpr (!full_ranges) {
        current_seeds.emplace_back(seeds_ints[i], 1);
        current_seeds.emplace_back(seeds_ints[i + 1], 1);
      } else {
        current_seeds.emplace_back(seeds_ints[i], seeds_ints[i + 1]);
      }
    }
  }();

  for (std::string_view line : aoc::views::read_lines(file)) {
    if (!aoc::is_number(line[0])) {
      std::ranges::sort(mapping, std::less<>{}, &single_mapping_t::src);
      current_seeds = apply_mapping(current_seeds, mapping);
      mapping.clear();
      continue;
    }
    auto [dest_start, source_start, range] =
        aoc::split<std::array<int_t, 3>>(line, ' ');
    mapping.emplace_back(range_t(source_start, range),
                         range_t(dest_start, range));
  }

  // Have to run this one last time
  std::ranges::sort(mapping, std::less<>{}, &single_mapping_t::src);
  current_seeds = apply_mapping(current_seeds, mapping);

  int_t lowest_location = current_seeds[0].begin;
  std::cout << filename << " -> " << lowest_location << std::endl;
  return lowest_location;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(35, (solve_case<false>("day05.example")));
  AOC_EXPECT_RESULT(486613012, (solve_case<false>("day05.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(46, (solve_case<true>("day05.example")));
  AOC_EXPECT_RESULT(56931769, (solve_case<true>("day05.input")));
  AOC_RETURN_CHECK_RESULT();
}
