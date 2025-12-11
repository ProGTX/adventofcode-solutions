// https://adventofcode.com/2025/day/11

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>
#include <set>

using Outputs = aoc::static_vector<usize, 20>;
using Input = std::pair<Vec<Outputs>, aoc::name_to_id>;

namespace stdv = std::views;
namespace stdr = std::ranges;

fn parse(String const& filename) -> Input {
  auto result = Vec<Outputs>{};
  auto name_to_id = aoc::name_to_id{};
  for (str line : aoc::views::read_lines(filename)) {
    let[source_str, dest] = aoc::split_once(line, ": ");
    let source = name_to_id.intern(source_str);
    let outputs =
        aoc::split(dest, " ") |
        stdv::transform([&](str out) { return name_to_id.intern(out); }) |
        aoc::ranges::to<Outputs>();
    result.resize(name_to_id.new_size(result.size()));
    result[source] = outputs;
  }
  return {std::move(result), std::move(name_to_id)};
}

fn search(Vec<usize>& unvisited, Vec<Outputs> const& device_map, usize start,
          usize end) -> u64 {
  unvisited.push_back(start);
  auto num_paths = u64{};
  while (!unvisited.empty()) {
    let current = aoc::pop_stack(unvisited);
    if (current == end) {
      num_paths += 1;
      continue;
    }
    aoc::ranges::extend(unvisited, device_map[current]);
  }
  return num_paths;
}

fn solve_case1(Input const& input) -> u64 {
  let & [ device_map, name_to_id ] = input;
  auto unvisited = Vec<usize>{};
  return search(unvisited, device_map,
                name_to_id.get("you")
                    .or_else([&] { return name_to_id.get("svr"); })
                    .value(),
                name_to_id.expect("out"));
}

int main() {
  std::println("Part 1");
  let example = parse("day11.example");
  AOC_EXPECT_RESULT(5, solve_case1(example));
  let example2 = parse("day11.example2");
  AOC_EXPECT_RESULT(8, solve_case1(example2));
  let input = parse("day11.input");
  AOC_EXPECT_RESULT(523, solve_case1(input));

  // std::println("Part 2");
  // AOC_EXPECT_RESULT(2, solve_case2(example2));
  // AOC_EXPECT_RESULT(100011612, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
