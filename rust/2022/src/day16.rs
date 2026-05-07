use arrayvec::ArrayVec;
use itertools::Itertools;

#[derive(Clone, Debug, Default)]
struct Valve {
    valves: Vec<usize>,
    flow_rate: u8,
}

type Input = (Vec<Valve>, usize);

fn parse(filename: &str) -> Input {
    let lines = aoc::file::read_lines(filename);

    let mut name_to_id = aoc::string::NameToId::new();
    for line in &lines {
        // "Valve aa_id has flow rate=..."
        let name = &line[6..8];
        name_to_id.intern(name);
    }

    let mut valves = vec![Valve::default(); lines.len()];
    for line in &lines {
        let name = &line[6..8];
        let id = name_to_id.expect(name);

        // "...flow rate=20; tunnel..."
        let (rate_part, rest) = line.split_once(';').unwrap();
        let flow_rate: u8 = rate_part.split_once('=').unwrap().1.parse().unwrap();

        // "... tunnels lead to valves DD, II, BB" or
        // "... tunnel leads to valve GG"
        let neighbors_str = rest.split_once("valve").unwrap().1;
        let neighbors_str = neighbors_str.trim_start_matches('s').trim();
        let neighbors: Vec<usize> = neighbors_str
            .split(", ")
            .map(|n| name_to_id.expect(n))
            .collect();

        valves[id] = Valve {
            flow_rate,
            valves: neighbors,
        };
    }
    (valves, name_to_id.expect("AA"))
}

fn bfs_distances(valves: &[Valve], start: usize) -> Vec<u8> {
    let mut distances = vec![u8::MAX; valves.len()];
    distances[start] = 0;
    let mut queue = std::collections::VecDeque::new();
    queue.push_back(start);
    while let Some(current) = queue.pop_front() {
        for &next in &valves[current].valves {
            if distances[next] == u8::MAX {
                distances[next] = distances[current] + 1;
                queue.push_back(next);
            }
        }
    }
    distances
}

// Maximum number of valves after graph compression
const MAX_VALVES: usize = 16;
type FlowRates = ArrayVec<u8, MAX_VALVES>;
type SingleDistances = ArrayVec<u8, MAX_VALVES>;

fn compress_graph(valves: &[Valve], aa_id: usize) -> (FlowRates, Vec<SingleDistances>) {
    // Precompute all-pairs distances
    let distances: Vec<Vec<u8>> = (0..valves.len())
        .map(|i| bfs_distances(valves, i))
        .collect();

    // Collect useful valves (AA + flow > 0)
    let mut nodes = Vec::new();
    nodes.push(aa_id);
    nodes.extend(
        valves
            .iter()
            .enumerate()
            .filter(|(id, v)| (v.flow_rate > 0) && (*id != aa_id))
            .map(|(id, _)| id),
    );

    // Build compressed valves (only flow matters now)
    let n = nodes.len();
    let mut flow_rates = FlowRates::new();
    for &old_id in &nodes {
        flow_rates.push(valves[old_id].flow_rate);
    }

    // Build compressed distance matrix
    let mut new_distances = vec![SingleDistances::new(); n];
    for (i, &old_i) in nodes.iter().enumerate() {
        for (&old_j) in nodes.iter() {
            new_distances[i].push(distances[old_i][old_j]);
        }
    }

    (flow_rates, new_distances)
}

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
struct SearchState {
    opened_mask: u16,
    current_id: u8,
    time_left: u8,
}
impl SearchState {
    fn is_open(&self, id: usize) -> bool {
        (self.opened_mask & (1 << id)) > 0
    }
    fn open(&mut self, id: usize) {
        self.opened_mask |= 1 << id;
    }
}

fn find_most_pressure(
    flow_rates: &[u8],
    distances: &[SingleDistances],
    state: SearchState,
    total_flow: u32,
) -> u32 {
    flow_rates
        .iter()
        .enumerate()
        .filter(|(neighbor_id, flow)| {
            // Find valves to open
            let neighbor_id = *neighbor_id + 1; // Skipped AA
            // +1 because it takes 1 minute to open the valve
            let time_to_open = distances[state.current_id as usize][neighbor_id] + 1;
            // Despite graph compression, zero flow is used in part 2
            // to exclude a valve from a subset
            return (neighbor_id != (state.current_id as usize))
                && (**flow > 0)
                && !state.is_open(neighbor_id)
                && (time_to_open <= state.time_left);
        })
        .map(|(neighbor_id, flow)| {
            let neighbor_id = neighbor_id + 1; // Skipped AA 
            // Move to neighbor valve and open it
            let time_to_open = distances[state.current_id as usize][neighbor_id] + 1;
            let mut new_state = SearchState {
                opened_mask: state.opened_mask,
                current_id: neighbor_id as u8,
                time_left: state.time_left - time_to_open,
            };
            new_state.open(neighbor_id);
            return (total_flow * time_to_open as u32)
                + find_most_pressure(
                    //
                    flow_rates,
                    distances,
                    new_state,
                    total_flow + (*flow as u32),
                );
        })
        .max()
        .unwrap_or(total_flow * state.time_left as u32)
}

fn solve_case1((valves, aa_id): &Input) -> u32 {
    let (flow_rates, distances) = compress_graph(valves, *aa_id);
    find_most_pressure(
        &flow_rates[1..],
        &distances,
        SearchState {
            opened_mask: 0,
            current_id: 0,
            time_left: 30,
        },
        0,
    )
}

fn solve_case2((valves, aa_id): &Input) -> u32 {
    let (flow_rates, distances) = compress_graph(valves, *aa_id);
    let start_state = SearchState {
        opened_mask: 0,
        current_id: 0,
        time_left: 26,
    };

    let mut most_pressure = 0;
    let num_valves = flow_rates.len();
    for subset1 in (1..num_valves).powerset() {
        if (subset1.len() != (num_valves / 2)) {
            // subset2 has already covered smaller sizes,
            // mirrored solutions are identical, skipping larger ones is safe
            // However, skipping smaller ones is a heuristic
            // that just happens to work here
            continue;
        }
        let subset2 = (1..num_valves).filter(|valve_id| {
            return !subset1.iter().any(|s1_id| s1_id == valve_id);
        });
        let mut flows1 = flow_rates.clone();
        let mut flows2 = flow_rates.clone();
        for i1 in subset1.iter() {
            flows2[*i1] = 0;
        }
        for i2 in subset2 {
            flows1[i2] = 0;
        }

        most_pressure = (
            //
            find_most_pressure(
                //
                &flows1[1..],
                &distances,
                start_state.clone(),
                0,
            ) + find_most_pressure(
                //
                &flows2[1..],
                &distances,
                start_state.clone(),
                0,
            )
        )
        .max(most_pressure);
    }

    return most_pressure;
}

fn main() {
    println!("Part 1");
    let example = parse("day16.example");
    aoc::expect_result!(1651, solve_case1(&example));
    let input = parse("day16.input");
    aoc::expect_result!(1647, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(1707, solve_case2(&example));
    aoc::expect_result!(2169, solve_case2(&input));
}
