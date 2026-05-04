use aoc::dijkstra::DijkstraState;

type Rooms = [[char; 2]; 4];
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
        [bottom[col], top[col]]
    })
}

const HALLWAY: Hallway = //
    ['.', '.', 'X', '.', 'X', '.', 'X', '.', 'X', '.', '.'];
const EMPTY: char = HALLWAY[0];
const FORBIDDEN: char = HALLWAY[2];
const ROOM_TOP: usize = 1;
const ROOM_BOTTOM: usize = 0;

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
        writeln!(
            f,
            "###{0}#{1}#{2}#{3}###",
            r0[ROOM_TOP], r1[ROOM_TOP], r2[ROOM_TOP], r3[ROOM_TOP]
        )?;
        writeln!(
            f,
            "  #{0}#{1}#{2}#{3}#",
            r0[ROOM_BOTTOM], r1[ROOM_BOTTOM], r2[ROOM_BOTTOM], r3[ROOM_BOTTOM]
        )?;
        writeln!(f, "  #########")
    }
}

fn solve_case1(rooms: &Rooms) -> u32 {
    let start = Configuration {
        hallway: HALLWAY,
        rooms: *rooms,
    };
    let end = Configuration {
        hallway: HALLWAY,
        rooms: [['A', 'A'], ['B', 'B'], ['C', 'C'], ['D', 'D']],
    };
    let distances = aoc::dijkstra::shortest_distances(
        //
        &start,
        |current| *current == end,
        |current| {
            let mut neighbors = Vec::new();

            // Try moving from room to the hallway
            for (room_index, room) in current.rooms.iter().enumerate() {
                let (amphipod, new_room, move_to_top) = if (room[ROOM_TOP] != EMPTY) {
                    // println!("{:?}", room);
                    if (!correct_room(room[ROOM_TOP], room_index)
                        || !correct_room(room[ROOM_BOTTOM], room_index))
                    {
                        (room[ROOM_TOP], ([room[ROOM_BOTTOM], EMPTY]), 1)
                    } else {
                        // Room is already correctly filled
                        continue;
                    }
                } else if (true
                    && (room[ROOM_BOTTOM] != EMPTY)
                    && !correct_room(room[ROOM_BOTTOM], room_index))
                {
                    (room[ROOM_BOTTOM], ([EMPTY, EMPTY]), 2)
                } else {
                    // Nothing to move out
                    continue;
                };

                for (hall_index, hall) in current.hallway.iter().enumerate() {
                    if (*hall != EMPTY) {
                        continue;
                    }
                    let path = hallway_path(&current.hallway, hall_index, room_index);
                    if (path.is_none()) {
                        continue;
                    }

                    let distance = (move_to_top + path.unwrap().1 - path.unwrap().0) as u32;

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
                for (room_index, room) in current.rooms.iter().enumerate() {
                    let path = hallway_path(&current.hallway, hall_index, room_index);
                    if (path.is_none()) {
                        continue;
                    }
                    let (room_pos, move_to_bottom) = if (room[ROOM_BOTTOM] == EMPTY) {
                        (ROOM_BOTTOM, 2)
                    } else if (correct_room(room[ROOM_BOTTOM], room_index)
                        && (room[ROOM_TOP] == EMPTY))
                    {
                        (ROOM_TOP, 1)
                    } else {
                        continue;
                    };

                    let amphipod = *hall;
                    let distance = (move_to_bottom + path.unwrap().1 - path.unwrap().0) as u32;

                    let mut new_hallway = current.hallway;
                    new_hallway[hall_index] = EMPTY;

                    let mut new_rooms = current.rooms;
                    new_rooms[room_index][room_pos] = amphipod;

                    neighbors.push(DijkstraState {
                        data: Configuration {
                            hallway: new_hallway,
                            rooms: new_rooms,
                        },
                        distance: distance * get_cost(amphipod),
                    });
                }
            }

            return neighbors;
        },
    );

    return distances[&end];
}

fn solve_case2(rooms: &Rooms) -> u64 {
    // TODO: Implement Part 2
    0
}

fn main() {
    println!("Part 1");
    let example = parse("day23.example");
    assert_eq!(12521, solve_case1(&example));
    let input = parse("day23.input");
    assert_eq!(14350, solve_case1(&input));

    println!("Part 2");
    // assert_eq!(XXX, solve_case2(&example));
    // assert_eq!(XXX, solve_case2(&input));
}
