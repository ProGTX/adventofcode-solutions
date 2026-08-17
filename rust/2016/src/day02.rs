use aoc::grid::{ConfigInput, Grid, Ipos};
use arrayvec::ArrayVec;

type Input = Vec<String>;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
}

fn solve_case<const COMPLICATED: bool>(input: &Input) -> String {
    let mut keypad_str = ArrayVec::<String, 5>::new();
    if (!COMPLICATED) {
        keypad_str.push("123".to_string());
        keypad_str.push("456".to_string());
        keypad_str.push("789".to_string());
    } else {
        keypad_str.push("  1  ".to_string());
        keypad_str.push(" 234 ".to_string());
        keypad_str.push("56789".to_string());
        keypad_str.push(" ABC ".to_string());
        keypad_str.push("  D  ".to_string());
    }
    let (keypad, config) = Grid::from_lines_config(
        &keypad_str,
        ConfigInput {
            start_char: Some('5'),
            ..Default::default()
        },
    );
    let pos = config.start_pos.unwrap();
    let mut pos = Ipos::new(pos.x as isize, pos.y as isize);
    let mut code = String::new();

    for line in input {
        for dir in line.chars() {
            let diff = match dir {
                'U' => Ipos::new(0, -1),
                'D' => Ipos::new(0, 1),
                'L' => Ipos::new(-1, 0),
                'R' => Ipos::new(1, 0),
                _ => unreachable!("Invalid direction"),
            };
            let new_pos = pos + diff;
            if keypad.in_bounds_signed(new_pos.y, new_pos.x)
                && (!COMPLICATED || (*keypad.get(new_pos.y as usize, new_pos.x as usize) != ' '))
            {
                pos = new_pos;
            }
        }
        code.push(*keypad.get(pos.y as usize, pos.x as usize))
    }

    code
}

fn main() {
    println!("Part 1");
    let example = parse("day02.example");
    aoc::expect_result!("1985", solve_case::<false>(&example));
    let input = parse("day02.input");
    aoc::expect_result!("73597", solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!("5DB3", solve_case::<true>(&example));
    aoc::expect_result!("A47DA", solve_case::<true>(&input));
}
