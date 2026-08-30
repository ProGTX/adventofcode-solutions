//! The assembunny language, shared between days 12, 23, and 25

use itertools::Itertools;

pub type Scalar = i32;
#[allow(dead_code)]
pub type Registers = [i64; 4];

/// An instruction's argument: either a literal, or the id of the register holding it
#[derive(Clone, Copy, Debug)]
pub enum Value {
    Literal(Scalar),
    Register(usize),
}

#[derive(Clone, Copy, Debug)]
pub enum Op {
    Copy(Value, usize),
    Increase(usize),
    Decrease(usize),
    JumpNotZero(Value, Value),
    // Days that don't toggle never read the target
    Toggle(#[allow(dead_code)] usize),
    Out(#[allow(dead_code)] Value),
}
/// Transformed instructions
#[derive(Clone, Copy, Debug)]
pub enum XOp {
    NoOp,
    Zero(usize),
    #[allow(dead_code)]
    ZeroToggled(usize),
    Add(usize, Scalar),
    LoopAdd(usize, usize, Value), // out_id, loop_reg, steps
    Copy(Value, Value),
    JumpNotZero(Value, Value),
    Toggle(#[allow(dead_code)] usize),
    Out(#[allow(dead_code)] Value),
}

/// Registers a to d, as ids 0 to 3
fn parse_register(word: &str) -> usize {
    let name = word.parse::<char>().unwrap();
    return (name as u8 - b'a') as usize;
}

fn parse_value(word: &str) -> Value {
    return match word.parse::<Scalar>() {
        Ok(literal) => Value::Literal(literal),
        Err(_) => Value::Register(parse_register(word)),
    };
}

pub fn parse(filename: &str) -> Vec<Op> {
    return aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let words = line.split(' ').collect::<Vec<_>>();
            match words[0] {
                "cpy" => Op::Copy(parse_value(words[1]), parse_register(words[2])),
                "inc" => Op::Increase(parse_register(words[1])),
                "dec" => Op::Decrease(parse_register(words[1])),
                "jnz" => Op::JumpNotZero(parse_value(words[1]), parse_value(words[2])),
                "tgl" => Op::Toggle(parse_register(words[1])),
                "out" => Op::Out(parse_value(words[1])),
                _ => panic!("Invalid instruction: {line}"),
            }
        })
        .collect();
}

pub fn read(registers: &[i64], value: Value) -> i64 {
    return match value {
        Value::Literal(literal) => literal as i64,
        Value::Register(id) => registers[id],
    };
}

pub fn transform(ops: &Vec<Op>) -> Vec<XOp> {
    // Transform individual instructions
    let mut ops = ops
        .iter()
        .map(|&op| match op {
            Op::Copy(Value::Literal(0), id) => XOp::Zero(id),
            Op::Copy(from, to) => XOp::Copy(from, Value::Register(to)),
            Op::Increase(id) => XOp::Add(id, 1),
            Op::Decrease(id) => XOp::Add(id, -1),
            Op::JumpNotZero(condition, offset) => XOp::JumpNotZero(condition, offset),
            Op::Toggle(x) => XOp::Toggle(x),
            Op::Out(x) => XOp::Out(x),
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
        // The source may be a literal or a register
        let copy_to = match ops[start] {
            XOp::Copy(_, Value::Register(id)) => id,
            _ => unreachable!("Unexpected input: {:?}", ops[start]),
        };
        let (out_id, inner_reg) = match ops[start + 1] {
            XOp::LoopAdd(out_id, loop_reg, Value::Literal(1)) => (out_id, loop_reg),
            _ => unreachable!("Unexpected input: {:?}", ops[start + 1]),
        };
        assert_eq!(copy_to, inner_reg, "The copy doesn't set the loop register");
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

pub fn exec(op: &XOp, registers: &mut [i64], counter: &mut i64) -> Option<i64> {
    match *op {
        XOp::Zero(id) => registers[id] = 0,
        XOp::Add(id, value) => registers[id] += value as i64,
        XOp::LoopAdd(out_id, loop_reg, steps) => {
            // Fused multiply-add
            registers[out_id] += registers[loop_reg] * read(&registers, steps);
            // Loop counter is zero at the end
            registers[loop_reg] = 0;
        }
        XOp::Copy(from, Value::Register(to)) => registers[to] = read(&registers, from),
        XOp::JumpNotZero(condition, offset) => {
            if (read(&registers, condition) != 0) {
                // The jump is relative to this instruction
                *counter += read(&registers, offset) - 1;
            }
        }
        XOp::Out(x) => {
            *counter += 1;
            return Some(read(&registers, x));
        }
        _ => {
            // Invalid instruction, do nothing
        }
    }
    *counter += 1;
    return None;
}
