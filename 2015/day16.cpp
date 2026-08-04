// https://adventofcode.com/2015/day/16

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <array>
#include <map>
#include <print>
#include <ranges>
#endif

// Each aunt is described by exactly three known compounds
struct Compound {
  String key;
  u32 value;
};
using Sue = std::array<Compound, 3>;

auto parse(String const& filename) -> Vec<Sue> {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) {
           let all_info = aoc::split_once(line, ": ");
           let split_info = aoc::split_to_array<3>(all_info[1], ", ");
           auto sue = Sue{};
           for (let[index, info] : split_info | stdv::enumerate) {
             let[key, value] = aoc::split_once(info, ": ");
             sue[index] = Compound{String{key}, aoc::to_number<u32>(value)};
           }
           return sue;
         }) |
         aoc::collect_vec<Sue>();
}

template <bool RANGES>
fn solve_case(Vec<Sue> const& sues) -> u32 {
  let generous_sue = aoc::flat_map<str, u32>{
      {"children", 3}, {"cats", 7},     {"samoyeds", 2}, {"pomeranians", 3},
      {"akitas", 0},   {"vizslas", 0},  {"goldfish", 5}, {"trees", 3},
      {"cars", 2},     {"perfumes", 1},
  };

  for (let&& [ index, sue ] : sues | stdv::enumerate) {
    let matches = stdr::all_of(sue, [&](Compound const& compound) {
      let key = str{compound.key};
      let actual = compound.value;
      let required = generous_sue.at(key);
      if constexpr (!RANGES) {
        return actual == required;
      } else {
        if ((key == "cats") || (key == "trees")) {
          return actual > required;
        } else if ((key == "pomeranians") || (key == "goldfish")) {
          return actual < required;
        } else {
          return actual == required;
        }
      }
    });
    if (matches) {
      return static_cast<u32>(index + 1);
    }
  }
  AOC_UNREACHABLE("Aunt not found!");
}

int main() {
  std::println("Part 1");
  let input = parse("day16.input");
  AOC_EXPECT_RESULT(373, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(260, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
