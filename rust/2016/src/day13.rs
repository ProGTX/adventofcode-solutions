use aoc::dijkstra::{DijkstraNeighborView, DijkstraState};
use aoc::grid::BASIC_NEIGHBOR_DIFFS;
use aoc::iter::CollectArrayVec;
use aoc::point::{Point, distance_manhattan};
use arrayvec::ArrayVec;

type UPos = Point<u32>;

const START: UPos = UPos::new(1, 1);

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

/// Anything further than `max_steps` away from the start
/// can never be reached in time,
/// which is what keeps a search of this endless maze finite
fn get_neighbors(current: UPos, favorite: u32, max_steps: u32) -> ArrayVec<DijkstraState<UPos>, 4> {
    return BASIC_NEIGHBOR_DIFFS
        .iter()
        // Only non-negative coordinates exist
        .filter_map(|diff| {
            Some(UPos::new(
                current.x.checked_add_signed(diff.x as i32)?,
                current.y.checked_add_signed(diff.y as i32)?,
            ))
        })
        .filter(|neighbor| {
            (distance_manhattan(*neighbor, START) <= max_steps) && is_space(*neighbor, favorite)
        })
        .dijkstra_uniform_neighbors()
        .collect_array_vec::<4>();
}

fn solve_case1((favorite, target): &Input) -> u32 {
    let distances = aoc::dijkstra::shortest_distances_astar(
        &START,
        |current| current == target,
        |current| get_neighbors(*current, *favorite, u32::MAX),
        |current| distance_manhattan(*current, *target),
    );
    return distances[target];
}

fn solve_case2((favorite, _): &Input) -> usize {
    const MAX_STEPS: u32 = 50;
    let distances = aoc::dijkstra::shortest_distances(
        &START,
        |_| false,
        |current| get_neighbors(*current, *favorite, MAX_STEPS),
    );
    // Every place the search settled, that is close enough to walk to
    return distances
        .values()
        .filter(|steps| **steps <= MAX_STEPS)
        .count();
}

fn main() {
    println!("Part 1");
    let example = parse("day13.example");
    aoc::expect_result!(11, solve_case1(&example));
    let input = parse("day13.input");
    aoc::expect_result!(86, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(151, solve_case2(&example));
    aoc::expect_result!(127, solve_case2(&input));
}
