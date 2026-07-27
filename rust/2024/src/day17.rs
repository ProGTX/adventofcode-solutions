use aoc::iter::CollectArrayVec;
use arrayvec::ArrayVec;

const MAX_3BIT_MASK: i64 = (1 << 3) - 1;
const PROGRAM_PREFIX: &str = "Program: ";

const ADV: u8 = 0;
const BXL: u8 = 1;
const BST: u8 = 2;
const JNZ: u8 = 3;
const BXC: u8 = 4;
const OUT: u8 = 5;
const BDV: u8 = 6;
const CDV: u8 = 7;

type Registers = [i64; 3];
type Program = ArrayVec<u8, 16>;

struct Input {
    program: Program,
    registers: Registers,
}

fn parse(filename: &str) -> Input {
    let mut program = Program::new();
    let mut registers: Registers = [0; 3];

    for line in aoc::file::read_lines(filename) {
        if line.is_empty() {
            continue;
        }
        if line.contains(PROGRAM_PREFIX) {
            program = line[PROGRAM_PREFIX.len()..]
                .split(',')
                .map(|s| s.parse().unwrap())
                .collect();
        } else {
            let register_line = &line[PROGRAM_PREFIX.len()..];
            let index = (register_line.as_bytes()[0] - b'A') as usize;
            registers[index] = register_line[3..].parse().unwrap();
        }
    }

    Input { program, registers }
}

fn combo(value: i64, registers: &Registers) -> i64 {
    debug_assert!(value < 7, "Invalid combo operand");
    if value < 4 {
        value
    } else {
        registers[(value - 4) as usize]
    }
}

fn division(program: &[u8], instruction_ptr: usize, registers: &Registers) -> i64 {
    let numerator = registers[0];
    let denominator = 1i64 << combo(program[instruction_ptr + 1] as i64, registers);
    numerator / denominator
}

fn run_program(program: &[u8], mut registers: Registers) -> (Program, Registers) {
    let mut instruction_ptr: usize = 0;
    let mut results = Program::new();

    while instruction_ptr < program.len() {
        let op = program[instruction_ptr];
        match op {
            ADV => {
                registers[0] = division(program, instruction_ptr, &registers);
                instruction_ptr += 2;
            }
            BXL => {
                registers[1] ^= program[instruction_ptr + 1] as i64;
                instruction_ptr += 2;
            }
            BST => {
                registers[1] =
                    combo(program[instruction_ptr + 1] as i64, &registers) & MAX_3BIT_MASK;
                instruction_ptr += 2;
            }
            JNZ => {
                if registers[0] == 0 {
                    instruction_ptr += 2;
                } else {
                    instruction_ptr = program[instruction_ptr + 1] as usize;
                }
            }
            BXC => {
                registers[1] ^= registers[2];
                instruction_ptr += 2;
            }
            OUT => {
                let value = combo(program[instruction_ptr + 1] as i64, &registers) & MAX_3BIT_MASK;
                results.push(value as u8);
                instruction_ptr += 2;
            }
            BDV => {
                registers[1] = division(program, instruction_ptr, &registers);
                instruction_ptr += 2;
            }
            CDV => {
                registers[2] = division(program, instruction_ptr, &registers);
                instruction_ptr += 2;
            }
            _ => unreachable!("Invalid instruction"),
        }
    }
    (results, registers)
}

fn solve_case1(input: &Input) -> String {
    let (results, _new_registers) = run_program(&input.program, input.registers);
    results
        .iter()
        .map(|value| value.to_string())
        .collect_array_vec::<16>()
        .join(",")
}

fn solve_case2(input: &Input) -> i64 {
    for a in 0..i64::MAX {
        let mut registers = input.registers;
        registers[0] = a;
        let (result, _) = run_program(&input.program, registers);
        if result == input.program {
            return a;
        }
    }
    unreachable!("A not found");
}

fn main() {
    println!("Asserts");
    assert_eq!(1, run_program(&[2, 6], [0, 0, 9]).1[1]);
    assert_eq!(
        [0u8, 1, 2].as_slice(),
        run_program(&[5, 0, 5, 1, 5, 4], [10, 0, 0]).0.as_slice()
    );
    assert_eq!(
        [4u8, 2, 5, 6, 7, 7, 7, 7, 3, 1, 0].as_slice(),
        run_program(&[0, 1, 5, 4, 3, 0], [2024, 0, 0]).0.as_slice()
    );
    assert_eq!(0, run_program(&[0, 1, 5, 4, 3, 0], [2024, 0, 0]).1[0]);
    assert_eq!(26, run_program(&[1, 7], [0, 29, 0]).1[1]);
    assert_eq!(44354, run_program(&[4, 0], [0, 2024, 43690]).1[1]);

    println!("Part 1");
    let example = parse("day17.example");
    aoc::expect_result!("4,6,3,5,6,3,5,2,1,0", solve_case1(&example));
    let example2 = parse("day17.example2");
    aoc::expect_result!("5,7,3,0", solve_case1(&example2));
    let input = parse("day17.input");
    aoc::expect_result!("1,7,6,5,1,0,5,0,7", solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(117440, solve_case2(&example2));
    aoc::return_incomplete();
    // aoc::expect_result!(1337, solve_case2(&input));
    // aoc::expect_result!(1337, solve_case2(&example));
}
