use std::fmt::Debug;
use std::fs::File;
use std::io::{self, BufRead};
use std::path::Path;
use std::str::FromStr;

pub fn read_string(filename: &str) -> String {
    std::fs::read_to_string(filename).unwrap()
}

// https://doc.rust-lang.org/rust-by-example/std_misc/file/read_lines.html
pub fn read_lines<P>(filename: P) -> io::Lines<io::BufReader<File>>
where
    P: AsRef<Path>,
{
    let file = File::open(filename).unwrap();
    io::BufReader::new(file).lines()
}

pub fn read_numbers<T>(filename: &str) -> Vec<T>
where
    T: FromStr,
    <T as FromStr>::Err: Debug,
{
    std::fs::read_to_string(filename)
        .unwrap()
        .trim()
        .lines()
        .map(|line| line.trim().parse::<T>().unwrap())
        .collect()
}
