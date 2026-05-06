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
        // "Valve AA has flow rate=..."
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

        // "... tunnels lead to valves DD, II, BB"  or  "... tunnel leads to valve GG"
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
    let n = valves.len();
    let mut dist = vec![u32::MAX; n];
    dist[start] = 0;
    let mut queue = std::collections::VecDeque::new();
    queue.push_back(start);
    while let Some(cur) = queue.pop_front() {
        for &next in &valves[cur].valves {
            if dist[next] == u32::MAX {
                dist[next] = dist[cur] + 1;
                queue.push_back(next);
            }
        }
    }
    dist
}

#[derive(Clone, Debug)]
struct SearchState {
    current_id: usize,
    time_left: u32,
    opened: Vec<bool>,
}

fn find_most_pressure(
    valves: &[Valve],
    distances: &[Vec<u32>],
    state: SearchState,
    total_flow: u32,
    total_pressure: u32,
) -> u32 {
    valves
        .iter()
        .enumerate()
        .filter(|(neighbor_id, _)| {
            // Find valves to open
            let neighbor_id = *neighbor_id;
            // +1 because it takes 1 minute to open the valve
            let time_to_open = distances[state.current_id][neighbor_id] + 1;
            return (neighbor_id != state.current_id)
                && !state.opened[neighbor_id]
                && (valves[neighbor_id].flow_rate > 0)
                && (time_to_open <= state.time_left);
        })
        .map(|(neighbor_id, neighbor)| {
            // Move to neighbor valve and open it
            let time_to_open = distances[state.current_id][neighbor_id] + 1;
            find_most_pressure(
                valves,
                distances,
                SearchState {
                    current_id: neighbor_id,
                    time_left: state.time_left - time_to_open,
                    opened: {
                        let mut opened = state.opened.clone();
                        opened[neighbor_id] = true;
                        opened
                    },
                },
                total_flow + neighbor.flow_rate,
                total_pressure + (total_flow * time_to_open),
            )
        })
        .max()
        .unwrap_or(total_pressure + (total_flow * state.time_left))
}

fn solve_case1((valves, AA): &Input) -> u32 {
    let distances: Vec<Vec<u32>> = (0..valves.len())
        .map(|i| bfs_distances(valves, i))
        .collect();
    find_most_pressure(
        valves,
        &distances,
        SearchState {
            current_id: *AA,
            time_left: 30,
            opened: vec![false; valves.len()],
        },
        0,
        0,
    )
}

fn main() {
    println!("Part 1");
    let example = parse("day16.example");
    aoc::expect_result!(1651, solve_case1(&example));
    let input = parse("day16.input");
    aoc::expect_result!(1647, solve_case1(&input));

    println!("Part 2");
}
