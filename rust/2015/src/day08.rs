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

fn solve_case1(filename: &str) -> u32 {
    std::fs::read_to_string(filename)
        .unwrap()
        .lines()
        .map(num_control_chars)
        .sum()
}

fn solve_case2(filename: &str) -> u32 {
    std::fs::read_to_string(filename)
        .unwrap()
        .lines()
        .map(num_encode_chars)
        .sum()
}

fn main() {
    println!("Part 1");
    assert_eq!(12, solve_case1("day08.example"));
    assert_eq!(1333, solve_case1("day08.input"));
    println!("Part 2");
    assert_eq!(19, solve_case2("day08.example"));
    assert_eq!(2046, solve_case2("day08.input"));
}
