// https://adventofcode.com/2024/day/12

#include "../common/common.h"

#include <array>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Pad the are with edges to remove the need for bounds checking
constexpr const char edge = '#';

using garden_t = aoc::char_grid<>;

struct garden_plot_t {
  char name;
  std::vector<point> interior;
  std::vector<point> fence;

  constexpr garden_plot_t(char name_, point start)
      : name{name_}, interior{start} {}

  constexpr int price() const { return interior.size() * fence.size(); }
};

using plots_t = std::vector<garden_plot_t>;
using plot_ids_t = aoc::flat_map<point, int>;

constexpr auto get_neighbors(point position) {
  return aoc::basic_sky_directions |
         std::views::transform([&](aoc::facing_t facing) {
           return position + aoc::get_diff(facing);
         }) |
         aoc::ranges::to<aoc::static_vector<point, 4>>();
}

constexpr void add_neighbors(const char name, const point position,
                             const int id, const garden_t& garden,
                             plots_t& plots, plot_ids_t& plot_ids) {
  for (point neighbor_pos : get_neighbors(position)) {
    auto neighbor = garden.at(neighbor_pos.y, neighbor_pos.x);
    if (neighbor != name) {
      continue;
    }
    auto [neighbor_it, inserted] = plot_ids.emplace(neighbor_pos, id);
    if (inserted) {
      plots[id].interior.push_back(neighbor_pos);
      // Recursively add neighbors of the neighbor
      add_neighbors(name, neighbor_pos, id, garden, plots, plot_ids);
    } else {
      // Neighbor already known
    }
  }
}

constexpr plots_t get_plots(const garden_t& garden) {
  plots_t plots;
  plot_ids_t plot_ids;
  for (int row = 1; row < garden.num_rows() - 1; ++row) {
    for (int col = 1; col < garden.num_columns() - 1; ++col) {
      auto position = point{col, row};
      auto name = garden.at(row, col);

      // Try to insert a new plot id
      auto [id_it, inserted] =
          plot_ids.emplace(position, static_cast<int>(plots.size()));
      const auto id = id_it->second;
      if (inserted) {
        plots.emplace_back(name, position);
      }

      // Recursively add all neighbors to the interior
      add_neighbors(name, position, id, garden, plots, plot_ids);

      // Check all 4 neighbors for the fence
      for (point neighbor_pos : get_neighbors(position)) {
        auto neighbor = garden.at(neighbor_pos.y, neighbor_pos.x);
        if (neighbor != name) {
          plots[id].fence.push_back(neighbor_pos);
        }
      }
    }
  }
  return plots;
}

constexpr int get_price(const plots_t& plots) {
  return aoc::ranges::accumulate(
      plots | std::views::transform(&garden_plot_t::price), 0);
}

template <bool>
int solve_case(const std::string& filename) {
  garden_t garden;

  for (std::string line : aoc::views::read_lines(filename)) {
    if (garden.empty()) {
      garden.add_row(aoc::views::repeat(edge, line.size() + 2));
    }
    garden.add_row(edge + std::move(line) + edge);
  }
  garden.add_row(aoc::views::repeat(edge, garden.row_length()));

  int sum = 0;
  sum = get_price(get_plots(garden));

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(140, solve_case<false>("day12.example"));
  AOC_EXPECT_RESULT(772, solve_case<false>("day12.example2"));
  AOC_EXPECT_RESULT(1930, solve_case<false>("day12.example3"));
  AOC_EXPECT_RESULT(1450422, solve_case<false>("day12.input"));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(281, solve_case<true>("day12.example"));
  // AOC_EXPECT_RESULT(53515, solve_case<true>("day12.input"));
  AOC_RETURN_CHECK_RESULT();
}
