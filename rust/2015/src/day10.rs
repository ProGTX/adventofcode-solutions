fn look_and_say(input: &Vec<u32>, output: &mut Vec<u32>) {
    output.clear();
    let mut index = 0;
    while index < input.len() {
        let current = input[index];
        let mut count = 0usize;
        input
            .iter()
            .skip(index)
            .take_while(|&&x| x == current)
            .for_each(|_| count += 1);
        output.push(count as u32);
        output.push(current);
        index += count;
    }
}

fn parse(filename: &str) -> Vec<u32> {
    std::fs::read_to_string(filename)
        .unwrap()
        .trim()
        .chars()
        .map(|c| c.to_digit(10).unwrap())
        .collect()
}

fn solve_case<const ITERATIONS: usize>(digits: &[u32]) -> usize {
    let mut input = digits.to_vec();
    let mut output = Vec::new();
    for _ in 0..ITERATIONS {
        look_and_say(&input, &mut output);
        std::mem::swap(&mut input, &mut output);
    }
    input.len()
}

fn main() {
    println!("Part 1");
    let example = parse("day10.example");
    aoc::expect_result!(237746, solve_case::<40>(&example));
    let input = parse("day10.input");
    aoc::expect_result!(360154, solve_case::<40>(&input));

    println!("Part 2");
    aoc::expect_result!(3369156, solve_case::<50>(&example));
    aoc::expect_result!(5103798, solve_case::<50>(&input));
}
