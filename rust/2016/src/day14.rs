use aoc::{
    md5::{self, md5},
    string,
};
use itertools::Itertools;

type Hash = md5::Digest;

/// How far ahead a candidate's confirming run of five is looked for
const LOOKAHEAD: u32 = 1000;

const NUM_DIGITS: usize = 2 * size_of::<Hash>();

/// How many distinct values a hex digit can take
const NUM_VALUES: usize = 16;

fn parse(filename: &str) -> String {
    return aoc::file::read_string(filename).trim().to_string();
}

fn hash(salt: &str, index: u32) -> Hash {
    let salt_size = salt.len();
    let mut buffer = [0; 16];
    buffer[..salt_size].copy_from_slice(salt.as_bytes());
    let size = salt_size + string::write_u32(&mut buffer[salt_size..], index);
    return md5(&buffer[..size]);
}

/// The hash as it is written out: one hex digit per nibble
fn digit(hash: &Hash, index: usize) -> u8 {
    let byte = hash[index / 2];
    return if ((index % 2) == 0) {
        byte >> 4
    } else {
        byte & 0xf
    };
}

/// What a single hash contributes
#[derive(Clone, Copy)]
struct Runs {
    /// The digit of its first run of three
    candidate_digit: Option<u8>,
    /// Which digits appear in a run of five, indexed by digit
    confirming_digits: [bool; NUM_VALUES],
}

/// Both answers come out of one pass over the hash's digits
fn find_runs(hash: &Hash) -> Runs {
    let mut runs = Runs {
        candidate_digit: None,
        confirming_digits: [false; NUM_VALUES],
    };
    let mut length = 1;
    for index in 1..NUM_DIGITS {
        let current = digit(hash, index);
        if (current == digit(hash, index - 1)) {
            length += 1;
        } else {
            length = 1;
        }
        if (length == 3) && runs.candidate_digit.is_none() {
            runs.candidate_digit = Some(current);
        }
        if (length >= 5) {
            runs.confirming_digits[current as usize] = true;
        }
    }
    return runs;
}

/// How many keys are needed to fill out the one-time pad
const NUM_KEYS: u32 = 64;

fn solve_case1(salt: &str) -> u32 {
    // A candidate needs the LOOKAHEAD hashes after it, so a block of that many
    // can only be checked once a second block is there to confirm against
    let mut runs = (0..(2 * LOOKAHEAD))
        .map(|index| find_runs(&hash(salt, index)))
        .collect_vec();
    // The candidate is the first hash of the window, the rest is what confirms it
    let is_key = |window: &[Runs]| -> bool {
        let Some(wanted) = window[0].candidate_digit else {
            return false;
        };
        return window[1..=LOOKAHEAD as usize]
            .iter()
            .any(|runs| runs.confirming_digits[wanted as usize]);
    };

    let mut num_keys = 0;
    let mut first = 0;
    loop {
        for offset in 0..LOOKAHEAD {
            if (is_key(&runs[offset as usize..])) {
                num_keys += 1;
                if (num_keys == NUM_KEYS) {
                    return first + offset;
                }
            }
        }

        // Drop the block just checked, and read another one in ahead of the search
        first += LOOKAHEAD;
        runs.drain(..LOOKAHEAD as usize);
        runs.extend(
            (first + LOOKAHEAD..first + 2 * LOOKAHEAD).map(|index| find_runs(&hash(salt, index))),
        );
    }
}

fn main() {
    println!("Part 1");
    let example = parse("day14.example");
    aoc::expect_result!(22728, solve_case1(&example));
    let input = parse("day14.input");
    aoc::expect_result!(18626, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
