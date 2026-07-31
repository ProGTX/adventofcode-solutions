use aoc::algorithm::{critical_distances, longest_simple_path};
use aoc::dijkstra::{DijkstraNeighborView, DijkstraState};
use aoc::grid::{BASIC_NEIGHBOR_DIFFS, Grid, Ipos};
use arrayvec::ArrayVec;

type Island = Grid<char>;

// Contracted island: only junctions (plus the start and the end) are nodes,
// the corridors between them become edges weighted by their length.
type Graph = Vec<Vec<(usize, u32)>>;

const NO_NODE: usize = usize::MAX;

fn parse(filename: &str) -> Island {
    Island::from_file(filename)
}

fn start_pos() -> Ipos {
    Ipos::new(1, 0)
}

fn end_pos(island_map: &Island) -> Ipos {
    Ipos::new(
        (island_map.num_columns - 2) as isize,
        (island_map.num_rows - 1) as isize,
    )
}

fn walkable_neighbors<const ALLOW_SLOPE: bool>(
    island_map: &Island,
    pos: Ipos,
) -> ArrayVec<Ipos, 4> {
    let slope = |neighbor| {
        if (!ALLOW_SLOPE) {
            // Slopes may only be entered in their direction in part 1,
            // which prevents cycles and keeps the graph a DAG.
            None
        } else {
            // Slopes are walkable in any direction in part 2
            Some(neighbor)
        }
    };

    BASIC_NEIGHBOR_DIFFS
        .iter()
        .filter_map(|&diff| {
            let neighbor = pos + diff;
            if !island_map.in_bounds_signed(neighbor.y, neighbor.x) {
                return None;
            }
            match *island_map.get(neighbor.y as usize, neighbor.x as usize) {
                '#' => None,
                '>' if diff != Ipos::new(1, 0) => slope(neighbor),
                '<' if diff != Ipos::new(-1, 0) => slope(neighbor),
                '^' if diff != Ipos::new(0, -1) => slope(neighbor),
                'v' if diff != Ipos::new(0, 1) => slope(neighbor),
                _ => Some(neighbor),
            }
        })
        .collect()
}

fn solve_case1(island_map: &Island) -> u32 {
    let start = start_pos();
    let end = end_pos(island_map);

    let distances = critical_distances(
        &start,
        |pos| *pos == end,
        |pos| {
            return walkable_neighbors::<false>(island_map, *pos)
                .into_iter()
                .dijkstra_uniform_neighbors();
        },
    );

    *distances.get(&end).unwrap()
}

// Follows the corridor leaving a node through `first_step` until another node is reached,
// returning that node and the number of steps taken to get there.
// Returns None for a dead end corridor, which carries no useful edge.
fn follow_corridor(
    island_map: &Island,
    node_ids: &[usize],
    node: Ipos,
    first_step: Ipos,
) -> Option<(usize, u32)> {
    let mut previous = node;
    let mut current = first_step;
    let mut distance = 1;
    // Corridor tiles have exactly two neighbors, so there is never a choice to make:
    // just keep walking away from where we came from.
    while node_ids[island_map.linear_index(current.y as usize, current.x as usize)] == NO_NODE {
        let next = walkable_neighbors::<true>(island_map, current)
            .into_iter()
            .find(|&neighbor| neighbor != previous)?;
        previous = current;
        current = next;
        distance += 1;
    }
    let id = node_ids[island_map.linear_index(current.y as usize, current.x as usize)];
    Some((id, distance))
}

// Returns the contracted graph plus the ids of the start and the end node.
fn contract(island_map: &Island) -> (Graph, usize, usize) {
    let start = start_pos();
    let end = end_pos(island_map);

    let mut node_ids = vec![NO_NODE; island_map.num_rows * island_map.num_columns];
    let mut nodes = Vec::new();
    for row in 0..island_map.num_rows {
        for column in 0..island_map.num_columns {
            if *island_map.get(row, column) == '#' {
                continue;
            }
            let pos = Ipos::new(column as isize, row as isize);
            let is_junction = walkable_neighbors::<true>(island_map, pos).len() > 2;
            if is_junction || pos == start || pos == end {
                node_ids[island_map.linear_index(row, column)] = nodes.len();
                nodes.push(pos);
            }
        }
    }

    let mut graph = vec![Vec::new(); nodes.len()];
    for (id, &node) in nodes.iter().enumerate() {
        for first_step in walkable_neighbors::<true>(island_map, node) {
            if let Some(edge) = follow_corridor(island_map, &node_ids, node, first_step) {
                graph[id].push(edge);
            }
        }
    }

    let start_id = node_ids[island_map.linear_index(start.y as usize, start.x as usize)];
    let end_id = node_ids[island_map.linear_index(end.y as usize, end.x as usize)];
    (graph, start_id, end_id)
}

fn solve_case2(island_map: &Island) -> u32 {
    let (graph, start, end) = contract(island_map);
    longest_simple_path(
        graph.len(),
        start,
        |node, _num_visited| node == end,
        |node| {
            graph[node].iter().map(|&(next, distance)| DijkstraState {
                data: next,
                distance,
            })
        },
    )
    .unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day23.example");
    aoc::expect_result!(94, solve_case1(&example));
    let input = parse("day23.input");
    aoc::expect_result!(2326, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(154, solve_case2(&example));
    aoc::expect_result!(6574, solve_case2(&input));
}
