type Manifolds = aoc::grid::Grid<char>;
type Upos = aoc::grid::Upos;
const EMPTY: char = '.';
const SPLITTER: char = '^';

fn parse(filename: &str) -> (Manifolds, Upos) {
    let (manifolds, config) = Manifolds::from_file_config(
        filename,
        aoc::grid::ConfigInput {
            padding: None,
            start_char: Some('S'),
            end_char: None,
        },
    );
    (manifolds, config.start_pos.unwrap())
}

fn solve_case<const QUANTUM: bool>((manifolds, start_pos): &(Manifolds, Upos)) -> u64 {
    let mut num_splits = 0;
    let mut row_timelines = vec![0; manifolds.num_columns];
    row_timelines[start_pos.x] = 1;
    for row in (start_pos.y + 1)..(manifolds.num_rows) {
        let mut next_timelines = vec![0; manifolds.num_columns];
        for (column, &timelines) in row_timelines.iter().enumerate() {
            let value = *manifolds.get(row, column);
            match value {
                EMPTY => {
                    next_timelines[column] += timelines;
                }
                SPLITTER => {
                    num_splits += (timelines > 0) as u64;
                    next_timelines[column - 1] += timelines;
                    next_timelines[column + 1] += timelines;
                }
                _ => unreachable!("Invalid value in the manifold"),
            };
        }
        row_timelines = next_timelines;
    }
    return if (!QUANTUM) {
        num_splits
    } else {
        row_timelines.iter().sum()
    };
}

fn main() {
    println!("Part 1");
    let example = parse("day07.example");
    aoc::expect_result!(21, solve_case::<false>(&example));
    let example2 = parse("day07.example2");
    aoc::expect_result!(16, solve_case::<false>(&example2));
    let input = parse("day07.input");
    aoc::expect_result!(1560, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(40, solve_case::<true>(&example));
    aoc::expect_result!(26, solve_case::<true>(&example2));
    aoc::expect_result!(25592971184998, solve_case::<true>(&input));
}
