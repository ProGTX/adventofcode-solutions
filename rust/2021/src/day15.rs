use aoc::dijkstra::DijkstraState;
use aoc::grid::Grid;
use aoc::grid::Upos;
use aoc::iter::CollectArrayVec;
use arrayvec::ArrayVec;

type Input = Grid<u32>;

fn parse(filename: &str) -> Input {
    aoc::grid::from_file(filename)
}

fn solve_case1(grid: &Input) -> u32 {
    let start = Upos { y: 0, x: 0 };
    let end = Upos {
        y: grid.num_rows - 1,
        x: grid.num_columns - 1,
    };
    let distances = aoc::dijkstra::shortest_distances(
        &start,
        |current: &Upos| *current == end,
        |current: &Upos| {
            grid.basic_neighbor_positions(*current)
                .iter()
                .map(|&pos| DijkstraState {
                    distance: *grid.get(pos.y, pos.x),
                    data: pos,
                })
                .collect_array_vec::<4>()
        },
    );
    return distances[&end];
}

fn virtual_neighbors(pos: Upos, rows: usize, cols: usize) -> ArrayVec<Upos, 4> {
    let mut neighbors = ArrayVec::new();
    if pos.y > 0 {
        neighbors.push(Upos {
            y: pos.y - 1,
            x: pos.x,
        });
    }
    if pos.x > 0 {
        neighbors.push(Upos {
            y: pos.y,
            x: pos.x - 1,
        });
    }
    if pos.y + 1 < rows {
        neighbors.push(Upos {
            y: pos.y + 1,
            x: pos.x,
        });
    }
    if pos.x + 1 < cols {
        neighbors.push(Upos {
            y: pos.y,
            x: pos.x + 1,
        });
    }
    neighbors
}

fn virtual_cost(grid: &Input, pos: Upos) -> u32 {
    let base = *grid.get(pos.y % grid.num_rows, pos.x % grid.num_columns);
    let offset = (pos.y / grid.num_rows + pos.x / grid.num_columns) as u32;
    (base + offset - 1) % 9 + 1
}

fn solve_case2(grid: &Input) -> u32 {
    let rows = grid.num_rows * 5;
    let cols = grid.num_columns * 5;
    let start = Upos { y: 0, x: 0 };
    let end = Upos {
        y: rows - 1,
        x: cols - 1,
    };
    let distances = aoc::dijkstra::shortest_distances(
        &start,
        |current: &Upos| *current == end,
        |current: &Upos| {
            virtual_neighbors(*current, rows, cols)
                .iter()
                .map(|&pos| DijkstraState {
                    distance: virtual_cost(grid, pos),
                    data: pos,
                })
                .collect_array_vec::<4>()
        },
    );
    return distances[&end];
}

fn main() {
    println!("Part 1");
    let example = parse("day15.example");
    assert_eq!(40, solve_case1(&example));
    let input = parse("day15.input");
    assert_eq!(592, solve_case1(&input));

    println!("Part 2");
    assert_eq!(315, solve_case2(&example));
    assert_eq!(2897, solve_case2(&input));
}
