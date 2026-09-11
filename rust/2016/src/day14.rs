use aoc::{
    md5::{self, md5_many, md5_stretch},
    string,
};
use std::thread;

type Hash = md5::Digest;

/// How many indices are hashed side by side
const LANES: usize = md5::LANES;

/// How far ahead a candidate's confirming run of five is looked for
const LOOKAHEAD: u32 = 1000;

const NUM_DIGITS: usize = 2 * size_of::<Hash>();

/// How many distinct values a hex digit can take
const NUM_VALUES: usize = 16;

fn parse(filename: &str) -> String {
    return aoc::file::read_string(filename).trim().to_string();
}

/// The hashes of `count` consecutive indices starting at `first`.
/// One index never depends on another,
/// so a whole batch of them goes through the SIMD lanes at once
fn hashes<const NUM_STRETCHES: u32>(salt: &str, first: u32, count: usize) -> [Hash; LANES] {
    // The salt and the index, which is as long as the index is wide
    let salt_size = salt.len();
    let mut buffers = [[0; 16]; LANES];
    let mut sizes = [0; LANES];
    for (lane, buffer) in buffers.iter_mut().enumerate().take(count) {
        buffer[..salt_size].copy_from_slice(salt.as_bytes());
        sizes[lane] = salt_size + string::write_u32(&mut buffer[salt_size..], first + lane as u32);
    }
    let mut hashes = [Hash::default(); LANES];
    let inputs: [&[u8]; LANES] = std::array::from_fn(|lane| &buffers[lane][..sizes[lane]]);
    md5_many(&inputs[..count], &mut hashes);

    // Every stretch hashes the 32 hex digits of the hash before it,
    // which the whole run does without the lanes coming back out
    md5_stretch(&mut hashes[..count], NUM_STRETCHES as usize);
    return hashes;
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
#[derive(Clone, Copy, Default)]
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

/// Fills in the runs of the hashes starting at index `first`, one per slot.
/// The indices are independent, so the slots are split across the available threads
fn find_runs_range<const NUM_STRETCHES: u32>(salt: &str, first: u32, runs: &mut [Runs]) {
    /// Below this a thread does not earn what it costs to start
    const MIN_HASHES_PER_THREAD: usize = 1 << 12;
    let num_hashes = runs.len() * (1 + NUM_STRETCHES as usize);
    let num_threads = thread::available_parallelism()
        .map_or(1, |n| n.get())
        .min(num_hashes.div_ceil(MIN_HASHES_PER_THREAD))
        .max(1);
    let fill = |first: u32, runs: &mut [Runs]| {
        for (batch, slots) in runs.chunks_mut(LANES).enumerate() {
            let start = first + (batch * LANES) as u32;
            let hashes = hashes::<NUM_STRETCHES>(salt, start, slots.len());
            for (slot, hash) in slots.iter_mut().zip(&hashes) {
                *slot = find_runs(hash);
            }
        }
    };
    let chunk = runs.len().div_ceil(num_threads);
    thread::scope(|scope| {
        for (index, slots) in runs.chunks_mut(chunk).enumerate() {
            let start = first + (index * chunk) as u32;
            scope.spawn(move || fill(start, slots));
        }
    });
}

/// How many keys are needed to fill out the one-time pad
const NUM_KEYS: u32 = 64;

fn solve_case<const NUM_STRETCHES: u32>(salt: &str) -> u32 {
    // A candidate needs the LOOKAHEAD hashes after it, so a block of that many
    // can only be checked once a second block is there to confirm against.
    // Both blocks live in this one buffer, which every refill overwrites in place
    let mut runs = vec![Runs::default(); 2 * LOOKAHEAD as usize];
    find_runs_range::<NUM_STRETCHES>(salt, 0, &mut runs);
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
        runs.copy_within(LOOKAHEAD as usize.., 0);
        find_runs_range::<NUM_STRETCHES>(salt, first + LOOKAHEAD, &mut runs[LOOKAHEAD as usize..]);
    }
}

fn main() {
    println!("Part 1");
    let example = parse("day14.example");
    aoc::expect_result!(22728, solve_case::<0>(&example));
    let input = parse("day14.input");
    aoc::expect_result!(18626, solve_case::<0>(&input));

    println!("Part 2");
    aoc::expect_result!(22551, solve_case::<2016>(&example));
    aoc::expect_result!(20092, solve_case::<2016>(&input));
}
