mod assembunny;
use assembunny::{Op, Registers, Value};
use itertools::Itertools;

type Input = Vec<Op>;

fn parse(filename: &str) -> Input {
    return assembunny::parse(filename);
}

fn transform(ops: &Input) -> Input {
    // Transform individual instructions
    let ops = ops
        .iter()
        .map(|&op| match op {
            Op::Copy(Value::Literal(0), Value::Register(id)) => Op::Zero(id),
            Op::Increase(id) => Op::Add(id, 1),
            Op::Decrease(id) => Op::Add(id, -1),
            _ => op,
        })
        .collect_vec();
    return ops;
}

/// The instruction an instruction turns into when toggled
fn toggle(op: Op) -> Op {
    match op {
        Op::Zero(id) => Op::NoOp(id),
        Op::NoOp(id) => Op::Zero(id),
        // Add replaces Increase and Decrease
        Op::Add(id, value) => Op::Add(id, -value),
        // Regular instructions
        Op::Toggle(x) => Op::Add(x, 1),
        Op::JumpNotZero(condition, offset) => Op::Copy(condition, offset),
        Op::Copy(from, to) => Op::JumpNotZero(from, to),
        _ => unreachable!("Invalid instruction"),
    }
}

/// The value in register a once the program halts
fn solve_case<const NUM_EGGS: i64>(ops: &Input) -> i64 {
    let mut ops = transform(ops);
    let mut registers: Registers = [NUM_EGGS, 0, 0, 0];
    let mut counter = 0_i64;
    while ((counter >= 0) && ((counter as usize) < ops.len())) {
        let op = ops[counter as usize];
        if let Op::Toggle(x) = op {
            // The target is relative to this instruction, and may be outside the program
            if let Some(target) = //
                usize::try_from(counter + registers[x])
                    .ok()
                    .and_then(|index| ops.get_mut(index))
            {
                *target = toggle(*target);
            }
            counter += 1;
            continue;
        }
        match op {
            Op::Zero(id) => registers[id] = 0,
            Op::Add(id, value) => registers[id] += value as i64,
            Op::Copy(from, Value::Register(to)) => {
                registers[to] = assembunny::read(&registers, from)
            }
            Op::JumpNotZero(condition, offset) => {
                if (assembunny::read(&registers, condition) != 0) {
                    // The jump is relative to this instruction
                    counter += assembunny::read(&registers, offset) - 1;
                }
            }
            _ => {
                // Invalid instruction, do nothing
            }
        }
        counter += 1;
    }
    // Register a
    return registers[0];
}

fn main() {
    println!("Part 1");
    let example = parse("day23.example");
    aoc::expect_result!(3, solve_case::<7>(&example));
    let input = parse("day23.input");
    aoc::expect_result!(11739, solve_case::<7>(&input));

    println!("Part 2");
    aoc::expect_result!(3, solve_case::<12>(&example));
    aoc::expect_result!(479008299, solve_case::<12>(&input));
}
