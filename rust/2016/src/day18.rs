/// The first row of tiles
type Input = String;

const SAFE: u8 = b'.';
const TRAP: u8 = b'^';

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)[0].clone()
}

/// The number of safe tiles in the whole room
fn solve_case<const NUM_ROWS: usize>(first_row: &Input) -> usize {
    // The row is kept as raw bytes, and never changes length.
    // A safe tile pads either end, standing in for the walls of the room,
    // so that no tile is ever a special case.
    // The padding is never written to, and so stays safe for every row
    let num_tiles = first_row.len();
    let mut row = Vec::with_capacity(num_tiles + 2);
    row.push(SAFE);
    row.extend_from_slice(first_row.as_bytes());
    row.push(SAFE);
    (0..NUM_ROWS)
        .map(move |_| {
            let mut num_safe = 0;
            // The row becomes the next one in place:
            // the only tile a new one overwrites is the one to its left,
            // so remembering that single tile is enough to look back at
            let mut left = SAFE;
            for index in 1..=num_tiles {
                let current = row[index];
                if (current == SAFE) {
                    num_safe += 1;
                }
                // A new tile is a trap exactly when its two neighbors differ,
                // which is what all four of the trap rules amount to
                row[index] = if (left == row[index + 1]) { SAFE } else { TRAP };
                left = current;
            }
            num_safe
        })
        .sum()
}

fn main() {
    println!("Part 1");
    aoc::expect_result!(6, solve_case::<3>(&"..^^.".to_string()));
    let example = parse("day18.example");
    aoc::expect_result!(38, solve_case::<10>(&example));
    let input = parse("day18.input");
    aoc::expect_result!(1913, solve_case::<40>(&input));

    println!("Part 2");
    aoc::expect_result!(1935478, solve_case::<400000>(&example));
    aoc::expect_result!(19993564, solve_case::<400000>(&input));
}
