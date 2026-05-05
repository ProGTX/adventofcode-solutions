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

fn solve_case<const WINDOW_SIZE: usize>(filename: &str) -> Vec<usize> {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| find_marker::<WINDOW_SIZE>(line))
        .collect()
}

fn main() {
    println!("Part 1");
    assert_eq!(vec![7, 5, 6, 10, 11], solve_case::<4>("day06.example"));
    assert_eq!(vec![1361], solve_case::<4>("day06.input"));

    println!("Part 2");
    assert_eq!(vec![19, 23, 23, 29, 26], solve_case::<14>("day06.example"));
    assert_eq!(vec![3263], solve_case::<14>("day06.input"));
}
