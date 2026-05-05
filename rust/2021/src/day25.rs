use aoc::grid::Grid;

type Seafloor = Grid<char>;

fn parse(filename: &str) -> Seafloor {
    Grid::from_file(filename)
}

const EAST: char = '>';
const SOUTH: char = 'v';
const EMPTY: char = '.';

fn solve_case1(seafloor: &Seafloor) -> u32 {
    let mut grid = seafloor.clone();
    let mut steps = 0;

    loop {
        steps += 1;
        let mut moved = false;

        // Move EAST simultaneously
        let prev = grid.clone();
        for row in 0..grid.num_rows {
            for col in 0..grid.num_columns {
                if *prev.get(row, col) == EAST {
                    let next_col = (col + 1) % grid.num_columns;
                    if *prev.get(row, next_col) == EMPTY {
                        grid.modify(EMPTY, row, col);
                        grid.modify(EAST, row, next_col);
                        moved = true;
                    }
                }
            }
        }

        // Move SOUTH simultaneously
        let prev = grid.clone();
        for row in 0..grid.num_rows {
            for col in 0..grid.num_columns {
                if *prev.get(row, col) == SOUTH {
                    let next_row = (row + 1) % grid.num_rows;
                    if *prev.get(next_row, col) == EMPTY {
                        grid.modify(EMPTY, row, col);
                        grid.modify(SOUTH, next_row, col);
                        moved = true;
                    }
                }
            }
        }

        if !moved {
            return steps;
        }
    }
}

#[allow(dead_code)]
fn solve_case2(_seafloor: &Seafloor) -> u32 {
    // TODO: Implement Part 2
    0
}

fn main() {
    println!("Part 1");
    let example = parse("day25.example");
    assert_eq!(58, solve_case1(&example));
    let input = parse("day25.input");
    assert_eq!(329, solve_case1(&input));

    println!("Part 2");
    // assert_eq!(XXX, solve_case2(&example));
    // assert_eq!(XXX, solve_case2(&input));
}
