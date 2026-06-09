// https://adventofcode.com/2023/day/5

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>
#include <ranges>

using range_t = aoc::closed_range<i64>;

struct single_mapping_t {
  range_t src;
  range_t dst;

  fn in_source_range(i64 value) const -> bool {
    return (value >= src.begin) && (value < src.end);
  }
};

struct Input {
  Vec<i64> seed_ints;
  Vec<Vec<single_mapping_t>> mappings;
};

auto parse(String const& filename) -> Input {
  Input result;
  std::ifstream file{filename};

  let line = aoc::read_line(file);
  result.seed_ints = aoc::split_to_vec<i64>(line.substr(sizeof("seeds:")), ' ');

  Vec<single_mapping_t> mapping;
  for (str line : aoc::views::read_lines(file)) {
    if (!aoc::is_number(line[0])) {
      stdr::sort(mapping, std::less<>{}, &single_mapping_t::src);
      result.mappings.push_back(std::move(mapping));
      mapping.clear();
      continue;
    }
    let[dest_start, source_start, range] =
        aoc::split<std::array<i64, 3>>(line, ' ');
    mapping.emplace_back(range_t{source_start, source_start + range},
                         range_t{dest_start, dest_start + range});
  }
  stdr::sort(mapping, std::less<>{}, &single_mapping_t::src);
  result.mappings.push_back(std::move(mapping));

  return result;
}

using seeds_t = Vec<range_t>;

fn map_single(range_t const& seed, single_mapping_t const& map) -> range_t {
  let offset = map.dst.begin - map.src.begin;
  return {seed.begin + offset, seed.end + offset};
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
fn apply_mapping(seeds_t current_seeds, Vec<single_mapping_t> const& mapping)
    -> seeds_t {
  // We want all the ranges to be sorted to simplify the algorithm
  AOC_ASSERT(stdr::is_sorted(mapping, std::less<>{}, &single_mapping_t::src),
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
      let a = map_it->src.begin;
      let b = map_it->src.end - 1;
      let x = seed_it->begin;
      let y = seed_it->end - 1;
      if (seed_it->contains(map_it->src)) {
        // (x-a-1)(a+b)(b+1-y)
        if ((a - x) > 1) {
          // (x-a-1)
          next_seeds.emplace_back(x, a);
        }
        // (a+b)
        next_seeds.push_back(map_single(map_it->src, *map_it));
        if ((y - b) > 0) {
          // (b+1-y)
          split_seed_range({x, b + 2}, {b + 1, y + 1});
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
        next_seeds.push_back(map_single({x, b + 1}, *map_it));
        // (b+1-y)
        split_seed_range({x, b + 2}, {b + 1, y + 1});
        // Important to break the loop after the split
        // because the map doesn't apply anymore
        break;
      } else {
        // (x,a,y,b) - (x-a-1)(a+y)
        // (x-a-1)
        next_seeds.emplace_back(x, a);
        // (a+y)
        next_seeds.push_back(map_single({a, y + 1}, *map_it));
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

  stdr::sort(next_seeds, std::less<>{});
  return next_seeds;
}

template <bool full_ranges>
fn solve_case(Input const& input) -> i64 {
  seeds_t current_seeds;
  for (usize i = 0; i + 1 < input.seed_ints.size(); i += 2) {
    let v0 = input.seed_ints[i];
    let v1 = input.seed_ints[i + 1];
    if constexpr (!full_ranges) {
      current_seeds.emplace_back(v0, v0 + 1);
      current_seeds.emplace_back(v1, v1 + 1);
    } else {
      current_seeds.emplace_back(v0, v0 + v1);
    }
  }

  for (let& mapping : input.mappings) {
    current_seeds = apply_mapping(current_seeds, mapping);
  }

  return current_seeds[0].begin;
}

int main() {
  std::println("Part 1");
  let example = parse("day05.example");
  AOC_EXPECT_RESULT(35, (solve_case<false>(example)));
  let input = parse("day05.input");
  AOC_EXPECT_RESULT(486613012, (solve_case<false>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(46, (solve_case<true>(example)));
  AOC_EXPECT_RESULT(56931769, (solve_case<true>(input)));

  AOC_RETURN_CHECK_RESULT();
}
