// https://adventofcode.com/2024/day/19

#include "../common/common.h"
#include "../common/rust.h"

#include <print>
#include <ranges>
#include <span>

struct Input {
  Vec<String> patterns;
  Vec<String> designs;
};

auto parse(String const& filename) -> Input {
  auto patterns = Vec<String>{};
  auto designs = Vec<String>{};
  for (int row = 0; String line : aoc::views::read_lines(filename)) {
    if (row < 1) {
      patterns = aoc::split_to_vec<String>(line, ',', aoc::trimmer<String>());
    } else {
      designs.push_back(std::move(line));
    }
    ++row;
  }
  return {std::move(patterns), std::move(designs)};
}

template <bool all_options>
fn num_possible_designs(str design, std::span<const String> patterns) -> i64;

template <>
fn num_possible_designs<false>(str design, std::span<const String> patterns)
    -> i64 {
  bool end_found = false;
  aoc::shortest_distances_dijkstra(
      String{""},
      [&](str current) {
        if (current.size() == design.size()) {
          end_found = true;
        }
        return end_found;
      },
      [&](String const& current) {
        auto neighbors = Vec<aoc::dijkstra_neighbor_t<String>>{};
        for (String const& pattern : patterns) {
          auto neighbor = current + pattern;
          if (design.starts_with(neighbor)) {
            neighbors.emplace_back(std::move(neighbor),
                                   design.size() - neighbor.size() + 1);
          }
        }
        return neighbors;
      });
  return end_found;
}

template <>
fn num_possible_designs<true>(str design, std::span<const String> patterns)
    -> i64 {
  // A design's number of arrangements is 1 if it's fully matched,
  // otherwise the sum of arrangement counts of what's left
  // after stripping off each pattern that matches its start
  let design_str = String{design};
  let num_designs = aoc::dfs<void, i64>(
      design_str, [](String const& remaining) { return remaining.empty(); },
      [&](String const& remaining) {
        return patterns |
               stdv::filter(
                   [&](String const& p) { return remaining.starts_with(p); }) |
               stdv::transform(
                   [&](String const& p) { return remaining.substr(p.size()); }) |
               aoc::dijkstra_uniform_neighbors_view();
      });
  return num_designs.at(design_str);
}

template <bool all_options>
fn solve_case(Input const& input) -> i64 {
  return aoc::ranges::accumulate(
      input.designs | stdv::transform([&](str design) {
        return num_possible_designs<all_options>(design, input.patterns);
      }),
      i64{0});
}

int main() {
  std::println("Part 1");
  let example = parse("day19.example");
  AOC_EXPECT_RESULT(6, solve_case<false>(example));
  let input = parse("day19.input");
  AOC_EXPECT_RESULT(317, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(16, solve_case<true>(example));
  AOC_EXPECT_RESULT(883443544805484, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
