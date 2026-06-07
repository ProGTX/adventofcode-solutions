// https://adventofcode.com/2015/day/13

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>

struct link_t {
  int to_id;
  int happiness;
};

using connections_t = Vec<Vec<link_t>>;

fn parse(const std::string& filename) -> connections_t {
  auto connections = connections_t{};
  auto name_to_id = aoc::name_to_id{};
  for (str line : aoc::views::read_lines(filename)) {
    // "Alice would gain 54 happiness units by sitting next to Bob."
    let[from, _w, gain_lose, val, _h, _u, _by, _s, _n, _t, to_dot] =
        aoc::split_to_array<11>(line, ' ');
    let from_id = name_to_id.intern(from);
    let to_id = name_to_id.intern(to_dot.substr(0, to_dot.size() - 1));
    connections.resize(name_to_id.new_size(connections.size()));
    let happiness = aoc::to_number<int>(val);
    connections[from_id].emplace_back(
        to_id, (gain_lose == "gain") ? happiness : -happiness);
  }
  return connections;
}

template <bool Part2>
fn solve_case(const std::string& filename) -> int {
  auto connections = parse(filename);
  return 0;
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(0, solve_case<false>("day13.example"));
  AOC_EXPECT_RESULT(0, solve_case<false>("day13.input"));

  std::println("Part 2");
  AOC_EXPECT_RESULT(0, solve_case<true>("day13.example"));
  AOC_EXPECT_RESULT(0, solve_case<true>("day13.input"));

  AOC_RETURN_CHECK_RESULT();
}
