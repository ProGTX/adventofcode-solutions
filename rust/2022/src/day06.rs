// Returns the 1-indexed position after the first WINDOW_SIZE chars
// where all chars are distinct
fn find_marker<const WINDOW_SIZE: usize>(s: &str) -> usize {
    s.as_bytes()
        .windows(WINDOW_SIZE)
        .position(|w| {
            // Set one bit per letter (a=bit 0, b=bit 1, ...)
            let mask = w.iter().fold(0u32, |acc, &b| {
                return acc | (1 << (b - b'a'));
            });
            // If all WINDOW_SIZE chars are distinct,
            // exactly WINDOW_SIZE bits will be set
            mask.count_ones() as usize == WINDOW_SIZE
        })
        .unwrap()
        + WINDOW_SIZE // Convert window start index to end position (1-indexed)
}

fn parse(filename: &str) -> Vec<String> {
    aoc::file::read_lines(filename)
}

fn solve_case<const WINDOW_SIZE: usize>(lines: &[String]) -> Vec<usize> {
    lines
        .iter()
        .map(|line| find_marker::<WINDOW_SIZE>(line))
        .collect()
}

fn main() {
    println!("Part 1");
    let example = parse("day06.example");
    assert_eq!(vec![7, 5, 6, 10, 11], solve_case::<4>(&example));
    let input = parse("day06.input");
    assert_eq!(vec![1361], solve_case::<4>(&input));

    println!("Part 2");
    assert_eq!(vec![19, 23, 23, 29, 26], solve_case::<14>(&example));
    assert_eq!(vec![3263], solve_case::<14>(&input));
}
