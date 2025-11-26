type Direction = (i32, i32);

fn parse(filename: &str) -> Vec<Direction> {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let (command, value_str) = line.split_once(' ').unwrap();
            let value = value_str.parse::<i32>().unwrap();
            match command {
                "forward" => (value, 0),
                "down" => (0, value),
                "up" => (0, -value),
                _ => unreachable!("Invalid command"),
            }
        })
        .collect::<Vec<_>>()
}

fn solve_case1(directions: &[Direction]) -> u32 {
    let mut position = (0, 0);
    for direction in directions {
        position.0 += direction.0;
        position.1 += direction.1;
    }
    (position.0 as u32) * (position.1 as u32)
}

fn solve_case2(directions: &[Direction]) -> u32 {
    let mut position = (0, 0);
    let mut aim = 0;
    for (forward, aim_dir) in directions {
        if (*forward > 0) {
            position.0 += *forward;
            position.1 += aim * *forward;
        } else {
            aim += *aim_dir;
        }
    }
    (position.0 as u32) * (position.1 as u32)
}

fn main() {
    println!("Part 1");
    let example = parse("day02.example");
    assert_eq!(150, solve_case1(&example));
    let input = parse("day02.input");
    assert_eq!(2120749, solve_case1(&input));

    println!("Part 2");
    assert_eq!(900, solve_case2(&example));
    assert_eq!(2138382217, solve_case2(&input));
}
