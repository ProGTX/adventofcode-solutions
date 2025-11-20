fn parse(filename: &str) -> Vec<u32> {
    aoc::file::read_numbers(filename)
}

fn solve_case1(depths: &[u32]) -> usize {
    depths
        .windows(2)
        .filter(|window| window[1] > window[0])
        .count()
}

fn solve_case2(depths: &[u32]) -> usize {
    solve_case1(
        &depths
            .windows(3)
            .map(|window| window.iter().sum())
            .collect::<Vec<_>>(),
    )
}

fn main() {
    println!("Part 1");
    let example = parse("day01.example");
    assert_eq!(7, solve_case1(&example));
    let input = parse("day01.input");
    assert_eq!(1228, solve_case1(&input));

    println!("Part 2");
    assert_eq!(5, solve_case2(&example));
    assert_eq!(1257, solve_case2(&input));
}
