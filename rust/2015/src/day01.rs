use std::fs::File;
use std::io::{self, Read};

fn solve_case(filename: &str) -> io::Result<i32> {
    let mut file = File::open(filename)?;
    let mut contents = String::new();
    file.read_to_string(&mut contents)?;

    let floor = contents
        .chars()
        .map(|c| match c {
            '(' => 1,
            ')' => -1,
            _ => 0,
        })
        .sum();

    Ok(floor)
}

fn main() {
    println!("Hello world!");
    let example_result = solve_case("day01.example");
    assert_eq!(example_result, 3);
}
