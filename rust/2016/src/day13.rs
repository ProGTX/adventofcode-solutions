use aoc::dijkstra::DijkstraNeighborView;
use aoc::grid::BASIC_NEIGHBOR_DIFFS;
use aoc::iter::CollectArrayVec;
use aoc::point::{Point, distance_manhattan};

type UPos = Point<u32>;

/// The office designer's favorite number, and the position to reach
type Input = (u32, UPos);

fn parse(filename: &str) -> Input {
    let favorite = aoc::file::read_string(filename)
        .trim()
        .parse::<u32>()
        .unwrap();
    // The example is a smaller maze, with a closer target
    let target = if (favorite == 10) {
        UPos::new(7, 4)
    } else {
        UPos::new(31, 39)
    };
    return (favorite, target);
}

/// Whether the position holds an open space rather than a wall
fn is_space(pos: UPos, favorite: u32) -> bool {
    let (x, y) = (pos.x, pos.y);
    let value = x * x + 3 * x + 2 * x * y + y + y * y + favorite;
    return (value.count_ones() % 2) == 0;
}

fn solve_case1((favorite, target): &Input) -> u32 {
    let start = UPos::new(1, 1);
    let distances = aoc::dijkstra::shortest_distances_astar(
        &start,
        |current| current == target,
        |current| {
            BASIC_NEIGHBOR_DIFFS
                .iter()
                // Only non-negative coordinates exist
                .filter_map(|diff| {
                    Some(UPos::new(
                        current.x.checked_add_signed(diff.x as i32)?,
                        current.y.checked_add_signed(diff.y as i32)?,
                    ))
                })
                .filter(|neighbor| is_space(*neighbor, *favorite))
                .dijkstra_uniform_neighbors()
                .collect_array_vec::<4>()
        },
        |current| distance_manhattan(*current, *target),
    );
    return distances[target];
}

fn main() {
    println!("Part 1");
    let example = parse("day13.example");
    aoc::expect_result!(11, solve_case1(&example));
    let input = parse("day13.input");
    aoc::expect_result!(86, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
