use aoc::iter::CollectArrayVec;

type Triangle = [u32; 3];
type Input = Vec<Triangle>;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            line.split_whitespace()
                .map(|v| v.parse().unwrap())
                .collect_array_vec::<3>()
                .into_inner()
                .unwrap()
        })
        .collect()
}

fn solve_case1(triangles: &Input) -> usize {
    let is_sum_ok = |a, b, target| (a + b) > target;
    triangles
        .iter()
        .filter(|&&[a, b, c]| is_sum_ok(a, b, c) && is_sum_ok(a, c, b) && is_sum_ok(b, c, a))
        .count()
}

fn main() {
    println!("Part 1");
    let example = parse("day03.example");
    aoc::expect_result!(1, solve_case1(&example));
    let input = parse("day03.input");
    aoc::expect_result!(993, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
    // aoc::expect_result!(1337, solve_case2(&example));
    // aoc::expect_result!(1337, solve_case2(&input));
}
