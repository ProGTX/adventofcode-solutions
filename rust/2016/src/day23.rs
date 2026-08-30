mod assembunny;
use assembunny::{Registers, XOp};

type Input = Vec<XOp>;

fn parse(filename: &str) -> Input {
    assembunny::transform(&assembunny::parse(filename))
}

fn toggle(op: XOp) -> XOp {
    match op {
        XOp::NoOp => XOp::NoOp,
        XOp::Zero(id) => XOp::ZeroToggled(id),
        XOp::ZeroToggled(id) => XOp::Zero(id),
        // Add replaces Increase and Decrease
        XOp::Add(id, value) => XOp::Add(id, -value),
        XOp::Toggle(x) => XOp::Add(x, 1),
        XOp::JumpNotZero(condition, offset) => XOp::Copy(condition, offset),
        XOp::Copy(from, to) => XOp::JumpNotZero(from, to),
        _ => unreachable!("Invalid op: {:?}", op),
    }
}

/// The value in register a once the program halts
fn solve_case<const NUM_EGGS: i64>(ops: &Input) -> i64 {
    let mut ops = ops.clone();
    let mut registers: Registers = [NUM_EGGS, 0, 0, 0];
    let mut counter = 0_i64;
    while ((counter >= 0) && ((counter as usize) < ops.len())) {
        let op = ops[counter as usize];
        if let XOp::Toggle(x) = op {
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
        assembunny::exec(&op, &mut registers, &mut counter);
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
