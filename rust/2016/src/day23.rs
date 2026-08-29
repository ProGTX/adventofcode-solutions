mod assembunny;
use assembunny::{Scalar, Value};
use itertools::Itertools;

/// Transformed instructions
#[derive(Clone, Copy, Debug)]
pub enum XOp {
    Zero(usize),
    ZeroToggled(usize),
    Add(usize, Scalar),
    Fma(usize, usize, Value), // out_id, in_id, input_val
    Copy(Value, Value),
    JumpNotZero(Value, Value),
    Toggle(usize),
}

type Input = Vec<XOp>;

/// Prints the original and the transformed instructions side by side
fn print(all_ops: &[Input; 2]) {
    const WIDTH: usize = 40;
    for (index, (original, transformed)) in all_ops[0].iter().zip(&all_ops[1]).enumerate() {
        let original = format!("{original:?}");
        let transformed = format!("{transformed:?}");
        // Mark the instructions that the loop transformation changed
        let marker = if (original == transformed) { ' ' } else { '*' };
        println!("{index:3}: {original:<WIDTH$} {marker} {transformed}");
    }
}

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
    let mut all_ops = [ops.clone(), Vec::with_capacity(ops.len())];

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
        // If we're decrementing register id within the last offset instructions,
        // we know this is a loop
        // NOTE: This assumes that the value of the register is positive
        let is_loop = (ops[start..index]
            .iter()
            .any(|&op| matches!(op, XOp::Add(id, -1) if (id == loop_reg))))
            && (
                // Only adds are supported in a loop
                ops[start..index]
                    .iter()
                    .all(|&op| matches!(op, XOp::Add(_, _)))
            );
        if !is_loop {
            continue;
        }

        // Modify instructions
        let mut new_ops = Vec::with_capacity(index + 1 - start);
        for op in &ops[start..index] {
            match *op {
                XOp::Add(id, -1) if (id == loop_reg) => {
                    // This is our loop counter, zero out at the end
                }
                XOp::Add(id, value) => {
                    new_ops.push(XOp::Fma(id, loop_reg, Value::Literal(value)));
                }
                _ => unreachable!(),
            }
        }
        new_ops.push(XOp::Zero(loop_reg));
        // Add a no-op to keep the number of instructions the same
        new_ops.push(XOp::Add(loop_reg, 0));

        ops[start..=index].copy_from_slice(&new_ops);
    }

    all_ops[1] = ops.clone();
    print(&all_ops);

    return ops;
}

fn toggle(op: XOp) -> XOp {
    match op {
        XOp::Zero(id) => XOp::ZeroToggled(id),
        XOp::ZeroToggled(id) => XOp::Zero(id),
        // Add replaces Increase and Decrease
        XOp::Add(id, value) => XOp::Add(id, -value),
        XOp::Fma(_, _, _) => {
            unimplemented!("Not sure how to toggle this op: {:?}", op)
        }
        // Regular instructions
        XOp::Toggle(x) => XOp::Add(x, 1),
        XOp::JumpNotZero(condition, offset) => XOp::Copy(condition, offset),
        XOp::Copy(from, to) => XOp::JumpNotZero(from, to),
        _ => unreachable!("Invalid op: {:?}", op),
    }
}

/// The value in register a once the program halts
fn solve_case<const NUM_EGGS: i64>(ops: &Input) -> i64 {
    let mut ops = ops.clone();
    let mut registers = [NUM_EGGS, 0, 0, 0];
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
            XOp::Fma(out_id, in_id, input_val) => {
                // Fused multiply-add
                registers[out_id] += registers[in_id] * assembunny::read(&registers, input_val);
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
