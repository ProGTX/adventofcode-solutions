// https://adventofcode.com/2015/day/13

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <limits>
#include <numeric>
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

fn adjacency_matrix(connections_t const& connections) -> aoc::grid<int> {
  let n = connections.size();
  auto matrix = aoc::grid<int>{0, n, n};
  for (let& [ from_id, links ] : connections | stdv::enumerate) {
    for (let& link : links) {
      matrix.at(from_id, link.to_id) = link.happiness;
    }
  }
  return matrix;
}

fn seating_happiness(Vec<int> const& seating, aoc::grid<int> const& matrix)
    -> int {
  let n = seating.size();
  return aoc::ranges::accumulate( //
      Range{0uz, n} | stdv::transform([&](let id) {
        let a = seating[id];
        let b = seating[(id + 1) % n];
        return matrix.at(a, b) + matrix.at(b, a);
      }),
      0);
}

fn max_happiness(aoc::grid<int> const& matrix) -> int {
  let n = matrix.num_rows();
  auto others = Vec<int>(n - 1);
  std::iota(others.begin(), others.end(), 1);
  auto best = std::numeric_limits<int>::min();
  do {
    auto seating = Vec<int>{0};
    aoc::ranges::extend(seating, others);
    best = std::max(best, seating_happiness(seating, matrix));
  } while (stdr::next_permutation(others).found);
  return best;
}

fn solve_case1(const std::string& filename) -> int {
  let connections = parse(filename);
  return max_happiness(adjacency_matrix(connections));
}

fn solve_case2(const std::string& filename) -> int {
  let connections = parse(filename);
  auto base = adjacency_matrix(connections);
  let n = base.num_rows();
  auto extended = aoc::grid<int>(0, n + 1, n + 1);
  for (let row : Range{0uz, n}) {
    for (let col : Range{0uz, n}) {
      extended.at(row, col) = base.at(row, col);
    }
  }
  return max_happiness(extended);
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(330, solve_case1("day13.example"));
  AOC_EXPECT_RESULT(664, solve_case1("day13.input"));

  std::println("Part 2");
  AOC_EXPECT_RESULT(286, solve_case2("day13.example"));
  AOC_EXPECT_RESULT(640, solve_case2("day13.input"));

  AOC_RETURN_CHECK_RESULT();
}
