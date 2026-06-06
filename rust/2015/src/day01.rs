use std::fs::File;
use std::io::{self, BufRead, BufReader};

fn solve_case<const CHECK_POSITION: bool>(filename: &str) -> io::Result<i32> {
    let mut floor = 0;
    let mut position = 0;
    let file = File::open(filename)?;
    let mut line = String::new();
    BufReader::new(file).read_line(&mut line)?;
    for c in line.chars() {
        floor += (c == '(') as i32 - (c == ')') as i32;
        if CHECK_POSITION {
            position += 1;
            if floor == -1 {
                break;
            }
        }
    }
    Ok(if CHECK_POSITION { position } else { floor })
}

fn main() {
    println!("Part 1");
    aoc::expect_result!(-1, solve_case::<false>("day01.example").unwrap());
    aoc::expect_result!(280, solve_case::<false>("day01.input").unwrap());
    println!("Part 2");
    aoc::expect_result!(5, solve_case::<true>("day01.example").unwrap());
    aoc::expect_result!(1797, solve_case::<true>("day01.input").unwrap());
}
