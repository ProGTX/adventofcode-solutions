// https://adventofcode.com/2022/day/15

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>

struct Sensor {
  point pos;
  int range;
};

struct Bounds {
  int min{0};
  int max{0};

  constexpr bool operator==(const Bounds&) const = default;
  constexpr auto operator<=>(const Bounds&) const = default;
};

using Beacon = point;

struct Input {
  Vec<Sensor> sensors;
  Vec<Beacon> beacons;
  aoc::min_max_helper min_max;
};

fn parse(String const& filename) -> Input {
  Input result;

  let convert_eq_str = [](str s) {
    // Works for "x=" and "y="
    return aoc::to_number<int>(aoc::trim(s).substr(2));
  };

  for (str line : aoc::views::read_lines(filename)) {
    auto [sensor_info, beacon_info] = aoc::split_once(line, ':');

    // Don't include the space at end, the null terminator is counted instead
    sensor_info = sensor_info.substr(sizeof("Sensor at"));
    beacon_info = beacon_info.substr(sizeof("closest beacon is at"));

    let beacon = [&] {
      let[x_eq, y_eq] = aoc::split_once(beacon_info, ',');
      let beacon = point{convert_eq_str(x_eq), convert_eq_str(y_eq)};
      result.min_max.update(beacon);
      result.beacons.push_back(beacon);
      return beacon;
    }();
    {
      let[x_eq, y_eq] = aoc::split_once(sensor_info, ',');
      let sensor = point{convert_eq_str(x_eq), convert_eq_str(y_eq)};
      result.min_max.update(sensor);
      let range = distance_manhattan(sensor, beacon);
      result.sensors.emplace_back(sensor, range);
    }
  }

  return result;
}

template <bool find_distress>
fn find_positions(Vec<Sensor> const& sensors, Vec<Beacon> const& beacons,
                  Bounds const& bounds, int inspect_row) -> Vec<point> {
  let sensors_on_row = sensors |
                       stdv::filter([&](Sensor const& sensor) {
                         return sensor.pos.y == inspect_row;
                       }) |
                       aoc::ranges::to<Vec<Sensor>>();
  let beacons_on_row = beacons |
                       stdv::filter([&](Beacon const& beacon) {
                         return beacon.y == inspect_row;
                       }) |
                       aoc::ranges::to<Vec<Beacon>>();
  return //
      Range{bounds.min, bounds.max + 1} |
      stdv::transform([&](int column) { return point{column, inspect_row}; }) |
      stdv::filter([&](point current) {
        let within_range = stdr::any_of(sensors, [&](Sensor const& sensor) {
          return distance_manhattan(sensor.pos, current) <= sensor.range;
        });
        return within_range != find_distress;
      }) |
      stdv::filter([&](point current) {
        return !stdr::any_of(sensors_on_row, [&](Sensor const& sensor) {
          return sensor.pos.x == current.x;
        }) && !stdr::any_of(beacons_on_row, [&](Beacon const& beacon) {
          return beacon.x == current.x;
        });
      }) |
      aoc::ranges::to<Vec<point>>();
}

template <int inspect_row>
fn solve_case1(Input const& input) -> i64 {
  let & [ sensors, beacons, min_max ] = input;
  let max_range = stdr::max(sensors, [](Sensor const& lhs, Sensor const& rhs) {
                    return lhs.range < rhs.range;
                  }).range;
  let largest_distance = 2 * max_range + 1;
  return find_positions<false>(sensors, beacons,
                               {min_max.min_value.x - largest_distance,
                                min_max.max_value.x + largest_distance},
                               inspect_row)
      .size();
}

fn find_distress_beacon(Vec<Sensor> const& sensors, Bounds const& bounds)
    -> Beacon {
  // Rotate coords: u = x+y, v = x-y.
  // Each diamond becomes an axis-aligned square.
  // The uncovered point lies at the intersection
  // of boundary lines from two sensors.
  auto u_lines = Vec<int>{};
  auto v_lines = Vec<int>{};
  for (let& sensor : sensors) {
    let u_center = sensor.pos.x + sensor.pos.y;
    let v_center = sensor.pos.x - sensor.pos.y;
    let outer_range = sensor.range + 1;
    u_lines.push_back(u_center + outer_range);
    u_lines.push_back(u_center - outer_range);
    v_lines.push_back(v_center + outer_range);
    v_lines.push_back(v_center - outer_range);
  }
  for (let u : u_lines) {
    for (let v : v_lines) {
      if ((u + v) % 2 != 0) {
        continue;
      }
      let x = (u + v) / 2;
      let y = (u - v) / 2;
      if (x < bounds.min ||
          x > bounds.max ||
          y < bounds.min ||
          y > bounds.max) {
        continue;
      }
      let candidate = point{x, y};
      if (stdr::all_of(sensors, [&](Sensor const& sensor) {
            return distance_manhattan(sensor.pos, candidate) > sensor.range;
          })) {
        return candidate;
      }
    }
  }
  return {};
}

template <int inspect_row>
fn solve_case2(Input const& input) -> i64 {
  constexpr i64 multiplier = 4000000;
  let max_pos = (inspect_row == 10) ? 20 : static_cast<int>(multiplier);
  let beacon = find_distress_beacon(input.sensors, {0, max_pos});
  return beacon.x * multiplier + beacon.y;
}

int main() {
  std::println("Part 1");
  let example = parse("day15.example");
  AOC_EXPECT_RESULT(26, solve_case1<10>(example));
  let input = parse("day15.input");
  AOC_EXPECT_RESULT(5870800, solve_case1<2000000>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(56000011, solve_case2<10>(example));
  AOC_EXPECT_RESULT(10908230916597, solve_case2<2000000>(input));

  AOC_RETURN_CHECK_RESULT();
}
