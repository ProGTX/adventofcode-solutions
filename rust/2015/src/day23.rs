type Instructions = Vec<(String, usize, i8)>;

fn parse(filename: &str) -> Instructions {
    aoc::file::read_lines(filename)
        .map(|line| {
            let line = line.unwrap();
            let (instruction, args) = line.split_once(' ').unwrap();
            let (first, second) = match (args.split_once(", ")) {
                Some((first, second)) => (
                    match first {
                        "a" => 0,
                        "b" => 1,
                        _ => unreachable!("Invalid first argument"),
                    },
                    second.parse::<i8>().unwrap(),
                ),
                None => match args {
                    "a" => (0, 0),
                    "b" => (1, 0),
                    _ => (0, args.parse::<i8>().unwrap()),
                },
            };
            (instruction.to_string(), first as usize, second)
        })
        .collect::<Instructions>()
}

fn solve_case<const START_A: u32>(instructions: &Instructions) -> u32 {
    let mut registers: [u32; 2] = [START_A, 0];
    let mut pc = 0;
    while (pc < instructions.len()) {
        let (inst, reg_index, operand) = &instructions[pc];
        let r = &mut registers[*reg_index];
        match inst.as_str() {
            "hlf" => {
                *r /= 2;
                pc += 1;
            }
            "tpl" => {
                *r *= 3;
                pc += 1;
            }
            "inc" => {
                *r += 1;
                pc += 1;
            }
            "jmp" => {
                pc = (pc as i8 + operand) as usize;
            }
            "jie" => {
                if ((*r % 2) == 0) {
                    pc = (pc as i8 + operand) as usize;
                } else {
                    pc += 1;
                }
            }
            "jio" => {
                if (*r == 1) {
                    pc = (pc as i8 + operand) as usize;
                } else {
                    pc += 1;
                }
            }
            _ => unreachable!("Invalid operation"),
        }
    }
    return registers[1];
}

fn main() {
    println!("Part 1");
    let example = parse("day23.example");
    assert_eq!(2, solve_case::<0>(&example));
    let input = parse("day23.input");
    assert_eq!(255, solve_case::<0>(&input));

    println!("Part 2");
    assert_eq!(2, solve_case::<1>(&example));
    assert_eq!(334, solve_case::<1>(&input));
}
