// https://adventofcode.com/2022/day/8

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

#include "../common.h"

class grid {
 public:
  using row_t = std::vector<int>;
  using data_t = std::vector<int>;
  using iterator = typename data_t::iterator;
  using const_iterator = typename data_t::const_iterator;

  iterator add_row(const row_t& row) {
    auto old_size = data.size();
    row_length = row.size();
    data.reserve(old_size + row_length);
    for (auto value : row) {
      data.push_back(value);
      visible.push_back(0);
      scenic_score.push_back(0);
    }
    ++num_rows;
    return std::begin(data) + old_size;
  }

  void check_visibility() {
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

    const auto update_wall = [this](int data_index, row_t& wall,
                                    int wall_index) {
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
  }

  void check_scenic_score() {
    const auto get_score = [](ranged_iterator<int> it, int num_iterations) {
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
  }

  int num_visible() const {
    return std::ranges::count_if(visible,
                                 [](int value) { return (value > 0); });
  }

  int highest_scenic_score() const { return std::ranges::max(scenic_score); }

 private:
  data_t data;
  std::vector<int> visible;
  std::vector<int> scenic_score;
  int row_length = 0;
  int num_rows = 0;
};

void solve_part1(const std::string& filename) {
  grid forrest;
  grid::row_t row;

  readfile_op(filename, [&](std::string_view line) {
    row.clear();
    for (auto number_str : line) {
      row.push_back(static_cast<int>(number_str - '0'));
    }
    forrest.add_row(row);
  });

  forrest.check_visibility();

  std::cout << filename << " -> " << forrest.num_visible() << std::endl;
}

void solve_part2(const std::string& filename) {
  grid forrest;
  grid::row_t row;

  readfile_op(filename, [&](std::string_view line) {
    row.clear();
    for (auto number_str : line) {
      row.push_back(static_cast<int>(number_str - '0'));
    }
    forrest.add_row(row);
  });

  forrest.check_scenic_score();

  std::cout << filename << " -> " << forrest.highest_scenic_score()
            << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_part1("day08.example");
  solve_part1("day08.input");
  // std::cout << "Part 2" << std::endl;
  solve_part2("day08.example");
  solve_part2("day08.input");
}
