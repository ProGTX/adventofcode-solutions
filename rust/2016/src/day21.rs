use itertools::{Either, Itertools};

/// A scrambling operation, taken from the first two words of a line,
/// with the position or letter values it works on
#[derive(Clone, Copy, Debug)]
enum Op {
    SwapPosition(usize, usize),
    SwapLetter(u8, u8),
    ReversePositions(usize, usize),
    RotateLeft(usize),
    RotateRight(usize),
    RotateBasedOnLetterPos(u8),
    MovePosition(usize, usize),
}

/// The scrambling operations
type Input = Vec<Op>;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let words = line.split_whitespace().collect_vec();
            let index = |word: usize| words[word].parse::<usize>().unwrap();
            let letter = |word: usize| words[word].as_bytes()[0];
            match (words[0], words[1]) {
                // swap position X with position Y
                ("swap", "position") => Op::SwapPosition(index(2), index(5)),
                // swap letter X with letter Y
                ("swap", "letter") => Op::SwapLetter(letter(2), letter(5)),
                // reverse positions X through Y
                ("reverse", "positions") => Op::ReversePositions(index(2), index(4)),
                // rotate left X steps
                ("rotate", "left") => Op::RotateLeft(index(2)),
                // rotate right X steps
                ("rotate", "right") => Op::RotateRight(index(2)),
                // rotate based on position of letter X
                ("rotate", "based") => Op::RotateBasedOnLetterPos(letter(6)),
                // move position X to position Y
                ("move", "position") => Op::MovePosition(index(2), index(5)),
                _ => panic!("Invalid instruction: {line}"),
            }
        })
        .collect_vec()
}

/// The scrambled password, or with `REVERSE`,
/// the password that scrambles into the given one
fn solve_case<const REVERSE: bool>(instructions: &Input, password: &[u8]) -> String {
    let mut password = password.to_vec();
    // Undoing the scrambling means walking the operations backwards,
    // each one undone rather than applied
    let ops = if (REVERSE) {
        Either::Left(instructions.iter().rev())
    } else {
        Either::Right(instructions.iter())
    };
    for op in ops {
        match *op {
            Op::SwapPosition(ix, iy) => {
                password.swap(ix, iy);
            }
            Op::SwapLetter(x, y) => {
                let ix = password.iter().position(|c| *c == x).unwrap();
                let iy = password.iter().position(|c| *c == y).unwrap();
                password.swap(ix, iy);
            }
            Op::RotateLeft(steps) => {
                if (REVERSE) {
                    password.rotate_right(steps);
                } else {
                    password.rotate_left(steps);
                }
            }
            Op::RotateRight(steps) => {
                if (REVERSE) {
                    password.rotate_left(steps);
                } else {
                    password.rotate_right(steps);
                }
            }
            Op::RotateBasedOnLetterPos(x) => {
                let len = password.len();
                let ix = password.iter().position(|c| *c == x).unwrap();
                if (REVERSE) {
                    // Which index the letter must have been at
                    // for the rotation to have left it where it is now
                    let ix_before = (0..len)
                        .find(|before| ((2 * before + 1 + ((*before >= 4) as usize)) % len) == ix)
                        .unwrap();
                    let steps = (ix + len - ix_before) % len;
                    password.rotate_left(steps);
                } else {
                    // The rotation can be longer than the password itself
                    let steps = (1 + ix + ((ix >= 4) as usize)) % len;
                    password.rotate_right(steps);
                }
            }
            Op::ReversePositions(ifrom, ito) => {
                password[ifrom..ito + 1].reverse();
            }
            Op::MovePosition(ifrom, ito) => {
                // Moving a letter back is the same move the other way round
                let (ifrom, ito) = if (REVERSE) {
                    (ito, ifrom)
                } else {
                    (ifrom, ito)
                };
                if (ifrom < ito) {
                    password[ifrom..ito + 1].rotate_left(1);
                } else {
                    password[ito..ifrom + 1].rotate_right(1);
                }
            }
        }
    }
    return String::from_utf8(password).unwrap();
}

fn main() {
    println!("Part 1");
    let example = parse("day21.example");
    aoc::expect_result!("decab", solve_case::<false>(&example, b"abcde"));
    let input = parse("day21.input");
    aoc::expect_result!("baecdfgh", solve_case::<false>(&input, b"abcdefgh"));

    println!("Part 2");
    // De-scrambling doesn't work on example input
    aoc::expect_result!("abcdefgh", solve_case::<true>(&input, b"baecdfgh"));
    aoc::expect_result!("cegdahbf", solve_case::<true>(&input, b"fbgdceah"));
}
