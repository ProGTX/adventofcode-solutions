use aoc::dijkstra::DijkstraNeighborView;
use aoc::grid::ConfigInput;
use aoc::grid::Grid;
use aoc::grid::Upos;
use aoc::iter::CollectArrayVec;
use std::ops::Range;

type Terminator = Range<Upos>;
type Input = (Grid<u8>, Terminator);

fn parse(filename: &str) -> Input {
    let (mut heightmap, config) = Grid::from_file_config(
        filename,
        ConfigInput {
            padding: None,
            start_char: Some('S'),
            end_char: Some('E'),
        },
    );
    // Replace begin and end with regular heights
    let terminator = Terminator {
        start: config.start_pos.unwrap(),
        end: config.end_pos.unwrap(),
    };
    heightmap.modify('a', terminator.start.y, terminator.start.x);
    heightmap.modify('z', terminator.end.y, terminator.end.x);
    return (
        Grid::from_iter(
            heightmap.data.iter().map(|&c| c as u8 - 'a' as u8),
            heightmap.num_rows,
            heightmap.num_columns,
        ),
        terminator,
    );
}

fn shortest_path(heightmap: &Grid<u8>, start: Upos, end: Upos) -> Option<u32> {
    let distances = aoc::dijkstra::shortest_distances(&start, &end, |current: &Upos| {
        let current_height = *heightmap.get(current.y, current.x);
        heightmap
            .basic_neighbor_positions(*current)
            .iter()
            .filter(|pos| {
                let neighbor = *heightmap.get(pos.y, pos.x);
                return neighbor <= (current_height + 1);
            })
            .cloned()
            .dijkstra_uniform_neighbors()
            .collect_array_vec::<4>()
    });
    return distances.get(&end).cloned();
}

fn solve_case1((heightmap, terminator): &Input) -> u32 {
    shortest_path(heightmap, terminator.start, terminator.end).unwrap()
}

fn solve_case2((heightmap, terminator): &Input) -> u32 {
    heightmap
        .data
        .iter()
        .enumerate()
        .filter(|(_, value)| **value == 0)
        .map(|(linear_index, _)| {
            shortest_path(heightmap, heightmap.position(linear_index), terminator.end)
                .unwrap_or(heightmap.data.len() as u32)
        })
        .min()
        .unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day12.example");
    assert_eq!(31, solve_case1(&example));
    let input = parse("day12.input");
    assert_eq!(504, solve_case1(&input));

    println!("Part 2");
    assert_eq!(29, solve_case2(&example));
    assert_eq!(500, solve_case2(&input));
}
