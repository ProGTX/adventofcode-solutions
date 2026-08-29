mod assembunny;
use assembunny::{Registers, Scalar, Value};
use itertools::Itertools;

/// Transformed instructions
#[derive(Clone, Copy, Debug)]
pub enum XOp {
    NoOp,
    Zero(usize),
    ZeroToggled(usize),
    Add(usize, Scalar),
    LoopAdd(usize, usize, Value), // out_id, loop_reg, steps
    Copy(Value, Value),
    JumpNotZero(Value, Value),
    Toggle(usize),
}

type Input = Vec<XOp>;

fn parse(filename: &str) -> Input {
    type Op = assembunny::Op;

    // Transform individual instructions
    let mut ops = assembunny::parse(filename)
        .into_iter()
        .map(|op| match op {
            Op::Copy(Value::Literal(0), id) => XOp::Zero(id),
            Op::Copy(from, to) => XOp::Copy(from, Value::Register(to)),
            Op::Increase(id) => XOp::Add(id, 1),
            Op::Decrease(id) => XOp::Add(id, -1),
            Op::JumpNotZero(condition, offset) => XOp::JumpNotZero(condition, offset),
            Op::Toggle(x) => XOp::Toggle(x),
        })
        .collect_vec();

    // Transform loops
    let mut search_from = 0;
    while let Some((index, loop_reg, loop_offset)) = //
        ops[search_from..]
            .iter()
            .enumerate()
            .find_map(|(index, op)| match *op {
                XOp::JumpNotZero(Value::Register(id), Value::Literal(value)) if (value < 0) => {
                    Some((index + search_from, id, value))
                }
                _ => None,
            })
    {
        search_from = index + 1;

        // offset is negative here
        let start = index - loop_offset.unsigned_abs() as usize;

        if !ops[start..index]
            .iter()
            .all(|&op| matches!(op, XOp::Add(_, _)))
        {
            // Only adds are supported in a loop
            continue;
        }

        // If we're incrementing or decrementing register id by 1
        // within the last offset instructions, we know this is a loop
        // NOTE: This assumes that the value of the register
        // is opposite of the sign of the increment/decrement
        let is_loop_counter = |id, value: Scalar| {
            return (id == loop_reg) && (value.abs() == 1);
        };
        let is_loop = ops[start..index]
            .iter()
            .any(|&op| matches!(op, XOp::Add(id, value) if is_loop_counter(id, value) ));
        if !is_loop {
            continue;
        }

        // Modify instructions
        let mut new_ops = Vec::with_capacity(index + 1 - start);
        for op in &ops[start..index] {
            match *op {
                XOp::Add(id, value) if is_loop_counter(id, value) => {
                    // Zero out loop counter as part of LoopAdd
                }
                XOp::Add(id, value) => {
                    new_ops.push(XOp::LoopAdd(id, loop_reg, Value::Literal(value)));
                }
                _ => unreachable!(),
            }
        }
        // Keep the number of instructions the same,
        // the offsets held in registers depend on it
        new_ops.push(XOp::NoOp); // Replaces loop counter
        new_ops.push(XOp::NoOp); // Replaces jump op

        ops[start..=index].copy_from_slice(&new_ops);
    }

    // Transform a very specific loop
    const SPECIFIC_LOOP_OFFSET: i32 = -5;
    if let Some((index, outer_reg)) = //
        ops.iter().enumerate().find_map(|(index, op)| match *op {
            XOp::JumpNotZero(Value::Register(id), Value::Literal(value))
                if (value == SPECIFIC_LOOP_OFFSET) =>
            {
                Some((index, id))
            }
            _ => None,
        })
    {
        let start = index - SPECIFIC_LOOP_OFFSET.unsigned_abs() as usize;
        assert!(
            matches!(
                ops[start],
                XOp::Copy(Value::Register(_), Value::Register(_))
            ),
            "Unexpected input: {:?}",
            ops[start]
        );
        let (out_id, inner_reg) = match ops[start + 1] {
            XOp::LoopAdd(out_id, loop_reg, Value::Literal(1)) => (out_id, loop_reg),
            _ => unreachable!("Unexpected input: {:?}", ops[start + 1]),
        };
        assert!(
            matches!(ops[start + 2], XOp::NoOp),
            "Expected NoOp, got {:?}",
            ops[start + 2]
        );
        assert!(
            matches!(ops[start + 3], XOp::NoOp),
            "Expected NoOp, got {:?}",
            ops[start + 3]
        );
        match ops[start + 4] {
            XOp::Add(loop_reg, value) => {
                assert_eq!(outer_reg, loop_reg, "Loop register doesn't match");
                assert_eq!(1, value.abs(), "Loop increment not 1");
            }
            _ => unreachable!("Unexpected input: {:?}", ops[start + 4]),
        }

        // ops[start]: Keep the copy as-is
        ops[start + 1] = XOp::LoopAdd(out_id, inner_reg, Value::Register(outer_reg));
        // Keep the number of instructions the same,
        // the offsets held in registers depend on it
        ops[start + 2] = XOp::NoOp;
        ops[start + 3] = XOp::NoOp;
        ops[start + 4] = XOp::NoOp;
        ops[index] = XOp::NoOp;
    }

    return ops;
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
        match op {
            XOp::Zero(id) => registers[id] = 0,
            XOp::Add(id, value) => registers[id] += value as i64,
            XOp::LoopAdd(out_id, loop_reg, steps) => {
                // Fused multiply-add
                registers[out_id] += registers[loop_reg] * assembunny::read(&registers, steps);
                // Loop counter is zero at the end
                registers[loop_reg] = 0;
            }
            XOp::Copy(from, Value::Register(to)) => {
                registers[to] = assembunny::read(&registers, from)
            }
            XOp::JumpNotZero(condition, offset) => {
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
