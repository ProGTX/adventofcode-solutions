// https://adventofcode.com/2022/day/15

#include "../common/common.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <ostream>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <vector>

struct sensor_t {
  point pos;
  int range;

  friend std::ostream& operator<<(std::ostream& out, const sensor_t& sensor) {
    out << sensor.pos << "," << sensor.range;
    return out;
  }
};

struct bounds_t {
  int min{0};
  int max{0};

  friend bool operator==(const bounds_t&, const bounds_t&) = default;

  friend bool operator<(const bounds_t& lhs, const bounds_t& rhs) {
    if (lhs.min < rhs.min) {
      return true;
    }
    return (lhs.min == rhs.min) && (lhs.max < rhs.max);
  }
};

using beacon_t = point;

template <bool find_distress>
std::vector<point> find_positions(const std::vector<sensor_t>& sensors,
                                  const std::vector<beacon_t>& beacons,
                                  const bounds_t& bounds, int inspect_row) {
  std::vector<sensor_t> sensors_on_row;
  std::ranges::copy_if(
      sensors, std::back_inserter(sensors_on_row),
      [&](const sensor_t& sensor) { return (sensor.pos.y == inspect_row); });
  std::vector<beacon_t> beacons_on_row;
  std::ranges::copy_if(
      beacons, std::back_inserter(beacons_on_row),
      [&](const beacon_t& beacon) { return (beacon.y == inspect_row); });

  std::vector<point> positions;

  for (int column = bounds.min; column <= bounds.max; ++column) {
    point current{column, inspect_row};
    bool within_range =
        std::ranges::any_of(sensors, [&](const sensor_t& sensor) {
          return (distance_manhattan(sensor.pos, current) <= sensor.range);
        });
    if (within_range == find_distress) {
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
      positions.push_back(current);
    }
  }

  return positions;
}

beacon_t find_distress_beacon(const std::vector<sensor_t>& sensors,
                              const bounds_t& bounds) {
  std::map<int, std::set<bounds_t>> row_exclusions;

  const auto add_exclusion = [&](int row, const sensor_t& sensor, int width) {
    if ((row < bounds.min) || (row > bounds.max)) {
      return;
    }
    bounds_t exclusion{std::max(sensor.pos.x - width, bounds.min),
                       std::min(sensor.pos.x + width, bounds.max)};
    if (exclusion.min > exclusion.max) {
      return;
    }
    row_exclusions[row].insert(exclusion);
  };

  // Collect exclusion zones for each row
  for (const sensor_t& sensor : sensors) {
    for (int row = sensor.pos.y - sensor.range, width = 0; row < sensor.pos.y;
         ++row, ++width) {
      add_exclusion(row, sensor, width);
    }
    for (int row = sensor.pos.y, width = sensor.range;
         row <= sensor.pos.y + sensor.range; ++row, --width) {
      add_exclusion(row, sensor, width);
    }
  }

  // Try to find a gap in the exclusion zones
  for (const auto& [row, exclusions] : row_exclusions) {
    bounds_t previous{bounds.min, bounds.min};
    for (const auto& row_exclusion : exclusions) {
      if (row_exclusion.min - previous.max > 0) {
        return point{previous.max + 1, row};
      }
      previous = bounds_t{row_exclusion.min,
                          std::max(row_exclusion.max, previous.max)};
    }
  }

  return {};
}

template <int inspect_row, bool find_distress>
std::int64_t solve_case(const std::string& filename) {
  std::vector<sensor_t> sensors;
  std::vector<beacon_t> beacons;

  const auto convert_eq_str = [](std::string_view str) {
    // Works for "x=" and "y="
    return to_number<int>(trim(str).substr(2));
  };

  min_max_helper min_max;

  readfile_op(filename, [&](std::string_view line) {
    auto [sensor_info, beacon_info] =
        split<std::array<std::string_view, 2>>(line, ':');

    // Don't include the space at end, the null terminator is counted instead
    sensor_info = sensor_info.substr(sizeof("Sensor at"));
    beacon_info = beacon_info.substr(sizeof("closest beacon is at"));

    point beacon;
    point sensor;
    {
      auto [x_eq, y_eq] =
          split<std::array<std::string_view, 2>>(beacon_info, ',');
      beacon = point{convert_eq_str(x_eq), convert_eq_str(y_eq)};
      min_max.update(beacon);
      beacons.push_back(beacon);
    }
    {
      auto [x_eq, y_eq] =
          split<std::array<std::string_view, 2>>(sensor_info, ',');
      sensor = point{convert_eq_str(x_eq), convert_eq_str(y_eq)};
      min_max.update(sensor);
      auto range = distance_manhattan(sensor, beacon);
      sensors.emplace_back(sensor, range);
    }
  });

  // Result for part 2 is huge, store it into a std::int64_t
  std::int64_t score = 0;

  if constexpr (!find_distress) {
    auto max_range =
        std::ranges::max(sensors, [](const sensor_t& lhs, const sensor_t& rhs) {
          return lhs.range < rhs.range;
        }).range;
    const auto largest_distance = 2 * max_range + 1;
    auto positions =
        find_positions<find_distress>(sensors, beacons,
                                      {min_max.min_value.x - largest_distance,
                                       min_max.max_value.x + largest_distance},
                                      inspect_row);
    score = positions.size();
  } else {
    constexpr std::int64_t multiplier = 4000000;
    const int max_pos = (inspect_row == 10) ? 20 : multiplier;
    auto beacon = find_distress_beacon(sensors, {0, max_pos});
    score = beacon.x * multiplier + beacon.y;
  }

  std::cout << filename << " -> " << score << std::endl;
  return score;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(26, (solve_case<10, false>("day15.example")));
  AOC_EXPECT_RESULT(5870800, (solve_case<2000000, false>("day15.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(56000011, (solve_case<10, true>("day15.example")));
  AOC_EXPECT_RESULT(10908230916597, (solve_case<2000000, true>("day15.input")));
  AOC_RETURN_CHECK_RESULT();
}
