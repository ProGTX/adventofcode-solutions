type Node = (usize, usize);

#[derive(Clone, Copy)]
enum Turn {
    Left,
    Right,
}

impl Turn {
    fn apply(self, node: Node) -> usize {
        match self {
            Turn::Left => node.0,
            Turn::Right => node.1,
        }
    }
}

fn get_direction(c: char) -> Turn {
    match c {
        'L' => Turn::Left,
        _ => Turn::Right,
    }
}

struct Input {
    instructions: Vec<Turn>,
    directions: Vec<Node>,
    starts: Vec<usize>,
    ends: Vec<usize>,
    ghost_starts: Vec<usize>,
    ghost_ends: Vec<usize>,
}

fn parse(filename: &str) -> Input {
    let lines = aoc::file::read_lines(filename);
    let instructions: Vec<Turn> = lines[0].chars().map(get_direction).collect();

    let mut name_to_id = aoc::string::NameToId::new();
    let mut directions: Vec<Node> = Vec::new();
    let mut starts = Vec::new();
    let mut ends = Vec::new();
    let mut ghost_starts = Vec::new();
    let mut ghost_ends = Vec::new();

    for line in lines.iter().skip(2).filter(|l| !l.is_empty()) {
        let (node_name, lr_raw) = line.split_once('=').unwrap();
        let node_name = node_name.trim();
        let lr = lr_raw.trim().trim_matches(['(', ')']);
        let (left_name, right_name) = lr.split_once(',').unwrap();
        let left_name = left_name.trim();
        let right_name = right_name.trim();

        let node_id = name_to_id.intern(node_name);
        let left_id = name_to_id.intern(left_name);
        let right_id = name_to_id.intern(right_name);
        directions.resize(name_to_id.new_len(directions.len()), (0, 0));

        let bytes = node_name.as_bytes();
        if node_name == "AAA" {
            starts.push(node_id);
        }
        if node_name == "ZZZ" {
            ends.push(node_id);
        }
        if bytes.len() == 3 && bytes[2] == b'A' {
            ghost_starts.push(node_id);
        }
        if bytes.len() == 3 && bytes[2] == b'Z' {
            ghost_ends.push(node_id);
        }

        directions[node_id] = (left_id, right_id);
    }

    Input {
        instructions,
        directions,
        starts,
        ends,
        ghost_starts,
        ghost_ends,
    }
}

fn num_steps(directions: &[Node], instructions: &[Turn], start: usize, ends: &[usize]) -> i64 {
    let mut next_index = start;
    let mut steps = 0;
    let mut inst_index = 0;
    while !ends.contains(&next_index) {
        next_index = instructions[inst_index].apply(directions[next_index]);
        inst_index = (inst_index + 1) % instructions.len();
        steps += 1;
    }
    steps
}

fn num_steps_total(
    directions: &[Node],
    instructions: &[Turn],
    starts: &[usize],
    ends: &[usize],
) -> i64 {
    starts
        .iter()
        .map(|&start| num_steps(directions, instructions, start, ends))
        .fold(1, |acc, n| aoc::math::lcm(acc, n))
}

fn solve_case<const ALL_PATHS: bool>(input: &Input) -> i64 {
    let (starts, ends) = if ALL_PATHS {
        (&input.ghost_starts, &input.ghost_ends)
    } else {
        (&input.starts, &input.ends)
    };
    num_steps_total(&input.directions, &input.instructions, starts, ends)
}

fn main() {
    println!("Asserts");
    aoc::expect_result!(
        6,
        num_steps(
            &[(1, 1), (0, 2), (2, 2)],
            &[get_direction('L'), get_direction('L'), get_direction('R')],
            0,
            &[2],
        )
    );

    println!("Part 1");
    let example = parse("day08.example");
    aoc::expect_result!(2, solve_case::<false>(&example));
    let example2 = parse("day08.example2");
    aoc::expect_result!(6, solve_case::<false>(&example2));
    let input = parse("day08.input");
    aoc::expect_result!(12737, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(2, solve_case::<true>(&example));
    aoc::expect_result!(6, solve_case::<true>(&example2));
    let example3 = parse("day08.example3");
    aoc::expect_result!(6, solve_case::<true>(&example3));
    aoc::expect_result!(9064949303801, solve_case::<true>(&input));
}
