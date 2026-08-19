use aoc::{
    grid::{Grid, Upos},
    iter::CollectArrayVec,
};

enum Op {
    Rect,
    RotateRow,
    RotateColumn,
}

struct Instruction {
    op: Op,
    row: usize,
    column: usize,
}

type Input = Vec<Instruction>;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let (op, rest) = line.split_once(' ').unwrap();
            return match op {
                "rect" => {
                    let (a, b) = rest.split_once('x').unwrap();
                    Instruction {
                        op: Op::Rect,
                        row: b.parse().unwrap(),
                        column: a.parse().unwrap(),
                    }
                }
                "rotate" => {
                    let parts = rest.split(' ').collect_array_vec::<4>();
                    let a = parts[1][2..].parse().unwrap();
                    let b = parts[3].parse().unwrap();
                    match parts[0] {
                        "row" => Instruction {
                            op: Op::RotateRow,
                            row: a,
                            column: b,
                        },
                        "column" => Instruction {
                            op: Op::RotateColumn,
                            row: b,
                            column: a,
                        },
                        _ => unreachable!("Invalid rotation"),
                    }
                }
                _ => unreachable!("Invalid operation"),
            };
        })
        .collect()
}

fn solve_case1(instructions: &Input) -> u32 {
    const DIMS: Upos = Upos::new(50, 6);
    let mut screen = Grid::<bool>::new(false, DIMS.y, DIMS.x);
    for instruction in instructions {
        match instruction.op {
            Op::Rect => {
                for row in 0..instruction.row {
                    screen.row_mut(row)[..instruction.column].fill(true);
                }
            }
            Op::RotateRow => {
                screen
                    .row_mut(instruction.row)
                    .rotate_right(instruction.column);
            }
            Op::RotateColumn => {
                let mut new_column = screen
                    .column(instruction.column)
                    .cloned()
                    .collect_array_vec::<{ DIMS.y }>();
                new_column.rotate_right(instruction.row);
                for (old, new) in screen.column_mut(instruction.column).zip(new_column) {
                    *old = new;
                }
            }
        }
    }
    screen.data.iter().map(|&p| p as u32).sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day08.example");
    aoc::expect_result!(6, solve_case1(&example));
    let input = parse("day08.input");
    aoc::expect_result!(119, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
    //aoc::expect_result!(1337, solve_case2(&example));
    //aoc::expect_result!(1337, solve_case2(&input));
}
