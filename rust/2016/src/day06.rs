use aoc::grid::Grid;
use itertools::Itertools;
use rustc_hash::FxBuildHasher;

type Input = Grid<char>;

fn parse(filename: &str) -> Input {
    Input::from_file(filename)
}

fn solve_case1(msg_grid: &Input) -> String {
    (0..msg_grid.num_columns)
        .map(|col| {
            msg_grid
                .column(col)
                .counts_with_hasher(FxBuildHasher)
                .into_iter()
                .max_by_key(|(_, count)| *count)
                .unwrap()
                .0
        })
        .collect()
}

fn main() {
    println!("Part 1");
    let example = parse("day06.example");
    aoc::expect_result!("easter", solve_case1(&example));
    let input = parse("day06.input");
    aoc::expect_result!("umejzgdw", solve_case1(&input));

    println!("Part 2");
    //aoc::expect_result!(1337, solve_case2(&example));
    //aoc::expect_result!(1337, solve_case2(&input));
}
