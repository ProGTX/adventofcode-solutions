// https://adventofcode.com/2025/day/11

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>
#include <set>

using Outputs = aoc::static_vector<usize, 20>;
using Input = std::pair<Vec<Outputs>, aoc::name_to_id>;

auto parse(String const& filename) -> Input {
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

fn search(Vec<Outputs> const& device_map, usize start, usize end) -> u64 {
  let num_paths = aoc::dfs(
      start, [end](usize node) { return node == end; },
      [&device_map](usize node) {
        return device_map[node] | aoc::dijkstra_uniform_neighbors_view();
      });
  return num_paths.at(start);
}

fn solve_case1(Input const& input) -> u64 {
  let & [ device_map, name_to_id ] = input;
  return search(device_map,
                name_to_id.get("you")
                    .or_else([&] { return name_to_id.get("svr"); })
                    .value(),
                name_to_id.expect("out"));
}

fn solve_case2(Input const& input) -> u64 {
  let & [ device_map, name_to_id ] = input;

  let svr = name_to_id.expect("svr");
  let dac = name_to_id.expect("dac");
  let fft = name_to_id.expect("fft");
  let out = name_to_id.expect("out");

  let svr_dac = search(device_map, svr, dac);
  let dac_fft = search(device_map, dac, fft);
  let fft_out = search(device_map, fft, out);

  let svr_fft = search(device_map, svr, fft);
  let fft_dac = search(device_map, fft, dac);
  let dac_out = search(device_map, dac, out);

  return (svr_dac * dac_fft * fft_out) + (svr_fft * fft_dac * dac_out);
}

int main() {
  std::println("Part 1");
  let example = parse("day11.example");
  AOC_EXPECT_RESULT(5, solve_case1(example));
  let example2 = parse("day11.example2");
  AOC_EXPECT_RESULT(8, solve_case1(example2));
  let input = parse("day11.input");
  AOC_EXPECT_RESULT(523, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(2, solve_case2(example2));
  AOC_EXPECT_RESULT(517315308154944, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
