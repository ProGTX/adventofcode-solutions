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

fn is_triangle([a, b, c]: &[u32; 3]) -> bool {
    let is_sum_ok = |a, b, target: &u32| (a + b) > *target;
    is_sum_ok(a, b, c) && is_sum_ok(a, c, b) && is_sum_ok(b, c, a)
}

fn solve_case1(triangles: &Input) -> usize {
    triangles.iter().filter(|&t| is_triangle(t)).count()
}

fn solve_case2(triangles: &Input) -> usize {
    debug_assert!(
        triangles.len() % 3 == 0,
        "Reading 3 triangle lines at a time"
    );
    triangles
        .chunks(3)
        .map(|chunk| {
            let t0 = chunk[0];
            let t1 = chunk[1];
            let t2 = chunk[2];
            return (is_triangle(&[t0[0], t1[0], t2[0]])) as usize
                + (is_triangle(&[t0[1], t1[1], t2[1]])) as usize
                + (is_triangle(&[t0[2], t1[2], t2[2]])) as usize;
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day03.example");
    aoc::expect_result!(3, solve_case1(&example));
    let input = parse("day03.input");
    aoc::expect_result!(993, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(6, solve_case2(&example));
    aoc::expect_result!(1849, solve_case2(&input));
}
