// https://adventofcode.com/2022/day/15

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

struct sensor_t {
  point pos;
  int range;

  friend std::ostream& operator<<(std::ostream& out, const sensor_t& sensor) {
    out << sensor.pos << "," << sensor.range;
    return out;
  }
};

using beacon_t = point;

template <int inspect_row, bool>
void solve_case(const std::string& filename) {
  std::vector<sensor_t> sensors;
  std::vector<beacon_t> beacons;

  const auto convert_eq_str = [](std::string_view str) {
    // Works for "x=" and "y="
    return std::stoi(std::string(trim(str).substr(2)));
  };

  min_max_helper min_max;

  readfile_op(filename, [&](std::string_view line) {
    auto [sensor_info, beacon_info] =
        split<std::array<std::string, 2>>(std::string{line}, ':');

    // Don't include the space at end, the null terminator is counted instead
    sensor_info = sensor_info.substr(sizeof("Sensor at"));
    beacon_info = beacon_info.substr(sizeof("closest beacon is at"));

    point beacon;
    point sensor;
    {
      auto [x_eq, y_eq] = split<std::array<std::string, 2>>(beacon_info, ',');
      beacon = point{convert_eq_str(x_eq), convert_eq_str(y_eq)};
      min_max.update(beacon);
      beacons.push_back(beacon);
    }
    {
      auto [x_eq, y_eq] = split<std::array<std::string, 2>>(sensor_info, ',');
      sensor = point{convert_eq_str(x_eq), convert_eq_str(y_eq)};
      min_max.update(sensor);
      auto range = point::distance_manhattan(sensor, beacon);
      sensors.emplace_back(sensor, range);
    }
  });

  decltype(sensors) sensors_on_row;
  std::ranges::copy_if(
      sensors, std::back_inserter(sensors_on_row),
      [](const sensor_t& sensor) { return (sensor.pos.y == inspect_row); });
  const auto largest_distance =
      1000 * // TODO: why do I need to go so high?
      std::ranges::max(sensors, [](const sensor_t& lhs, const sensor_t& rhs) {
        return lhs.range > rhs.range;
      }).range;

  decltype(beacons) beacons_on_row;
  std::ranges::copy_if(
      beacons, std::back_inserter(beacons_on_row),
      [](const beacon_t& beacon) { return (beacon.y == inspect_row); });

  int num_empty = 0;
  for (int column = min_max.min_value.x - largest_distance;
       column <= min_max.max_value.x + largest_distance; ++column) {
    point current{column, inspect_row};
    bool within_range =
        std::ranges::any_of(sensors, [&](const sensor_t& sensor) {
          return (point::distance_manhattan(sensor.pos, current) <=
                  sensor.range);
        });
    if (!within_range) {
      continue;
    }
    bool is_empty =
        !std::ranges::any_of(sensors_on_row,
                             [&](const sensor_t& sensor) {
                               return sensor.pos.x == current.x;
                             }) &&
        !std::ranges::any_of(beacons_on_row, [&](const beacon_t& beacon) {
          return beacon.x == current.x;
        });
    if (is_empty) {
      ++num_empty;
    }
  }

  std::cout << filename << " -> " << num_empty << std::endl;
}

int main() {
  std::cout << "Part 1" << std::endl;
  solve_case<10, false>("day15.example");
  solve_case<2000000, false>("day15.input");
  // std::cout << "Part 2" << std::endl;
  // solve_case<10, true>("day15.example");
  // solve_case<2000000, true>("day15.input");
}
