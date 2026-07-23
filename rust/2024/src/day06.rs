use aoc::direction::Direction;
use aoc::grid::{ConfigInput, Grid, Ipos};

const EMPTY_SPACE: char = '.';
const OBSTACLE: char = '#';
const STARTING_GUARD: char = '^';
const VISITED_SPACE: char = 'X';
// We'll add edges to the map to remove the need for bounds checking.
const EDGE: char = '?';

// The guard only ever faces one of the 4 basic directions
// Rather than tracking direction as a point and rotating it,
// we track it as a facing_t index
// Turning right becomes a plain `(direction + 1) % aoc::NUM_FACING`
// instead of a rotation on a point,
// since the directions are already ordered clockwise
const NUM_DIRECTIONS: usize = 4;

struct Input {
    lab_map: Grid<char>,
    starting_guard_pos: Ipos,
}

fn parse(filename: &str) -> Input {
    let (lab_map, config) = Grid::<char>::from_file_config(
        filename,
        ConfigInput {
            padding: Some(EDGE),
            start_char: Some(STARTING_GUARD),
            end_char: None,
        },
    );
    let start = config.start_pos.unwrap();
    Input {
        lab_map,
        starting_guard_pos: Ipos::new(start.x as isize, start.y as isize),
    }
}

fn to_index(lab_map: &Grid<char>, pos: Ipos) -> isize {
    lab_map.linear_index(pos.y as usize, pos.x as usize) as isize
}

// Everything below operates on linear indices into the grid
// rather than (row, column) points:
// moving is just `index + deltas[direction]`
fn deltas_for(columns: isize) -> [isize; NUM_DIRECTIONS] {
    // Indexed by direction: east, south, west, north
    [1, columns, -1, -columns]
}

// Tries to turn right, at most 3 times, stopping as soon as a non-obstacle
// cell is found, which may be the edge -- in which case the walk ends (None).
fn step(
    lab_map: &Grid<char>,
    index: isize,
    deltas: &[isize; NUM_DIRECTIONS],
    direction: &mut usize,
) -> Option<isize> {
    let candidates = [
        *direction,
        (*direction + 1) % NUM_DIRECTIONS,
        (*direction + 2) % NUM_DIRECTIONS,
    ];
    let chosen = candidates
        .iter()
        .find(|&&d| lab_map.data[(index + deltas[d]) as usize] != OBSTACLE)
        .copied()
        .unwrap_or(*candidates.last().unwrap());
    *direction = chosen;
    let new_index = index + deltas[chosen];
    if lab_map.data[new_index as usize] == EDGE {
        None
    } else {
        Some(new_index)
    }
}

fn count_visited(
    lab_map: &mut Grid<char>,
    start_index: isize,
    deltas: &[isize; NUM_DIRECTIONS],
) -> usize {
    let mut index = start_index;
    let mut direction = Direction::North as usize;
    loop {
        lab_map.data[index as usize] = VISITED_SPACE;
        match step(lab_map, index, deltas, &mut direction) {
            Some(new_index) => index = new_index,
            None => break,
        }
    }
    lab_map.data.iter().filter(|&&c| c == VISITED_SPACE).count()
}

fn detects_loop(
    lab_map: &Grid<char>,
    start_index: isize,
    deltas: &[isize; NUM_DIRECTIONS],
    visited_directions: &mut [u8],
) -> bool {
    let mut index = start_index;
    let mut direction = Direction::North as usize;
    loop {
        let bit = 1u8 << direction;
        if visited_directions[index as usize] & bit != 0 {
            // Detected a loop
            return true;
        }
        visited_directions[index as usize] |= bit;
        match step(lab_map, index, deltas, &mut direction) {
            Some(new_index) => index = new_index,
            None => return false,
        }
    }
}

// Checks whether placing an obstacle at index would catch the guard in a loop.
// Ignores edges and the guard itself.
fn is_looping_obstacle(
    lab_map: &mut Grid<char>,
    index: isize,
    start_index: isize,
    deltas: &[isize; NUM_DIRECTIONS],
    visited_directions: &mut [u8],
) -> bool {
    if index == start_index {
        return false;
    }
    if lab_map.data[index as usize] != EMPTY_SPACE {
        return false;
    }
    lab_map.data[index as usize] = OBSTACLE;
    visited_directions.fill(0);
    let looped = detects_loop(lab_map, start_index, deltas, visited_directions);
    lab_map.data[index as usize] = EMPTY_SPACE;
    looped
}

fn interfere_with_guard(mut lab_map: Grid<char>, start_pos: Ipos) -> usize {
    let num_rows = lab_map.num_rows;
    let num_columns = lab_map.num_columns;
    let deltas = deltas_for(num_columns as isize);
    let start_index = to_index(&lab_map, start_pos);
    // Reused across every candidate obstacle position
    let mut visited_directions = vec![0u8; lab_map.data.len()];
    // Go through each point on the map and check if placing an obstacle there
    // would catch the guard in a loop.
    (1..num_rows - 1)
        .flat_map(|row| (1..num_columns - 1).map(move |col| row * num_columns + col))
        .filter(|&index| {
            is_looping_obstacle(
                &mut lab_map,
                index as isize,
                start_index,
                &deltas,
                &mut visited_directions,
            )
        })
        .count()
}

fn solve_part1(input: &Input) -> usize {
    // count_visited mutates the map, so start from a copy
    let mut lab_map = input.lab_map.clone();
    let deltas = deltas_for(lab_map.num_columns as isize);
    let start_index = to_index(&lab_map, input.starting_guard_pos);
    count_visited(&mut lab_map, start_index, &deltas)
}

fn solve_part2(input: &Input) -> usize {
    interfere_with_guard(input.lab_map.clone(), input.starting_guard_pos)
}

fn main() {
    println!("Part 1");
    let example = parse("day06.example");
    aoc::expect_result!(41, solve_part1(&example));
    let input = parse("day06.input");
    aoc::expect_result!(5030, solve_part1(&input));

    println!("Part 2");
    aoc::expect_result!(6, solve_part2(&example));
    aoc::expect_result!(1928, solve_part2(&input));
}
