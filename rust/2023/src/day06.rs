struct Input {
    time_tokens: Vec<String>,
    dist_tokens: Vec<String>,
}

fn parse(filename: &str) -> Input {
    let lines = aoc::file::read_lines(filename);
    Input {
        time_tokens: lines[0]["Time:".len()..].split_whitespace().map(String::from).collect(),
        dist_tokens: lines[1]["Distance:".len()..].split_whitespace().map(String::from).collect(),
    }
}

// d=v*tr
// tr=t-tb
// v=tb
//
// -> d=tb*t-tb^2
fn calculate_distance(total_time: i64, button_time: i64) -> i64 {
    button_time * total_time - button_time * button_time
}

fn num_ways_to_win(total_time: i64, record_distance: i64) -> i64 {
    (1..total_time)
        .filter(|&button_time| calculate_distance(total_time, button_time) > record_distance)
        .count() as i64
}

fn solve_case1(input: &Input) -> i64 {
    let total_times: Vec<i64> = input.time_tokens.iter().map(|s| s.parse().unwrap()).collect();
    let record_distances: Vec<i64> = input.dist_tokens.iter().map(|s| s.parse().unwrap()).collect();
    debug_assert_eq!(
        total_times.len(),
        record_distances.len(),
        "Number of times must match number of distances"
    );
    total_times
        .iter()
        .zip(record_distances.iter())
        .map(|(&t, &d)| num_ways_to_win(t, d))
        .product()
}

fn solve_case2(input: &Input) -> i64 {
    let time_str: String = input.time_tokens.concat();
    let dist_str: String = input.dist_tokens.concat();
    num_ways_to_win(time_str.parse().unwrap(), dist_str.parse().unwrap())
}

fn main() {
    println!("Part 1");
    let example = parse("day06.example");
    aoc::expect_result!(288, solve_case1(&example));
    let input = parse("day06.input");
    aoc::expect_result!(1624896, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(71503, solve_case2(&example));
    aoc::expect_result!(32583852, solve_case2(&input));
}
