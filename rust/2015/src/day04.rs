use aoc::md5::md5;
use std::sync::atomic::{AtomicU32, Ordering};
use std::thread;

fn parse(filename: &str) -> String {
    aoc::file::read_string(filename).trim().to_string()
}

fn solve_case<const PART2: bool>(key: &str) -> u32 {
    let num_threads = thread::available_parallelism().map_or(1, |n| n.get()) as u32;
    let result = AtomicU32::new(u32::MAX);

    thread::scope(|s| {
        for start in 0..num_threads {
            let result = &result;
            s.spawn(move || {
                let mut n = start;
                while n < result.load(Ordering::Relaxed) {
                    let full_key = format!("{key}{n}");
                    let hash = md5(full_key.as_bytes());
                    if hash[0] == 0 && hash[1] == 0 && hash[2] & 0xF0 == 0 {
                        if PART2 && (hash[2] & 0x0F > 0) {
                            n += num_threads;
                            continue;
                        }
                        let mut current = result.load(Ordering::Relaxed);
                        while n < current {
                            match result.compare_exchange_weak(
                                current,
                                n,
                                Ordering::Relaxed,
                                Ordering::Relaxed,
                            ) {
                                Ok(_) => break,
                                Err(c) => current = c,
                            }
                        }
                        return;
                    }
                    n += num_threads;
                }
            });
        }
    });

    result.load(Ordering::Relaxed)
}

fn main() {
    println!("Part 1");
    let example = parse("day04.example");
    aoc::expect_result!(609043, solve_case::<false>(&example));
    aoc::expect_result!(1048970, solve_case::<false>("pqrstuv"));
    let input = parse("day04.input");
    aoc::expect_result!(254575, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(6742839, solve_case::<true>(&example));
    aoc::expect_result!(5714438, solve_case::<true>("pqrstuv"));
    aoc::expect_result!(1038736, solve_case::<true>(&input));
}
