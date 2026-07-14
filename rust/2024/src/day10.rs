use aoc::dijkstra::DijkstraNeighborView;
use aoc::grid::{BASIC_NEIGHBOR_DIFFS, ConfigInput, Grid, Ipos};
use aoc::iter::CollectArrayVec;
use rustc_hash::FxHashMap;

const TRAILHEAD_CHAR: char = '0';
const EDGE_CHAR: char = '.';
// Highest value on the map is 9, so 11 can never be reached
const EDGE: u8 = 11;
const TRAIL_END: u8 = 9;

struct Input {
    top_map: Grid<u8>,
    trailheads: Vec<Ipos>,
}

fn parse(filename: &str) -> Input {
    let (char_map, _) = Grid::<char>::from_file_config(
        filename,
        ConfigInput {
            padding: Some(EDGE_CHAR),
            start_char: None,
            end_char: None,
        },
    );

    let trailheads: Vec<Ipos> = char_map
        .data
        .iter()
        .enumerate()
        .filter(|&(_, &c)| c == TRAILHEAD_CHAR)
        .map(|(i, _)| {
            let upos = char_map.position(i);
            Ipos::new(upos.x as isize, upos.y as isize)
        })
        .collect();

    let heights: Vec<u8> = char_map
        .data
        .iter()
        .map(|&c| {
            if c == EDGE_CHAR {
                EDGE
            } else {
                c.to_digit(10).unwrap() as u8
            }
        })
        .collect();

    let top_map = Grid::from_vec(heights, char_map.num_rows, char_map.num_columns);

    Input {
        top_map,
        trailheads,
    }
}

// Height strictly increases by 1 along every edge,
// so the graph is a DAG and every path to a given cell has the same length -
// reachability and "shortest distance" coincide,
// so we can use Dijkstra to explore the whole trail from the trailhead
fn get_score(top_map: &Grid<u8>, trailhead: Ipos) -> usize {
    let distances = aoc::dijkstra::shortest_distances(
        &trailhead,
        |_current: &Ipos| false,
        |&current: &Ipos| {
            let current_height = *top_map.get(current.y as usize, current.x as usize);
            BASIC_NEIGHBOR_DIFFS
                .iter()
                .map(move |&diff| current + diff)
                .filter(move |neighbor| {
                    *top_map.get(neighbor.y as usize, neighbor.x as usize) == current_height + 1
                })
                .dijkstra_uniform_neighbors()
        },
    );

    distances
        .keys()
        .filter(|&&pos| *top_map.get(pos.y as usize, pos.x as usize) == TRAIL_END)
        .count()
}

fn solve_case1(input: &Input) -> usize {
    input
        .trailheads
        .iter()
        .map(|&trailhead| get_score(&input.top_map, trailhead))
        .sum()
}

#[derive(Clone, PartialEq, Eq, Hash)]
struct SearchState {
    pos: Ipos,
    trail: Vec<Ipos>,
}

fn get_rating(top_map: &Grid<u8>, trailhead: Ipos) -> u64 {
    let start = SearchState {
        pos: trailhead,
        trail: Vec::new(),
    };
    let num_trails: FxHashMap<SearchState, u64> = aoc::algorithm::dfs_uniform(
        start.clone(),
        |state: &SearchState| *top_map.get(state.pos.y as usize, state.pos.x as usize) == TRAIL_END,
        |state: &SearchState| {
            let current_height = *top_map.get(state.pos.y as usize, state.pos.x as usize);
            BASIC_NEIGHBOR_DIFFS
                .iter()
                .map(|&diff| state.pos + diff)
                .filter(|&neighbor| {
                    *top_map.get(neighbor.y as usize, neighbor.x as usize) == current_height + 1
                })
                .map(|neighbor| {
                    let mut new_state = state.clone();
                    new_state.pos = neighbor;
                    new_state.trail.push(state.pos);
                    new_state
                })
                .collect_array_vec::<4>()
        },
    );
    num_trails[&start]
}

fn solve_case2(input: &Input) -> u64 {
    input
        .trailheads
        .iter()
        .map(|&trailhead| get_rating(&input.top_map, trailhead))
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day10.example");
    aoc::expect_result!(1, solve_case1(&example));
    let example2 = parse("day10.example2");
    aoc::expect_result!(36, solve_case1(&example2));
    let input = parse("day10.input");
    aoc::expect_result!(652, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(81, solve_case2(&example2));
    aoc::expect_result!(1432, solve_case2(&input));
}
