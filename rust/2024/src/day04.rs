use aoc::grid::Grid;

// Empty character used both as separator and to signal out of bounds.
const EMPTY_CHAR: char = '?';

fn parse(filename: &str) -> Grid<char> {
    // This pads all four sides, so real data starts at row 1/column 1
    // instead of row 0/column 0.
    // The scans below account for that extra top row explicitly.
    Grid::from_file_config(
        filename,
        aoc::grid::ConfigInput {
            padding: Some(EMPTY_CHAR),
            ..Default::default()
        },
    )
    .0
}

////////////////////////////////////////////////////////////////////////////////
// PART 1

fn count_substrings(haystack: &[char], needle: &str) -> u32 {
    let needle: Vec<char> = needle.chars().collect();
    if haystack.len() < needle.len() {
        return 0;
    }
    (0..=haystack.len() - needle.len())
        .filter(|&i| haystack[i..i + needle.len()] == needle[..])
        .count() as u32
}

fn count_xmas(chars: &[char]) -> u32 {
    count_substrings(chars, "XMAS") + count_substrings(chars, "SAMX")
}

fn strided_chars(grid: &Grid<char>, start: usize, stride: usize) -> Vec<char> {
    grid.data[start..]
        .iter()
        .step_by(stride)
        .copied()
        .take_while(|&c| c != EMPTY_CHAR)
        .collect()
}

fn count_xmas_vertical(grid: &Grid<char>) -> u32 {
    // Start at row 1 to skip the extra top sentinel row (see `parse`).
    (0..grid.num_columns)
        .map(|column| {
            let start = grid.linear_index(1, column);
            count_xmas(&strided_chars(grid, start, grid.num_columns))
        })
        .sum()
}

fn count_xmas_diagonal(grid: &Grid<char>) -> u32 {
    let num_rows = grid.num_rows;
    let num_columns = grid.num_columns;
    let right_stride = num_columns + 1;
    let left_stride = num_columns - 1;

    let mut sum = 0;

    // Count diagonals along the top.
    // Skip first and last column because they contain empty terminators.
    // Start at row 1 to skip the extra top sentinel row (see `parse`).
    for column in 1..num_columns - 1 {
        let start = grid.linear_index(1, column);
        sum += count_xmas(&strided_chars(grid, start, right_stride));
        sum += count_xmas(&strided_chars(grid, start, left_stride));
    }
    // Count right diagonals along the left edge.
    // Skip row 1 because it was already counted in the first loop
    // (row 0 is the top sentinel, so real rows start at 1).
    // Skip last row because it contains empty terminators.
    for row in 2..num_rows - 1 {
        let start = grid.linear_index(row, 1);
        sum += count_xmas(&strided_chars(grid, start, right_stride));
    }
    // Count left diagonals along the right edge.
    // Skip row 1 because it was already counted in the first loop.
    // Skip last row because it contains empty terminators.
    let column = num_columns - 2;
    for row in 2..num_rows - 1 {
        let start = grid.linear_index(row, column);
        sum += count_xmas(&strided_chars(grid, start, left_stride));
    }

    sum
}

fn solve_case1(grid: &Grid<char>) -> u32 {
    count_xmas(&grid.data) + count_xmas_vertical(grid) + count_xmas_diagonal(grid)
}

////////////////////////////////////////////////////////////////////////////////
// PART 2

fn set_space(c: char) -> char {
    if c != EMPTY_CHAR { ' ' } else { c }
}

fn normalize_local_grid(row1: [char; 3], row2: [char; 3], row3: [char; 3]) -> [char; 9] {
    let mut result = [
        row1[0], row1[1], row1[2], row2[0], row2[1], row2[2], row3[0], row3[1], row3[2],
    ];
    result[1] = set_space(result[1]);
    result[3] = set_space(result[3]);
    result[5] = set_space(result[5]);
    result[7] = set_space(result[7]);
    result
}

fn x_mas_patterns() -> [[char; 9]; 4] {
    [
        normalize_local_grid(['M', '.', 'S'], ['.', 'A', '.'], ['M', '.', 'S']),
        normalize_local_grid(['M', '.', 'M'], ['.', 'A', '.'], ['S', '.', 'S']),
        normalize_local_grid(['S', '.', 'S'], ['.', 'A', '.'], ['M', '.', 'M']),
        normalize_local_grid(['S', '.', 'M'], ['.', 'A', '.'], ['S', '.', 'M']),
    ]
}

fn get_substr(grid: &Grid<char>, row: usize, column: usize) -> [char; 3] {
    [
        *grid.get(row, column),
        *grid.get(row, column + 1),
        *grid.get(row, column + 2),
    ]
}

// The algorithm is to find each A, collect the surrounding area,
// transform that into a normalized area, and compare against all valid x_mas areas.
fn count_x_mas_crosses(grid: &Grid<char>) -> u32 {
    let patterns = x_mas_patterns();
    let mut count = 0;
    // Go through the board, but skip edges to avoid bound checking.
    for row in 1..grid.num_rows - 1 {
        for col in 1..grid.num_columns - 1 {
            if *grid.get(row, col) != 'A' {
                continue;
            }
            // Collect area around the A.
            let area = normalize_local_grid(
                get_substr(grid, row - 1, col - 1),
                get_substr(grid, row, col - 1),
                get_substr(grid, row + 1, col - 1),
            );
            count += patterns.contains(&area) as u32;
        }
    }
    count
}

fn solve_case2(grid: &Grid<char>) -> u32 {
    count_x_mas_crosses(grid)
}

////////////////////////////////////////////////////////////////////////////////
// MAIN

fn main() {
    println!("Part 1");
    let example2 = parse("day04.example2");
    aoc::expect_result!(4, solve_case1(&example2));
    let example = parse("day04.example");
    aoc::expect_result!(18, solve_case1(&example));
    let input = parse("day04.input");
    aoc::expect_result!(2517, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(9, solve_case2(&example));
    aoc::expect_result!(1960, solve_case2(&input));
}
