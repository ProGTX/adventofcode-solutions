fn num_control_chars(s: &str) -> u32 {
    assert!(
        (s.len() >= 2)
            && (s.chars().next().unwrap() == '"')
            && (s.chars().next_back().unwrap() == '"'),
        "Doesn't contain a string literal"
    );
    let mut count = 0;
    let mut sv = s;
    while !sv.is_empty() {
        let backslash = sv.find('\\');
        if backslash.is_none() {
            break;
        }
        let skip = match sv.chars().nth(backslash.unwrap() + 1).unwrap() {
            'x' => {
                count += 3;
                4
            }
            _ => {
                count += 1;
                2
            }
        };
        sv = &sv[backslash.unwrap() + skip..];
    }
    return count + 2;
}

fn num_encode_chars(s: &str) -> u32 {
    (s.chars().filter(|&c| (c == '"') || (c == '\\')).count() + 2) as u32
}

fn parse(filename: &str) -> Vec<String> {
    std::fs::read_to_string(filename)
        .unwrap()
        .lines()
        .map(String::from)
        .collect()
}

fn solve_case1(lines: &[String]) -> u32 {
    lines.iter().map(|line| num_control_chars(line)).sum()
}

fn solve_case2(lines: &[String]) -> u32 {
    lines.iter().map(|line| num_encode_chars(line)).sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day08.example");
    aoc::expect_result!(12, solve_case1(&example));
    let input = parse("day08.input");
    aoc::expect_result!(1333, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(19, solve_case2(&example));
    aoc::expect_result!(2046, solve_case2(&input));
}
