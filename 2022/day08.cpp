// https://adventofcode.com/2022/day/8

#include "../common/common.h"
#include "../common/grid.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using forrest_t = grid<int>;
using row_t = forrest_t::row_t;

std::vector<int> check_visibility(forrest_t& forrest) {
  auto num_rows = forrest.num_rows();
  auto row_length = forrest.row_length();

  if (num_rows != row_length) {
    throw std::runtime_error("Only square grids are supported");
  }

  // Create walls that we move from one side to another
  // Each tree can only increase the height of the wall
  // 0 is a valid tree height, so initialize wall to -1
  // ltr - left-to-right, rtl - right-to-left
  // ttb - top-to-bottom, btt - bottom-to-top
  row_t ltr_wall(num_rows, -1);
  row_t rtl_wall(num_rows, -1);
  row_t ttb_wall(row_length, -1);
  row_t btt_wall(row_length, -1);

  const auto& data = forrest.data();
  auto visible = std::vector<int>(data.size());

  const auto update_wall = [&](int data_index, row_t& wall, int wall_index) {
    int tree_height = data[data_index];
    if (tree_height > wall[wall_index]) {
      ++visible[data_index];
      wall[wall_index] = tree_height;
    }
  };

  for (int ltr = 0, ttb = 0; ltr < row_length; ++ltr, ++ttb) {
    for (int wall_index = 0; wall_index < num_rows; ++wall_index) {
      int ltr_index = wall_index * row_length + ltr;
      update_wall(ltr_index, ltr_wall, wall_index);

      int rtl_index = (wall_index + 1) * row_length - ltr - 1;
      update_wall(rtl_index, rtl_wall, wall_index);

      int ttb_index = ttb * row_length + wall_index;
      update_wall(ttb_index, ttb_wall, wall_index);

      int btt_index = (num_rows - ttb - 1) * row_length + wall_index;
      update_wall(btt_index, btt_wall, wall_index);
    }
  }

  return visible;
}

int solve_part1(const std::string& filename) {
  forrest_t forrest;
  row_t row;

  readfile_op(filename, [&](std::string_view line) {
    row.clear();
    for (auto number_str : line) {
      row.push_back(static_cast<int>(number_str - '0'));
    }
    forrest.add_row(row);
  });

  auto visible = check_visibility(forrest);
  auto score =
      std::ranges::count_if(visible, [](int value) { return (value > 0); });

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

std::vector<int> check_scenic_score(forrest_t& forrest) {
  const auto get_score = [](ranged_iterator<const int> it, int num_iterations) {
    int num_trees = 0;
    int origin_height = *it;
    for (int iteration = 0; iteration < num_iterations; ++iteration) {
      ++it;
      ++num_trees;
      int current_height = *it;
      if (current_height >= origin_height) {
        break;
      }
    }
    return num_trees;
  };

  auto num_rows = forrest.num_rows();
  auto row_length = forrest.row_length();
  const auto& data = forrest.data();
  auto scenic_score = std::vector<int>(data.size());

  // First and last row will all score 0 anyway
  // The same for first and last column, but not worth checking for that
  for (int i = row_length; i < (data.size() - row_length); ++i) {
    int score = 1;
    int row = i / row_length;
    int column = i % row_length;

    auto ltr_it = ranged_iterator(&data[i], 1);
    score *= get_score(ltr_it, row_length - 1 - column);

    auto rtl_it = ranged_iterator(&data[i], -1);
    score *= get_score(rtl_it, column);

    auto ttb_it = ranged_iterator(&data[i], row_length);
    score *= get_score(ttb_it, num_rows - 1 - row);

    auto btt_it = ranged_iterator(&data[i], -row_length);
    score *= get_score(btt_it, row);

    scenic_score[i] = score;
  }

  return scenic_score;
}

int solve_part2(const std::string& filename) {
  forrest_t forrest;
  row_t row;

  readfile_op(filename, [&](std::string_view line) {
    row.clear();
    for (auto number_str : line) {
      row.push_back(static_cast<int>(number_str - '0'));
    }
    forrest.add_row(row);
  });

  auto scenic_score = check_scenic_score(forrest);
  auto score = std::ranges::max(scenic_score);

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(21, solve_part1("day08.example"));
  AOC_EXPECT_RESULT(1681, solve_part1("day08.input"));
  // std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(8, solve_part2("day08.example"));
  AOC_EXPECT_RESULT(201684, solve_part2("day08.input"));
  AOC_RETURN_CHECK_RESULT();
}
