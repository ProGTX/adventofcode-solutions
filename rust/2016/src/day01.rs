use aoc::direction::{Arrow, Direction};
use aoc::point::{Point, distance_manhattan};
use rustc_hash::FxHashSet;

type Pos = Point<i32>;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
enum Rotation {
    Clockwise,
    Anticlockwise,
}

fn parse(filename: &str) -> Vec<(Rotation, u32)> {
    aoc::file::read_string(filename)
        .trim()
        .split(", ")
        .map(|instruction| {
            let (rotation, blocks) = instruction.split_at(1);
            let rotation = match rotation {
                "R" => Rotation::Clockwise,
                "L" => Rotation::Anticlockwise,
                _ => panic!("Unknown rotation: {rotation}"),
            };
            (rotation, blocks.parse::<u32>().unwrap())
        })
        .collect()
}

fn solve_case<const DETECT_CYCLE: bool>(instructions: &[(Rotation, u32)]) -> i32 {
    let mut arrow = Arrow {
        pos: Pos::default(),
        dir: Direction::North,
    };

    let mut visited = FxHashSet::default();
    if DETECT_CYCLE {
        visited.insert(arrow.pos);
    }
    for &(rotation, blocks) in instructions {
        arrow.dir = match rotation {
            Rotation::Clockwise => arrow.dir.clockwise(),
            Rotation::Anticlockwise => arrow.dir.counterclockwise(),
        };
        if DETECT_CYCLE {
            // The first location visited twice can be mid-leg, so check every block
            for _ in 0..blocks {
                arrow.pos += arrow.dir.diff();
                if !visited.insert(arrow.pos) {
                    return distance_manhattan(Pos::default(), arrow.pos);
                }
            }
        } else {
            arrow.pos += arrow.dir.diff().scale(blocks as i32);
        }
    }

    return distance_manhattan(Pos::default(), arrow.pos);
}

fn main() {
    println!("Part 1");
    let example = parse("day01.example");
    aoc::expect_result!(12, solve_case::<false>(&example));
    let example2 = parse("day01.example2");
    aoc::expect_result!(8, solve_case::<false>(&example2));
    let input = parse("day01.input");
    aoc::expect_result!(279, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(12, solve_case::<true>(&example));
    aoc::expect_result!(4, solve_case::<true>(&example2));
    aoc::expect_result!(163, solve_case::<true>(&input));
}
