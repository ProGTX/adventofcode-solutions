fn parse(filename: &str) -> String {
    aoc::file::read_string(filename).trim().to_string()
}

fn decompressed_len<const RECURSION: bool>(compressed: &str) -> usize {
    let mut count = 0;
    let mut slice = compressed;
    while let Some(open) = slice.find('(') {
        count += open;
        let close = slice.find(')').unwrap();
        debug_assert!(close > open, "Invalid parenthesis");
        let repetition = &slice[open + 1..close].split_once('x').unwrap();
        let num_chars = repetition.0.parse::<usize>().unwrap();
        let num_repeats = repetition.1.parse::<usize>().unwrap();
        let repeat_end_index = close + 1 + num_chars;
        let subslice_len = if !RECURSION {
            num_chars
        } else {
            decompressed_len::<RECURSION>(&slice[close + 1..repeat_end_index])
        };
        count += subslice_len * num_repeats;
        slice = &slice[repeat_end_index..];
    }
    count += slice.len();

    return count;
}

fn solve_case1(compressed: &str) -> usize {
    decompressed_len::<false>(compressed)
}
fn solve_case2(compressed: &str) -> usize {
    decompressed_len::<true>(compressed)
}

fn main() {
    println!("Asserts");
    assert_eq!(6, decompressed_len::<false>("ADVENT"));
    assert_eq!(7, decompressed_len::<false>("A(1x5)BC"));
    assert_eq!(9, decompressed_len::<false>("(3x3)XYZ"));
    assert_eq!(11, decompressed_len::<false>("A(2x2)BCD(2x2)EFG"));
    assert_eq!(6, decompressed_len::<false>("(6x1)(1x3)A"));
    assert_eq!(9, decompressed_len::<true>("(3x3)XYZ"));
    assert_eq!(
        241920,
        decompressed_len::<true>("(27x12)(20x12)(13x14)(7x10)(1x12)A")
    );
    assert_eq!(
        445,
        decompressed_len::<true>("(25x3)(3x3)ABC(2x3)XY(5x2)PQRSTX(18x9)(3x2)TWO(5x7)SEVEN")
    );

    println!("Part 1");
    let example = parse("day09.example");
    aoc::expect_result!(18, solve_case1(&example));
    let input = parse("day09.input");
    aoc::expect_result!(152851, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(20, solve_case2(&example));
    aoc::expect_result!(11797310782, solve_case2(&input));
}
