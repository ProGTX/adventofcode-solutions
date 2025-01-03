// https://adventofcode.com/2024/day/16

#include "../common/common.h"

#include <iostream>
#include <optional>
#include <ranges>
#include <string>
#include <utility>

constexpr const char empty = '.';
constexpr const char start = 'S';
constexpr const char end = 'E';

using maze_t = aoc::char_grid<>;
using arrow_t = aoc::arrow_type<int>;
using predecessors_t = aoc::predecessor_map<arrow_t>;

constexpr auto get_distances(const maze_t& maze, point start_pos,
                             std::optional<point> end_pos,
                             predecessors_t* predecessors) {
  auto distances = aoc::shortest_distances_dijkstra(
      arrow_t{start_pos, aoc::east},
      [&](const arrow_t current) {
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
      },
      [&](const arrow_t current) {
        if (!end_pos.has_value()) {
          return false;
        }
        return current.position == *end_pos;
      },
      predecessors);

  return distances;
}

constexpr auto get_end_arrows(point end_pos) {
  return std::array{aoc::east, aoc::north} |
         std::views::transform([&](const aoc::facing_t facing) {
           return arrow_t{end_pos, facing};
         }) |
         aoc::ranges::to<aoc::static_vector<arrow_t, 2>>();
}

constexpr int lowest_score(const maze_t& maze, point start_pos, point end_pos) {
  auto distances = get_distances(maze, start_pos, end_pos, nullptr);

  int lowest = 1 << 30;
  for (auto end_arrow : get_end_arrows(end_pos)) {
    auto it = distances.find(end_arrow);
    if (it != std::end(distances)) {
      lowest = std::min(lowest, it->second);
    }
  }
  return lowest;
}

constexpr void add_tiles(const maze_t& maze, const auto& distances,
                         const predecessors_t& predecessors,
                         aoc::flat_set<point>& tiles, arrow_t current_arrow) {

  const auto try_explore_neighbor = [&](arrow_t neighbor_arrow,
                                        const arrow_t pred_arrow,
                                        const int pred_distance) {
    if (neighbor_arrow == pred_arrow) {
      return;
    }
    const auto distance_it = distances.find(neighbor_arrow);
    if ((distance_it != std::end(distances)) &&
        (distance_it->second == pred_distance)) {
      add_tiles(maze, distances, predecessors, tiles, neighbor_arrow);
    }
  };

  while (true) {
    const auto current_pos = current_arrow.position;
    tiles.insert(current_pos);
    auto pred_it = predecessors.find(current_arrow);
    if (pred_it == std::end(predecessors)) {
      return;
    }
    const auto current_distance = distances.at(current_arrow);
    // Explore other potential predecessors that have the same score
    // This is done in reverse of the original neighbor search

    {
      const auto neighbor_pos =
          current_pos - aoc::get_diff(current_arrow.direction);
      // Note we don't care for checking the starting node here
      // because that's guaranteed to be on the shortest path
      if (maze.at(neighbor_pos.y, neighbor_pos.x) == empty) {
        try_explore_neighbor(arrow_t{neighbor_pos, current_arrow.direction},
                             pred_it->second, current_distance - 1);
      }
    }
    try_explore_neighbor(
        arrow_t{current_pos, aoc::anticlockwise_basic(current_arrow.direction)},
        pred_it->second, current_distance - 1000);
    try_explore_neighbor(
        arrow_t{current_pos, aoc::clockwise_basic(current_arrow.direction)},
        pred_it->second, current_distance - 1000);

    current_arrow = pred_it->second;
  }
}

constexpr int tiles_on_best_paths(const maze_t& maze, point start_pos,
                                  point end_pos) {
  // In this case we don't want to terminate the search
  // when finding the best path, so we don't pass any end arrows
  predecessors_t predecessors;
  auto distances = get_distances(maze, start_pos, std::nullopt, &predecessors);

  aoc::flat_set<point> tiles;
  for (const auto end_arrow : get_end_arrows(end_pos)) {
    add_tiles(maze, distances, predecessors, tiles, end_arrow);
  }
  return tiles.size();
}

template <bool tiles>
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
  if constexpr (!tiles) {
    sum = lowest_score(maze, start_pos, end_pos);
  } else {
    sum = tiles_on_best_paths(maze, start_pos, end_pos);
  }
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
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(45, solve_case<true>("day16.example"));
  // AOC_EXPECT_RESULT(64, solve_case<true>("day16.example2"));
  AOC_EXPECT_RESULT(481, solve_case<true>("day16.input"));
  AOC_RETURN_CHECK_RESULT();
}
