fn parse(filename: &str) -> String {
    aoc::file::read_string(filename).trim().to_string()
}

fn solve_case1(compressed: &str) -> usize {
    let mut count = 0;
    let mut slice = compressed;
    while !slice.is_empty() {
        if let Some(open) = slice.find('(') {
            count += open;
            let close = slice.find(')').unwrap();
            debug_assert!(close > open, "Invalid parenthesis");
            let repetition = &slice[open + 1..close].split_once('x').unwrap();
            let num_chars = repetition.0.parse::<usize>().unwrap();
            let num_repeats = repetition.1.parse::<usize>().unwrap();
            count += num_chars * num_repeats;
            slice = &slice[close + 1 + num_chars..];
        } else {
            count += slice.len();
            break;
        }
    }
    return count;
}

fn main() {
    println!("Asserts");
    assert_eq!(6, solve_case1("ADVENT"));
    assert_eq!(7, solve_case1("A(1x5)BC"));
    assert_eq!(9, solve_case1("(3x3)XYZ"));
    assert_eq!(11, solve_case1("A(2x2)BCD(2x2)EFG"));
    assert_eq!(6, solve_case1("(6x1)(1x3)A"));

    println!("Part 1");
    let example = parse("day09.example");
    aoc::expect_result!(18, solve_case1(&example));
    let input = parse("day09.input");
    aoc::expect_result!(152851, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
    // aoc::expect_result!(1337, solve_case2(&example));
    // aoc::expect_result!(1337, solve_case2(&input));
}
