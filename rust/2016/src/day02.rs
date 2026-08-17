use aoc::grid::{ConfigInput, Grid, Ipos};

type Input = Vec<String>;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
}

fn solve_case1(input: &Input) -> u32 {
    let keypad = Grid::from_lines_config(
        //
        &["123".to_string(), "456".to_string(), "789".to_string()],
        ConfigInput::default(),
    )
    .0;
    let mut pos = Ipos::new(1, 1);
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
            if keypad.in_bounds_signed(new_pos.y, new_pos.x) {
                pos = new_pos;
            }
        }
        code.push(*keypad.get(pos.y as usize, pos.x as usize))
    }

    code.parse().unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day02.example");
    aoc::expect_result!(1985, solve_case1(&example));
    let input = parse("day02.input");
    aoc::expect_result!(73597, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
    // aoc::expect_result!(1337, solve_case2(&example));
    // aoc::expect_result!(1337, solve_case2(&input));
}
