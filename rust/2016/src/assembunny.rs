//! The assembunny language, shared between days 12 and 23

/// An instruction's argument: either a literal, or the id of the register holding it
#[derive(Clone, Copy)]
pub enum Value {
    Literal(i32),
    Register(usize),
}

#[derive(Clone, Copy)]
pub enum Op {
    Copy(Value, Value),
    Increase(Value),
    Decrease(Value),
    JumpNotZero(Value, Value),
    // Days that don't toggle never read the target
    Toggle(#[allow(dead_code)] Value),
}

pub type Registers = [i64; 4];

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

pub fn parse(filename: &str) -> Vec<Op> {
    return aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let words = line.split(' ').collect::<Vec<_>>();
            match words[0] {
                "cpy" => Op::Copy(parse_value(words[1]), parse_value(words[2])),
                "inc" => Op::Increase(parse_value(words[1])),
                "dec" => Op::Decrease(parse_value(words[1])),
                "jnz" => Op::JumpNotZero(parse_value(words[1]), parse_value(words[2])),
                "tgl" => Op::Toggle(parse_value(words[1])),
                _ => panic!("Invalid instruction: {line}"),
            }
        })
        .collect();
}

pub fn read(registers: &Registers, value: Value) -> i64 {
    return match value {
        Value::Literal(literal) => literal as i64,
        Value::Register(id) => registers[id],
    };
}

/// Executes a single instruction and advances the instruction counter
///
/// Toggling has to be handled by the caller
pub fn execute(op: Op, registers: &mut Registers, counter: &mut i64) {
    match op {
        Op::Copy(from, Value::Register(to)) => registers[to] = read(registers, from),
        Op::Increase(Value::Register(id)) => registers[id] += 1,
        Op::Decrease(Value::Register(id)) => registers[id] -= 1,
        Op::JumpNotZero(condition, offset) => {
            if (read(registers, condition) != 0) {
                // The jump is relative to this instruction
                *counter += read(registers, offset) - 1;
            }
        }
        Op::Toggle(_) => unreachable!("Toggling has to be handled by the caller"),
        _ => {
            // Invalid instruction, do nothing
        }
    }
    *counter += 1;
}
