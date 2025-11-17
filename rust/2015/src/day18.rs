type Grid = aoc::grid::Grid<char>;
const LIGHT_ON: char = '#';
const LIGHT_OFF: char = '.';

fn parse(filename: &str) -> Grid {
    Grid::from_file(&filename)
}

fn solve_case<const STEPS: u32, const STUCK_CORNERS: bool>(original_lights: &Grid) -> u32 {
    let mut current_lights = original_lights.clone();
    let side = current_lights.num_rows;
    let corners = [(0, 0), (0, side - 1), (side - 1, 0), (side - 1, side - 1)];
    if (STUCK_CORNERS) {
        for corner in corners {
            current_lights.modify(LIGHT_ON, corner.1, corner.0);
        }
    }
    let mut next_lights = current_lights.clone();
    for _ in 0..STEPS {
        for (index, light) in current_lights.data.iter().enumerate() {
            let (column, row) = current_lights.position(index);
            if (STUCK_CORNERS) {
                if (corners.contains(&(column, row))) {
                    continue;
                }
            }
            let num_on_neighbors = current_lights
                .all_neighbor_values(row, column)
                .into_iter()
                .filter(|light| *light == LIGHT_ON)
                .count();
            let next = if (*light == LIGHT_ON) {
                if ((num_on_neighbors == 2) || (num_on_neighbors == 3)) {
                    LIGHT_ON
                } else {
                    LIGHT_OFF
                }
            } else {
                if (num_on_neighbors == 3) {
                    LIGHT_ON
                } else {
                    LIGHT_OFF
                }
            };
            next_lights.modify(next, row, column);
        }
        std::mem::swap(&mut current_lights, &mut next_lights);
    }
    return current_lights
        .data
        .iter()
        .filter(|light| **light == LIGHT_ON)
        .count() as u32;
}

fn main() {
    println!("Part 1");
    let example = parse("day18.example");
    assert_eq!(4, solve_case::<4, false>(&example));
    let input = parse("day18.input");
    assert_eq!(1061, solve_case::<100, false>(&input));
    println!("Part 2");
    assert_eq!(17, solve_case::<5, true>(&example));
    assert_eq!(1006, solve_case::<100, true>(&input));
}
