use aoc::grid::{ConfigInput, Grid, Ipos};

type Garden = Grid<char>;
type Pos = Ipos;
type Positions = Vec<Pos>;

const PLOT: char = '.';

struct Input {
    garden: Garden,
    positions: Positions,
}

fn parse(filename: &str) -> Input {
    let (mut garden, config) = Garden::from_file_config(
        filename,
        ConfigInput {
            padding: None,
            start_char: Some('S'),
            end_char: None,
        },
    );

    let start = config.start_pos.unwrap();
    garden.modify(PLOT, start.y, start.x);

    Input {
        garden,
        positions: vec![Pos::new(start.x as isize, start.y as isize)],
    }
}

/// Performs a single step of movement for the pots in the garden.
///
/// If `infinite` is true, the garden wraps around:
/// a pot moving off one edge reappears on the opposite edge.
fn move_one(
    ending: &mut Positions,
    garden: &Garden,
    starting: &Positions,
    infinite: bool,
) -> Positions {
    ending.clear();

    let bounds = Pos::new(garden.num_columns as isize, garden.num_rows as isize);

    for &start in starting {
        for diff in aoc::grid::BASIC_NEIGHBOR_DIFFS {
            let neighbor = start + diff;

            if !infinite {
                if garden.in_bounds_signed(neighbor.y, neighbor.x)
                    && *garden.get(neighbor.y as usize, neighbor.x as usize) == PLOT
                {
                    ending.push(neighbor);
                }
            } else {
                let neighbor_in_bounds = Pos::new(
                    neighbor.x.rem_euclid(bounds.x),
                    neighbor.y.rem_euclid(bounds.y),
                );
                if *garden.get(neighbor_in_bounds.y as usize, neighbor_in_bounds.x as usize) == PLOT
                {
                    ending.push(neighbor);
                }
            }
        }
    }

    ending.sort();
    ending.dedup();

    std::mem::take(ending)
}

fn solve_case(input: &Input, num_steps: i32, infinite: bool) -> u64 {
    let mut positions = input.positions.clone();
    let mut ending = Positions::new();
    for _ in 0..num_steps {
        positions = move_one(&mut ending, &input.garden, &positions, infinite);
    }
    positions.len() as u64
}

fn main() {
    println!("Part 1");
    let example = parse("day21.example");
    aoc::expect_result!(16, solve_case(&example, 6, false));
    let input = parse("day21.input");
    aoc::expect_result!(3646, solve_case(&input, 64, false));

    println!("Part 2");
    aoc::expect_result!(16, solve_case(&example, 6, true));
    aoc::expect_result!(50, solve_case(&example, 10, true));
    aoc::expect_result!(1594, solve_case(&example, 50, true));
    aoc::expect_result!(6536, solve_case(&example, 100, true));
    aoc::return_incomplete();
    aoc::expect_result!(167004, solve_case(&example, 500, true));
    aoc::expect_result!(668697, solve_case(&example, 1000, true));
    aoc::expect_result!(16733044, solve_case(&example, 5000, true));
    aoc::expect_result!(1337, solve_case(&input, 26501365, true));
}
