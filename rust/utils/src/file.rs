use std::fmt::Debug;
use std::str::FromStr;

pub fn read_numbers<T>(filename: &str) -> Vec<T>
where
    T: FromStr,
    <T as FromStr>::Err: Debug,
{
    std::fs::read_to_string(filename)
        .unwrap()
        .lines()
        .map(|line| line.trim().parse::<T>().unwrap())
        .collect()
}
