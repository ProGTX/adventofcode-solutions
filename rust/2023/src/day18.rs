use aoc::direction::Direction;
use aoc::point::{Point, calculate_area};

type Pos = Point<i64>;

const HEX_DIRECTION_MAPPING: [Direction; 4] = [
    Direction::East,
    Direction::South,
    Direction::West,
    Direction::North,
];

fn parse(filename: &str) -> Vec<String> {
    aoc::file::read_lines(filename)
}

fn trench_step(direction: Direction, num_cubes: i64) -> (Pos, i64) {
    (direction.diff().cast::<i64>(), num_cubes)
}

fn lagoon_size(trench: &[Pos], trench_length: i64) -> i64 {
    let area: i64 = calculate_area(trench);
    // https://en.wikipedia.org/wiki/Pick%27s_theorem
    // A = i + b/2 - 1
    //   -> i = A - b/2 + 1
    // But we also need to add back b to account for the area of the edge
    //   -> A + b/2 + 1
    area + (trench_length / 2) + 1
}

fn solve(lines: &[String], parse_line: impl Fn(&str) -> (Pos, i64)) -> i64 {
    let mut trench = Vec::new();
    let mut trench_length = 0i64;
    let mut current = Pos::default();

    for line in lines {
        let (diff, num_cubes) = parse_line(line);
        trench.push(current);
        current += Pos::new(diff.x * num_cubes, diff.y * num_cubes);
        trench_length += num_cubes;
    }

    lagoon_size(&trench, trench_length)
}

fn solve_case1(lines: &[String]) -> i64 {
    solve(lines, |line| {
        let direction = match line.as_bytes()[0] {
            b'R' => Direction::East,
            b'D' => Direction::South,
            b'L' => Direction::West,
            b'U' => Direction::North,
            _ => unreachable!("Invalid direction"),
        };
        let (num_cubes_str, _color_str) = line[2..].split_once(' ').unwrap();
        let num_cubes = num_cubes_str.parse().unwrap();
        trench_step(direction, num_cubes)
    })
}

fn solve_case2(lines: &[String]) -> i64 {
    solve(lines, |line| {
        let (_, hex_str) = line[2..].split_once(' ').unwrap();
        let hex_bytes = hex_str.as_bytes();
        let direction = HEX_DIRECTION_MAPPING[(hex_bytes[7] - b'0') as usize];
        let num_cubes = i64::from_str_radix(&hex_str[2..7], 16).unwrap();
        trench_step(direction, num_cubes)
    })
}

fn main() {
    println!("Part 1");
    let example = parse("day18.example");
    aoc::expect_result!(62, solve_case1(&example));
    let input = parse("day18.input");
    aoc::expect_result!(40745, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(952408144115, solve_case2(&example));
    aoc::expect_result!(90111113594927, solve_case2(&input));
}
