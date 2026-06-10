use aoc::direction::Direction;
use aoc::grid::{Grid, Upos};
use aoc::point::{Point, calculate_area};

type Pos = Point<i32>;

const VERTICAL_PIPE: char = '|';
const HORIZONTAL_PIPE: char = '-';
const NORTH_EAST_L: char = 'L';
const NORTH_WEST_J: char = 'J';
const SOUTH_EAST_F: char = 'F';
const SOUTH_WEST_7: char = '7';
const START: char = 'S';

const NORTH_ALLOWED: [char; 3] = [VERTICAL_PIPE, SOUTH_EAST_F, SOUTH_WEST_7];
const SOUTH_ALLOWED: [char; 3] = [VERTICAL_PIPE, NORTH_EAST_L, NORTH_WEST_J];
const WEST_ALLOWED: [char; 3] = [HORIZONTAL_PIPE, NORTH_EAST_L, SOUTH_EAST_F];
const EAST_ALLOWED: [char; 3] = [HORIZONTAL_PIPE, NORTH_WEST_J, SOUTH_WEST_7];

struct Input {
    field: Grid<char>,
    start_index: usize,
}

fn parse(filename: &str) -> Input {
    let field = Grid::<char>::from_file(filename);
    let start_index = field.data.iter().position(|&c| c == START).unwrap();
    Input { field, start_index }
}

fn to_pos(p: Upos) -> Pos {
    Pos::new(p.x as i32, p.y as i32)
}

fn in_bounds(field: &Grid<char>, pos: Pos) -> bool {
    field.in_bounds_signed(pos.y as isize, pos.x as isize)
}

fn linear_index(field: &Grid<char>, pos: Pos) -> usize {
    field.linear_index(pos.y as usize, pos.x as usize)
}

fn get_pipe_loop(field: &Grid<char>, start_index: usize) -> Vec<usize> {
    let mut loop_indexes = vec![start_index];
    let pos_2d = to_pos(field.position(start_index));

    let north_diff = Direction::North.diff();
    let south_diff = Direction::South.diff();
    let west_diff = Direction::West.diff();
    let east_diff = Direction::East.diff();

    // Separate logic for finding the neighbors of the start position
    let mut start_neighbor_diffs: Vec<Pos> = Vec::new();
    for (diff, valid_values) in [
        (south_diff, SOUTH_ALLOWED),
        (north_diff, NORTH_ALLOWED),
        (west_diff, WEST_ALLOWED),
        (east_diff, EAST_ALLOWED),
    ] {
        if start_neighbor_diffs.len() > 1 {
            break;
        }
        let pos = pos_2d + diff;
        if !in_bounds(field, pos) {
            continue;
        }
        let value = *field.get(pos.y as usize, pos.x as usize);
        assert_ne!(
            value, START,
            "Something went wrong, cannot use start as neighbor at this point"
        );
        if !valid_values.contains(&value) {
            continue;
        }
        start_neighbor_diffs.push(diff);
    }
    loop_indexes.push(linear_index(field, pos_2d + start_neighbor_diffs[0]));

    let mut current_index = *loop_indexes.last().unwrap();
    loop {
        let pos_2d = to_pos(field.position(current_index));
        // No need to check edges, assume each pipe always has two neighbors
        let neighbors: [Pos; 2] = match field.data[current_index] {
            VERTICAL_PIPE => [pos_2d + north_diff, pos_2d + south_diff],
            HORIZONTAL_PIPE => [pos_2d + east_diff, pos_2d + west_diff],
            NORTH_EAST_L => [pos_2d + north_diff, pos_2d + east_diff],
            NORTH_WEST_J => [pos_2d + north_diff, pos_2d + west_diff],
            SOUTH_EAST_F => [pos_2d + south_diff, pos_2d + east_diff],
            SOUTH_WEST_7 => [pos_2d + south_diff, pos_2d + west_diff],
            _ => unreachable!("Invalid pipe tile"),
        };

        let neighbor_index = if in_bounds(field, neighbors[0]) {
            let index = linear_index(field, neighbors[0]);
            if loop_indexes[loop_indexes.len() - 2] == index {
                linear_index(field, neighbors[1])
            } else {
                index
            }
        } else {
            linear_index(field, neighbors[1])
        };

        if neighbor_index == start_index {
            break;
        }
        loop_indexes.push(neighbor_index);
        current_index = neighbor_index;
    }
    loop_indexes
}

fn get_num_steps(field: &Grid<char>, start_index: usize) -> i32 {
    (get_pipe_loop(field, start_index).len() / 2) as i32
}

fn solve_case1(input: &Input) -> i32 {
    get_num_steps(&input.field, input.start_index)
}

// https://www.reddit.com/r/adventofcode/comments/18fgddy/2023_day_10_part_2_using_a_rendering_algorithm_to/
// https://en.wikipedia.org/wiki/Point_in_polygon
fn num_inside(field: &Grid<char>, start_index: usize) -> i32 {
    let pipe_loop = get_pipe_loop(field, start_index);
    let corners: Vec<Pos> = pipe_loop
        .iter()
        .filter(|&&index| !matches!(field.data[index], VERTICAL_PIPE | HORIZONTAL_PIPE))
        .map(|&index| to_pos(field.position(index)))
        .collect();
    let area: i64 = calculate_area(&corners);
    (area - (pipe_loop.len() / 2) as i64 + 1) as i32
}

fn solve_case2(input: &Input) -> i32 {
    num_inside(&input.field, input.start_index)
}

fn test_field() -> Grid<char> {
    let data = "\
.....
.S-7.
.|.|.
.L-J.
.....";
    Grid::from_vec(
        data.lines().flat_map(|line| line.chars()).collect(), //
        5,
        5,
    )
}

fn main() {
    // Asserts
    assert_eq!(4, get_num_steps(&test_field(), 6));
    assert_eq!(
        vec![6, 11, 16, 17, 18, 13, 8, 7],
        get_pipe_loop(&test_field(), 6)
    );
    assert_eq!(1, num_inside(&test_field(), 6));

    println!("Part 1");
    let example = parse("day10.example");
    aoc::expect_result!(4, solve_case1(&example));
    let example2 = parse("day10.example2");
    aoc::expect_result!(8, solve_case1(&example2));
    let input = parse("day10.input");
    aoc::expect_result!(6820, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(1, solve_case2(&example));
    aoc::expect_result!(1, solve_case2(&example2));
    let example3 = parse("day10.example3");
    aoc::expect_result!(4, solve_case2(&example3));
    aoc::expect_result!(337, solve_case2(&input));
}
