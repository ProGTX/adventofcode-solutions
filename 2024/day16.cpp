// https://adventofcode.com/2024/day/16

#include "../common/common.h"

#include <iostream>
#include <string>
#include <utility>

constexpr const char empty = '.';
constexpr const char start = 'S';
constexpr const char end = 'E';

using maze_t = aoc::char_grid<>;
using arrow_t = aoc::arrow_type<int>;

constexpr int lowest_score(const maze_t& maze, point start_pos, point end_pos) {
  auto distances = aoc::shortest_distances_dijkstra(
      arrow_t{start_pos, aoc::east}, [&](const arrow_t current) {
        auto neighbors =
            aoc::static_vector<aoc::dijkstra_neighbor_t<arrow_t>, 3>{};

        {
          // Try going forward by 1
          auto neighbor_node =
              arrow_t{current.position + aoc::get_diff(current.direction),
                      current.direction};
          const auto neighbor_pos = neighbor_node.position;
          // No need for bounds checking because there are walls all around
          const auto neighbor_value = maze.at(neighbor_pos.y, neighbor_pos.x);
          if ((neighbor_value == empty) || (neighbor_value == end)) {
            neighbors.emplace_back(std::move(neighbor_node), 1);
          }
        }

        auto new_direction = aoc::clockwise_basic(current.direction);
        neighbors.emplace_back(arrow_t{current.position, new_direction}, 1000);

        new_direction = aoc::anticlockwise_basic(current.direction);
        neighbors.emplace_back(arrow_t{current.position, new_direction}, 1000);

        return neighbors;
      });

  int lowest = 1 << 30;
  for (auto direction : aoc::basic_sky_directions) {
    auto it = distances.find(arrow_t{end_pos, direction});
    if (it != std::end(distances)) {
      lowest = std::min(lowest, it->second);
    }
  }
  return lowest;
}

template <bool>
int solve_case(const std::string& filename) {
  maze_t maze;
  point start_pos;
  point end_pos;

  for (std::string line : aoc::views::read_lines(filename)) {
    if (auto pos = line.find(start); pos != std::string::npos) {
      start_pos = point(pos, maze.num_rows());
    } else if (auto pos = line.find(end); pos != std::string::npos) {
      end_pos = point(pos, maze.num_rows());
    }
    maze.add_row(std::move(line));
  }

  int sum = 0;
  sum = lowest_score(maze, start_pos, end_pos);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(7036, solve_case<false>("day16.example"));
  AOC_EXPECT_RESULT(11048, solve_case<false>("day16.example2"));
  // https://www.reddit.com/r/adventofcode/comments/1hfhgl1/2024_day_16_part_1_alternate_test_case/
  AOC_EXPECT_RESULT(21148, solve_case<false>("day16.example3"));
  // https://www.reddit.com/r/adventofcode/comments/1hgyuqm/2024_day_16_part_1/
  AOC_EXPECT_RESULT(5027, solve_case<false>("day16.example4"));
  AOC_EXPECT_RESULT(94436, solve_case<false>("day16.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day16.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day16.input"));
  AOC_RETURN_CHECK_RESULT();
}
