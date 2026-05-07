use std::collections::HashMap;

use itertools::Itertools;

#[derive(Clone, Debug, Default)]
struct Valve {
    flow_rate: u32,
    valves: Vec<usize>,
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
        let flow_rate: u32 = rate_part.split_once('=').unwrap().1.parse().unwrap();

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

fn bfs_distances(valves: &[Valve], start: usize) -> Vec<u32> {
    let mut distances = vec![u32::MAX; valves.len()];
    distances[start] = 0;
    let mut queue = std::collections::VecDeque::new();
    queue.push_back(start);
    while let Some(current) = queue.pop_front() {
        for &next in &valves[current].valves {
            if distances[next] == u32::MAX {
                distances[next] = distances[current] + 1;
                queue.push_back(next);
            }
        }
    }
    distances
}

fn compress_graph(valves: &[Valve], aa_id: usize) -> (Vec<u32>, Vec<Vec<u32>>) {
    // Precompute all-pairs distances
    let distances: Vec<Vec<u32>> = (0..valves.len())
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
    let mut flow_rates = Vec::with_capacity(n);
    for &old_id in &nodes {
        flow_rates.push(valves[old_id].flow_rate);
    }

    // Build compressed distance matrix
    let mut new_distances = vec![vec![0; n]; n];
    for (i, &old_i) in nodes.iter().enumerate() {
        for (j, &old_j) in nodes.iter().enumerate() {
            new_distances[i][j] = distances[old_i][old_j];
        }
    }

    (flow_rates, new_distances)
}

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
struct SearchState {
    current_id: usize,
    opened_mask: u32,
    time_left: u32,
}
impl SearchState {
    fn is_open(&self, id: usize) -> bool {
        (self.opened_mask & (1 << id)) > 0
    }
    fn open(&mut self, id: usize) {
        self.opened_mask |= 1 << id;
    }
}
type Cache = HashMap<SearchState, u32>;

fn find_most_pressure(
    flow_rates: &[u32],
    distances: &[Vec<u32>],
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
            let time_to_open = distances[state.current_id][neighbor_id] + 1;
            // Despite graph compression, zero flow is used in part 2
            // to exclude a valve from a subset
            return (neighbor_id != state.current_id)
                && (**flow > 0)
                && !state.is_open(neighbor_id)
                && (time_to_open <= state.time_left);
        })
        .map(|(neighbor_id, flow)| {
            let neighbor_id = neighbor_id + 1; // Skipped AA 
            // Move to neighbor valve and open it
            let time_to_open = distances[state.current_id][neighbor_id] + 1;
            let mut new_state = SearchState {
                current_id: neighbor_id,
                opened_mask: state.opened_mask,
                time_left: state.time_left - time_to_open,
            };
            new_state.open(neighbor_id);
            return (total_flow * time_to_open)
                + find_most_pressure(
                    //
                    flow_rates,
                    distances,
                    new_state,
                    total_flow + flow,
                );
        })
        .max()
        .unwrap_or(total_flow * state.time_left)
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
        current_id: 0,
        opened_mask: 0,
        time_left: 26,
    };

    let mut most_pressure = 0;
    let num_valves = flow_rates.len();
    for subset1 in (1..num_valves).powerset() {
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
