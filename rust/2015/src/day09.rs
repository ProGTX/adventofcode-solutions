use std::collections::{HashMap, HashSet, hash_map::Entry};

#[derive(Clone)]
struct LinkT {
    to_id: usize,
    distance: u32,
}

type ConnectionsT = Vec<Vec<LinkT>>;

fn parse(filename: &str) -> ConnectionsT {
    let mut current_id = 0;
    let mut place_ids = HashMap::<String, usize>::new();
    let mut get_id = |place: &str| -> usize {
        match place_ids.entry(place.to_string()) {
            Entry::Vacant(e) => {
                let id = e.insert(current_id);
                current_id += 1;
                *id
            }
            Entry::Occupied(e) => *e.into_mut(),
        }
    };
    let mut connections = ConnectionsT::new();
    for line in std::fs::read_to_string(filename).unwrap().lines() {
        let parts = line.split(' ').collect::<Vec<_>>();
        let from_id = get_id(parts[0]);
        let to_id = get_id(parts[2]);
        connections.resize(
            connections
                .len()
                .max((from_id + 1) as usize)
                .max((to_id + 1) as usize),
            Default::default(),
        );
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

fn best_distance<const GREATER: bool>(connections: ConnectionsT) -> u32 {
    let mut best = if GREATER { 0 } else { u32::MAX };
    let mut current_path = HashSet::<usize>::new();
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
        if !GREATER {
            if current_distance < best {
                best = current_distance;
            }
        } else {
            if current_distance > best {
                best = current_distance;
            }
        }
    }
    best
}

fn solve_case<const GREATER: bool>(filename: &str) -> u32 {
    let mut connections = parse(filename);
    for place in &mut connections {
        if !GREATER {
            place.sort_by(|lhs, rhs| lhs.distance.cmp(&rhs.distance));
        } else {
            place.sort_by(|lhs, rhs| rhs.distance.cmp(&lhs.distance));
        }
    }
    best_distance::<GREATER>(connections)
}

fn main() {
    println!("Part 1");
    assert_eq!(605, solve_case::<false>("day09.example"));
    assert_eq!(141, solve_case::<false>("day09.input"));
    println!("Part 2");
    assert_eq!(982, solve_case::<true>("day09.example"));
    assert_eq!(736, solve_case::<true>("day09.input"));
}
