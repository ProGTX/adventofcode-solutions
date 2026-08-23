use itertools::Itertools;

/// An instruction's argument: either a literal, or the id of the register holding it
#[derive(Clone, Copy)]
enum Value {
    Literal(i32),
    Register(usize),
}

#[derive(Clone, Copy)]
enum Op {
    Copy(Value, usize),
    Increase(usize),
    Decrease(usize),
    JumpNotZero(Value, Value),
}

type Input = Vec<Op>;

/// Registers a to d, as ids 0 to 3
fn parse_register(word: &str) -> usize {
    let name = word.parse::<char>().unwrap();
    return (name as u8 - b'a') as usize;
}

fn parse_value(word: &str) -> Value {
    return match word.parse::<i32>() {
        Ok(literal) => Value::Literal(literal),
        Err(_) => Value::Register(parse_register(word)),
    };
}

fn parse(filename: &str) -> Input {
    return aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let words = line.split(' ').collect_vec();
            match words[0] {
                "cpy" => Op::Copy(parse_value(words[1]), parse_register(words[2])),
                "inc" => Op::Increase(parse_register(words[1])),
                "dec" => Op::Decrease(parse_register(words[1])),
                "jnz" => Op::JumpNotZero(parse_value(words[1]), parse_value(words[2])),
                _ => panic!("Invalid instruction: {line}"),
            }
        })
        .collect();
}

fn read(registers: &[i64; 4], value: Value) -> i64 {
    return match value {
        Value::Literal(literal) => literal as i64,
        Value::Register(id) => registers[id],
    };
}

fn solve_case<const REG_C_INIT: i64>(ops: &Input) -> i64 {
    let mut registers = [0_i64, 0, REG_C_INIT, 0];
    let mut counter = 0_i64;
    while ((counter >= 0) && ((counter as usize) < ops.len())) {
        match ops[counter as usize] {
            Op::Copy(from, to) => registers[to] = read(&registers, from),
            Op::Increase(id) => registers[id] += 1,
            Op::Decrease(id) => registers[id] -= 1,
            Op::JumpNotZero(condition, offset) => {
                if (read(&registers, condition) != 0) {
                    // The jump is relative to this instruction
                    counter += read(&registers, offset) - 1;
                }
            }
        }
        counter += 1;
    }

    // Register a
    return registers[0];
}

fn main() {
    println!("Part 1");
    let example = parse("day12.example");
    aoc::expect_result!(42, solve_case::<0>(&example));
    let input = parse("day12.input");
    aoc::expect_result!(318083, solve_case::<0>(&input));

    println!("Part 2");
    aoc::expect_result!(42, solve_case::<1>(&example));
    aoc::expect_result!(9227737, solve_case::<1>(&input));
}
