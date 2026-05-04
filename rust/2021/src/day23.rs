use aoc::dijkstra::DijkstraState;

type Rooms = [[char; 4]; 4];
type Hallway = [char; 11];

const fn hallway_index(room_index: usize) -> usize {
    room_index * 2 + 2
}

fn parse(filename: &str) -> Rooms {
    let lines = aoc::file::read_lines(filename);
    let top: Vec<char> = lines[2].chars().collect();
    let bottom: Vec<char> = lines[3].chars().collect();
    std::array::from_fn(|room_index| {
        // +1 for the leading # in the file line
        let col = hallway_index(room_index) + 1;
        [bottom[col], top[col], EMPTY, EMPTY]
    })
}

const HALLWAY: Hallway = //
    ['.', '.', 'X', '.', 'X', '.', 'X', '.', 'X', '.', '.'];
const EMPTY: char = HALLWAY[0];
const FORBIDDEN: char = HALLWAY[2];

const fn numeric(c: char) -> u8 {
    c as u8 - b'A' as u8
}

const fn get_cost(c: char) -> u32 {
    10_u32.pow(numeric(c) as u32)
}

const fn correct_room(c: char, room_index: usize) -> bool {
    (numeric(c) as usize) == room_index
}

fn hallway_path(hallway: &Hallway, hall_index: usize, room_index: usize) -> Option<(usize, usize)> {
    let hall_above_room = hallway_index(room_index);
    let (from, to) = (
        hall_above_room.min(hall_index),
        hall_above_room.max(hall_index),
    );
    if (hallway[from..=to].iter().enumerate().any(|(i, c)| {
        return ((from + i) != hall_index) && ['A', 'B', 'C', 'D'].contains(c);
    })) {
        // There is another amphipod in the way
        return None;
    }
    return Some((from, to));
}

#[derive(Clone, Debug, PartialEq, Eq, Hash, PartialOrd, Ord)]
struct Configuration {
    hallway: Hallway,
    rooms: Rooms,
}

impl std::fmt::Display for Configuration {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        let hallway: String = self
            .hallway
            .iter()
            .map(|&c| if c == FORBIDDEN { '.' } else { c })
            .collect();
        let [r0, r1, r2, r3] = self.rooms;
        writeln!(f, "#############")?;
        writeln!(f, "#{hallway}#")?;
        writeln!(f, "###{0}#{1}#{2}#{3}###", r0[1], r1[1], r2[1], r3[1])?;
        writeln!(f, "  #{0}#{1}#{2}#{3}#", r0[0], r1[0], r2[0], r3[0])?;
        writeln!(f, "  #########")
    }
}

fn solve(rooms: &Rooms, room_size: usize) -> u32 {
    let start = Configuration {
        hallway: HALLWAY,
        rooms: *rooms,
    };
    let mut end_rooms = [[EMPTY; 4]; 4];
    for room_index in 0..4 {
        for slot in 0..room_size {
            end_rooms[room_index][slot] = (b'A' + room_index as u8) as char;
        }
    }
    let end = Configuration {
        hallway: HALLWAY,
        rooms: end_rooms,
    };
    let distances = aoc::dijkstra::shortest_distances(
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
    );

    return distances[&end];
}

fn solve_case1(rooms: &Rooms) -> u32 {
    solve(rooms, 2)
}

fn solve_case2(rooms: &Rooms) -> u32 {
    // #D#C#B#A# -> slot 2
    // #D#B#A#C# -> slot 1
    let inserted_upper = ['D', 'C', 'B', 'A'];
    let inserted_lower = ['D', 'B', 'A', 'C'];
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
    assert_eq!(12521, solve_case1(&example));
    let input = parse("day23.input");
    assert_eq!(14350, solve_case1(&input));

    println!("Part 2");
    assert_eq!(44169, solve_case2(&example));
    assert_eq!(49742, solve_case2(&input));
}
