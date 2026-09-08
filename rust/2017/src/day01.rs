use itertools::Itertools;

type Input = Vec<u8>;

fn parse(filename: &str) -> Input {
    aoc::file::read_string(filename)
        .trim()
        .bytes()
        .map(|b| b - b'0')
        .collect()
}

fn solve_case1(numbers: &[u8]) -> u32 {
    numbers
        .iter()
        .chain(&numbers[0..1])
        .array_windows::<2>()
        .map(|[a, b]| if a == b { *a as u32 } else { 0 })
        .sum()
}

fn solve_case2(numbers: &[u8]) -> u32 {
    let size = numbers.len();
    numbers
        .iter()
        .enumerate()
        .map(|(i, &a)| {
            let b = numbers[(i + (size / 2)) % size];
            if a == b { a as u32 } else { 0 }
        })
        .sum()
}

fn main() {
    println!("Part 1");
    aoc::expect_result!(3, solve_case1(&[1, 1, 2, 2]));
    aoc::expect_result!(4, solve_case1(&[1, 1, 1, 1]));
    aoc::expect_result!(0, solve_case1(&[1, 2, 3, 4]));
    let example = parse("day01.example");
    aoc::expect_result!(9, solve_case1(&example));
    let input = parse("day01.input");
    aoc::expect_result!(1203, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(6, solve_case2(&[1, 2, 1, 2]));
    aoc::expect_result!(0, solve_case2(&[1, 2, 2, 1]));
    aoc::expect_result!(4, solve_case2(&[1, 2, 3, 4, 2, 5]));
    aoc::expect_result!(12, solve_case2(&[1, 2, 3, 1, 2, 3]));
    aoc::expect_result!(4, solve_case2(&[1, 2, 1, 3, 1, 4, 1, 5]));
    aoc::expect_result!(6, solve_case2(&example));
    aoc::expect_result!(1146, solve_case2(&input));
}
