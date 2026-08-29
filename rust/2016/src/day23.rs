mod assembunny;
use assembunny::{Op, Registers};

type Input = Vec<Op>;

fn parse(filename: &str) -> Input {
    return assembunny::parse(filename);
}

/// The instruction an instruction turns into when toggled
fn toggle(op: Op) -> Op {
    match op {
        Op::Increase(id) => Op::Decrease(id),
        Op::Decrease(id) => Op::Increase(id),
        Op::Toggle(x) => Op::Increase(x),
        Op::JumpNotZero(condition, offset) => Op::Copy(condition, offset),
        Op::Copy(from, to) => Op::JumpNotZero(from, to),
    }
}

/// The value in register a once the program halts
fn solve_case<const NUM_EGGS: i64>(ops: &Input) -> i64 {
    let mut ops = ops.clone();
    let mut registers: Registers = [NUM_EGGS, 0, 0, 0];
    let mut counter = 0_i64;
    while ((counter >= 0) && ((counter as usize) < ops.len())) {
        let op = ops[counter as usize];
        if let Op::Toggle(x) = op {
            // The target is relative to this instruction, and may be outside the program
            if let Some(target) = //
                usize::try_from(counter + assembunny::read(&registers, x))
                    .ok()
                    .and_then(|index| ops.get_mut(index))
            {
                *target = toggle(*target);
            }
            counter += 1;
            continue;
        }
        assembunny::execute(op, &mut registers, &mut counter);
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
