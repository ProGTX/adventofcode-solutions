// The individual operands can be 32-bit ints
type Operands = arrayvec::ArrayVec<u32, 12>;

struct Equation {
    test_value: u64,
    operands: Operands,
}

fn parse(filename: &str) -> Vec<Equation> {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let (test_value_str, operands_str) = line.split_once(':').unwrap();
            let test_value: u64 = test_value_str.parse().unwrap();
            let operands: Operands = operands_str
                .trim()
                .split(' ')
                .map(|s| s.parse().unwrap())
                .collect();
            Equation {
                test_value,
                operands,
            }
        })
        .collect()
}

fn next_power_of_10(x: u64) -> u64 {
    let mut result = 1;
    while result < x {
        result *= 10;
    }
    result
}

fn concat_numbers(a: u64, b: u64) -> u64 {
    // The plus one is important
    a * next_power_of_10(b + 1) + b
}

// 2 possible operators (+, *), or 3 with concatenation (+, *, ||).
// Enumerate operator assignments directly in that base, rather than in
// binary/base-4 with a step to skip invalid combinations, so every id
// enumerated below is valid -- none are ever generated only to be discarded.
fn evaluate_equation<const CONCAT: bool>(test_value: u64, operands: &[u32]) -> u64 {
    let max_operators = (operands.len() - 1) as u32;
    let base: u32 = if CONCAT { 3 } else { 2 };
    let max_evaluations = base.pow(max_operators);
    for id in 0..max_evaluations {
        let mut sum: u64 = operands[0] as u64;
        let mut code = id;
        for &operand in &operands[1..] {
            let current_num = operand as u64;
            let operation_id = code % base;
            code /= base;
            if operation_id == 0 {
                sum += current_num;
            } else if operation_id == 1 {
                sum *= current_num;
            } else if CONCAT {
                sum = concat_numbers(sum, current_num);
            }
        }
        if sum == test_value {
            return sum;
        }
    }
    0
}

fn solve_case<const CONCAT: bool>(input: &[Equation]) -> u64 {
    input
        .iter()
        .map(|equation| evaluate_equation::<CONCAT>(equation.test_value, &equation.operands))
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day07.example");
    aoc::expect_result!(3749, solve_case::<false>(&example));
    let input = parse("day07.input");
    aoc::expect_result!(5702958180383, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(11387, solve_case::<true>(&example));
    aoc::expect_result!(92612386119138, solve_case::<true>(&input));
}
