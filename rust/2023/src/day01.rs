fn parse(filename: &str) -> Vec<String> {
    aoc::file::read_lines(filename)
}

fn find_number(line: &str) -> i32 {
    let digit_value = |c: char| c.to_digit(10).unwrap() as i32;
    let first_digit = line.chars().find(|c| c.is_ascii_digit()).unwrap();
    let last_digit = line.chars().rfind(|c| c.is_ascii_digit()).unwrap();
    digit_value(first_digit) * 10 + digit_value(last_digit)
}

const WORDS: [&str; 20] = [
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "zero", "one", "two", "three", "four",
    "five", "six", "seven", "eight", "nine",
];

fn find_number_from_words(line: &str) -> i32 {
    let first_index = WORDS
        .iter()
        .enumerate()
        .filter_map(|(word_index, word)| line.find(word).map(|pos| (word_index, pos)))
        .min_by_key(|&(_, pos)| pos)
        .unwrap()
        .0;
    let last_index = WORDS
        .iter()
        .enumerate()
        .filter_map(|(word_index, word)| line.rfind(word).map(|pos| (word_index, pos)))
        .max_by_key(|&(_, pos)| pos)
        .unwrap()
        .0;
    (first_index % 10) as i32 * 10 + (last_index % 10) as i32
}

fn solve_case1(lines: &[String]) -> i32 {
    lines.iter().map(|line| find_number(line)).sum()
}

fn solve_case2(lines: &[String]) -> i32 {
    lines.iter().map(|line| find_number_from_words(line)).sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day01.example");
    aoc::expect_result!(142, solve_case1(&example));
    let input = parse("day01.input");
    aoc::expect_result!(54388, solve_case1(&input));

    println!("Part 2");
    let example2 = parse("day01.example2");
    aoc::expect_result!(281, solve_case2(&example2));
    aoc::expect_result!(53515, solve_case2(&input));
}
