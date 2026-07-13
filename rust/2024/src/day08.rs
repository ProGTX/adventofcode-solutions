use aoc::grid::Ipos;
use rustc_hash::{FxHashMap, FxHashSet};

const EMPTY_SPACE: char = '.';

type Antennas = FxHashMap<char, Vec<Ipos>>;

struct Input {
    antennas: Antennas,
    grid_size: Ipos,
}

fn parse(filename: &str) -> Input {
    let mut antennas = Antennas::default();
    let mut grid_size = Ipos::new(0, 0);
    for (row, line) in aoc::file::read_lines(filename).iter().enumerate() {
        if grid_size.x == 0 {
            grid_size.x = line.len() as isize;
        }
        for (column, value) in line.chars().enumerate() {
            if value != EMPTY_SPACE {
                antennas
                    .entry(value)
                    .or_default()
                    .push(Ipos::new(column as isize, row as isize));
            }
        }
        grid_size.y += 1;
    }
    Input {
        antennas,
        grid_size,
    }
}

fn within_bounds(check: Ipos, grid_size: Ipos) -> bool {
    check.x >= 0 && check.y >= 0 && check.x < grid_size.x && check.y < grid_size.y
}

fn calculate_antinodes<const HARMONICS: bool>(antennas: &Antennas, grid_size: Ipos) -> usize {
    let mut antinodes: FxHashSet<Ipos> = FxHashSet::default();
    for locations in antennas.values() {
        // Check every pair of locations
        for (loc_id, &location1) in locations.iter().enumerate() {
            if HARMONICS && locations.len() > 1 {
                antinodes.insert(location1);
            }
            for &location2 in &locations[loc_id + 1..] {
                let distance12 = location2 - location1;
                let mut anode = location1;
                loop {
                    anode -= distance12;
                    if !within_bounds(anode, grid_size) {
                        break;
                    }
                    antinodes.insert(anode);
                    if !HARMONICS {
                        break;
                    }
                }
                let mut anode = location2;
                loop {
                    anode += distance12;
                    if !within_bounds(anode, grid_size) {
                        break;
                    }
                    antinodes.insert(anode);
                    if !HARMONICS {
                        break;
                    }
                }
            }
        }
    }
    antinodes.len()
}

fn solve_case<const HARMONICS: bool>(input: &Input) -> usize {
    calculate_antinodes::<HARMONICS>(&input.antennas, input.grid_size)
}

fn main() {
    println!("Part 1");
    let example = parse("day08.example");
    aoc::expect_result!(14, solve_case::<false>(&example));
    let input = parse("day08.input");
    aoc::expect_result!(220, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(34, solve_case::<true>(&example));
    aoc::expect_result!(813, solve_case::<true>(&input));
}
