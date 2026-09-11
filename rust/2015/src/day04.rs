use aoc::md5::{self, Digest, md5_many};
use aoc::string;
use std::sync::atomic::{AtomicU32, Ordering};
use std::thread;

fn parse(filename: &str) -> String {
    aoc::file::read_string(filename).trim().to_string()
}

/// Whether the hash starts with the five or six zeroes the part asks for
fn is_answer<const PART2: bool>(hash: &Digest) -> bool {
    if (hash[0] != 0) || (hash[1] != 0) {
        return false;
    }
    return if PART2 {
        hash[2] == 0
    } else {
        (hash[2] & 0xF0) == 0
    };
}

fn solve_case<const PART2: bool>(key: &str) -> u32 {
    let num_threads = thread::available_parallelism().map_or(1, |n| n.get()) as u32;
    let result = AtomicU32::new(u32::MAX);

    // A thread takes a whole batch at a time,
    // so the numbers it hashes side by side are the ones next to each other
    let step = num_threads * md5::LANES as u32;
    thread::scope(|s| {
        for thread in 0..num_threads {
            let result = &result;
            s.spawn(move || {
                let key_bytes = key.as_bytes();
                let mut buffers = [[0u8; 64]; md5::LANES];
                let mut sizes = [0; md5::LANES];
                for buffer in &mut buffers {
                    buffer[..key_bytes.len()].copy_from_slice(key_bytes);
                }
                let mut hashes = [Digest::default(); md5::LANES];
                let mut n = thread * md5::LANES as u32;
                while n < result.load(Ordering::Relaxed) {
                    for (lane, buffer) in buffers.iter_mut().enumerate() {
                        sizes[lane] = key_bytes.len()
                            + string::write_u32(&mut buffer[key_bytes.len()..], n + lane as u32);
                    }
                    let inputs: [&[u8]; md5::LANES] =
                        std::array::from_fn(|lane| &buffers[lane][..sizes[lane]]);
                    md5_many(&inputs, &mut hashes);

                    // The batch runs upwards, so its first answer is its smallest
                    if let Some(lane) = hashes.iter().position(is_answer::<PART2>) {
                        let answer = n + lane as u32;
                        let mut current = result.load(Ordering::Relaxed);
                        while answer < current {
                            match result.compare_exchange_weak(
                                current,
                                answer,
                                Ordering::Relaxed,
                                Ordering::Relaxed,
                            ) {
                                Ok(_) => break,
                                Err(c) => current = c,
                            }
                        }
                        return;
                    }
                    n += step;
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
