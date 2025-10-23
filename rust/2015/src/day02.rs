use std::fs::File;
use std::io::{self, BufRead, BufReader};

fn solve_case<const CHECK_POSITION: bool>(filename: &str) -> io::Result<i32> {
    Ok(0)
}

fn main() {
    println!("Part 1");
    assert_eq!((58 + 43), solve_case::<false>("day02.example").unwrap());
    assert_eq!(1606483, solve_case::<false>("day02.input").unwrap());
    // println!("Part 2");
    // assert_eq!(5, solve_case::<true>("day02.example").unwrap());
    // assert_eq!(1797, solve_case::<true>("day02.input").unwrap());
}
