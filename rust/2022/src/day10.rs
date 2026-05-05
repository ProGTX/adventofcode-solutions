const CRT_WIDTH: usize = 40;
const CRT_HEIGHT: usize = 6;

type Monitor = aoc::grid::Grid<char>;
type Instructions = Vec<Instruction>;

#[derive(Clone, Copy, PartialEq)]
enum Operation {
    Noop = 0,
    Addx = 1,
}

struct Instruction {
    operation: Operation,
    value: i32,
}

fn parse_instruction(line: &str) -> Instruction {
    if line == "noop" {
        return Instruction {
            operation: Operation::Noop,
            value: 0,
        };
    }

    let (operation, value_str) = line.split_once(' ').unwrap();
    if operation == "addx" {
        return Instruction {
            operation: Operation::Addx,
            value: value_str.parse::<i32>().unwrap(),
        };
    }

    panic!("Invalid instruction {operation}");
}

fn parse(filename: &str) -> Instructions {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| parse_instruction(line))
        .collect()
}

fn draw(monitor: &mut Monitor, cycle: i32, register_x: i32) {
    let column = (cycle - 1) % CRT_WIDTH as i32;
    let row = ((cycle - 1) / CRT_WIDTH as i32) % CRT_HEIGHT as i32;
    let pixel = if (column - register_x).abs() <= 1 {
        '#'
    } else {
        '.'
    };
    monitor.modify(pixel, row as usize, column as usize);
}

fn signal_strength(cycle: i32, register_x: i32) -> i32 {
    if (cycle == 20) || (((cycle - 20) % 40) == 0) {
        return cycle * register_x;
    }
    0
}

fn duration(operation: Operation) -> i32 {
    operation as i32 + 1
}

fn solve_case(instructions: &Instructions) -> (i32, String) {
    let mut signal = 0;
    let mut monitor = Monitor::new('.', CRT_HEIGHT, CRT_WIDTH);
    let mut cycle = 1;
    let mut register_x = 1;

    for instruction in instructions {
        for _ in 0..duration(instruction.operation) {
            signal += signal_strength(cycle, register_x);
            draw(&mut monitor, cycle, register_x);
            cycle += 1;
        }

        if instruction.operation == Operation::Addx {
            register_x += instruction.value;
        }
    }

    (signal, monitor.data.into_iter().collect())
}

fn main() {
    println!("Part 1");
    let example = parse("day10.example");
    let (example_signal, example_monitor) = solve_case(&example);
    assert_eq!(13140, example_signal);
    let input = parse("day10.input");
    let (input_signal, input_monitor) = solve_case(&input);
    assert_eq!(11820, input_signal);

    println!("Part 2");
    let expected_example_out = "\
##..##..##..##..##..##..##..##..##..##..\
###...###...###...###...###...###...###.\
####....####....####....####....####....\
#####.....#####.....#####.....#####.....\
######......######......######......####\
#######.......#######.......#######.....";
    assert_eq!(expected_example_out, example_monitor);
    let expected_input_out = "\
####.###....##.###..###..#..#..##..#..#.\
#....#..#....#.#..#.#..#.#.#..#..#.#..#.\
###..#..#....#.###..#..#.##...#..#.####.\
#....###.....#.#..#.###..#.#..####.#..#.\
#....#....#..#.#..#.#.#..#.#..#..#.#..#.\
####.#.....##..###..#..#.#..#.#..#.#..#.";
    assert_eq!(expected_input_out, input_monitor);
}
