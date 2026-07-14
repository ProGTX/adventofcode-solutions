use arrayvec::ArrayVec;
use std::collections::HashMap;

type BlinkState = (u64, u64); // (stone, blink_counter)

fn parse(filename: &str) -> Vec<u64> {
    aoc::file::read_lines(filename)
        .last()
        .unwrap()
        .split(' ')
        .map(|s| s.parse().unwrap())
        .collect()
}

fn num_digits(n: u64) -> u32 {
    if n >= 10 { num_digits(n / 10) + 1 } else { 1 }
}

fn pown(x: u64, p: u32) -> u64 {
    if p == 0 {
        return 1;
    }
    let mut x = x;
    let mut p = p;
    let mut result: u64 = 1;
    loop {
        if p & 0x1 == 1 {
            result *= x;
        }
        p >>= 1;
        if p == 0 {
            break;
        }
        x *= x;
    }
    result
}

fn blink_neighbors(state: &BlinkState) -> ArrayVec<BlinkState, 2> {
    let &(stone, blink_counter) = state;
    let mut neighbors = ArrayVec::new();
    if stone == 0 {
        neighbors.push((1, blink_counter - 1));
    } else {
        let digits = num_digits(stone);
        if digits % 2 == 0 {
            // The stone is replaced by two stones
            let divider = pown(10, digits / 2);
            // The left half of the digits are engraved on the new left stone
            // The right half of the digits are engraved on the new right stone
            neighbors.push((stone / divider, blink_counter - 1));
            neighbors.push((stone % divider, blink_counter - 1));
        } else {
            neighbors.push((stone * 2024, blink_counter - 1));
        }
    }
    neighbors
}

fn solve_case<const TIMES: u64>(stones: &[u64]) -> u64 {
    // We need to use dynamic programming to speed up the calculation
    stones
        .iter()
        .map(|&stone| {
            let start: BlinkState = (stone, TIMES);
            let num_stones: HashMap<_, u64, _> = aoc::algorithm::dfs_uniform(
                start,
                |&(_, blink_counter)| blink_counter == 0,
                blink_neighbors,
            );
            num_stones[&start]
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day11.example");
    aoc::expect_result!(55312, solve_case::<25>(&example));
    let input = parse("day11.input");
    aoc::expect_result!(191690, solve_case::<25>(&input));

    println!("Part 2");
    aoc::expect_result!(65601038650482, solve_case::<75>(&example));
    aoc::expect_result!(228651922369703, solve_case::<75>(&input));
}
