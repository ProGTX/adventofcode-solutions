use std::fs::File;
use std::io::{self, BufRead, BufReader};

fn parse(filename: &str) -> io::Result<String> {
    let file = File::open(filename)?;
    let mut line = String::new();
    BufReader::new(file).read_line(&mut line)?;
    Ok(line)
}

fn solve_case<const CHECK_POSITION: bool>(directions: &str) -> i32 {
    let mut floor = 0;
    let mut position = 0;
    for c in directions.chars() {
        floor += (c == '(') as i32 - (c == ')') as i32;
        if CHECK_POSITION {
            position += 1;
            if floor == -1 {
                break;
            }
        }
    }
    if CHECK_POSITION { position } else { floor }
}

fn main() {
    println!("Part 1");
    let example = parse("day01.example").unwrap();
    aoc::expect_result!(-1, solve_case::<false>(&example));
    let input = parse("day01.input").unwrap();
    aoc::expect_result!(280, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(5, solve_case::<true>(&example));
    aoc::expect_result!(1797, solve_case::<true>(&input));
}
