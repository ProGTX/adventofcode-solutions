use aoc::dijkstra::DijkstraState;
use aoc::direction::{BASIC_DIRECTIONS, Direction};
use aoc::grid::Grid;
use aoc::point::{Point, distance_manhattan};

// Because the provided examples have a lot of blizzards,
// we represent them in a compressed way.
// Each position in a grid can store multiple blizzards -
// this is done by flipping bits of an integer.
// And because each blizzard can have one of 4 directions,
// two bits are needed per blizzard.
// We also count the number of blizzards in this position
// in the lowest bits -
// 2 bits for now to allow up to 3 blizzards per position.
type BlizzardGrid = Grid<u16>;
const BLIZ_COUNT_NUM_BITS: usize = 4;
const BLIZ_COUNT_MASK: u16 = (1 << BLIZ_COUNT_NUM_BITS) - 1;

type Pos = Point<i32>;
type Input = (Pos, BlizzardGrid);

fn parse(filename: &str) -> Input {
    let grid = Grid::<char>::from_file(filename);
    let dims = Pos::new(grid.num_columns as i32, grid.num_rows as i32);
    let mut blizzards = Grid::new(0u16, grid.num_rows, grid.num_columns);
    for row in 0..grid.num_rows {
        for col in 0..grid.num_columns {
            let dir = match grid.get(row, col) {
                '>' => Some(Direction::East),
                '<' => Some(Direction::West),
                '^' => Some(Direction::North),
                'v' => Some(Direction::South),
                _ => None,
            };
            if let Some(dir) = dir {
                *blizzards.get_mut(row, col) |= 1 << (BLIZ_COUNT_NUM_BITS + dir as usize);
            }
        }
    }
    (dims, blizzards)
}

fn move_blizzards(blizzards: &BlizzardGrid, dimensions: &Pos) -> BlizzardGrid {
    let inner = Pos::new(dimensions.x - 2, dimensions.y - 2);
    let mut result = Grid::new(0u16, blizzards.num_rows, blizzards.num_columns);
    for row in 0..blizzards.num_rows {
        for col in 0..blizzards.num_columns {
            let cell = *blizzards.get(row, col);
            if cell == 0 {
                continue;
            }
            for dir in BASIC_DIRECTIONS {
                let bit = 1u16 << (BLIZ_COUNT_NUM_BITS + dir as usize);
                if cell & bit != 0 {
                    let diff = dir.diff();
                    let new_col = (col as i32 - 1 + diff.x).rem_euclid(inner.x) + 1;
                    let new_row = (row as i32 - 1 + diff.y).rem_euclid(inner.y) + 1;
                    let new_cell = result.get_mut(new_row as usize, new_col as usize);
                    debug_assert!(
                        *new_cell & BLIZ_COUNT_MASK < BLIZ_COUNT_MASK,
                        "Too many blizzards at ({new_col}, {new_row})"
                    );
                    *new_cell += 1;
                    *new_cell |= bit;
                }
            }
        }
    }
    result
}

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
struct SearchState {
    pos: Pos,
    blizzards: BlizzardGrid,
}

fn find_distance(
    dimensions: &Pos,
    blizzards: &BlizzardGrid,
    start_pos: Pos,
    end_pos: Pos,
) -> (BlizzardGrid, u32) {
    let start = SearchState {
        pos: start_pos,
        blizzards: blizzards.clone(),
    };
    let distances = aoc::dijkstra::shortest_distances_astar(
        &start,
        |current| current.pos == end_pos,
        |current| {
            let mut neighbors = Vec::new();
            let new_blizzards = move_blizzards(&current.blizzards, dimensions);
            let on_blizzard =
                |pos: Point<i32>| *new_blizzards.get(pos.y as usize, pos.x as usize) > 0;
            // Consider all possible directions
            for dir in BASIC_DIRECTIONS {
                let new_pos = current.pos + dir.diff();
                // Position can go in the negative
                // when modifying the start/end position
                if ((new_pos != start_pos)
                    && (new_pos != end_pos)
                    && ((new_pos.x <= 0)
                        || (new_pos.y <= 0)
                        || (new_pos.x == dimensions.x - 1)
                        || (new_pos.y == dimensions.y - 1)))
                {
                    continue;
                }
                if (!current
                    .blizzards
                    .in_bounds_signed(new_pos.y as isize, new_pos.x as isize)
                    || on_blizzard(new_pos))
                {
                    continue;
                }
                neighbors.push(DijkstraState {
                    data: SearchState {
                        pos: new_pos,
                        blizzards: new_blizzards.clone(),
                    },
                    distance: 1,
                });
            }
            // Also consider the option of not moving,
            // just letting the blizzards move
            if !on_blizzard(current.pos) {
                neighbors.push(DijkstraState {
                    data: SearchState {
                        pos: current.pos,
                        blizzards: new_blizzards,
                    },
                    distance: 1,
                });
            }
            return neighbors;
        },
        |current| return distance_manhattan(current.pos, end_pos) as u32,
    );
    return distances
        .iter()
        .find(|(state, _)| state.pos == end_pos)
        .map(|(state, distance)| (state.blizzards.clone(), *distance))
        .unwrap();
}

fn solve_case1((dimensions, blizzards): &Input) -> u32 {
    let start_pos = Pos { x: 1, y: 0 };
    let end_pos = Pos {
        x: dimensions.x - 2,
        y: dimensions.y - 1,
    };
    find_distance(dimensions, blizzards, start_pos, end_pos).1
}

fn solve_case2((dimensions, blizzards): &Input) -> u32 {
    let start_pos = Pos { x: 1, y: 0 };
    let end_pos = Pos {
        x: dimensions.x - 2,
        y: dimensions.y - 1,
    };
    let (blizzards, d1) = find_distance(dimensions, blizzards, start_pos, end_pos);
    let (blizzards, d2) = find_distance(dimensions, &blizzards, end_pos, start_pos);
    let (_, d3) = find_distance(dimensions, &blizzards, start_pos, end_pos);
    d1 + d2 + d3
}

fn main() {
    println!("Part 1");
    let example = parse("day24.example");
    aoc::expect_result!(10, solve_case1(&example));
    let example2 = parse("day24.example2");
    aoc::expect_result!(18, solve_case1(&example2));
    let input = parse("day24.input");
    aoc::expect_result!(228, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(30, solve_case2(&example));
    aoc::expect_result!(54, solve_case2(&example2));
    aoc::expect_result!(723, solve_case2(&input));
}
