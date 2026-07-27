use aoc::dijkstra::DijkstraNeighborView;
use aoc::grid::{ConfigInput, Grid, Upos};
use aoc::point::distance_manhattan;

const EMPTY: char = '.';
const WALL: char = '#';
const START: char = 'S';
const END: char = 'E';

struct Input {
    track: Grid<char>,
    start_pos: Upos,
    end_pos: Upos,
}

fn parse(filename: &str) -> Input {
    let (mut track, config) = Grid::<char>::from_file_config(
        filename,
        ConfigInput {
            padding: None,
            start_char: Some(START),
            end_char: Some(END),
        },
    );
    let start_pos = config.start_pos.unwrap();
    let end_pos = config.end_pos.unwrap();
    track.modify(EMPTY, start_pos.y, start_pos.x);
    track.modify(EMPTY, end_pos.y, end_pos.x);
    Input {
        track,
        start_pos,
        end_pos,
    }
}

fn solve_case<const TARGET_PS_SAVED: i32, const MAX_CHEAT_LENGTH: usize>(input: &Input) -> u32 {
    // Distance from `start_pos` to every track cell reachable from it
    // The track is a single, branch-free corridor, so this covers the whole track
    let distances = aoc::dijkstra::shortest_distances(
        &input.start_pos,
        |current| *current == input.end_pos,
        |current| {
            input
                .track
                .basic_neighbor_positions(*current)
                .into_iter()
                .filter(|neighbor| *input.track.get(neighbor.y, neighbor.x) != WALL)
                .dijkstra_uniform_neighbors()
        },
    );
    let track_cells: Vec<Upos> = distances.keys().copied().collect();

    let mut count = 0;
    for &cheat_start in &track_cells {
        for &cheat_end in &track_cells {
            let cheat_length = distance_manhattan(cheat_start, cheat_end);
            if !(2..=MAX_CHEAT_LENGTH).contains(&cheat_length) {
                continue;
            }
            let saved = (distances[&cheat_end] as i32)
                - (distances[&cheat_start] as i32)
                - (cheat_length as i32);
            if saved >= TARGET_PS_SAVED {
                count += 1;
            }
        }
    }
    count
}

fn main() {
    println!("Part 1");
    let example = parse("day20.example");
    aoc::expect_result!(5, solve_case::<20, 2>(&example));
    let input = parse("day20.input");
    aoc::expect_result!(1459, solve_case::<100, 2>(&input));

    println!("Part 2");
    aoc::expect_result!(285, solve_case::<50, 20>(&example));
    aoc::expect_result!(1016066, solve_case::<100, 20>(&input));
}
