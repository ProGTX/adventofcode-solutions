use aoc::algorithm::longest_simple_path;
use aoc::dijkstra::DijkstraState;
use aoc::string::NameToId;
use arrayvec::ArrayVec;

#[derive(Clone)]
struct LinkT {
    to_id: usize,
    distance: u32,
}

// There are exactly this many connections
// from one city to all other cities in the input data
const MAX_LINKS: usize = 7;

type LinksT = ArrayVec<LinkT, MAX_LINKS>;
type ConnectionsT = Vec<LinksT>;

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

// Only the longest route is searched for directly.
// Every complete route visits every city,
// so it always has exactly `num_cities - 1` edges -
// which means the shortest route is the longest one
// over the complemented weights (max_distance - distance),
// and its real length is `(num_cities - 1) * max_distance` minus what's found.
fn solve_case<const LONGEST: bool>(connections: &ConnectionsT) -> u32 {
    let num_cities = connections.len();
    let max_distance = connections
        .iter()
        .flatten()
        .map(|link| link.distance)
        .max()
        .unwrap();
    // Any city may be the start, the route is finished once it has visited all.
    // Only 8 cities, so searching every route exhaustively is cheap.
    let best = (0..num_cities)
        .filter_map(|start| {
            longest_simple_path(
                num_cities,
                start,
                |_city, num_visited| num_visited == num_cities,
                |city| {
                    connections[city].iter().map(|link| DijkstraState {
                        data: link.to_id,
                        distance: if LONGEST {
                            link.distance
                        } else {
                            max_distance - link.distance
                        },
                    })
                },
            )
        })
        .max()
        .unwrap();
    if LONGEST {
        best
    } else {
        (num_cities as u32 - 1) * max_distance - best
    }
}

fn main() {
    println!("Part 1");
    let example = parse("day09.example");
    aoc::expect_result!(605, solve_case::<false>(&example));
    let input = parse("day09.input");
    aoc::expect_result!(141, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(982, solve_case::<true>(&example));
    aoc::expect_result!(736, solve_case::<true>(&input));
}
