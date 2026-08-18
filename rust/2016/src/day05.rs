use aoc::{md5::md5, string};

type Input = String;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)[0].clone()
}

fn solve_case<const ENHANCED: bool>(door_id: &Input) -> String {
    let door_id_size = door_id.len();
    let mut buffer = [0; 16];
    buffer[..door_id_size].copy_from_slice(door_id.as_bytes());
    let mut index = 0;
    const PASSLEN: usize = 8;
    let mut password = [' '; PASSLEN];
    for pid in 0..PASSLEN {
        loop {
            let size = door_id_size + string::write_u32(&mut buffer[door_id_size..], index);
            let hash = md5(&buffer[..size]);
            index += 1;
            if (hash[0] == 0) && (hash[1] == 0) && (hash[2] & 0xf0 == 0) {
                let sixth = (hash[2] & 0x0f) as usize;
                let (pid, nibble) = if !ENHANCED {
                    (pid, sixth)
                } else {
                    if (sixth >= PASSLEN) || (password[sixth] != ' ') {
                        continue;
                    }
                    (sixth, (hash[3] >> 4) as usize)
                };
                password[pid] = b"0123456789abcdef"[nibble] as char;
                break;
            }
        }
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
