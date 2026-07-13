use aoc::grid::{ConfigInput, Grid, Ipos};
use rustc_hash::FxHashSet;

const EMPTY_SPACE: char = '.';
const OBSTACLE: char = '#';
const STARTING_GUARD: char = '^';
const VISITED_SPACE: char = 'X';
// We'll add edges to the map to remove the need for bounds checking.
const EDGE: char = '?';

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

fn rotate_right(d: Ipos) -> Ipos {
    let scale = if d.x == 0 { -1 } else { 1 };
    Ipos::new(d.y * scale, d.x * scale)
}

// Tries to turn right, at most 3 times, stopping as soon as a non-obstacle
// cell is found, which may be the edge -- in which case the walk ends (None).
fn step(lab_map: &Grid<char>, pos: Ipos, direction: &mut Ipos) -> Option<Ipos> {
    let candidates = [
        *direction,
        rotate_right(*direction),
        rotate_right(rotate_right(*direction)),
    ];
    let chosen = candidates
        .iter()
        .find(|&&d| {
            let p = pos + d;
            *lab_map.get(p.y as usize, p.x as usize) != OBSTACLE
        })
        .copied()
        .unwrap_or(*candidates.last().unwrap());
    *direction = chosen;
    let new_pos = pos + chosen;
    if *lab_map.get(new_pos.y as usize, new_pos.x as usize) == EDGE {
        None
    } else {
        Some(new_pos)
    }
}

fn count_visited(lab_map: &mut Grid<char>, start_pos: Ipos) -> usize {
    let mut pos = start_pos;
    let mut direction = Ipos::new(0, -1);
    loop {
        *lab_map.get_mut(pos.y as usize, pos.x as usize) = VISITED_SPACE;
        match step(lab_map, pos, &mut direction) {
            Some(new_pos) => pos = new_pos,
            None => break,
        }
    }
    lab_map.data.iter().filter(|&&c| c == VISITED_SPACE).count()
}

fn detects_loop(lab_map: &Grid<char>, start_pos: Ipos) -> bool {
    let mut pos = start_pos;
    let mut direction = Ipos::new(0, -1);
    let mut visited = FxHashSet::default();
    loop {
        if !visited.insert((pos, direction)) {
            // Detected a loop
            return true;
        }
        match step(lab_map, pos, &mut direction) {
            Some(new_pos) => pos = new_pos,
            None => return false,
        }
    }
}

// Checks whether placing an obstacle at pos would catch the guard in a loop.
// Ignores edges and the guard itself.
fn is_looping_obstacle(lab_map: &mut Grid<char>, pos: Ipos, start_pos: Ipos) -> bool {
    if pos == start_pos {
        return false;
    }
    if *lab_map.get(pos.y as usize, pos.x as usize) != EMPTY_SPACE {
        return false;
    }
    *lab_map.get_mut(pos.y as usize, pos.x as usize) = OBSTACLE;
    let looped = detects_loop(lab_map, start_pos);
    *lab_map.get_mut(pos.y as usize, pos.x as usize) = EMPTY_SPACE;
    looped
}

fn interfere_with_guard(lab_map: &mut Grid<char>, start_pos: Ipos) -> usize {
    let num_rows = lab_map.num_rows;
    let num_columns = lab_map.num_columns;
    // Go through each point on the map and check if placing an obstacle there
    // would catch the guard in a loop.
    (1..num_rows - 1)
        .flat_map(|row| (1..num_columns - 1).map(move |col| (row, col)))
        .filter(|&(row, col)| {
            is_looping_obstacle(
                &mut *lab_map,
                Ipos::new(col as isize, row as isize),
                start_pos,
            )
        })
        .count()
}

fn solve_case<const INTERFERE: bool>(input: &Input) -> usize {
    // count_visited/interfere_with_guard mutate the map,
    // so each part needs to start from its own copy
    let mut lab_map = input.lab_map.clone();
    if !INTERFERE {
        count_visited(&mut lab_map, input.starting_guard_pos)
    } else {
        interfere_with_guard(&mut lab_map, input.starting_guard_pos)
    }
}

fn main() {
    println!("Part 1");
    let example = parse("day06.example");
    aoc::expect_result!(41, solve_case::<false>(&example));
    let input = parse("day06.input");
    aoc::expect_result!(5030, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(6, solve_case::<true>(&example));
    aoc::expect_result!(1928, solve_case::<true>(&input));
}
