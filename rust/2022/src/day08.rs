type Forrest = aoc::grid::Grid<u32>;
type Score = aoc::grid::Grid<u32>;

fn parse(filename: &str) -> Forrest {
    aoc::grid::from_file(filename)
}

fn score_row<ColumnIt, F>(row: usize, column_it: ColumnIt, score_init: u32, check: F) -> u32
where
    ColumnIt: Iterator<Item = usize>,
    F: Fn(usize, usize, &mut u32) -> bool,
{
    let mut score = score_init;
    for column in column_it {
        if (!check(row, column, &mut score)) {
            break;
        }
    }
    return score;
}

fn score_column<RowIt, F>(column: usize, row_it: RowIt, score_init: u32, check: F) -> u32
where
    RowIt: Iterator<Item = usize>,
    F: Fn(usize, usize, &mut u32) -> bool,
{
    let mut score = score_init;
    for row in row_it {
        if (!check(row, column, &mut score)) {
            break;
        }
    }
    return score;
}

fn solve_case1(forrest: &Forrest) -> u32 {
    let mut visibility = Score::new(0, forrest.num_rows, forrest.num_columns);
    for (index, tree_height) in forrest.data.iter().enumerate() {
        let (tree_column, tree_row) = forrest.position(index);
        let check = |row, column, visible: &mut u32| -> bool {
            if (forrest.get(row, column) >= tree_height) {
                *visible = 0;
                return false;
            }
            return true;
        };
        let visible = false
            || (score_row(tree_row, (tree_column + 1..forrest.num_columns), 1, check) > 0)
            || (score_row(tree_row, (0..tree_column).rev(), 1, check) > 0)
            || (score_column(tree_column, (tree_row + 1..forrest.num_rows), 1, check) > 0)
            || (score_column(tree_column, (0..tree_row).rev(), 1, check) > 0);
        visibility.modify(visible as u32, tree_row, tree_column);
    }
    visibility.data.iter().sum()
}

fn solve_case2(forrest: &Forrest) -> u32 {
    let mut score = Score::new(0, forrest.num_rows, forrest.num_columns);
    for (index, tree_height) in forrest.data.iter().enumerate() {
        let (tree_column, tree_row) = forrest.position(index);
        let check = |row, column, scenic_score: &mut u32| -> bool {
            *scenic_score += 1;
            if (forrest.get(row, column) >= tree_height) {
                return false;
            }
            return true;
        };
        let tree_score = 1
            * score_row(tree_row, (tree_column + 1..forrest.num_columns), 0, check)
            * score_row(tree_row, (0..tree_column).rev(), 0, check)
            * score_column(tree_column, (tree_row + 1..forrest.num_rows), 0, check)
            * score_column(tree_column, (0..tree_row).rev(), 0, check);
        score.modify(tree_score, tree_row, tree_column);
    }
    *score.data.iter().max().unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day08.example");
    assert_eq!(21, solve_case1(&example));
    let input = parse("day08.input");
    assert_eq!(1681, solve_case1(&input));

    println!("Part 2");
    assert_eq!(8, solve_case2(&example));
    assert_eq!(201684, solve_case2(&input));
}
