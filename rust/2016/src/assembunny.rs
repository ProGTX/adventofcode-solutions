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
    Increase(usize),
    Decrease(usize),
    JumpNotZero(Value, Value),
    // Days that don't toggle never read the target
    Toggle(#[allow(dead_code)] usize),
    // Transformed instructions
    #[allow(dead_code)]
    Zero(usize),
    #[allow(dead_code)]
    NoOp(usize), // toggled Zero
    #[allow(dead_code)]
    Add(usize, i32),
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
                "inc" => Op::Increase(parse_register(words[1])),
                "dec" => Op::Decrease(parse_register(words[1])),
                "jnz" => Op::JumpNotZero(parse_value(words[1]), parse_value(words[2])),
                "tgl" => Op::Toggle(parse_register(words[1])),
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
