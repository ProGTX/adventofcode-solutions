use aoc::dijkstra::DijkstraState;
use aoc::string::NameToId;
use itertools::Itertools;

#[derive(Clone, Eq, Hash, Ord, PartialEq, PartialOrd)]
struct Slot {
    chip: bool,
    generator: bool,
}

type Floors = [Vec<Slot>; 4];

#[derive(Clone, Eq, Hash, Ord, PartialEq, PartialOrd)]
struct State {
    floors: Floors,
    elevator: usize,
}

fn parse(filename: &str) -> Floors {
    let mut floors: Floors = [const { Vec::new() }; 4];
    let mut elements = NameToId::new();

    for (floor, line) in aoc::file::read_lines(filename).iter().enumerate() {
        let words = line
            .split_whitespace()
            .map(|word| word.trim_matches([',', '.']))
            .collect_vec();
        for (i, &word) in words.iter().enumerate() {
            // <element> generator | <element>-compatible microchip
            let (name, chip) = match word {
                "generator" => (words[i - 1], false),
                "microchip" => (words[i - 1].strip_suffix("-compatible").unwrap(), true),
                _ => continue,
            };
            let id = elements.intern(name);
            let slots = &mut floors[floor];
            if slots.len() <= id {
                slots.resize_with(id + 1, || Slot {
                    chip: false,
                    generator: false,
                });
            }
            if chip {
                slots[id].chip = true;
            } else {
                slots[id].generator = true;
            }
        }
    }

    let num_elements = floors.iter().map(|slots| slots.len()).max().unwrap();
    for slots in &mut floors {
        slots.resize_with(num_elements, || Slot {
            chip: false,
            generator: false,
        });
    }

    return floors;
}

/// A chip is fried unless it sits with its own generator,
/// or the floor holds no generators at all.
fn is_valid(slots: &[Slot]) -> bool {
    if slots.iter().all(|slot| !slot.generator) {
        return true;
    }
    return slots.iter().all(|slot| !slot.chip || slot.generator);
}

/// An item on a floor: the element id and whether it's the chip or the generator
type Item = (usize, bool);

fn move_item(floors: &mut Floors, from: usize, to: usize, (id, chip): Item) {
    if chip {
        floors[from][id].chip = false;
        floors[to][id].chip = true;
    } else {
        floors[from][id].generator = false;
        floors[to][id].generator = true;
    }
}

fn solve_case1(floors: &Floors) -> u32 {
    let num_elements = floors[0].len();
    let empty = vec![
        Slot {
            chip: false,
            generator: false,
        };
        num_elements
    ];
    let full = vec![
        Slot {
            chip: true,
            generator: true,
        };
        num_elements
    ];
    let start = State {
        floors: floors.clone(),
        elevator: 0,
    };
    let end = State {
        floors: [empty.clone(), empty.clone(), empty, full],
        elevator: 3,
    };

    let distances = aoc::dijkstra::shortest_distances(
        &start,
        |current| *current == end,
        |current| {
            let floor = current.elevator;
            let items = current.floors[floor]
                .iter()
                .enumerate()
                .flat_map(|(id, slot)| {
                    slot.chip
                        .then_some((id, true))
                        .into_iter()
                        .chain(slot.generator.then_some((id, false)))
                })
                .collect_vec();

            let mut neighbors = Vec::new();
            for next in [floor.wrapping_sub(1), floor + 1] {
                if (next >= 4) {
                    continue;
                }
                // Take one item, or a pair of them, along with the elevator
                for (i, &first) in items.iter().enumerate() {
                    let others = items[i + 1..].iter().map(|&item| Some(item));
                    for second in std::iter::once(None).chain(others) {
                        let mut floors = current.floors.clone();
                        move_item(&mut floors, floor, next, first);
                        if let Some(second) = second {
                            move_item(&mut floors, floor, next, second);
                        }
                        if (is_valid(&floors[floor]) && is_valid(&floors[next])) {
                            neighbors.push(DijkstraState {
                                data: State {
                                    floors,
                                    elevator: next,
                                },
                                distance: 1,
                            });
                        }
                    }
                }
            }

            return neighbors;
        },
    );
    return distances[&end];
}

fn main() {
    println!("Part 1");
    let example = parse("day11.example");
    aoc::expect_result!(11, solve_case1(&example));
    let input = parse("day11.input");
    aoc::expect_result!(31, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
