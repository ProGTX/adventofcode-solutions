use aoc::grid::Grid;

const fn turn_on_fn1(_: u32) -> u32 {
    1
}
const fn turn_off_fn1(_: u32) -> u32 {
    0
}
const fn toggle_fn1(c: u32) -> u32 {
    if c == 1 { 0 } else { 1 }
}
const fn turn_on_fn2(c: u32) -> u32 {
    c + 1
}
const fn turn_off_fn2(c: u32) -> u32 {
    if c == 0 { 0 } else { c - 1 }
}
const fn toggle_fn2(c: u32) -> u32 {
    c + 2
}

// Include spaces in strings so we don't have to trim later
const TURN_ON: &str = "turn on ";
const TURN_OFF: &str = "turn off ";
const TOGGLE: &str = "toggle ";
const THROUGH: &str = " through ";

type OpFnT = fn(u32) -> u32;

fn parse_point(s: &str, separator: &str) -> (usize, usize) {
    let (column_s, row_s) = s.split_once(separator).unwrap();
    (column_s.parse().unwrap(), row_s.parse().unwrap())
}

fn solve_case<const BRIGHTNESS: bool>(filename: &str) -> u32 {
    let mut lights = Grid::new(0, 1000, 1000);
    for line in std::fs::read_to_string(filename).unwrap().lines() {
        let (operation, rest) = if line.starts_with(TURN_ON) {
            (
                if !BRIGHTNESS {
                    turn_on_fn1 as OpFnT
                } else {
                    turn_on_fn2 as OpFnT
                },
                &line[TURN_ON.len()..],
            )
        } else if line.starts_with(TURN_OFF) {
            (
                if !BRIGHTNESS {
                    turn_off_fn1 as OpFnT
                } else {
                    turn_off_fn2 as OpFnT
                },
                &line[TURN_OFF.len()..],
            )
        } else if line.starts_with(TOGGLE) {
            (
                if !BRIGHTNESS {
                    toggle_fn1 as OpFnT
                } else {
                    toggle_fn2 as OpFnT
                },
                &line[TOGGLE.len()..],
            )
        } else {
            unreachable!("Unexpected line: {}", line)
        };
        let (begin, end) = rest.split_once(THROUGH).unwrap();
        let (beginx, beginy) = parse_point(begin, ",");
        let (endx, endy) = parse_point(end, ",");

        for row in beginy..(endy + 1) {
            for column in beginx..(endx + 1) {
                let c = lights.get_mut(row, column);
                *c = operation(*c);
            }
        }
    }

    if !BRIGHTNESS {
        lights.data.iter().filter(|&&l| l == 1).count() as u32
    } else {
        lights.data.iter().fold(0, |acc, x| acc + x)
    }
}

fn main() {
    println!("Part 1");
    assert_eq!(998996, solve_case::<false>("day06.example"));
    assert_eq!(569999, solve_case::<false>("day06.input"));
    println!("Part 2");
    assert_eq!(1001996, solve_case::<true>("day06.example"));
    assert_eq!(17836115, solve_case::<true>("day06.input"));
}
