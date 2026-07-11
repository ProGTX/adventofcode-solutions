use rustc_hash::FxHashMap;

struct Input {
    left: Vec<i32>,
    right: Vec<i32>,
}

fn parse(filename: &str) -> Input {
    let (left, right) = aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let mut numbers = line.split_whitespace();
            let left = numbers.next().unwrap().parse::<i32>().unwrap();
            let right = numbers.next().unwrap().parse::<i32>().unwrap();
            (left, right)
        })
        .unzip();
    Input { left, right }
}

fn solve_case1(input: &Input) -> i32 {
    let mut left_list = input.left.clone();
    let mut right_list = input.right.clone();
    left_list.sort_unstable();
    right_list.sort_unstable();

    left_list
        .iter()
        .zip(right_list.iter())
        .map(|(&left, &right)| (left - right).abs())
        .sum()
}

fn solve_case2(input: &Input) -> i32 {
    let mut right_counts = FxHashMap::default();
    for &value in &input.right {
        *right_counts.entry(value).or_insert(0) += 1;
    }

    input
        .left
        .iter()
        .map(|value| value * right_counts.get(value).copied().unwrap_or(0))
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day01.example");
    aoc::expect_result!(11, solve_case1(&example));
    let input = parse("day01.input");
    aoc::expect_result!(3569916, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(31, solve_case2(&example));
    aoc::expect_result!(26407426, solve_case2(&input));
}
