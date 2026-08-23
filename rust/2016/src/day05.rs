use aoc::{md5::md5, string};
use std::thread;

type Input = String;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)[0].clone()
}

/// Num indices each thread scans before the results are merged back in order
const CHUNK: u32 = 1 << 16;

/// Scans `[base, base + CHUNK * num_threads)` in parallel,
/// collecting the 6th and 7th hash digits of every index
/// whose hash starts with five zeroes, in index order.
fn scan(door_id: &str, base: u32, num_threads: u32) -> Vec<(u8, u8)> {
    thread::scope(|s| {
        let handles: Vec<_> = (0..num_threads)
            .map(|t| {
                s.spawn(move || {
                    let door_id_size = door_id.len();
                    let mut buffer = [0; 16];
                    buffer[..door_id_size].copy_from_slice(door_id.as_bytes());
                    let start = base + t * CHUNK;
                    let mut found = Vec::new();
                    for index in start..start + CHUNK {
                        let size =
                            door_id_size + string::write_u32(&mut buffer[door_id_size..], index);
                        let hash = md5(&buffer[..size]);
                        if (hash[0] == 0) && (hash[1] == 0) && (hash[2] & 0xf0 == 0) {
                            found.push((hash[2] & 0x0f, hash[3] >> 4));
                        }
                    }
                    found
                })
            })
            .collect();
        handles
            .into_iter()
            .flat_map(|handle| handle.join().unwrap())
            .collect()
    })
}

fn solve_case<const ENHANCED: bool>(door_id: &Input) -> String {
    let num_threads = thread::available_parallelism().map_or(1, |n| n.get()) as u32;
    const PASSLEN: usize = 8;
    let mut password = [' '; PASSLEN];
    let mut filled = 0;
    let mut base = 0;
    while filled < PASSLEN {
        for (sixth, seventh) in scan(door_id, base, num_threads) {
            let sixth = sixth as usize;
            let (pid, nibble) = if !ENHANCED {
                (filled, sixth)
            } else {
                if (sixth >= PASSLEN) || (password[sixth] != ' ') {
                    continue;
                }
                (sixth, seventh as usize)
            };
            password[pid] = b"0123456789abcdef"[nibble] as char;
            filled += 1;
            if filled == PASSLEN {
                break;
            }
        }
        base += CHUNK * num_threads;
    }
    password.into_iter().collect()
}

fn main() {
    println!("Part 1");
    let example = parse("day05.example");
    aoc::expect_result!("18f47a30", solve_case::<false>(&example));
    let input = parse("day05.input");
    aoc::expect_result!("c6697b55", solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!("05ace8e3", solve_case::<true>(&example));
    aoc::expect_result!("8c35d1ab", solve_case::<true>(&input));
}
