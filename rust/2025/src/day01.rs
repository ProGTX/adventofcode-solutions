fn parse(filename: &str) -> Vec<i32> {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let mut distance = line[1..].parse::<i32>().unwrap();
            if (line.chars().next().unwrap() == 'L') {
                distance = -distance;
            }
            distance
        })
        .collect::<Vec<_>>()
}

fn solve_case1(sequence: &[i32]) -> i32 {
    let mut current = 50;
    sequence
        .iter()
        .map(|&rotation| {
            // Not fully correct calculation for the modulo,
            // but it identifies zeros correctly
            current = (current + rotation) % 100;
            (current == 0) as i32
        })
        .sum()
}

fn solve_case2(sequence: &[i32]) -> i32 {
    let mut current = 50;
    sequence
        .iter()
        .map(|&rotation| {
            let current_zero = (current == 0);
            current += rotation;
            let num_zeros = (current / 100).abs() + ((current <= 0) && !current_zero) as i32;
            // Here we have to use the full formula for the modulo
            current = current.rem_euclid(100);
            num_zeros
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day01.example");
    assert_eq!(3, solve_case1(&example));
    let input = parse("day01.input");
    assert_eq!(1048, solve_case1(&input));

    println!("Part 2");
    assert_eq!(6, solve_case2(&example));
    assert_eq!(6498, solve_case2(&input));
}
