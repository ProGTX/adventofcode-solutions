use aoc::dijkstra::DijkstraState;

// Stored as bytes rather than chars
// The whole state is then 27 bytes instead of 108,
// which makes hashing and copying far cheaper
type Rooms = [[u8; 4]; 4];
type Hallway = [u8; 11];

const fn hallway_index(room_index: usize) -> usize {
    room_index * 2 + 2
}

fn parse(filename: &str) -> Rooms {
    let lines = aoc::file::read_lines(filename);
    let top = lines[2].as_bytes();
    let bottom = lines[3].as_bytes();
    std::array::from_fn(|room_index| {
        // +1 for the leading # in the file line
        let col = hallway_index(room_index) + 1;
        [bottom[col], top[col], EMPTY, EMPTY]
    })
}

const HALLWAY: Hallway = *b"..X.X.X.X..";
const EMPTY: u8 = HALLWAY[0];
const FORBIDDEN: u8 = HALLWAY[2];

const fn numeric(c: u8) -> u8 {
    c - b'A'
}

const fn get_cost(c: u8) -> u32 {
    10_u32.pow(numeric(c) as u32)
}

const fn correct_room(c: u8, room_index: usize) -> bool {
    (numeric(c) as usize) == room_index
}

fn hallway_path(hallway: &Hallway, hall_index: usize, room_index: usize) -> Option<(usize, usize)> {
    let hall_above_room = hallway_index(room_index);
    let (from, to) = (
        hall_above_room.min(hall_index),
        hall_above_room.max(hall_index),
    );
    if (hallway[from..=to].iter().enumerate().any(|(i, c)| {
        return ((from + i) != hall_index) && (b'A'..=b'D').contains(c);
    })) {
        // There is another amphipod in the way
        return None;
    }
    return Some((from, to));
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
struct Configuration {
    hallway: Hallway,
    rooms: Rooms,
}
impl std::hash::Hash for Configuration {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        // The derived impl feeds the rooms to the hasher one row at a time
        state.write(&self.hallway);
        state.write(self.rooms.as_flattened());
    }
}

/// Admissible lower bound on the cost of finishing from `config`:
/// every amphipod that is not already settled has to pay at least
/// its own way out of the room, along the hallway and back down.
fn heuristic(config: &Configuration, room_size: usize) -> u32 {
    let mut cost = 0_u32;

    for (h, &c) in config.hallway.iter().enumerate() {
        if !(b'A'..=b'D').contains(&c) {
            continue;
        }
        let target_room = numeric(c) as usize;
        let steps = h.abs_diff(hallway_index(target_room)) as u32 + 1;
        cost += steps * get_cost(c);
    }

    for (r, room) in config.rooms.iter().enumerate() {
        for s in 0..room_size {
            let c = room[s];
            if c == EMPTY {
                continue;
            }
            let target_room = numeric(c) as usize;

            if (target_room == r) {
                // Settled if all slots below are also in the correct room
                if (0..s).all(|b| room[b] != EMPTY && correct_room(room[b], r)) {
                    continue;
                }
                // Must exit and re-enter (wrong amphipod is stuck below)
                cost += ((room_size - s) as u32 + 1) * get_cost(c);
            } else {
                let steps_out = (room_size - s) as u32;
                let h_steps = hallway_index(r).abs_diff(hallway_index(target_room)) as u32;
                cost += (steps_out + h_steps + 1) * get_cost(c);
            }
        }
    }

    return cost;
}

fn solve(rooms: &Rooms, room_size: usize) -> u32 {
    let start = Configuration {
        hallway: HALLWAY,
        rooms: *rooms,
    };
    let mut end_rooms = [[EMPTY; 4]; 4];
    for room_index in 0..4 {
        for slot in 0..room_size {
            end_rooms[room_index][slot] = b'A' + room_index as u8;
        }
    }
    let end = Configuration {
        hallway: HALLWAY,
        rooms: end_rooms,
    };
    let distances = aoc::dijkstra::shortest_distances_astar(
        &start,
        |current| *current == end,
        |current| {
            let mut neighbors = Vec::new();

            // Try moving from room to the hallway
            for (room_index, room) in current.rooms.iter().enumerate() {
                let Some(top_slot) = (0..room_size).rev().find(|&i| room[i] != EMPTY)
                //
                else {
                    continue;
                };
                if (0..=top_slot).all(|i| correct_room(room[i], room_index)) {
                    continue; // Room settled
                }
                let amphipod = room[top_slot];
                let steps_up = (room_size - top_slot) as u32;

                let mut new_room = *room;
                new_room[top_slot] = EMPTY;

                for (hall_index, hall) in current.hallway.iter().enumerate() {
                    if (*hall != EMPTY) {
                        continue;
                    }
                    let Some(path) = hallway_path(&current.hallway, hall_index, room_index)
                    //
                    else {
                        continue;
                    };

                    let distance = (steps_up + (path.1 - path.0) as u32);

                    let mut new_hallway = current.hallway;
                    new_hallway[hall_index] = amphipod;

                    let mut new_rooms = current.rooms;
                    new_rooms[room_index] = new_room;

                    neighbors.push(DijkstraState {
                        data: Configuration {
                            hallway: new_hallway,
                            rooms: new_rooms,
                        },
                        distance: distance * get_cost(amphipod),
                    });
                }
            }

            // Try moving from the hallway to a room
            for (hall_index, hall) in current.hallway.iter().enumerate() {
                if ((*hall == EMPTY) || (*hall == FORBIDDEN)) {
                    continue;
                }

                let amphipod = *hall;
                let room_index = numeric(amphipod) as usize;
                let room = &current.rooms[room_index];

                if (0..room_size).any(|i| room[i] != EMPTY && !correct_room(room[i], room_index)) {
                    continue; // Wrong amphipod in room
                }
                let Some(target) = (0..room_size).find(|&i| room[i] == EMPTY)
                //
                else {
                    continue;
                };
                let Some(path) = hallway_path(&current.hallway, hall_index, room_index)
                //
                else {
                    continue;
                };

                let steps_down = (room_size - target) as u32;
                let distance = (steps_down + (path.1 - path.0) as u32);

                let mut new_hallway = current.hallway;
                new_hallway[hall_index] = EMPTY;

                let mut new_rooms = current.rooms;
                new_rooms[room_index][target] = amphipod;

                neighbors.push(DijkstraState {
                    data: Configuration {
                        hallway: new_hallway,
                        rooms: new_rooms,
                    },
                    distance: distance * get_cost(amphipod),
                });
            }

            return neighbors;
        },
        |current| heuristic(current, room_size),
    );

    return distances[&end];
}

fn solve_case1(rooms: &Rooms) -> u32 {
    solve(rooms, 2)
}

fn solve_case2(rooms: &Rooms) -> u32 {
    // #D#C#B#A# -> slot 2
    // #D#B#A#C# -> slot 1
    let inserted_upper = *b"DCBA";
    let inserted_lower = *b"DBAC";
    let rooms4 = std::array::from_fn(|i| {
        [
            rooms[i][0],
            inserted_lower[i],
            inserted_upper[i],
            rooms[i][1],
        ]
    });
    solve(&rooms4, 4)
}

fn main() {
    println!("Part 1");
    let example = parse("day23.example");
    aoc::expect_result!(12521, solve_case1(&example));
    let input = parse("day23.input");
    aoc::expect_result!(14350, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(44169, solve_case2(&example));
    aoc::expect_result!(49742, solve_case2(&input));
}
