type CharGrid = aoc::grid::Grid<char>;

fn parse(filename: &str) -> CharGrid {
    CharGrid::from_file(filename)
}

fn solve_case1(char_grid: &CharGrid) -> u64 {
    let numbers = {
        let mut num_columns = 0;
        let mut data = Vec::new();
        for row_id in (0..char_grid.num_rows - 1) {
            let line = char_grid.row(row_id).iter().collect::<String>();
            data.extend(
                line.split_ascii_whitespace()
                    .map(|num| num.parse::<u64>().unwrap()),
            );
            if (num_columns == 0) {
                num_columns = data.len();
            }
        }
        aoc::grid::Grid::<u64> {
            data,
            num_rows: char_grid.num_rows - 1,
            num_columns: num_columns,
        }
    };
    let operations = char_grid
        .row(char_grid.num_rows - 1)
        .iter()
        .filter(|&&c| (c == '+') || (c == '*'))
        .collect::<Vec<_>>();
    return (0..numbers.num_columns)
        .map(|column_id| {
            let add = (*operations[column_id] == '+');
            let init = if (add) { 0_u64 } else { 1_u64 };
            numbers.column(column_id).fold(
                init,
                |acc, current| {
                    if (add) { acc + current } else { acc * current }
                },
            )
        })
        .sum();
}

fn solve_case2(char_grid: &CharGrid) -> u64 {
    let mut result = 0;
    let mut current = Vec::new();
    const EMPTY: char = ' ';
    for column_id in (0..char_grid.num_columns).rev() {
        let mut number = 0;
        for &value_char in char_grid.column(column_id).take(char_grid.num_rows - 1) {
            if (value_char == EMPTY) {
                continue;
            }
            number = number * 10 + value_char.to_digit(10).unwrap() as u64;
        }
        if (number == 0) {
            continue;
        }
        current.push(number);
        let op = *char_grid.get(char_grid.num_rows - 1, column_id);
        if (op == EMPTY) {
            continue;
        }
        let add = (op == '+');
        let init = if (add) { 0_u64 } else { 1_u64 };
        result += current.iter().fold(
            init,
            |acc, current| {
                if (add) { acc + current } else { acc * current }
            },
        );
        current.clear();
    }
    return result;
}

fn main() {
    println!("Part 1");
    let example = parse("day06.example");
    assert_eq!(4277556, solve_case1(&example));
    let input = parse("day06.input");
    assert_eq!(5322004718681, solve_case1(&input));

    println!("Part 2");
    assert_eq!(3263827, solve_case2(&example));
    assert_eq!(9876636978528, solve_case2(&input));
}
