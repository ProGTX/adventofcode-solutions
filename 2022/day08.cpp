// https://adventofcode.com/2022/day/8

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

using Forrest = aoc::char_grid<>;
using Score = aoc::grid<u32>;

fn parse(String const& filename) -> Forrest {
  return aoc::read_char_grid(filename);
}

template <class ColumnIt, class F>
fn score_row(const usize row, ColumnIt&& column_it, const u32 score_init,
             F&& check) -> u32 {
  auto score = score_init;
  for (let column : column_it) {
    if (!check(row, column, score)) {
      break;
    }
  }
  return score;
}

template <class RowIt, class F>
fn score_column(const usize column, RowIt&& row_it, const u32 score_init,
                F&& check) -> u32 {
  auto score = score_init;
  for (let row : row_it) {
    if (!check(row, column, score)) {
      break;
    }
  }
  return score;
}

fn solve_case1(Forrest const& forrest) -> u32 {
  auto visibility = Score{0, forrest.num_rows(), forrest.num_columns()};
  for (let[index, tree_height] : forrest | std::views::enumerate) {
    let position = forrest.position(index);
    let tree_column = static_cast<usize>(position.x);
    let tree_row = static_cast<usize>(position.y);
    let check = [&](usize row, usize column, u32& visible) {
      if (forrest.at(row, column) >= tree_height) {
        visible = 0;
        return false;
      }
      return true;
    };
    let visible =
        (score_row(tree_row, Range{tree_column + 1, forrest.num_columns()}, 1,
                   check) > 0) ||
        (score_row(tree_row, Range{0uz, tree_column} | std::views::reverse, 1,
                   check) > 0) ||
        (score_column(tree_column, Range{tree_row + 1, forrest.num_rows()}, 1,
                      check) > 0) ||
        (score_column(tree_column, Range{0uz, tree_row} | std::views::reverse,
                      1, check) > 0);
    visibility.modify(static_cast<u32>(visible), tree_row, tree_column);
  }
  return aoc::ranges::accumulate(visibility, 0);
}

fn solve_case2(Forrest const& forrest) -> u32 {
  auto score = Score{0, forrest.num_rows(), forrest.num_columns()};
  for (let[index, tree_height] : forrest | std::views::enumerate) {
    let position = forrest.position(index);
    let tree_column = static_cast<usize>(position.x);
    let tree_row = static_cast<usize>(position.y);
    let check = [&](usize row, usize column, u32& scenic_score) {
      scenic_score += 1;
      if (forrest.at(row, column) >= tree_height) {
        return false;
      }
      return true;
    };
    let tree_score =
        score_row(tree_row, Range{tree_column + 1, forrest.num_columns()}, 0,
                  check) *
        score_row(tree_row, Range{0uz, tree_column} | std::views::reverse, 0,
                  check) *
        score_column(tree_column, Range{tree_row + 1, forrest.num_rows()}, 0,
                     check) *
        score_column(tree_column, Range{0uz, tree_row} | std::views::reverse, 0,
                     check);
    score.modify(tree_score, tree_row, tree_column);
  }
  return std::ranges::max(score);
}

int main() {
  std::println("Part 1");
  let example = parse("day08.example");
  AOC_EXPECT_RESULT(21, solve_case1(example));
  let input = parse("day08.input");
  AOC_EXPECT_RESULT(1681, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(8, solve_case2(example));
  AOC_EXPECT_RESULT(201684, solve_case2(input));
}
