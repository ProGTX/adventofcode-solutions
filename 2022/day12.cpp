// https://adventofcode.com/2022/day/12

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "../common.h"

struct heightmap_t : public grid<std::vector<int>> {
  point begin_pos;
  point end_pos;
};

using neighbors_t = std::vector<point>;
using path_t = std::vector<point>;

neighbors_t find_neighbors(const heightmap_t& heightmap, const point current,
                           const auto& unvisited) {
  neighbors_t neighbors;
  const auto current_height = heightmap.at(current.y, current.x);
  for (int dy = -1; dy <= 1; dy += 1) {
    for (int dx = -1; dx <= 1; dx += 1) {
      if ((dy * dy * dx * dx) == 1) {
        // Cannot move diagonally
        continue;
      }
      point neighbor = current + point{dx, dy};
      if ((neighbor.y < 0) || (neighbor.y >= heightmap.num_rows()) ||
          (neighbor.x < 0) || (neighbor.x >= heightmap.row_length())) {
        // Invalid index
        continue;
      }
      auto height_diff = heightmap.at(neighbor.y, neighbor.x) - current_height;
      if (height_diff > 1) {
        // Cannot climb very high, but can drop a lot
        continue;
      }
      if (unvisited.contains(neighbor)) {
        neighbors.push_back(neighbor);
      }
    }
  }
  return neighbors;
}

// https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm#Algorithm
int shortest_path_length_dijkstra(const heightmap_t& heightmap) {
  // 2. Assign to every node a tentative distance value:
  // set it to zero for our initial node and to infinity for all other nodes.
  // During the run of the algorithm,
  // the tentative distance of a node v is the length of the shortest path
  // discovered so far between the node v and the starting node.
  using distances_t = grid<std::vector<int>>;
  distances_t distances;

  // 1. Mark all nodes unvisited. Create a set of all the unvisited nodes called
  // the unvisited set.
  const auto compare_points = [&](const point& lhs, const point& rhs) {
    return distances.at(lhs.y, lhs.x) < distances.at(rhs.y, rhs.x);
  };
  using unvisited_t = sorted_flat_set<point, decltype(compare_points)>;
  unvisited_t unvisited{compare_points};

  // 2. Since initially no path is known to any other vertex than the source
  // itself (which is a path of length zero), all other tentative distances are
  // initially set to infinity.
  typename distances_t::row_t distances_row(heightmap.row_length(),
                                            heightmap.size());

  for (int row = 0; row < heightmap.num_rows(); ++row) {
    distances.add_row(distances_row);
    for (int column = 0; column < heightmap.row_length(); ++column) {
      unvisited.emplace(column, row);
    }
  }

  const auto modify_distance = [&](const point& p, int new_distance) {
    unvisited.erase(p);
    distances.modify(new_distance, p.y, p.x);
    unvisited.insert(p);
  };

  // 2. Set the initial node as current.
  auto current = heightmap.begin_pos;
  modify_distance(current, 0);

  while (true) {
    // 5. If the destination node has been marked visited
    // (when planning a route between two specific nodes)
    // or if the smallest tentative distance among the nodes
    // in the unvisited set is infinity
    // (when planning a complete traversal;
    // occurs when there is no connection between the initial node
    // and remaining unvisited nodes),
    // then stop. The algorithm has finished.
    if (current == heightmap.end_pos) {
      break;
    }

    // 3. For the current node, consider all of its unvisited neighbors
    // and calculate their tentative distances through the current node.
    auto neighbors = find_neighbors(heightmap, current, unvisited);
    auto current_distance = distances.at(current.y, current.x);
    for (const auto& neighbor : neighbors) {
      auto neighbor_distance = distances.at(neighbor.y, neighbor.x);
      // Compare the newly calculated tentative distance to the one
      // currently assigned to the neighbor and assign it the smaller one.
      modify_distance(neighbor,
                      std::min(neighbor_distance, (current_distance + 1)));
    }

    // 4. When we are done considering all of the unvisited neighbors
    // of the current node, mark the current node as visited
    // and remove it from the unvisited set.
    // A visited node will never be checked again
    // (this is valid and optimal in connection with the behavior in step 6.:
    // that the next nodes to visit will always be in the order of
    // 'smallest distance from initial node first'
    // so any visits after would have a greater distance).
    unvisited.erase(current);

    if (unvisited.empty()) {
      break;
    }

    // 6. Otherwise, select the unvisited node
    // that is marked with the smallest tentative distance,
    // set it as the new current node, and go back to step 3.
    current = *unvisited.begin();
  }

  return distances.at(current.y, current.x);
}

template <bool>
void solve_case(const std::string& filename) {
  using row_t = typename heightmap_t::row_t;
  heightmap_t heightmap;
  row_t row;

  readfile_op(filename, [&](std::string_view line) {
    if (line.empty()) {
      return;
    }
    row.clear();
    row.reserve(line.size());
    int column = 0;
    std::ranges::transform(line, std::back_inserter(row), [&](char value) {
      switch (value) {
        case 'S':
          heightmap.begin_pos = point{column, heightmap.num_rows()};
          ++column;
          return 0;
        case 'E':
          heightmap.end_pos = point{column, heightmap.num_rows()};
          ++column;
          return static_cast<int>('z' - 'a');
        default:
          ++column;
          return static_cast<int>(value - 'a');
      }
    });
    heightmap.add_row(row);
  });

  auto fewest_steps = shortest_path_length_dijkstra(heightmap);

  std::cout << filename << " -> " << fewest_steps << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<false>("day12.example");
  solve_case<false>("day12.input");
  // std::cout << "Part 2" << std::endl;
  // solve_case<true>("day12.example");
  // solve_case<true>("day12.input");
}
