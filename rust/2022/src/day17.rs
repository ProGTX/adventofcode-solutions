use std::collections::HashMap;

type Input = Vec<i32>;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
        .into_iter()
        .next()
        .unwrap()
        .chars()
        .map(|c| if c == '<' { -1 } else { 1 })
        .collect()
}

// Each rock is 5 (x, y) points, some possibly duplicated, arranged so that
// specific indices are the boundary points used for fast collision checks.
const ROCK_SIZE: usize = 5;
type Rock = [(i64, i64); ROCK_SIZE];
const LEFT: usize = 0; // index of leftmost point
const TOP: usize = 1; // index of topmost point
const BOTTOM: usize = 3; // index of bottommost point
const RIGHT: usize = 4; // index of rightmost point

const WIDTH: i64 = 7;
const ROW_SIZE: usize = (WIDTH as usize) + 2; // 7 wide + 2 side walls
type ChamberRow = [char; ROW_SIZE];

const EMPTY: char = ' ';
const WALL: char = '|'; // floor and side walls
const ROCK: char = '#'; // settled rocks

// Polynomial rolling hash of the top 30 chamber rows.
// Each character is folded in as: hash = hash * 131 + c.
// 131 is prime and larger than the printable ASCII range,
// which spreads bits well and keeps collisions rare.
// A false positive would yield a wrong answer,
// but with a 64-bit result and only thousands of states
// visited before a cycle is found,
// the collision probability is negligible.
fn surface_hash(chamber: &[ChamberRow], current_height: i64) -> u64 {
    let mut hash: u64 = 0;
    for k in 0i64..30 {
        let row = current_height - 1 - k;
        if row < 0 {
            break;
        }
        for &c in &chamber[row as usize] {
            hash = hash.wrapping_mul(131).wrapping_add(c as u64);
        }
    }
    hash
}

fn solve_case(pattern: &Input, num_rocks: i64) -> i64 {
    // Each rock's 5 points are pre-displaced by (2, 3) so that the left edge
    // starts 2 units from the left wall and the bottom edge starts 3 units
    // above the current pile height (as required by the puzzle rules).
    const INIT_DX: i64 = 2;
    const INIT_DY: i64 = 3;
    const NUM_ROCK_TYPES: i64 = 5;
    let rock_templates: [Rock; NUM_ROCK_TYPES as usize] = {
        let offsets = [
            [(0, 0), (1, 0), (2, 0), (3, 0), (3, 0)], // ####
            [(0, 1), (1, 2), (1, 1), (1, 0), (2, 1)], // plus
            [(0, 0), (2, 2), (2, 0), (1, 0), (2, 1)], // reverse-L
            [(0, 1), (0, 3), (0, 0), (0, 0), (0, 2)], // |
            [(0, 0), (0, 1), (0, 0), (1, 0), (1, 1)], // square
        ];
        offsets.map(|rock| rock.map(|(x, y)| (x + INIT_DX, y + INIT_DY)))
    };

    // Chamber rows indexed by y-coordinate: row 0 is the floor.
    let mut chamber: Vec<ChamberRow> = vec![[WALL; ROW_SIZE]];
    let empty_row: ChamberRow = {
        let mut r = [EMPTY; ROW_SIZE];
        r[0] = WALL;
        r[8] = WALL;
        r
    };

    let mut current_height: i64 = 1;
    let mut pattern_index: usize = 0;

    // Cycle detection: key = (rock_type, pattern_index, surface_hash).
    // When the same key recurs the full simulation state is periodic.
    let mut seen: HashMap<(usize, usize, u64), (i64, i64)> = HashMap::new();
    let mut skipped_height: i64 = 0;
    let mut cycle_found = false;

    let mut index: i64 = 0;
    while index < num_rocks {
        // Only compute the key and update the map until the cycle is detected;
        // afterwards just simulate the remaining leftover rocks.
        if !cycle_found {
            let rock_type = (index % NUM_ROCK_TYPES) as usize;
            let key = (
                rock_type,
                pattern_index,
                surface_hash(&chamber, current_height),
            );
            if let Some(&(prev_index, prev_height)) = seen.get(&key) {
                let cycle_rocks = index - prev_index;
                let cycle_height = (current_height - 1) - prev_height;
                let full_cycles = (num_rocks - index) / cycle_rocks;
                skipped_height = full_cycles * cycle_height;
                index += full_cycles * cycle_rocks;
                cycle_found = true;
                if index >= num_rocks {
                    break;
                }
            } else {
                seen.insert(key, (index, current_height - 1));
            }
        }

        let mut rock = rock_templates[(index % NUM_ROCK_TYPES) as usize];
        let rock_height = rock[TOP].1 - rock[BOTTOM].1 + 1;

        // Initial falling stage: consume one jet per row
        // for the 3 empty rows above the pile, without collision checking
        // (the rock is guaranteed to be above everything at this point).
        for _ in 0..INIT_DY {
            let sideways = pattern[pattern_index] as i64;
            let leftmost = rock[LEFT].0 + sideways;
            let rightmost = rock[RIGHT].0 + sideways;
            if leftmost >= 0 && rightmost < WIDTH {
                for p in rock.iter_mut() {
                    p.0 += sideways;
                }
            }
            pattern_index = (pattern_index + 1) % pattern.len();
        }

        // Extend the chamber to fit the rock,
        // then shift into chamber coordinates:
        // +1 for the left wall, y to current_height - INIT_DY
        let missing = current_height + rock_height - chamber.len() as i64;
        for _ in 0..missing.max(0) {
            chamber.push(empty_row);
        }
        for pos in rock.iter_mut() {
            pos.0 += 1;
            pos.1 += current_height - INIT_DY;
        }

        // Move rock until it settles
        loop {
            let sideways = pattern[pattern_index] as i64;
            // Try horizontal jet
            let can_move_h = rock
                .iter()
                .all(|&(x, y)| chamber[y as usize][(x + sideways) as usize] == EMPTY);
            if can_move_h {
                for pos in rock.iter_mut() {
                    pos.0 += sideways;
                }
            }
            pattern_index = (pattern_index + 1) % pattern.len();

            // Try to fall one step
            let can_fall = rock
                .iter()
                .all(|&(x, y)| chamber[(y - 1) as usize][x as usize] == EMPTY);
            if can_fall {
                for p in rock.iter_mut() {
                    p.1 -= 1;
                }
            } else {
                // Rock settled: engrave it and update height
                for &(x, y) in &rock {
                    chamber[y as usize][x as usize] = ROCK;
                }
                current_height = current_height.max(rock[TOP].1 + 1);
                break;
            }
        }

        index += 1;
    }

    // Physical height of simulated rocks plus the height of all skipped periods
    current_height - 1 + skipped_height
}

fn main() {
    println!("Part 1");
    let example = parse("day17.example");
    aoc::expect_result!(17, solve_case(&example, 10));
    aoc::expect_result!(3068, solve_case(&example, 2022));
    let input = parse("day17.input");
    aoc::expect_result!(3085, solve_case(&input, 2022));

    println!("Part 2");
    aoc::expect_result!(1514285714288_i64, solve_case(&example, 1_000_000_000_000));
    aoc::expect_result!(1535483870924_i64, solve_case(&input, 1_000_000_000_000));
}
