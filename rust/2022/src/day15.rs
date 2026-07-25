use aoc::point::distance_manhattan;
use itertools::Itertools;

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

/// How far a sensor still reaches sideways along the inspected row
struct Reach {
    x: i32,
    reach: i32,
}

fn count_positions(
    sensors: &[Sensor],
    beacons: &[Point],
    bounds: Bounds,
    inspect_row: i32,
    //
) -> i64 {
    let sensors_on_row: Vec<_> = sensors.iter().filter(|s| s.pos.y == inspect_row).collect();
    let beacons_on_row: Vec<_> = beacons.iter().filter(|b| b.y == inspect_row).collect();

    // Everything about a sensor that only depends on the row is the same for
    // every column, so compute it once instead of once per column.
    // A sensor whose reach comes out negative cannot cover this row at all.
    let reaches = sensors
        .iter()
        .filter_map(|s| {
            let reach = s.range - (s.pos.y - inspect_row).abs();
            (reach >= 0).then_some(Reach { x: s.pos.x, reach })
        })
        .collect_vec();

    let mut count = 0_i64;
    for column in bounds.min..=bounds.max {
        if !reaches.iter().any(|r| (r.x - column).abs() <= r.reach) {
            continue;
        }
        if sensors_on_row.iter().any(|s| s.pos.x == column)
            || beacons_on_row.iter().any(|b| b.x == column)
        {
            continue;
        }
        count += 1;
    }
    count
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
    count_positions(&input.sensors, &input.beacons, bounds, INSPECT_ROW)
}

fn find_distress_beacon(sensors: &[Sensor], bounds: Bounds) -> Point {
    // Rotate coords: u = x+y, v = x-y.
    // Each diamond becomes an axis-aligned square.
    // The uncovered point lies at the intersection
    // of boundary lines from two sensors.
    // Each sensor contributes u = (sx+sy) +/- (range+1)
    // and v = (sx-sy) +/- (range+1).
    let mut u_lines = Vec::new();
    let mut v_lines = Vec::new();
    for sensor in sensors {
        let u_center = sensor.pos.x + sensor.pos.y;
        let v_center = sensor.pos.x - sensor.pos.y;
        let outer_range = sensor.range + 1;
        u_lines.push(u_center + outer_range);
        u_lines.push(u_center - outer_range);
        v_lines.push(v_center + outer_range);
        v_lines.push(v_center - outer_range);
    }
    for &u in &u_lines {
        for &v in &v_lines {
            if (u + v) % 2 != 0 {
                continue;
            }
            let x = (u + v) / 2;
            let y = (u - v) / 2;
            if x < bounds.min || x > bounds.max || y < bounds.min || y > bounds.max {
                continue;
            }
            let candidate = Point::new(x, y);
            if sensors
                .iter()
                .all(|s| distance_manhattan(s.pos, candidate) > s.range)
            {
                return candidate;
            }
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
