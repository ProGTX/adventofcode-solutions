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

fn get_operation_id<const CONCAT: bool>(evaluation_id: u32, bit_pos: u32) -> u32 {
    (evaluation_id >> ((if CONCAT { 2 } else { 1 }) * bit_pos)) & (if CONCAT { 3 } else { 1 })
}

fn bit_pos_view(max_operators: u32) -> impl Iterator<Item = u32> + Clone {
    (0..max_operators).rev()
}

fn evaluate_equation<const CONCAT: bool>(test_value: u64, operands: &[u32]) -> u64 {
    let max_operators = (operands.len() - 1) as u32;
    let max_evaluations: u32 = 1 << ((if CONCAT { 2 } else { 1 }) * max_operators);
    for id in 0..max_evaluations {
        if CONCAT
            && bit_pos_view(max_operators)
                .any(|bit_pos| get_operation_id::<CONCAT>(id, bit_pos) == 3)
        {
            // We only have 3 possible operations, but we use 2 bits to represent
            // them, so we need to skip every time we get the 4th operation
            continue;
        }
        let mut sum: u64 = operands[0] as u64;
        for bit_pos in bit_pos_view(max_operators) {
            let current_num = operands[(max_operators - bit_pos) as usize] as u64;
            let operation_id = get_operation_id::<CONCAT>(id, bit_pos);
            if operation_id == 0 {
                sum += current_num;
            } else if operation_id == 1 {
                sum *= current_num;
            } else if CONCAT && operation_id == 2 {
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
