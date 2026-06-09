// https://adventofcode.com/2023/day/8

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <functional>
#include <print>
#include <span>
#include <string>

using node_t = std::pair<int, int>;
using node_select_func_t = decltype(&node_t::first);

fn get_direction(char c) -> node_select_func_t {
  if (c == 'L') {
    return &node_t::first;
  } else {
    return &node_t::second;
  }
}

static_assert(std::invoke(get_direction('R'), node_t{5, 6}) == 6);
static_assert(std::invoke(get_direction('L'), node_t{5, 6}) == 5);

struct Input {
  Vec<node_select_func_t> instructions;
  Vec<node_t> directions;
  Vec<i32> starts;       // exact "AAA" match
  Vec<i32> ends;         // exact "ZZZ" match
  Vec<i32> ghost_starts; // name[2] == 'A'
  Vec<i32> ghost_ends;   // name[2] == 'Z'
};

fn parse(String const& filename) -> Input {
  let raw_instructions = aoc::read_single_line(filename);

  auto name_to_id = aoc::name_to_id{};
  auto directions = Vec<node_t>{};
  auto starts = Vec<i32>{};
  auto ends = Vec<i32>{};
  auto ghost_starts = Vec<i32>{};
  auto ghost_ends = Vec<i32>{};

  for (str line : aoc::views::read_lines(filename)) {
    let[node_name, lr_raw] =
        aoc::split_once<String>(line, '=', aoc::trimmer<>{});
    let lr = aoc::trim(lr_raw, " ()");
    let[left_name, right_name] =
        aoc::split_once<String>(lr, ',', aoc::trimmer<>{});

    let node_id = static_cast<i32>(name_to_id.intern(node_name));
    let left_id = static_cast<i32>(name_to_id.intern(left_name));
    let right_id = static_cast<i32>(name_to_id.intern(right_name));
    directions.resize(name_to_id.new_size(directions.size()));

    if (node_name == "AAA") {
      starts.push_back(node_id);
    }
    if (node_name == "ZZZ") {
      ends.push_back(node_id);
    }
    if (node_name.size() == 3 && node_name[2] == 'A') {
      ghost_starts.push_back(node_id);
    }
    if (node_name.size() == 3 && node_name[2] == 'Z') {
      ghost_ends.push_back(node_id);
    }
    directions[node_id] = node_t{left_id, right_id};
  }

  let instructions = raw_instructions |
                     stdv::transform(get_direction) |
                     aoc::collect_vec<node_select_func_t>();

  return Input{std::move(instructions), std::move(directions),
               std::move(starts),       std::move(ends),
               std::move(ghost_starts), std::move(ghost_ends)};
}

fn num_steps(std::span<const node_t> directions,
             std::span<const node_select_func_t> instructions, i32 start_index,
             std::span<const i32> end_indexes) -> i64 {
  auto next_index = start_index;
  auto steps = i64{0};
  auto inst_index = 0uz;
  while (!stdr::contains(end_indexes, next_index)) {
    next_index = std::invoke(instructions[inst_index], directions[next_index]);
    inst_index = (inst_index + 1) % instructions.size();
    ++steps;
  }
  return steps;
}

static_assert(6 ==
              num_steps(std::array{node_t{1, 1}, node_t{0, 2}, node_t{2, 2}},
                        std::array{get_direction('L'), get_direction('L'),
                                   get_direction('R')},
                        0, std::array{2}));

fn num_steps(std::span<const node_t> directions,
             std::span<const node_select_func_t> instructions,
             std::span<const i32> start_indexes,
             std::span<const i32> end_indexes) -> i64 {
  return aoc::ranges::lcm( //
      start_indexes | stdv::transform([&](i32 start_index) {
        return num_steps(directions, instructions, start_index, end_indexes);
      }));
}

template <bool all_paths>
fn solve_case(Input const& input) -> i64 {
  let& starts = all_paths ? input.ghost_starts : input.starts;
  let& ends = all_paths ? input.ghost_ends : input.ends;
  return num_steps(input.directions, input.instructions, starts, ends);
}

int main() {
  std::println("Part 1");
  let example = parse("day08.example");
  AOC_EXPECT_RESULT(2, solve_case<false>(example));
  let example2 = parse("day08.example2");
  AOC_EXPECT_RESULT(6, solve_case<false>(example2));
  let input = parse("day08.input");
  AOC_EXPECT_RESULT(12737, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(2, solve_case<true>(example));
  AOC_EXPECT_RESULT(6, solve_case<true>(example2));
  let example3 = parse("day08.example3");
  AOC_EXPECT_RESULT(6, solve_case<true>(example3));
  AOC_EXPECT_RESULT(9064949303801, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
