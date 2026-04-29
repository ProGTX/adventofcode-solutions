fn parse(filename: &str) -> Vec<String> {
    aoc::file::read_lines(filename)
}

fn solve_case1(data: &[String]) -> u64 {
    // TODO: Implement Part 1
    0
}

fn solve_case2(data: &[String]) -> u64 {
    // TODO: Implement Part 2
    0
}

fn main() {
    println!("Part 1");
    let example = parse("day24.example");
    assert_eq!(0, solve_case1(&example));

    let input = parse("day24.input");
    // assert_eq!(XXX, solve_case1(&input));

    println!("Part 2");
    // assert_eq!(XXX, solve_case2(&example));
    // assert_eq!(XXX, solve_case2(&input));
}
