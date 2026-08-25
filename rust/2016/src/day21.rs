use itertools::Itertools;

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

/// The scrambling operations, and the password they are run on
type Input = (Vec<Op>, Vec<u8>);

fn parse(filename: &str) -> Input {
    let instructions = aoc::file::read_lines(filename)
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
        .collect_vec();
    // The example scrambles a shorter password than the real input
    let password = if (instructions.len() <= 8) {
        b"abcde".to_vec()
    } else {
        b"abcdefgh".to_vec()
    };
    return (instructions, password);
}

/// The scrambled password
fn solve_case1((instructions, password): &Input) -> String {
    let mut scrambled = password.clone();
    for op in instructions {
        match *op {
            Op::SwapPosition(ix, iy) => {
                scrambled.swap(ix, iy);
            }
            Op::SwapLetter(x, y) => {
                let ix = scrambled.iter().position(|c| *c == x).unwrap();
                let iy = scrambled.iter().position(|c| *c == y).unwrap();
                scrambled.swap(ix, iy);
            }
            Op::RotateLeft(steps) => {
                scrambled.rotate_left(steps);
            }
            Op::RotateRight(steps) => {
                scrambled.rotate_right(steps);
            }
            Op::RotateBasedOnLetterPos(x) => {
                let ix = scrambled.iter().position(|c| *c == x).unwrap();
                // The rotation can be longer than the password itself
                let steps = (1 + ix + ((ix >= 4) as usize)) % scrambled.len();
                scrambled.rotate_right(steps);
            }
            Op::ReversePositions(ifrom, ito) => {
                scrambled[ifrom..ito + 1].reverse();
            }
            Op::MovePosition(ifrom, ito) => {
                if (ifrom < ito) {
                    scrambled[ifrom..ito + 1].rotate_left(1);
                } else {
                    scrambled[ito..ifrom + 1].rotate_right(1);
                }
            }
        }
    }
    return String::from_utf8(scrambled).unwrap();
}

fn main() {
    println!("Part 1");
    let example = parse("day21.example");
    aoc::expect_result!("decab", solve_case1(&example));
    let input = parse("day21.input");
    aoc::expect_result!("baecdfgh", solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
