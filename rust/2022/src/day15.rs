use std::collections::{BTreeMap, BTreeSet};

use aoc::point::distance_manhattan;

type Point = aoc::point::Point<i32>;

struct Sensor {
    pos: Point,
    range: i32,
}

struct Input {
    sensors: Vec<Sensor>,
    beacons: Vec<Point>,
}

fn parse_point(s: &str) -> Point {
    let (x_str, y_str) = s.split_once(',').unwrap();
    let x = x_str.trim()[2..].parse().unwrap();
    let y = y_str.trim()[2..].parse().unwrap();
    Point::new(x, y)
}

fn parse(filename: &str) -> Input {
    let mut sensors = Vec::new();
    let mut beacons = Vec::new();
    for line in aoc::file::read_lines(filename) {
        let (sensor_part, beacon_part) = line.split_once(':').unwrap();
        let sensor_pos = parse_point(sensor_part.strip_prefix("Sensor at ").unwrap());
        let beacon_pos = parse_point(beacon_part.strip_prefix(" closest beacon is at ").unwrap());
        let range = distance_manhattan(sensor_pos, beacon_pos);
        sensors.push(Sensor {
            pos: sensor_pos,
            range,
        });
        beacons.push(beacon_pos);
    }
    Input { sensors, beacons }
}

#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
struct Bounds {
    min: i32,
    max: i32,
}

fn find_positions(
    sensors: &[Sensor],
    beacons: &[Point],
    bounds: Bounds,
    inspect_row: i32,
) -> Vec<Point> {
    let sensors_on_row: Vec<_> = sensors.iter().filter(|s| s.pos.y == inspect_row).collect();
    let beacons_on_row: Vec<_> = beacons.iter().filter(|b| b.y == inspect_row).collect();
    (bounds.min..=bounds.max)
        .map(|column| Point::new(column, inspect_row))
        .filter(|&current| {
            sensors
                .iter()
                .any(|s| distance_manhattan(s.pos, current) <= s.range)
        })
        .filter(|current| {
            !sensors_on_row.iter().any(|s| s.pos.x == current.x)
                && !beacons_on_row.iter().any(|b| b.x == current.x)
        })
        .collect()
}

fn solve_case1<const INSPECT_ROW: i32>(input: &Input) -> i64 {
    let max_range = input.sensors.iter().map(|s| s.range).max().unwrap();
    let largest_distance = 2 * max_range + 1;
    let (min_x, max_x) = input
        .sensors
        .iter()
        .map(|s| s.pos.x)
        .chain(input.beacons.iter().map(|b| b.x))
        .fold((i32::MAX, i32::MIN), |(mn, mx), x| (mn.min(x), mx.max(x)));
    let bounds = Bounds {
        min: min_x - largest_distance,
        max: max_x + largest_distance,
    };
    find_positions(&input.sensors, &input.beacons, bounds, INSPECT_ROW).len() as i64
}

fn find_distress_beacon(sensors: &[Sensor], bounds: Bounds) -> Point {
    let mut row_exclusions: BTreeMap<i32, BTreeSet<Bounds>> = BTreeMap::new();
    let mut add_exclusion = |row: i32, sensor: &Sensor, width: i32| {
        if row < bounds.min || row > bounds.max {
            return;
        }
        let excl = Bounds {
            min: (sensor.pos.x - width).max(bounds.min),
            max: (sensor.pos.x + width).min(bounds.max),
        };
        if excl.min > excl.max {
            return;
        }
        row_exclusions.entry(row).or_default().insert(excl);
    };
    // Collect exclusion zones for each row
    for sensor in sensors {
        for (row, width) in ((sensor.pos.y - sensor.range)..sensor.pos.y).zip(0..) {
            add_exclusion(row, sensor, width);
        }
        for (row, width) in
            (sensor.pos.y..=(sensor.pos.y + sensor.range)).zip((0..=sensor.range).rev())
        {
            add_exclusion(row, sensor, width);
        }
    }
    // Try to find a gap in the exclusion zones
    for (row, exclusions) in &row_exclusions {
        let mut previous = Bounds {
            min: bounds.min,
            max: bounds.min,
        };
        for &excl in exclusions {
            if excl.min - previous.max > 0 {
                return Point::new(previous.max + 1, *row);
            }
            previous = Bounds {
                min: excl.min,
                max: excl.max.max(previous.max),
            };
        }
    }
    unreachable!("no distress beacon found")
}

fn solve_case2<const INSPECT_ROW: i32>(input: &Input) -> i64 {
    const MULTIPLIER: i64 = 4_000_000;
    let max_pos = if INSPECT_ROW == 10 {
        20
    } else {
        MULTIPLIER as i32
    };
    let beacon = find_distress_beacon(
        &input.sensors,
        Bounds {
            min: 0,
            max: max_pos,
        },
    );
    beacon.x as i64 * MULTIPLIER + beacon.y as i64
}

fn main() {
    println!("Part 1");
    let example = parse("day15.example");
    aoc::expect_result!(26, solve_case1::<10>(&example));
    let input = parse("day15.input");
    aoc::expect_result!(5870800, solve_case1::<2000000>(&input));

    println!("Part 2");
    aoc::expect_result!(56000011, solve_case2::<10>(&example));
    aoc::expect_result!(10908230916597_i64, solve_case2::<2000000>(&input));
}
