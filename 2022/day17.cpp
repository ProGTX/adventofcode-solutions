// https://adventofcode.com/2022/day/17

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

using rock_t = std::array<point, 5>;

constexpr void move_rock(rock_t& rock, point move) {
  for (point& p : rock) {
    p += move;
  }
}

template <int num_rocks>
int get_final_height(const std::vector<int>& pattern) {
  constexpr int left_most_index = 0;
  constexpr int right_most_index = 4;
  constexpr int top_most_index = 1;
  constexpr int bottom_most_index = 3;

  static constexpr auto initial_rock_displacement = point{2, 3};

  constexpr std::array<rock_t, 5> rock_structures = std::invoke([] {
    std::array<rock_t, 5> rocks;
    rocks[0] = {point{0, 0}, point{1, 0}, point{2, 0}, point{3, 0},
                point{3, 0}};
    rocks[1] = {point{0, 1}, point{1, 2}, point{1, 1}, point{1, 0},
                point{2, 1}};
    rocks[2] = {point{0, 0}, point{2, 2}, point{2, 0}, point{1, 0},
                point{2, 1}};
    rocks[3] = {point{0, 1}, point{0, 3}, point{0, 0}, point{0, 0},
                point{0, 2}};
    rocks[4] = {point{0, 0}, point{0, 1}, point{0, 0}, point{1, 0},
                point{1, 1}};

    // Each rock appears so that its left edge is two units away
    // from the left wall and its bottom edge is three units above
    // the highest rock in the room (or the floor, if there isn't one).
    for (auto& rock : rocks) {
      move_rock(rock, initial_rock_displacement);
    }
    return rocks;
  });

  static constexpr std::string_view rock_chars = "|=#$%*";
  int rock_char_index = 0;

  static constexpr char empty_char = ' ';
  // static constexpr char rock_char = '#';
  static constexpr int chamber_width = 7;

  // Just use rocks as the walls, no need to do bounds checking
  using chamber_row_t = std::array<char, chamber_width + 2>;
  grid<char, chamber_row_t, std::vector<char>> chamber;
  // And use rocks for the floor
  chamber_row_t floor;
  std::ranges::fill(floor, rock_chars[0]);
  chamber.add_row(floor);

  constexpr chamber_row_t empty_new_row = std::invoke([] {
    chamber_row_t new_row;
    std::ranges::fill(new_row, empty_char);
    new_row[0] = new_row[new_row.size() - 1] = rock_chars[0];
    return new_row;
  });

  const auto try_move_rock = [&](rock_t& falling_rock, point move) -> bool {
    move_rock(falling_rock, move);
    bool intersects_with_rock =
        std::ranges::any_of(falling_rock, [&](const point& rp) {
          const auto value = chamber.at(rp.y, rp.x);
          return std::ranges::any_of(
              rock_chars, [&](char rock_char) { return (value == rock_char); });
        });
    if (intersects_with_rock) {
      // Invalid movement, move it back
      move_rock(falling_rock, -move);
      return false;
    }
    return true;
  };

  const auto print_chamber = [&]() {
    chamber.print_all([&](std::ostream& out, int row, int column) {
      auto value = chamber.at(chamber.num_rows() - row - 1, column);
      if (value == empty_char) {
        value = '.';
      } else if ((column == 0) || (column == chamber.row_length() - 1)) {
        value = '|';
      } else if (row == chamber.num_rows() - 1) {
        value = '-';
      }
      out << value;
    });
  };

  int current_height = 1;
  int pattern_index = 0;
  for (int r = 0; r < num_rocks; ++r) {
    rock_char_index = (rock_char_index + 1) % rock_chars.size();

    rock_t falling_rock = rock_structures[r % rock_structures.size()];
    const auto current_rock_height =
        falling_rock[top_most_index].y - falling_rock[bottom_most_index].y + 1;

    // Do initial falling stage without a chamber
    for (int y = 0; y < initial_rock_displacement.y; ++y) {
      // Only do horizontal movement in  this loop
      auto sideways = pattern[pattern_index];
      auto leftmost = falling_rock[left_most_index].x + sideways;
      auto rightmost = falling_rock[right_most_index].x + sideways;
      if ((leftmost >= 0) && (rightmost < chamber_width)) {
        move_rock(falling_rock, point{sideways, 0});
      }
      pattern_index = (pattern_index + 1) % pattern.size();
    }

    { // Place falling rock into the chamber
      auto missing_rows =
          current_height + current_rock_height - chamber.num_rows();
      for (int r = 0; r < missing_rows; ++r) {
        chamber.add_row(empty_new_row);
      }
      // This move includes the vertical movement from the initial falling stage
      // It also includes the left chamber wall
      move_rock(falling_rock,
                point{1, current_height - initial_rock_displacement.y});
    }

    // Move rock until it settles
    while (true) {
      auto sideways = pattern[pattern_index];
      try_move_rock(falling_rock, point{sideways, 0});
      if (!try_move_rock(falling_rock, point{0, -1})) {
        // Rock settled, engrave it into the chamber
        for (const point& rp : falling_rock) {
          auto actual_rock_char_index =
              (rock_char_index % (rock_chars.size() - 1)) + 1;
          chamber.modify(rock_chars[actual_rock_char_index], rp.y, rp.x);
        }
        current_height =
            std::max(current_height, falling_rock[top_most_index].y + 1);
        pattern_index = (pattern_index + 1) % pattern.size();
        break;
      }
      pattern_index = (pattern_index + 1) % pattern.size();
    }
  }

  return current_height - 1;
}

template <int num_rocks, bool>
void solve_case(const std::string& filename) {
  std::vector<int> pattern;

  readfile_op(filename, [&](std::string_view line) {
    if (line.empty()) {
      return;
    }
    std::ranges::transform(line, std::back_inserter(pattern), [](char value) {
      if (value == '<') {
        return -1;
      } else {
        return 1;
      }
    });
  });

  int final_height = get_final_height<num_rocks>(pattern);
  std::cout << filename << " -> " << final_height << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<2022, false>("day17.example");
  solve_case<2022, false>("day17.input");
  // std::cout << "Part 2" << std::endl;
  // solve_case<2022, true>("day17.example");
  // solve_case<2022, true>("day17.input");
}
