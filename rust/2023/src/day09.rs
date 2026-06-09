type Reading = Vec<i32>;
type Input = Vec<Reading>;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            line.split_whitespace()
                .map(|s| s.parse().unwrap())
                .collect()
        })
        .collect()
}

fn predict_value<const FORWARD: bool>(mut reading: Reading) -> i32 {
    let mult = if FORWARD { 1 } else { -1 };
    let mut edge_values = Vec::new();

    for _ in 0..reading.len() {
        edge_values.push(if FORWARD {
            *reading.last().unwrap()
        } else {
            *reading.first().unwrap()
        });
        let next_reading = reading.windows(2).map(|w| w[1] - w[0]).collect::<Reading>();
        if next_reading.iter().all(|&x| x == 0) {
            break;
        }
        reading = next_reading;
    }

    edge_values
        .iter()
        .rev()
        .fold(0i32, |acc, &e| e + acc * mult)
}

fn solve_case<const FORWARD: bool>(input: &Input) -> i32 {
    input
        .iter()
        .map(|r| predict_value::<FORWARD>(r.clone()))
        .sum()
}

fn main() {
    // Asserts
    assert_eq!(18, predict_value::<true>(vec![0, 3, 6, 9, 12, 15]));
    assert_eq!(28, predict_value::<true>(vec![1, 3, 6, 10, 15, 21]));
    assert_eq!(68, predict_value::<true>(vec![10, 13, 16, 21, 30, 45]));
    assert_eq!(-3, predict_value::<false>(vec![0, 3, 6, 9, 12, 15]));
    assert_eq!(0, predict_value::<false>(vec![1, 3, 6, 10, 15, 21]));
    assert_eq!(5, predict_value::<false>(vec![10, 13, 16, 21, 30, 45]));

    println!("Part 1");
    let example = parse("day09.example");
    aoc::expect_result!(114, solve_case::<true>(&example));
    let input = parse("day09.input");
    aoc::expect_result!(1980437560, solve_case::<true>(&input));

    println!("Part 2");
    aoc::expect_result!(2, solve_case::<false>(&example));
    aoc::expect_result!(977, solve_case::<false>(&input));
}
