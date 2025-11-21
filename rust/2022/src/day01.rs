fn parse(filename: &str) -> Vec<u32> {
    aoc::file::read_string(filename)
        .trim()
        .split("\n\n")
        .map(|food: &str| {
            food.split('\n')
                .map(|calories: &str| calories.parse::<u32>().unwrap())
                .sum()
        })
        .collect()
}

fn solve_case1(elves: &[u32]) -> u32 {
    *elves.iter().max().unwrap()
}

fn solve_case2(elves: &[u32]) -> u32 {
    let mut elves = elves.to_vec();
    elves.sort();
    elves.iter().rev().take(3).sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day01.example");
    assert_eq!(24000, solve_case1(&example));
    let input = parse("day01.input");
    assert_eq!(75622, solve_case1(&input));

    println!("Part 2");
    assert_eq!(45000, solve_case2(&example));
    assert_eq!(213159, solve_case2(&input));
}
