use aoc::direction::{Arrow, Direction};
use aoc::grid::Grid;
use aoc::point::Point;

type Pos = Point<i32>;

const EMPTY: char = '.';
const MIRROR_RIGHT: char = '/';
const MIRROR_LEFT: char = '\\';
const SPLITTER_VERTICAL: char = '|';
const SPLITTER_HORIZONTAL: char = '-';
const ENERGIZED: char = '#';

fn parse(filename: &str) -> Grid<char> {
    Grid::<char>::from_file(filename)
}

#[derive(Clone, Copy, PartialEq)]
struct Beam {
    pos: Pos,
    direction: Pos,
}

fn neg(p: Pos) -> Pos {
    Pos::new(-p.x, -p.y)
}

fn split_beam(
    beams_tracker: &mut Vec<Beam>,
    beams: &mut Vec<Beam>,
    pos: Pos,
    direction: Pos,
) -> Pos {
    // Treat splitter as-if mirror_left
    let direction = Pos::new(direction.y, direction.x);
    // Create a new beam as-if mirror_right
    let beam2 = Beam {
        pos: pos - direction,
        direction: neg(direction),
    };
    if !beams_tracker.contains(&beam2) {
        beams_tracker.push(beam2);
        beams.push(beam2);
    }
    // Modify current beam as-if mirror_left
    let beam1 = Beam {
        pos: pos + direction,
        direction,
    };
    if !beams_tracker.contains(&beam1) {
        beams_tracker.push(beam1);
        return direction;
    }
    Pos::default()
}

fn energize(machine: &Grid<char>, start: Beam) -> i32 {
    let mut energized_machine = machine.clone();

    let mut beams_tracker = vec![start];
    let mut beams = vec![start];

    while let Some(Beam {
        mut pos,
        mut direction,
    }) = beams.pop()
    {
        while machine.in_bounds_signed(pos.y as isize, pos.x as isize) {
            let value = *machine.get(pos.y as usize, pos.x as usize);
            match value {
                EMPTY => {}
                MIRROR_LEFT => std::mem::swap(&mut direction.x, &mut direction.y),
                MIRROR_RIGHT => {
                    direction = neg(direction);
                    std::mem::swap(&mut direction.x, &mut direction.y);
                }
                SPLITTER_VERTICAL => {
                    if direction.y == 0 {
                        direction = split_beam(&mut beams_tracker, &mut beams, pos, direction);
                    }
                }
                SPLITTER_HORIZONTAL => {
                    if direction.x == 0 {
                        direction = split_beam(&mut beams_tracker, &mut beams, pos, direction);
                    }
                }
                _ => unreachable!("Invalid value in the machine"),
            }
            if direction == Pos::default() {
                // Current beam already encountered, stop bouncing
                break;
            }
            energized_machine.modify(ENERGIZED, pos.y as usize, pos.x as usize);
            pos += direction;
        }
    }

    energized_machine
        .data
        .iter()
        .filter(|&&c| c == ENERGIZED)
        .count() as i32
}

fn solve_case1(machine: &Grid<char>) -> i32 {
    energize(
        machine,
        Beam {
            pos: Pos::default(),
            direction: Direction::East.diff(),
        },
    )
}

fn solve_case2(machine: &Grid<char>) -> i32 {
    assert_eq!(
        machine.num_rows, machine.num_columns,
        "We're assuming an n*n square grid for simplicity"
    );
    let n = machine.num_rows as i32;
    let mut beams = Vec::new();
    for i in 0..n {
        beams.push(Beam {
            pos: Pos::new(0, i),
            direction: Direction::East.diff(),
        });
        beams.push(Beam {
            pos: Pos::new(n - 1, i),
            direction: Direction::West.diff(),
        });
        beams.push(Beam {
            pos: Pos::new(i, 0),
            direction: Direction::South.diff(),
        });
        beams.push(Beam {
            pos: Pos::new(i, n - 1),
            direction: Direction::North.diff(),
        });
    }
    beams
        .iter()
        .map(|&beam| energize(machine, beam))
        .max()
        .unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day16.example");
    aoc::expect_result!(46, solve_case1(&example));
    let input = parse("day16.input");
    aoc::expect_result!(7798, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(51, solve_case2(&example));
    aoc::expect_result!(8026, solve_case2(&input));
}
