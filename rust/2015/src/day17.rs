use itertools::Itertools;
use std::ops::{Add, Mul};

fn dot_product<T>(a: &[T], b: &[T]) -> T
where
    T: Copy + Default + Add<Output = T> + Mul<Output = T>,
{
    assert_eq!(a.len(), b.len());
    let mut acc = T::default();
    for (&x, &y) in a.iter().zip(b) {
        acc = acc + x * y;
    }
    acc
}

fn solve_case<const LITERS: u32, const COUNT_WAYS: bool>(filename: &str) -> u32 {
    let containers = std::fs::read_to_string(filename)
        .unwrap()
        .lines()
        .map(|line| line.parse().unwrap())
        .collect::<Vec<u32>>();
    let mut count = [0; 21];
    for combination in std::iter::repeat([0, 1])
        .take(containers.len())
        .multi_cartesian_product()
    {
        let exact_match = dot_product(&containers, &combination) == LITERS;
        if (!COUNT_WAYS) {
            count[0] += exact_match as u32;
        } else if (exact_match) {
            let num_used = combination.iter().sum::<u32>();
            count[num_used as usize] += 1;
        }
    }
    if (!COUNT_WAYS) {
        return count[0];
    } else {
        return *count.iter().skip_while(|c| **c == 0).next().unwrap();
    }
}

fn main() {
    println!("Part 1");
    assert_eq!(4, solve_case::<25, false>("day17.example"));
    assert_eq!(1304, solve_case::<150, false>("day17.input"));
    println!("Part 2");
    assert_eq!(3, solve_case::<25, true>("day17.example"));
    assert_eq!(18, solve_case::<150, true>("day17.input"));
}
