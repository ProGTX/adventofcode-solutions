type Input = Vec<i32>;

fn parse(filename: &str) -> Input {
    Input::default()
}

fn solve_case1(input: &Input) -> i32 {
    0
}

fn main() {
    println!("Part 1");
    let example = parse("day01.example");
    aoc::return_incomplete();
    // aoc::expect_result!(1337, solve_case1(&example));
    let input = parse("day01.input");
    // aoc::expect_result!(1337, solve_case1(&input));

    println!("Part 2");
    // aoc::expect_result!(1337, solve_case2(&example));
    // aoc::expect_result!(1337, solve_case2(&input));
}
