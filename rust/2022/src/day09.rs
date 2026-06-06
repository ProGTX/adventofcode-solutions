use std::collections::HashSet;

type Point = aoc::point::Point<i32>;
type Moves = Vec<Point>;
type Visited = HashSet<Point>;

fn parse_move(line: &str) -> Point {
    let (operation, value_str) = line.split_once(' ').unwrap();
    let value = value_str.parse::<i32>().unwrap();
    match operation {
        "R" => Point::new(value, 0),
        "L" => Point::new(-value, 0),
        "U" => Point::new(0, value),
        "D" => Point::new(0, -value),
        _ => panic!("Invalid command {operation}"),
    }
}

fn parse(filename: &str) -> Moves {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| parse_move(line))
        .collect()
}

fn move_tail<const NUM_KNOTS: usize>(rope: &mut [Point; NUM_KNOTS], visited: &mut Visited) {
    for i in 1..NUM_KNOTS {
        let in_front = rope[i - 1];
        let current = &mut rope[i];
        let diff = in_front - *current;
        let diff_abs = diff.abs();
        if diff_abs.x >= 2 || diff_abs.y >= 2 {
            current.x += diff.x.signum();
            current.y += diff.y.signum();
        }
    }
    visited.insert(*rope.last().unwrap());
}

fn solve_case<const NUM_KNOTS: usize>(moves: &[Point]) -> usize {
    let mut rope = [Point::default(); NUM_KNOTS];
    let mut visited = Visited::new();
    visited.insert(*rope.last().unwrap());

    for diff in moves {
        // Can only move vertically or horizontally
        if (diff.x * diff.y) != 0 {
            panic!("Diagonal movement");
        }

        let diff_abs = diff.abs();
        if diff.x != 0 {
            for _ in 0..diff_abs.x {
                rope[0].x += diff.x.signum();
                move_tail(&mut rope, &mut visited);
            }
        } else {
            for _ in 0..diff_abs.y {
                rope[0].y += diff.y.signum();
                move_tail(&mut rope, &mut visited);
            }
        }
    }

    visited.len()
}

fn main() {
    println!("Part 1");
    let example = parse("day09.example");
    aoc::expect_result!(13, solve_case::<2>(&example));
    let input = parse("day09.input");
    aoc::expect_result!(5695, solve_case::<2>(&input));

    println!("Part 2");
    aoc::expect_result!(1, solve_case::<10>(&example));
    let example2 = parse("day09.example2");
    aoc::expect_result!(36, solve_case::<10>(&example2));
    aoc::expect_result!(2434, solve_case::<10>(&input));
}
