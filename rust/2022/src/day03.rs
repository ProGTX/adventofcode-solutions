type Input = Vec<Vec<u8>>;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
        .into_iter()
        .map(|line| line.into_bytes())
        .collect()
}

// a-z -> bits 0-25 (priority 1-26)
// A-Z -> bits 26-51 (priority 27-52)
fn char_mask(c: u8) -> u64 {
    if c >= b'a' {
        1u64 << (c - b'a')
    } else {
        1u64 << (c - b'A' + 26)
    }
}

// Builds a bitmask: OR in each character's bit
// so we know which letters appear
fn line_mask(bytes: &[u8]) -> u64 {
    bytes.iter().fold(0u64, |mask, &c| mask | char_mask(c))
}

// Bit position == index, and index + 1 == priority,
// so trailing_zeros recovers the priority directly
fn mask_priority(mask: u64) -> i32 {
    mask.trailing_zeros() as i32 + 1
}

fn solve_case1(input: &Input) -> i32 {
    input
        .iter()
        .map(|line| {
            let mid = line.len() / 2;
            // AND the two halves: only bits set in both survive,
            // giving the shared letter
            mask_priority(line_mask(&line[..mid]) & line_mask(&line[mid..]))
        })
        .sum()
}

fn solve_case2(input: &Input) -> i32 {
    input
        .chunks(3)
        .map(|group| {
            // Start with all bits set, AND each line's mask:
            // only the badge letter's bit survives
            let common = group.iter().fold(!0u64, |acc, line| acc & line_mask(line));
            mask_priority(common)
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day03.example");
    aoc::expect_result!(157, solve_case1(&example));
    let input = parse("day03.input");
    aoc::expect_result!(7793, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(70, solve_case2(&example));
    aoc::expect_result!(2499, solve_case2(&input));
}
