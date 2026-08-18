use aoc::grid::Grid;
use itertools::Itertools;
use rustc_hash::FxBuildHasher;

type Input = Grid<char>;

fn parse(filename: &str) -> Input {
    Input::from_file(filename)
}

const LEAST: bool = false;
const MOST: bool = true;

fn solve_case<const ORDER: bool>(msg_grid: &Input) -> String {
    (0..msg_grid.num_columns)
        .map(|col| {
            msg_grid
                .column(col)
                .counts_with_hasher(FxBuildHasher)
                .into_iter()
                .max_by(|(_, lhs), (_, rhs)| {
                    if ORDER == MOST {
                        lhs.cmp(rhs)
                    } else {
                        rhs.cmp(lhs)
                    }
                })
                .unwrap()
                .0
        })
        .collect()
}

fn main() {
    println!("Part 1");
    let example = parse("day06.example");
    aoc::expect_result!("easter", solve_case::<MOST>(&example));
    let input = parse("day06.input");
    aoc::expect_result!("umejzgdw", solve_case::<MOST>(&input));

    println!("Part 2");
    aoc::expect_result!("advent", solve_case::<LEAST>(&example));
    aoc::expect_result!("aovueakv", solve_case::<LEAST>(&input));
}
