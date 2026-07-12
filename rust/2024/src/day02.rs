fn parse(filename: &str) -> Vec<Vec<i32>> {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            line.split_whitespace()
                .map(|token| token.parse::<i32>().unwrap())
                .collect()
        })
        .collect()
}

// A report only counts as safe if both of the following are true:
//   The levels are either all increasing or all decreasing.
//   Any two adjacent levels differ by at least one and at most three.
fn is_report_safe(report: &[i32]) -> bool {
    let increasing_factor = (report[1] - report[0]).signum();
    report.windows(2).all(|w| {
        let diff = increasing_factor * (w[1] - w[0]);
        (1..=3).contains(&diff)
    })
}

fn is_dampened_safe(report: &[i32]) -> bool {
    (0..report.len()).any(|drop_index| {
        let candidate: Vec<i32> = report
            .iter()
            .enumerate()
            .filter(|&(i, _)| i != drop_index)
            .map(|(_, &level)| level)
            .collect();
        is_report_safe(&candidate)
    })
}

fn solve_case<const DAMPEN: bool>(reports: &[Vec<i32>]) -> i32 {
    reports
        .iter()
        .filter(|report| is_report_safe(report) || (DAMPEN && is_dampened_safe(report)))
        .count() as i32
}

fn main() {
    println!("Part 1");
    let example = parse("day02.example");
    aoc::expect_result!(2, solve_case::<false>(&example));
    let input = parse("day02.input");
    aoc::expect_result!(502, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(4, solve_case::<true>(&example));
    aoc::expect_result!(544, solve_case::<true>(&input));
}
