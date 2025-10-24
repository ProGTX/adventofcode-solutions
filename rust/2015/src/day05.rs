use std::io;

fn contains_three_vowels(s: &str) -> bool {
    let mut count = 0;
    for c in s.chars() {
        count += "aeiuo".contains(c) as i32;
        if count >= 3 {
            return true;
        }
    }
    return false;
}

const BAD_STRINGS: [&str; 4] = ["ab", "cd", "pq", "xy"];

fn double_letters_ok(s: &str) -> bool {
    let mut repeated = false;
    for (first, second) in s.chars().zip(s.chars().skip(1)) {
        if BAD_STRINGS.contains(&String::from_iter([first, second]).as_str()) {
            return false;
        }
        repeated |= first == second;
    }
    return repeated;
}

fn is_nice1(s: &str) -> bool {
    contains_three_vowels(s) && double_letters_ok(s)
}

fn is_nice2(s: &str) -> bool {
    let mut pair_repeats = false;
    let mut letter_repeats = false;
    for (index, c) in s[0..s.len() - 1].chars().enumerate() {
        let pair = &s[index..index + 2];
        let rest = &s[index + 2..];
        pair_repeats |= rest.contains(pair);
        letter_repeats |= (rest.len() > 0) && (c == rest.chars().next().unwrap());
    }
    pair_repeats && letter_repeats
}

fn solve_case1(filename: &str) -> io::Result<i32> {
    Ok(std::fs::read_to_string(filename)?
        .lines()
        .filter(|s| is_nice1(s))
        .count() as i32)
}

fn solve_case2(filename: &str) -> io::Result<i32> {
    Ok(std::fs::read_to_string(filename)?
        .lines()
        .filter(|s| is_nice2(s))
        .count() as i32)
}

fn main() {
    println!("Part 1");
    assert_eq!(2, solve_case1("day05.example").unwrap());
    assert_eq!(258, solve_case1("day05.input").unwrap());
    println!("Part 2");
    assert_eq!(1, solve_case2("day05.example").unwrap());
    assert_eq!(53, solve_case2("day05.input").unwrap());
}
