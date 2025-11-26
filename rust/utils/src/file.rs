use std::fmt::Debug;
use std::str::FromStr;

pub fn read_string(filename: &str) -> String {
    std::fs::read_to_string(filename).unwrap()
}

pub fn read_lines(filename: &str) -> Vec<String> {
    std::fs::read_to_string(filename)
        .unwrap()
        .trim()
        .lines()
        .map(|line| line.to_string())
        .collect()
}

pub fn read_numbers<T>(filename: &str) -> Vec<T>
where
    T: FromStr,
    <T as FromStr>::Err: Debug,
{
    read_lines(filename)
        .iter()
        .map(|line| line.trim().parse::<T>().unwrap())
        .collect()
}
