// https://adventofcode.com/2022/day/15

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <print>
#include <ranges>
#endif

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

// Plain loop drop in for stdr::any_of
// Same early exit semantics, but without the iterator/view abstraction overhead
// that makes stdr::any_of costly in Debug builds
// Note that the predicate needs to be forced inline to match loop performance
template <class R, class F>
fn any_of(R const& range, F&& pred) -> bool {
  for (let& elem : range) {
    if (pred(elem)) {
      return true;
    }
  }
  return false;
}

// How far a sensor still reaches sideways along the inspected row
struct Reach {
  int x;
  int reach;
};

fn count_positions(Vec<Sensor> const& sensors, Vec<Beacon> const& beacons,
                   Bounds const& bounds, int inspect_row) -> i64 {
  let sensors_on_row = sensors |
                       stdv::filter([&](Sensor const& sensor) {
                         return sensor.pos.y == inspect_row;
                       }) |
                       aoc::collect_vec<Sensor>();
  let beacons_on_row = beacons |
                       stdv::filter([&](Beacon const& beacon) {
                         return beacon.y == inspect_row;
                       }) |
                       aoc::collect_vec<Beacon>();

  // Everything about a sensor that only depends on the row is the same for
  // every column, so compute it once instead of once per column.
  // A sensor whose reach comes out negative cannot cover this row at all.
  auto reaches = Vec<Reach>{};
  for (let& sensor : sensors) {
    let reach = sensor.range - std::abs(sensor.pos.y - inspect_row);
    if (reach >= 0) {
      reaches.emplace_back(sensor.pos.x, reach);
    }
  }

  auto count = i64{0};
  for (let column : Range{bounds.min, bounds.max + 1}) {
    let within_range = any_of(reaches, [&](Reach const& r) AOC_FORCE_INLINE {
      return std::abs(r.x - column) <= r.reach;
    });
    if (!within_range) {
      continue;
    }

    let is_known = //
        any_of(sensors_on_row,
               [&](Sensor const& sensor)
                   AOC_FORCE_INLINE { return sensor.pos.x == column; }) ||
        any_of(beacons_on_row, [&](Beacon const& beacon) AOC_FORCE_INLINE {
          return beacon.x == column;
        });
    if (is_known) {
      continue;
    }
    ++count;
  }
  return count;
}

template <int inspect_row>
fn solve_case1(Input const& input) -> i64 {
  let & [ sensors, beacons, min_max ] = input;
  let max_range = stdr::max(sensors, [](Sensor const& lhs, Sensor const& rhs) {
                    return lhs.range < rhs.range;
                  }).range;
  let largest_distance = 2 * max_range + 1;
  return count_positions(sensors, beacons,
                         {min_max.min_value.x - largest_distance,
                          min_max.max_value.x + largest_distance},
                         inspect_row);
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
