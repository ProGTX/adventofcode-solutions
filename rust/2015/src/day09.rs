use aoc::algorithm::longest_simple_path;
use aoc::dijkstra::DijkstraState;
use aoc::string::NameToId;
use rustc_hash::FxHashSet;

#[derive(Clone)]
struct LinkT {
    to_id: usize,
    distance: u32,
}

type ConnectionsT = Vec<Vec<LinkT>>;

fn parse(filename: &str) -> ConnectionsT {
    let mut name_to_id = NameToId::new();
    let mut connections = ConnectionsT::new();
    for line in std::fs::read_to_string(filename).unwrap().lines() {
        let parts = line.split(' ').collect::<Vec<_>>();
        let from_id = name_to_id.intern(parts[0]);
        let to_id = name_to_id.intern(parts[2]);
        connections.resize(name_to_id.new_len(connections.len()), Default::default());
        let distance = parts[4].parse::<u32>().unwrap();
        connections[from_id].push(LinkT {
            to_id: to_id,
            distance,
        });
        connections[to_id].push(LinkT {
            to_id: from_id,
            distance,
        });
    }
    connections
}

// Greedy nearest neighbor: from every starting city,
// repeatedly hop to the closest city not visited yet.
// This is a heuristic, not an exhaustive search -
// a cheap first hop can force an expensive tail
// and nothing here backs out of it -
// but it does find the optimum for this input.
fn shortest_distance(connections: ConnectionsT) -> u32 {
    let mut best = u32::MAX;
    let mut current_path = FxHashSet::default();
    for (from_id, place) in connections.iter().enumerate() {
        debug_assert!(
            place.is_sorted_by_key(|link| link.distance),
            "Destinations must be sorted by distance"
        );
        let mut current_link = place;
        current_path.clear();
        current_path.insert(from_id);
        let mut current_distance = 0u32;
        while current_path.len() < connections.len() {
            let it = current_link
                .iter()
                .find(|link: &&LinkT| !current_path.contains(&link.to_id));
            debug_assert!(it.is_some(), "Graph must be fully connected");
            let link = it.unwrap();
            current_path.insert(link.to_id);
            current_distance += link.distance;
            current_link = &connections[link.to_id];
        }
        if current_distance < best {
            best = current_distance;
        }
    }
    best
}

fn solve_case1(filename: &str) -> u32 {
    let mut connections = parse(filename);
    for place in &mut connections {
        place.sort_by(|lhs, rhs| lhs.distance.cmp(&rhs.distance));
    }
    shortest_distance(connections)
}

fn solve_case2(filename: &str) -> u32 {
    let connections = parse(filename);
    let num_places = connections.len();
    // Any city may be the start, and the route is finished once it has visited all.
    // Only 8 cities, so searching every route exhaustively is cheap.
    (0..num_places)
        .filter_map(|start| {
            longest_simple_path(
                num_places,
                start,
                |_place, num_visited| num_visited == num_places,
                |place| {
                    connections[place].iter().map(|link| DijkstraState {
                        data: link.to_id,
                        distance: link.distance,
                    })
                },
            )
        })
        .max()
        .unwrap()
}

fn main() {
    println!("Part 1");
    aoc::expect_result!(605, solve_case1("day09.example"));
    aoc::expect_result!(141, solve_case1("day09.input"));

    println!("Part 2");
    aoc::expect_result!(982, solve_case2("day09.example"));
    aoc::expect_result!(736, solve_case2("day09.input"));
}
