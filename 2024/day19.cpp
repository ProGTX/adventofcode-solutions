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

fn count_designs(aoc::flat_map<String, i64>& cache, String const& design,
                 std::span<const String> patterns) -> i64 {
  if (design.empty()) {
    return 1;
  }
  let it = cache.find(design);
  if (it != std::end(cache)) {
    return it->second;
  }
  i64 sum = 0;
  for (str p : patterns) {
    if (design.starts_with(p)) {
      sum += count_designs(cache, design.substr(p.size()), patterns);
    }
  }
  cache[design] = sum;
  return sum;
}

template <>
fn num_possible_designs<true>(str design, std::span<const String> patterns)
    -> i64 {
  auto cache = aoc::flat_map<String, i64>{};
  return count_designs(cache, String{design}, patterns);
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
