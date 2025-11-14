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

#[derive(PartialEq)]
enum ScopeType {
    Object(bool), // bool indicates whether it's red or not
    Array,
}

fn sum_non_red(s: &str) -> i32 {
    let mut scope_stack = Vec::<ScopeType>::new();
    let mut value_stack = Vec::<i32>::new();
    value_stack.push(0);
    let mut value_str = String::new();
    for c in s.chars() {
        if ((c == '-') || c.is_ascii_alphanumeric()) {
            value_str.push(c);
        } else if (!value_str.is_empty()) {
            let first = value_str.chars().next().unwrap();
            if ((first == '-') || first.is_ascii_digit()) {
                let value = value_str.parse::<i32>().unwrap();
                let top = value_stack.last_mut().unwrap();
                *top += value;
            } else if (value_str == "red") {
                *scope_stack.last_mut().unwrap() = ScopeType::Object(true);
            }
            value_str.clear();
        }

        let red =
            !scope_stack.is_empty() && (scope_stack.last().unwrap() == &ScopeType::Object(true));
        match c {
            '{' => {
                value_stack.push(0);
                scope_stack.push(ScopeType::Object(false));
            }
            '}' => {
                let top = value_stack.pop().unwrap();
                if (!red) {
                    *value_stack.last_mut().unwrap() += top;
                }
                scope_stack.pop();
            }
            '[' => {
                value_stack.push(0);
                scope_stack.push(ScopeType::Array);
            }
            ']' => {
                let top = value_stack.pop().unwrap();
                *value_stack.last_mut().unwrap() += top;
                scope_stack.pop();
            }
            _ => {}
        }
    }
    *value_stack.last().unwrap()
}

fn solve_case1(filename: &str) -> i32 {
    let input = std::fs::read_to_string(filename).unwrap();
    sum(input.trim())
}

fn solve_case2(filename: &str) -> i32 {
    let input = std::fs::read_to_string(filename).unwrap();
    sum_non_red(input.trim())
}

fn main() {
    println!("Part 1");
    assert_eq!(6, solve_case1("day12.example"));
    assert_eq!(156366, solve_case1("day12.input"));
    println!("Part 2");
    assert_eq!(4, solve_case2("day12.example"));
    assert_eq!(96852, solve_case2("day12.input"));
}
