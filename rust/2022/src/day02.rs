type Input = Vec<(u8, u8)>;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
        .into_iter()
        .map(|line| {
            let b = line.into_bytes();
            (b[0], b[2])
        })
        .collect()
}

fn solve_case1(input: &Input) -> i32 {
    // Hands: rock=0, paper=1, scissors=2
    input
        .iter()
        .map(|(opp, resp)| {
            let opponent = (opp - b'A') as i32;
            let response = (resp - b'X') as i32;
            // Add 1 because hand values are zero-based
            let mut score = response + 1;
            if opponent == response {
                score += 3;
            } else if (response == 0 && opponent == 2) || (response - opponent == 1) {
                score += 6;
            }
            score
        })
        .sum()
}

fn solve_case2(input: &Input) -> i32 {
    // Outcomes: lose=0, draw=3, win=6
    input
        .iter()
        .map(|(opp, out)| {
            let opponent = (opp - b'A') as i32;
            let outcome = match out {
                b'X' => 0,
                b'Y' => 3,
                b'Z' => 6,
                _ => unreachable!(),
            };
            let mut score = outcome;
            if outcome == 3 {
                // Add 1 because hand values are zero-based
                score += opponent + 1;
            } else {
                let response = if outcome == 6 {
                    (opponent + 1) % 3
                } else {
                    (3 + opponent - 1) % 3
                };
                // Add 1 because hand values are zero-based
                score += response + 1;
            }
            score
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day02.example");
    aoc::expect_result!(15, solve_case1(&example));
    let input = parse("day02.input");
    aoc::expect_result!(14375, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(12, solve_case2(&example));
    aoc::expect_result!(10274, solve_case2(&input));
}
