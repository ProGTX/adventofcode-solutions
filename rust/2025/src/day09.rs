use itertools::Itertools;

type Point = aoc::point::Point<i64>;

fn parse(filename: &str) -> Vec<Point> {
    let parse = |s: &str| s.parse::<i64>().unwrap();
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let coords = line.split(',').collect::<Vec<_>>();
            Point::new(parse(coords[0]), parse(coords[1]))
        })
        .collect()
}

fn solve_case1(points: &Vec<Point>) -> u64 {
    points
        .iter()
        .cartesian_product(points.iter())
        .filter(|(a, b)| a < b)
        .map(|(&a, &b)| {
            let diff = (b - a).abs() + Point::new(1, 1);
            (diff.x as u64) * (diff.y as u64)
        })
        .max()
        .unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day09.example");
    assert_eq!(50, solve_case1(&example));
    let input = parse("day09.input");
    assert_eq!(4744899849, solve_case1(&input));

    // println!("Part 2");
    // assert_eq!(25272, solve_case::<{ usize::MAX }>(&example));
    // assert_eq!(100011612, solve_case::<{ usize::MAX }>(&input));
}
