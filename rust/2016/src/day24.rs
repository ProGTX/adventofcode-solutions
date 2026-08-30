use aoc::algorithm::longest_simple_path;
use aoc::dijkstra::{DijkstraNeighborView, DijkstraState};
use aoc::grid::{Grid, Upos};

/// The maze, and the locations of the numbered locations
type Input = (Grid<char>, Vec<Upos>);

fn parse(filename: &str) -> Input {
    let mut maze = Grid::<char>::from_file(filename);
    let mut numbers = Vec::<(u32, usize)>::new();
    for (index, tile) in maze.data.iter_mut().enumerate() {
        if let Some(number) = tile.to_digit(10) {
            numbers.push((number, index));
            // The numbers are just markers, the maze is open at their locations
            *tile = '.';
        }
    }
    numbers.sort_unstable();
    let locations = numbers
        .into_iter()
        .map(|(_, index)| maze.position(index))
        .collect();
    return (maze, locations);
}

/// The number of steps between every pair of numbered locations.
///
/// One search per location, over the open tiles of the maze,
/// which settles the distance to every other location at once.
fn location_distances((maze, locations): &Input) -> Grid<u32> {
    let num_locations = locations.len();
    let mut distances = Grid::new(0_u32, num_locations, num_locations);
    for (from_id, from_pos) in locations.iter().enumerate() {
        let steps = aoc::dijkstra::shortest_distances(
            from_pos,
            |_| false,
            |current| {
                maze.basic_neighbor_positions(*current)
                    .into_iter()
                    .filter(|neighbor| *maze.get(neighbor.y, neighbor.x) != '#')
                    .dijkstra_uniform_neighbors()
            },
        );
        for (to_id, to_pos) in locations.iter().enumerate() {
            distances.modify(steps[to_pos], from_id, to_id);
        }
    }
    return distances;
}

/// The fewest steps needed to visit every numbered location,
/// starting from location 0.
fn solve_case1(input: &Input) -> u32 {
    // Only the longest route is searched for directly.
    // Every complete route visits every location,
    // so it always has exactly `num_locations - 1` legs -
    // which means the shortest route is the longest one
    // over the complemented weights (max_steps - steps),
    // and its real length is `(num_locations - 1) * max_steps`
    // minus what's found.
    let distances = location_distances(input);
    let num_locations = distances.num_rows;
    let max_steps = *distances.data.iter().max().unwrap();
    let distances = &distances;
    let longest = longest_simple_path(
        num_locations,
        0,
        |_location, num_visited| num_visited == num_locations,
        |location| {
            (0..num_locations).map(move |to_id| DijkstraState {
                data: to_id,
                distance: max_steps - *distances.get(location, to_id),
            })
        },
    )
    .unwrap();
    return (num_locations as u32 - 1) * max_steps - longest;
}

fn main() {
    println!("Part 1");
    let example = parse("day24.example");
    aoc::expect_result!(14, solve_case1(&example));
    let input = parse("day24.input");
    aoc::expect_result!(500, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
