use std::collections::HashSet;

use aoc::point::Point;

type Dots = HashSet<Point<usize>>;
type Folds = Vec<Point<usize>>;
type Input = (Dots, Folds);

fn parse(filename: &str) -> Input {
    let mut dots = HashSet::new();
    let mut folds = Vec::new();
    let mut parsing_folds = false;
    for line in aoc::file::read_lines(filename) {
        if (line.is_empty()) {
            parsing_folds = true;
        } else if (parsing_folds) {
            let rest = line.strip_prefix("fold along ").unwrap();
            let (axis, value) = rest.split_once('=').unwrap();
            let n: usize = value.parse().unwrap();
            folds.push(if (axis == "x") {
                Point { x: n, y: 0 }
            } else {
                Point { x: 0, y: n }
            });
        } else {
            dots.insert(Point::parse(&line, ",").unwrap());
        }
    }
    (dots, folds)
}

fn fold_once(dots: &Dots, fold: Point<usize>) -> Dots {
    let mut new_dots = Dots::new();
    for dot in dots {
        if (fold.x == 0) {
            if (dot.y < fold.y) {
                new_dots.insert(*dot);
            } else {
                let new_y = fold.y - (dot.y - fold.y);
                new_dots.insert(Point { x: dot.x, y: new_y });
            }
        } else {
            if (dot.x < fold.x) {
                new_dots.insert(*dot);
            } else {
                let new_x = fold.x - (dot.x - fold.x);
                new_dots.insert(Point { x: new_x, y: dot.y });
            }
        }
    }
    return new_dots;
}

fn solve_case1((dots, folds): &Input) -> usize {
    fold_once(dots, folds[0]).len()
}

fn dots_to_string(dots: &Dots) -> String {
    let max_x = dots.iter().map(|p| p.x).max().unwrap_or(0);
    let max_y = dots.iter().map(|p| p.y).max().unwrap_or(0);
    (0..=max_y)
        .map(|y| {
            (0..=max_x)
                .map(|x| {
                    if (dots.contains(&Point { x, y })) {
                        '#'
                    } else {
                        '.'
                    }
                })
                .collect::<String>()
        })
        .collect::<Vec<_>>()
        .join("\n")
}

fn solve_case2((dots, folds): &Input) -> Dots {
    let mut dots = dots.clone();
    for fold in folds {
        dots = fold_once(&dots, *fold);
    }
    println!("{}", dots_to_string(&dots));
    dots
}

fn main() {
    println!("Part 1");

    let example = parse("day13.example");
    aoc::expect_result!(17, solve_case1(&example));
    let input = parse("day13.input");
    aoc::expect_result!(785, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(
        "\
#####
#...#
#...#
#...#
#####",
        dots_to_string(&solve_case2(&example))
    );
    aoc::expect_result!(
        "\
####...##..##..#..#...##..##...##..#..#
#.......#.#..#.#..#....#.#..#.#..#.#..#
###.....#.#..#.####....#.#....#..#.####
#.......#.####.#..#....#.#.##.####.#..#
#....#..#.#..#.#..#.#..#.#..#.#..#.#..#
#.....##..#..#.#..#..##...###.#..#.#..#",
        dots_to_string(&solve_case2(&input))
    );
}
