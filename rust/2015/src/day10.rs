fn look_and_say(input: &Vec<u32>, output: &mut Vec<u32>) {
    output.clear();
    let mut index = 0;
    while index < input.len() {
        let current = input[index];
        // Counted with a plain loop rather than skip + take_while:
        // every digit of the sequence passes through here,
        // and the iterator machinery costs far more than the comparison
        // in a Debug build
        let mut count = 0usize;
        while ((index + count) < input.len()) && (input[index + count] == current) {
            count += 1;
        }
        output.push(count as u32);
        output.push(current);
        index += count;
    }
}

fn solve_case<const ITERATIONS: usize>(filename: &str) -> usize {
    let mut input = std::fs::read_to_string(filename)
        .unwrap()
        .trim()
        .chars()
        .map(|c| c.to_digit(10).unwrap())
        .collect::<Vec<u32>>();
    let mut output = Vec::new();
    for _ in 0..ITERATIONS {
        look_and_say(&input, &mut output);
        std::mem::swap(&mut input, &mut output);
    }
    input.len()
}

fn main() {
    println!("Part 1");
    aoc::expect_result!(237746, solve_case::<40>("day10.example"));
    aoc::expect_result!(360154, solve_case::<40>("day10.input"));
    println!("Part 2");
    aoc::expect_result!(3369156, solve_case::<50>("day10.example"));
    aoc::expect_result!(5103798, solve_case::<50>("day10.input"));
}
