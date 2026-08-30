mod assembunny;
use arrayvec::ArrayVec;
use assembunny::{Registers, XOp};

type Input = Vec<XOp>;

fn parse(filename: &str) -> Input {
    return assembunny::transform(&assembunny::parse(filename));
}

/// The lowest positive value for register a
/// that makes the program transmit a clock signal
fn solve_case1(ops: &Input) -> i64 {
    let mut out_values = ArrayVec::<i64, 8>::default();
    assert!(out_values.capacity() % 2 == 0);
    for reg_a_init in 1.. {
        out_values.clear();
        let mut registers: Registers = [reg_a_init, 0, 0, 0];
        let mut counter = 0_i64;
        while ((counter >= 0) && ((counter as usize) < ops.len())) {
            let op = ops[counter as usize];
            if let Some(out) = //
                assembunny::exec(&op, &mut registers, &mut counter)
            {
                out_values.push(out);
                if (out_values.len() == out_values.capacity()) {
                    if out_values
                        .chunks_exact(2)
                        .all(|chunk| (chunk[0] == 0) && (chunk[1] == 1))
                    {
                        return reg_a_init;
                    } else {
                        break;
                    }
                }
            }
        }
    }
    unreachable!("Every input has an answer");
}

fn main() {
    println!("Part 1");
    let input = parse("day25.input");
    aoc::expect_result!(196, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
