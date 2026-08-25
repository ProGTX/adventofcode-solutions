use std::mem::swap;

/// The first row of tiles
type Input = String;

const SAFE_CHAR: char = '.';
const TRAP_CHAR: char = '^';
const SAFE: u8 = SAFE_CHAR as u8;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)[0].clone()
}

/// The number of safe tiles in the whole room
fn solve_case1<const NUM_ROWS: usize>(first_row: &Input) -> usize {
    let mut current_row = first_row.clone();
    let mut next_row = String::new();
    (0..NUM_ROWS)
        .map(move |_| {
            let num_safe = current_row.bytes().filter(|tile| *tile == SAFE).count();
            // A new tile is a trap exactly when its two neighbors differ,
            // which is what all four of the trap rules amount to
            let tiles = current_row.as_bytes();
            next_row.clear();
            for index in 0..tiles.len() {
                // The walls on either side of the room count as safe
                let left = if (index == 0) { SAFE } else { tiles[index - 1] };
                let right = *tiles.get(index + 1).unwrap_or(&SAFE);
                next_row.push(if (left == right) {
                    SAFE_CHAR
                } else {
                    TRAP_CHAR
                });
            }
            swap(&mut current_row, &mut next_row);
            num_safe
        })
        .sum()
}

fn main() {
    println!("Part 1");
    aoc::expect_result!(6, solve_case1::<3>(&"..^^.".to_string()));
    let example = parse("day18.example");
    aoc::expect_result!(38, solve_case1::<10>(&example));
    let input = parse("day18.input");
    aoc::expect_result!(1913, solve_case1::<40>(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
