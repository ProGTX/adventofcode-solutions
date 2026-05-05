// https://adventofcode.com/2021/day/12

#include "../common/common.h"
#include "../common/rust.h"

#include <cctype>
#include <print>
#include <ranges>

using Adjacency = Vec<Vec<usize>>;

struct Input {
  Adjacency connections;
  usize start;
  usize end;
  Vec<bool> is_big_cave;
};

auto parse(String const& filename) -> Input {
  auto name_to_id = aoc::name_to_id{};
  auto is_big_cave = Vec<bool>{};
  auto connections = Adjacency{};

  for (std::string_view line : aoc::views::read_lines(filename)) {
    let[start_name, end_name] = aoc::split_once(line, '-');

    let start_id = name_to_id.intern(start_name);
    if (start_id >= is_big_cave.size()) {
      is_big_cave.push_back(
          std::isupper(static_cast<unsigned char>(start_name.front())));
      connections.emplace_back();
    }

    let end_id = name_to_id.intern(end_name);
    if (end_id >= is_big_cave.size()) {
      is_big_cave.push_back(
          std::isupper(static_cast<unsigned char>(end_name.front())));
      connections.emplace_back();
    }

    connections[start_id].push_back(end_id);
    connections[end_id].push_back(start_id);
  }

  return {
      .connections = std::move(connections),
      .start = name_to_id.expect("start"),
      .end = name_to_id.expect("end"),
      .is_big_cave = std::move(is_big_cave),
  };
}

template <bool ALLOW_TWICE>
fn visit_caves_dfs(usize current, Input const& input, Vec<bool>& visited_small,
                   Option<usize> second_visit) -> usize {
  if (current == input.end) {
    return 1;
  }
  auto inserted = false;
  if (!input.is_big_cave[current]) {
    if (ALLOW_TWICE && visited_small[current]) {
      // Don't insert again
    } else {
      inserted = !visited_small[current];
      visited_small[current] = true;
    }
  }
  auto count = 0;
  for (let neighbor : input.connections[current]) {
    if (input.is_big_cave[neighbor] || !visited_small[neighbor]) {
      count += visit_caves_dfs<ALLOW_TWICE>(neighbor, input, visited_small,
                                            second_visit);
    } else if (ALLOW_TWICE &&
               !second_visit.has_value() &&
               (neighbor != input.start) &&
               (neighbor != input.end)) {
      count += visit_caves_dfs<ALLOW_TWICE>(neighbor, input, visited_small,
                                            neighbor);
    }
  }
  if (inserted) {
    visited_small[current] = false;
  }
  return count;
}

template <bool ALLOW_TWICE>
fn solve_case(Input const& input) -> usize {
  auto visited_small = Vec<bool>(input.is_big_cave.size(), false);
  return visit_caves_dfs<ALLOW_TWICE>(input.start, input, visited_small, None);
}

int main() {
  std::println("Part 1");

  let example1 = parse("day12.example");
  AOC_EXPECT_RESULT(10, (solve_case<false>(example1)));
  let example2 = parse("day12.example2");
  AOC_EXPECT_RESULT(19, (solve_case<false>(example2)));
  let example3 = parse("day12.example3");
  AOC_EXPECT_RESULT(226, (solve_case<false>(example3)));
  let input = parse("day12.input");
  AOC_EXPECT_RESULT(4104, (solve_case<false>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(36, (solve_case<true>(example1)));
  AOC_EXPECT_RESULT(103, (solve_case<true>(example2)));
  AOC_EXPECT_RESULT(3509, (solve_case<true>(example3)));
  AOC_EXPECT_RESULT(119760, (solve_case<true>(input)));

  AOC_RETURN_CHECK_RESULT();
}
