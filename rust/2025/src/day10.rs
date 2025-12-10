use aoc::math::binary_to_number;
use std::cmp::Ordering;
use std::collections::BinaryHeap;
use std::collections::HashMap;
use std::collections::hash_map::Entry;

type Lights = arrayvec::ArrayVec<u8, 10>;
type Button = arrayvec::ArrayVec<usize, 9>;
type Buttons = arrayvec::ArrayVec<Button, 13>;
type Joltage = arrayvec::ArrayVec<u16, 10>;
type Input = Vec<(Lights, Buttons, Joltage)>;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let (lights, rest) = line.split_once("] (").unwrap();
            let (buttons, joltage) = rest.split_once(") {").unwrap();
            let lights = lights[1..]
                .chars()
                .map(|c| (c == '#') as u8)
                .collect::<Lights>();
            let buttons = buttons
                .split(") (")
                .map(|button| {
                    button
                        .split(',')
                        .map(|num| num.parse().unwrap())
                        .collect::<Button>()
                })
                .collect::<Buttons>();
            let joltage = joltage[..joltage.len() - 1]
                .split(',')
                .map(|num| num.parse().unwrap())
                .collect::<Joltage>();
            (lights, buttons, joltage)
        })
        .collect()
}

#[derive(Clone, Debug, Eq, PartialEq)]
struct DijkstraState {
    distance: u32,
    lights: Lights,
}
// https://doc.rust-lang.org/std/collections/binary_heap/index.html
impl Ord for DijkstraState {
    fn cmp(&self, other: &Self) -> Ordering {
        other
            .distance
            .cmp(&self.distance)
            .then_with(|| self.lights.cmp(&other.lights))
    }
}
impl PartialOrd for DijkstraState {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

fn solve_case1(input: &Input) -> u32 {
    input
        .iter()
        .map(|(lights, buttons, _)| {
            let target = binary_to_number::<u32>(&lights);
            // Dijkstra
            let mut distances = HashMap::new();
            let mut unvisited = BinaryHeap::new();
            distances.insert(0, 0_u32);
            unvisited.push(DijkstraState {
                distance: 0,
                lights: lights.iter().map(|_| 0).collect::<Lights>(),
            });
            while (!unvisited.is_empty()) {
                let current = unvisited.pop().unwrap();
                let value = binary_to_number::<u32>(&current.lights);
                if (value == target) {
                    break;
                }
                if (current.distance > *distances.get(&value).unwrap()) {
                    continue;
                }
                // See if we can find a way with a lower cost going through this node
                for button in buttons {
                    let mut next = DijkstraState {
                        distance: current.distance + 1,
                        lights: current.lights.clone(),
                    };
                    for &pos in button {
                        next.lights[pos] = (next.lights[pos] == 0) as u8;
                    }
                    let next_value = binary_to_number::<u32>(&next.lights);
                    match distances.entry(next_value) {
                        Entry::Vacant(e) => {
                            e.insert(next.distance);
                            unvisited.push(next);
                        }
                        Entry::Occupied(mut e) => {
                            if (next.distance < *e.get()) {
                                *e.get_mut() = next.distance;
                                unvisited.push(next);
                            }
                        }
                    }
                }
            }
            return *distances.get(&target).unwrap();
        })
        .sum::<u32>()
}

fn main() {
    println!("Part 1");
    let example = parse("day10.example");
    assert_eq!(7, solve_case1(&example));
    let input = parse("day10.input");
    assert_eq!(535, solve_case1(&input));

    // println!("Part 2");
    // assert_eq!(33, solve_case2(&example));
    // assert_eq!(100011612, solve_case2(&input));
}
