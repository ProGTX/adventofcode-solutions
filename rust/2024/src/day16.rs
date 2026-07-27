use aoc::dijkstra::DijkstraState;
use aoc::direction::{Arrow, Direction};
use aoc::grid::{ConfigInput, Grid};
use aoc::point::Point;
use arrayvec::ArrayVec;
use rustc_hash::{FxHashMap, FxHashSet};

const EMPTY: char = '.';
const START: char = 'S';
const END: char = 'E';

type Pos = Point<i32>;
type Maze = Grid<char>;
type Distances = FxHashMap<Arrow, u32>;
type Predecessors = FxHashMap<Arrow, Arrow>;

struct Input {
    maze: Maze,
    start_pos: Pos,
    end_pos: Pos,
}

fn parse(filename: &str) -> Input {
    let (maze, config) = Grid::<char>::from_file_config(
        filename,
        ConfigInput {
            padding: None,
            start_char: Some(START),
            end_char: Some(END),
        },
    );
    let start_pos = config.start_pos.unwrap();
    let end_pos = config.end_pos.unwrap();
    Input {
        maze,
        start_pos: Pos::new(start_pos.x as i32, start_pos.y as i32),
        end_pos: Pos::new(end_pos.x as i32, end_pos.y as i32),
    }
}

fn get_neighbors(maze: &Maze, current: &Arrow) -> ArrayVec<DijkstraState<Arrow>, 3> {
    let mut neighbors = ArrayVec::new();

    {
        // Try going forward by 1
        let pos = current.pos + current.dir.diff();
        // No need for bounds checking because there are walls all around
        let value = *maze.get(pos.y as usize, pos.x as usize);
        if (value == EMPTY) || (value == END) {
            neighbors.push(DijkstraState {
                data: Arrow {
                    pos,
                    dir: current.dir,
                },
                distance: 1,
            });
        }
    }

    for dir in [current.dir.clockwise(), current.dir.counterclockwise()] {
        neighbors.push(DijkstraState {
            data: Arrow {
                pos: current.pos,
                dir,
            },
            distance: 1000,
        });
    }

    neighbors
}

fn start_arrow(start_pos: Pos) -> Arrow {
    Arrow {
        pos: start_pos,
        dir: Direction::East,
    }
}

fn end_arrows(end_pos: Pos) -> [Arrow; 2] {
    [Direction::East, Direction::North].map(|dir| Arrow { pos: end_pos, dir })
}

fn best_end_distance(distances: &Distances, end_pos: Pos) -> Option<u32> {
    end_arrows(end_pos)
        .iter()
        .filter_map(|end_arrow| distances.get(end_arrow).copied())
        .min()
}

// lowest_score
fn solve_case1(input: &Input) -> u32 {
    let distances = aoc::dijkstra::shortest_distances(
        &start_arrow(input.start_pos),
        |current| current.pos == input.end_pos,
        |current| get_neighbors(&input.maze, current),
    );

    best_end_distance(&distances, input.end_pos).unwrap()
}

fn try_explore_neighbor(
    maze: &Maze,
    distances: &Distances,
    predecessors: &Predecessors,
    tiles: &mut FxHashSet<Pos>,
    neighbor_arrow: Arrow,
    pred_arrow: Arrow,
    pred_distance: Option<u32>,
) {
    if (neighbor_arrow == pred_arrow) {
        return;
    }
    // The score can't have been reached by this neighbor if the subtraction underflowed
    let Some(pred_distance) = pred_distance else {
        return;
    };
    if (distances.get(&neighbor_arrow) == Some(&pred_distance)) {
        add_tiles(maze, distances, predecessors, tiles, neighbor_arrow);
    }
}

fn add_tiles(
    maze: &Maze,
    distances: &Distances,
    predecessors: &Predecessors,
    tiles: &mut FxHashSet<Pos>,
    mut current_arrow: Arrow,
) {
    loop {
        let current_pos = current_arrow.pos;
        tiles.insert(current_pos);
        let Some(&pred_arrow) = predecessors.get(&current_arrow) else {
            return;
        };
        let current_distance = distances[&current_arrow];
        // Explore other potential predecessors that have the same score
        // This is done in reverse of the original neighbor search

        {
            let neighbor_pos = current_pos - current_arrow.dir.diff();
            // Note we don't care for checking the starting node here
            // because that's guaranteed to be on the shortest path
            if (*maze.get(neighbor_pos.y as usize, neighbor_pos.x as usize) == EMPTY) {
                try_explore_neighbor(
                    maze,
                    distances,
                    predecessors,
                    tiles,
                    Arrow {
                        pos: neighbor_pos,
                        dir: current_arrow.dir,
                    },
                    pred_arrow,
                    current_distance.checked_sub(1),
                );
            }
        }
        for dir in [
            current_arrow.dir.counterclockwise(),
            current_arrow.dir.clockwise(),
        ] {
            try_explore_neighbor(
                maze,
                distances,
                predecessors,
                tiles,
                Arrow {
                    pos: current_pos,
                    dir,
                },
                pred_arrow,
                current_distance.checked_sub(1000),
            );
        }

        current_arrow = pred_arrow;
    }
}

// tiles_on_best_paths
fn solve_case2(input: &Input) -> u32 {
    // In this case we don't want to terminate the search
    // when finding the best path, so we never report the end
    let mut predecessors = Predecessors::default();
    let distances = aoc::dijkstra::shortest_distances_with_predecessors(
        &start_arrow(input.start_pos),
        |_| false,
        |current| get_neighbors(&input.maze, current),
        &mut predecessors,
    );

    // Only arrows that actually reach the end with the best score
    // are on a best path
    // Arriving facing another way can cost a whole extra turn
    // and tracing that back would pull in tiles
    // that are only ever on a suboptimal route
    let best = best_end_distance(&distances, input.end_pos).unwrap();

    let mut tiles = FxHashSet::default();
    for end_arrow in end_arrows(input.end_pos) {
        if (distances.get(&end_arrow) == Some(&best)) {
            add_tiles(
                &input.maze,
                &distances,
                &predecessors,
                &mut tiles,
                end_arrow,
            );
        }
    }
    tiles.len() as u32
}

fn main() {
    println!("Part 1");
    let example = parse("day16.example");
    aoc::expect_result!(7036, solve_case1(&example));
    let example2 = parse("day16.example2");
    aoc::expect_result!(11048, solve_case1(&example2));
    // https://www.reddit.com/r/adventofcode/comments/1hfhgl1/2024_day_16_part_1_alternate_test_case/
    let example3 = parse("day16.example3");
    aoc::expect_result!(21148, solve_case1(&example3));
    // https://www.reddit.com/r/adventofcode/comments/1hgyuqm/2024_day_16_part_1/
    let example4 = parse("day16.example4");
    aoc::expect_result!(5027, solve_case1(&example4));
    let input = parse("day16.input");
    aoc::expect_result!(94436, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(45, solve_case2(&example));
    aoc::expect_result!(64, solve_case2(&example2));
    aoc::expect_result!(481, solve_case2(&input));
}
