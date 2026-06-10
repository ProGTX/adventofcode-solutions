use aoc::point::{Point, distance_manhattan};
use itertools::Itertools;

type Pos = Point<i64>;

const GALAXY: char = '#';

struct Input {
    space_rows: Vec<String>,
    row_populated: Vec<bool>,
    column_populated: Vec<bool>,
}

fn parse(filename: &str) -> Input {
    let space_rows = aoc::file::read_lines(filename);

    let row_populated: Vec<bool> = space_rows.iter().map(|row| row.contains(GALAXY)).collect();

    let num_columns = space_rows.first().map_or(0, String::len);
    let column_populated: Vec<bool> = (0..num_columns)
        .map(|column| {
            space_rows
                .iter()
                .any(|row| row.as_bytes()[column] == GALAXY as u8)
        })
        .collect();

    Input {
        space_rows,
        row_populated,
        column_populated,
    }
}

fn expand_space(input: &Input, factor: i64) -> Vec<Pos> {
    let Input {
        space_rows,
        row_populated,
        column_populated,
    } = input;

    let mut galaxies = Vec::new();
    let mut row = 0;
    for (r, current_row) in space_rows.iter().enumerate() {
        if !row_populated[r] {
            row += factor;
            continue;
        }
        let mut column = 0;
        for (c, &byte) in current_row.as_bytes().iter().enumerate() {
            if !column_populated[c] {
                column += factor;
                continue;
            }
            if byte == GALAXY as u8 {
                galaxies.push(Pos::new(column, row));
            }
            column += 1;
        }
        row += 1;
    }
    galaxies
}

fn sum_distances(galaxies: &[Pos]) -> i64 {
    galaxies
        .iter()
        .tuple_combinations()
        .map(|(&a, &b)| distance_manhattan(a, b))
        .sum()
}

fn solve_case(input: &Input, factor: i64) -> i64 {
    sum_distances(&expand_space(input, factor))
}

fn main() {
    println!("Part 1");
    let example = parse("day11.example");
    aoc::expect_result!(374, solve_case(&example, 2));
    let input = parse("day11.input");
    aoc::expect_result!(10228230, solve_case(&input, 2));

    println!("Part 2");
    aoc::expect_result!(1030, solve_case(&example, 10));
    aoc::expect_result!(8410, solve_case(&example, 100));
    aoc::expect_result!(82000210, solve_case(&example, 1000000));
    aoc::expect_result!(447073334102, solve_case(&input, 1000000));
}
