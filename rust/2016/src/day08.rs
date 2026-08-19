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

fn solve_case<const PRINT: bool>(instructions: &Input) -> String {
    const LIT: char = '#';
    const DIMS: Upos = Upos::new(50, 6);
    let mut screen = Grid::<char>::new(' ', DIMS.y, DIMS.x);
    for instruction in instructions {
        match instruction.op {
            Op::Rect => {
                for row in 0..instruction.row {
                    screen.row_mut(row)[..instruction.column].fill(LIT);
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
    if !PRINT {
        screen
            .data
            .into_iter()
            .filter(|&p| p == LIT)
            .count()
            .to_string()
    } else {
        screen.data.into_iter().collect()
    }
}

fn main() {
    println!("Part 1");
    let example = parse("day08.example");
    aoc::expect_result!("6", solve_case::<false>(&example));
    let input = parse("day08.input");
    aoc::expect_result!("119", solve_case::<false>(&input));

    println!("Part 2");

    const EXAMPLE: &str = concat!(
        "    # #                                           ",
        "# #                                               ",
        " #                                                ",
        " #                                                ",
        "                                                  ",
        "                                                  "
    );
    aoc::expect_result!(EXAMPLE, solve_case::<true>(&example));

    const INPUT: &str = concat!(
        "#### #### #  # ####  ### ####  ##   ##  ###   ##  ",
        "   # #    #  # #    #    #    #  # #  # #  # #  # ",
        "  #  ###  #### ###  #    ###  #  # #    #  # #  # ",
        " #   #    #  # #     ##  #    #  # # ## ###  #  # ",
        "#    #    #  # #       # #    #  # #  # #    #  # ",
        "#### #    #  # #    ###  #     ##   ### #     ##  ",
    );
    aoc::expect_result!(INPUT, solve_case::<true>(&input));
}
