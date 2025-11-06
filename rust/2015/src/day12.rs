fn sum(s: &str) -> i32 {
    let mut value = 0;
    let mut s_view = s;
    while !s_view.is_empty() {
        let digit_pos = s_view.find(|c: char| c.is_digit(10));
        if digit_pos.is_none() {
            break;
        }
        let digit_pos = digit_pos.unwrap();
        debug_assert!(digit_pos > 0, "Assuming number not at beginning of string");
        let nondigit_pos = s_view[digit_pos + 1..].find(|c: char| !c.is_digit(10));
        debug_assert!(
            nondigit_pos.is_some(),
            "Assuming number is never at the end"
        );
        let nondigit_pos = nondigit_pos.unwrap() + digit_pos + 1;
        let current = s_view[digit_pos..nondigit_pos].parse::<i32>();
        debug_assert!(current.is_ok(), "Parsing should have succeeded");
        let current = current.unwrap();
        value += if s_view.chars().nth(digit_pos - 1).unwrap() == '-' {
            -current
        } else {
            current
        };
        s_view = &s_view[nondigit_pos..];
    }
    value
}

fn solve_case(filename: &str) -> i32 {
    let input = std::fs::read_to_string(filename).unwrap();
    sum(input.trim())
}

fn main() {
    println!("Part 1");
    assert_eq!(3, solve_case("day12.example"));
    assert_eq!(156366, solve_case("day12.input"));
    //println!("Part 2");
    //assert_eq!(3369156, solve_case::<50>("day12.example"));
    //assert_eq!(5103798, solve_case::<50>("day12.input"));
}
