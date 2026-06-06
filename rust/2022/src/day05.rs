type Stacks = Vec<Vec<u8>>;

#[derive(Clone, Copy)]
struct Move {
    count: usize,
    from: usize, // 0-indexed
    to: usize,   // 0-indexed
}

type Input = (Stacks, Vec<Move>);

fn parse(filename: &str) -> Input {
    let content = aoc::file::read_string(filename);
    let (crate_section, move_section) = content.split_once("\n\n").unwrap();

    let crate_lines: Vec<&str> = crate_section.lines().collect();

    // Last line of the crate section is the stack number row "1  2  3 ..."
    let num_stacks = crate_lines
        .last()
        .unwrap()
        .split_whitespace()
        .last()
        .unwrap()
        .parse::<usize>()
        .unwrap();

    let mut stacks: Stacks = vec![Vec::new(); num_stacks];

    // Crate rows are everything above the stack number row
    // Crate letters sit at byte positions 1, 5, 9, ... (1 + 4*i)
    for line in &crate_lines[..crate_lines.len() - 1] {
        let bytes = line.as_bytes();
        for i in 0..num_stacks {
            let pos = 4 * i + 1;
            if pos < bytes.len() && bytes[pos] != b' ' {
                stacks[i].push(bytes[pos]);
            }
        }
    }

    // Crates were read top-to-bottom, reverse to get bottom-to-top
    for stack in &mut stacks {
        stack.reverse();
    }

    let moves = move_section
        .trim()
        .lines()
        .map(|line| {
            // "move N from A to B"
            let mut parts = line.split_ascii_whitespace();
            let count = parts.nth(1).unwrap().parse::<usize>().unwrap();
            let from = parts.nth(1).unwrap().parse::<usize>().unwrap() - 1;
            let to = parts.nth(1).unwrap().parse::<usize>().unwrap() - 1;
            Move { count, from, to }
        })
        .collect();

    (stacks, moves)
}

fn solve_case<const GRAB_MULTIPLE: bool>(input: &Input) -> String {
    let (stacks, moves) = input;
    let mut stacks = stacks.clone();

    for Move { count, from, to } in moves.iter().copied() {
        if !GRAB_MULTIPLE {
            // Part 1: move one at a time, reversing order
            for _ in 0..count {
                let c = stacks[from].pop().unwrap();
                stacks[to].push(c);
            }
        } else {
            // Part 2: move all at once, preserving order
            let new_len = stacks[from].len() - count;
            let bunch: Vec<u8> = stacks[from][new_len..].to_vec();
            stacks[from].truncate(new_len);
            stacks[to].extend_from_slice(&bunch);
        }
    }

    stacks
        .iter()
        .filter(|s| !s.is_empty())
        .map(|s| *s.last().unwrap() as char)
        .collect()
}

fn main() {
    println!("Part 1");
    let example = parse("day05.example");
    aoc::expect_result!("CMZ", solve_case::<false>(&example));
    let input = parse("day05.input");
    aoc::expect_result!("QGTHFZBHV", solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!("MCD", solve_case::<true>(&example));
    aoc::expect_result!("MGDMPSZTM", solve_case::<true>(&input));
}
