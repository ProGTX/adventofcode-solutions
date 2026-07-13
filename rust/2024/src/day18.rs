use aoc::dijkstra::DijkstraNeighborView;
use aoc::grid::{Grid, Upos};

const CORRUPTED: char = '#';
const EMPTY: char = '.';

fn parse(filename: &str) -> Vec<Upos> {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| Upos::parse(line, ",").unwrap())
        .collect()
}

fn bytes_fall(mut memspace: Grid<char>, falling_bytes: &[Upos]) -> Grid<char> {
    for &byte_pos in falling_bytes {
        memspace.modify(CORRUPTED, byte_pos.y, byte_pos.x);
    }
    memspace
}

fn shortest_path(memspace: &Grid<char>) -> Option<u32> {
    let start_pos = Upos::new(0, 0);
    let end_pos = Upos::new(memspace.num_columns - 1, memspace.num_rows - 1);

    let distances = aoc::dijkstra::shortest_distances(
        &start_pos,
        |current: &Upos| *current == end_pos,
        |current: &Upos| {
            memspace
                .basic_neighbor_positions(*current)
                .into_iter()
                .filter(|neighbor| *memspace.get(neighbor.y, neighbor.x) != CORRUPTED)
                .dijkstra_uniform_neighbors()
        },
    );

    distances.get(&end_pos).copied()
}

fn solve_case1<const NUM_ROWS: usize, const NUM_COLUMNS: usize, const NUM_FALLEN: usize>(
    falling_bytes: &[Upos],
) -> u32 {
    let memspace = Grid::new(EMPTY, NUM_ROWS, NUM_COLUMNS);
    shortest_path(&bytes_fall(memspace, &falling_bytes[..NUM_FALLEN])).unwrap()
}

fn first_problematic_byte(
    memspace: &Grid<char>,
    falling_bytes: &[Upos],
    num_fallen: usize,
) -> Option<Upos> {
    // Adding corrupted cells can only ever block a path, never reopen one,
    // so whether a path exists is monotonic in the number of fallen bytes
    // That lets us binary search for the first blocking byte
    // instead of testing every byte one at a time
    let has_path_after = |count: usize| {
        shortest_path(&bytes_fall(memspace.clone(), &falling_bytes[..count])).is_some()
    };

    // We know from part 1 that num_fallen bytes don't cause a problem yet
    let mut low = num_fallen;
    let mut high = falling_bytes.len();
    if has_path_after(high) {
        return None;
    }
    while low + 1 < high {
        let mid = low + (high - low) / 2;
        if has_path_after(mid) {
            low = mid;
        } else {
            high = mid;
        }
    }

    Some(falling_bytes[high - 1])
}

fn solve_case2<const NUM_ROWS: usize, const NUM_COLUMNS: usize, const NUM_FALLEN: usize>(
    falling_bytes: &[Upos],
) -> Upos {
    let memspace = Grid::new(EMPTY, NUM_ROWS, NUM_COLUMNS);
    first_problematic_byte(&memspace, falling_bytes, NUM_FALLEN).unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day18.example");
    aoc::expect_result!(22, solve_case1::<7, 7, 12>(&example));
    let input = parse("day18.input");
    aoc::expect_result!(234, solve_case1::<71, 71, 1024>(&input));

    println!("Part 2");
    aoc::expect_result!(Upos::new(6, 1), solve_case2::<7, 7, 12>(&example));
    aoc::expect_result!(Upos::new(58, 19), solve_case2::<71, 71, 1024>(&input));
}
