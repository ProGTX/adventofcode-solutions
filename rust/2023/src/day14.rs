use aoc::direction::Direction;
use aoc::grid::Grid;
use aoc::point::Point;
use std::collections::HashMap;

const ROUND_ROCK: char = 'O';
const EMPTY: char = '.';

fn parse(filename: &str) -> Grid<char> {
    Grid::<char>::from_file(filename)
}

fn north_load(platform: &Grid<char>) -> i32 {
    let mut total_load = 0i32;
    for row in 0..platform.num_rows {
        for col in 0..platform.num_columns {
            if *platform.get(row, col) == ROUND_ROCK {
                total_load += (platform.num_rows - row) as i32;
            }
        }
    }
    total_load
}

fn single_rock_fall(data: &mut [char], start: usize, begin: usize, end: usize, fall_stride: isize) {
    let mut previous = start;
    let mut current = start as isize + fall_stride;
    while (current >= begin as isize) && (current < end as isize) {
        let current_pos = current as usize;
        if data[current_pos] != EMPTY {
            break;
        }
        previous = current_pos;
        current += fall_stride;
    }
    if previous != start {
        data[previous] = ROUND_ROCK;
        data[start] = EMPTY;
    }
}

fn rocks_fall_vertical(platform: &mut Grid<char>, fall_diff: Point<i32>) {
    assert_eq!(fall_diff.x, 0, "Invalid fall_diff for vertical function");
    let num_columns = platform.num_columns as isize;
    let fall_stride = num_columns * fall_diff.y as isize;
    let data_len = platform.data.len();
    let mut row_start = if fall_stride < 0 {
        0
    } else {
        (platform.num_rows as isize - 1) * num_columns
    };
    for _ in 0..platform.num_rows {
        for col in 0..num_columns {
            let start = (row_start + col) as usize;
            if platform.data[start] != ROUND_ROCK {
                continue;
            }
            single_rock_fall(&mut platform.data, start, 0, data_len, fall_stride);
        }
        row_start -= fall_stride;
    }
}

fn rocks_fall_horizontal(platform: &mut Grid<char>, fall_diff: Point<i32>) {
    assert_eq!(fall_diff.y, 0, "Invalid fall_diff for horizontal function");
    let fall_stride = fall_diff.x as isize;
    let num_columns = platform.num_columns;
    let mut current_col = if fall_stride < 0 {
        0
    } else {
        num_columns as isize - 1
    };
    for _ in 0..num_columns {
        for row in 0..platform.num_rows {
            let row_begin = row * num_columns;
            let start = (row_begin as isize + current_col) as usize;
            if platform.data[start] != ROUND_ROCK {
                continue;
            }
            single_rock_fall(
                &mut platform.data,
                start,
                row_begin,
                row_begin + num_columns,
                fall_stride,
            );
        }
        current_col -= fall_stride;
    }
}

fn rocks_fall_inplace(platform: &mut Grid<char>, direction: Direction) {
    let fall_diff = direction.diff();
    if fall_diff.x == 0 {
        rocks_fall_vertical(platform, fall_diff);
    } else {
        rocks_fall_horizontal(platform, fall_diff);
    }
}

fn solve_case1(input: &Grid<char>) -> i32 {
    let mut platform = input.clone();
    rocks_fall_inplace(&mut platform, Direction::North);
    north_load(&platform)
}

fn spin_cycle(platform: &mut Grid<char>) {
    rocks_fall_vertical(platform, Direction::North.diff());
    rocks_fall_horizontal(platform, Direction::West.diff());
    rocks_fall_vertical(platform, Direction::South.diff());
    rocks_fall_horizontal(platform, Direction::East.diff());
}

fn solve_case2(input: &Grid<char>) -> i32 {
    let mut platform = input.clone();

    // Imagine an iteration that looks something like this:
    // |........|.................|.................|.................|........|
    // 0        S                S+A               S+2A              S+3A      N
    //
    // S is the first iteration where the cycle with a period of A begins
    // This cycle repeats until almost the end, where we need to figure out
    // the state of the cycle at point N, which represents NUM_ITER.
    // So the cycle map will be used to store possible values of S,
    // then when we find a cycle we can figure out A and the remainder (x)
    // and calculate the state of the cycle at point N:
    // x = (N-S) % A

    let mut cycle_map: HashMap<String, i32> = HashMap::new();
    const NUM_ITER: i32 = 1_000_000_000;

    for i in 0..NUM_ITER {
        let platform_str: String = platform.data.iter().collect();
        if let Some(&start) = cycle_map.get(&platform_str) {
            // We found a cycle, run only for the minimum amount of iterations
            let remainder = (NUM_ITER - start) % (i - start);
            for _ in 0..remainder {
                spin_cycle(&mut platform);
            }
            break;
        }
        cycle_map.insert(platform_str, i);
        spin_cycle(&mut platform);
    }
    north_load(&platform)
}

fn rocks_fall(mut platform: Grid<char>, direction: Direction) -> Grid<char> {
    rocks_fall_inplace(&mut platform, direction);
    platform
}

fn test_platform() -> Grid<char> {
    let data = "\
O....#....
O.OO#....#
.....##...
OO.#O....O
.O.....O#.
O.#..O.#.#
..O..#O..O
.......O..
#....###..
#OO..#....";
    Grid::from_vec(data.lines().flat_map(|line| line.chars()).collect(), 10, 10)
}

fn main() {
    // Asserts
    let expected: Vec<char> = "\
OOOO.#.O..
OO..#....#
OO..O##..O
O..#.OO...
........#.
..#....#.#
..O..#.O.O
..O.......
#....###..
#....#...."
        .lines()
        .flat_map(|line| line.chars())
        .collect();
    assert_eq!(expected, rocks_fall(test_platform(), Direction::North).data);
    assert_eq!(
        136,
        north_load(&rocks_fall(test_platform(), Direction::North))
    );

    println!("Part 1");
    let example = parse("day14.example");
    aoc::expect_result!(136, solve_case1(&example));
    let input = parse("day14.input");
    aoc::expect_result!(108857, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(64, solve_case2(&example));
    aoc::expect_result!(95273, solve_case2(&input));
}
