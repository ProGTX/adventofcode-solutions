use aoc::dijkstra::{DijkstraState, longest_distances};
use aoc::grid::{BASIC_NEIGHBOR_DIFFS, Grid, Ipos};

type Island = Grid<char>;

fn parse(filename: &str) -> Island {
    Island::from_file(filename)
}

fn solve_case1(island_map: &Island) -> u32 {
    let start = Ipos::new(1, 0);
    let end = Ipos::new(
        (island_map.num_columns - 2) as isize,
        (island_map.num_rows - 1) as isize,
    );

    let distances = longest_distances(
        &start,
        |pos| *pos == end,
        |pos| {
            BASIC_NEIGHBOR_DIFFS
                .iter()
                .filter_map(|&diff| {
                    let neighbor = *pos + diff;
                    if !island_map.in_bounds_signed(neighbor.y, neighbor.x) {
                        return None;
                    }
                    // Slopes may only be entered in their direction,
                    // which prevents cycles and keeps the graph a DAG.
                    match *island_map.get(neighbor.y as usize, neighbor.x as usize) {
                        '#' => None,
                        '>' if diff != Ipos::new(1, 0) => None,
                        '<' if diff != Ipos::new(-1, 0) => None,
                        '^' if diff != Ipos::new(0, -1) => None,
                        'v' if diff != Ipos::new(0, 1) => None,
                        _ => Some(DijkstraState {
                            data: neighbor,
                            distance: 1,
                        }),
                    }
                })
                .collect::<Vec<_>>()
        },
    );

    *distances.get(&end).unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day23.example");
    aoc::expect_result!(94, solve_case1(&example));
    let input = parse("day23.input");
    aoc::expect_result!(2326, solve_case1(&input));

    aoc::return_incomplete();
}
