use aoc::{md5::md5, string};

type Input = String;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)[0].clone()
}

fn solve_case1(door_id: &Input) -> String {
    let door_id_size = door_id.len();
    let mut buffer = [0; 16];
    buffer[..door_id_size].copy_from_slice(door_id.as_bytes());
    let mut index = 0;
    (0..8)
        .map(|_| {
            loop {
                let size = door_id_size + string::write_u32(&mut buffer[door_id_size..], index);
                let hash = md5(&buffer[..size]);
                index += 1;
                if (hash[0] == 0) && (hash[1] == 0) && (hash[2] & 0xf0 == 0) {
                    return b"0123456789abcdef"[(hash[2] & 0x0f) as usize] as char;
                }
            }
        })
        .collect()
}

fn main() {
    println!("Part 1");
    let example = parse("day05.example");
    aoc::expect_result!("18f47a30", solve_case1(&example));
    let input = parse("day05.input");
    aoc::expect_result!("c6697b55", solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
    // aoc::expect_result!(1337, solve_case2(&example));
    // aoc::expect_result!(1337, solve_case2(&input));
}
