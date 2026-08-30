mod assembunny;
use assembunny::{Registers, XOp};

type Input = Vec<XOp>;

fn parse(filename: &str) -> Input {
    return assembunny::transform(&assembunny::parse(filename));
}

fn solve_case<const REG_C_INIT: i64>(ops: &Input) -> i64 {
    let mut registers: Registers = [0, 0, REG_C_INIT, 0];
    let mut counter = 0_i64;
    while ((counter >= 0) && ((counter as usize) < ops.len())) {
        assembunny::exec(&ops[counter as usize], &mut registers, &mut counter);
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
