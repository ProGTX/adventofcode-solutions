use aoc::dijkstra::DijkstraState;
use aoc::string::NameToId;
use itertools::Itertools;

/// A floor holds one bit per element's chip, and one per its generator.
type Floor = u16;
const GENERATOR_SHIFT: usize = 8;

type Floors = [Floor; 4];

#[derive(Clone, Copy, Eq, Hash, Ord, PartialEq, PartialOrd)]
struct State {
    floors: Floors,
    elevator: usize,
}

/// The parsed floors, and how many elements they have room for
type Input = (Floors, usize);

fn parse(filename: &str) -> Input {
    let mut floors: Floors = [0; 4];
    let mut elements = NameToId::new();

    for (floor, line) in aoc::file::read_lines(filename).iter().enumerate() {
        let words = line
            .split(' ')
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
            floors[floor] |= 1_u16 << (id + if chip { 0 } else { GENERATOR_SHIFT });
        }
    }

    // Two extra elements, for the parts added in part 2
    let num_elements = elements.new_len(0) + 2;
    debug_assert!(num_elements <= GENERATOR_SHIFT, "Too many elements");

    return (floors, num_elements);
}

/// A chip is fried unless it sits with its own generator,
/// or the floor holds no generators at all.
fn is_valid(floor: Floor) -> bool {
    let generators = floor >> GENERATOR_SHIFT;
    return (generators == 0) || ((floor & !generators & 0xff) == 0);
}

/// Elements are interchangeable, so a state is only its multiset of
/// (chip floor, generator floor) pairs.
/// Relabelling the elements into sorted order picks one representative
/// per class of equivalent states.
///
/// https://www.reddit.com/r/adventofcode/comments/5hoia9/comment/db1v1ws/
fn canonicalize(floors: Floors) -> Floors {
    let mut elements = Vec::new();
    for id in 0..GENERATOR_SHIFT {
        let chip = floors.iter().position(|items| (items & (1 << id)) != 0);
        let generator = floors
            .iter()
            .position(|items| (items & (1 << (id + GENERATOR_SHIFT))) != 0);
        if let (Some(chip), Some(generator)) = (chip, generator) {
            elements.push((chip, generator));
        }
    }
    elements.sort_unstable();

    let mut canonical: Floors = [0; 4];
    for (id, (chip, generator)) in elements.into_iter().enumerate() {
        canonical[chip] |= 1 << id;
        canonical[generator] |= 1 << (id + GENERATOR_SHIFT);
    }
    return canonical;
}

fn solve_case<const EXTRA_PARTS: bool>((floors, num_elements): &Input) -> u32 {
    // The extra parts start on the first floor, otherwise their bits stay unused
    let mut start_floors = *floors;
    if (EXTRA_PARTS) {
        for id in (num_elements - 2)..(*num_elements) {
            start_floors[0] |= (1_u16 << id) | (1_u16 << (id + GENERATOR_SHIFT));
        }
    }
    let start = State {
        floors: canonicalize(start_floors),
        elevator: 0,
    };
    // Everything ends up on the top floor
    let all_items = start_floors[0] | start_floors[1] | start_floors[2] | start_floors[3];
    let end = State {
        floors: canonicalize([0, 0, 0, all_items]),
        elevator: 3,
    };

    let distances = aoc::dijkstra::shortest_distances(
        &start,
        |current| *current == end,
        |current| {
            let floor = current.elevator;
            // Each item on this floor, as a mask of the one bit that represents it
            let mut items = Vec::new();
            let mut left = current.floors[floor];
            while (left != 0) {
                // The lowest set bit, i.e. one chip or one generator
                items.push(left & left.wrapping_neg());
                // Clear it, leaving the items still to be listed
                left &= left - 1;
            }

            // Every pair of items that could be taken along together
            let mut pairs = Vec::new();
            for (i, &first) in items.iter().enumerate() {
                for second in &items[i + 1..] {
                    pairs.push(first | second);
                }
            }

            let mut neighbors = Vec::new();
            for next in [floor.wrapping_sub(1), floor + 1] {
                if (next >= 4) {
                    continue;
                }
                // Carrying a pair up is never worse than carrying a single item,
                // and bringing a single item down is never worse than bringing a pair
                // so only fall back to the other kind when the preferred one has no valid move
                let groups = if (next > floor) {
                    [&pairs, &items]
                } else {
                    [&items, &pairs]
                };
                let before = neighbors.len();
                for group in groups {
                    for &moved in group {
                        let mut floors = current.floors;
                        floors[floor] &= !moved;
                        floors[next] |= moved;
                        if (is_valid(floors[floor]) && is_valid(floors[next])) {
                            neighbors.push(DijkstraState {
                                data: State {
                                    floors: canonicalize(floors),
                                    elevator: next,
                                },
                                distance: 1,
                            });
                        }
                    }
                    if (neighbors.len() > before) {
                        break;
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
    aoc::expect_result!(11, solve_case::<false>(&example));
    let input = parse("day11.input");
    aoc::expect_result!(31, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(55, solve_case::<true>(&input));
}
