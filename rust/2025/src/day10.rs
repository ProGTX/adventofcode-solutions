use aoc::dijkstra::DijkstraState;
use aoc::iter::CollectArrayVec;
use aoc::math::binary_to_number;
use std::cmp::Ordering;
use std::hash::{Hash, Hasher};

type Lights = arrayvec::ArrayVec<u8, 10>;
type Button = arrayvec::ArrayVec<usize, 9>;
const MAX_BUTTONS: usize = 13;
type Buttons = arrayvec::ArrayVec<Button, MAX_BUTTONS>;
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
struct DijkstraLights {
    lights: Lights,
    decimal: u32,
}
impl DijkstraLights {
    fn from(lights: Lights) -> Self {
        let decimal = binary_to_number(&lights);
        Self {
            lights: lights,
            decimal: decimal,
        }
    }
}
impl Ord for DijkstraLights {
    fn cmp(&self, other: &Self) -> Ordering {
        other.decimal.cmp(&self.decimal)
    }
}
impl PartialOrd for DijkstraLights {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}
impl Hash for DijkstraLights {
    fn hash<H: Hasher>(&self, state: &mut H) {
        state.write_u32(self.decimal);
    }
}

fn solve_case1(input: &Input) -> u32 {
    input
        .iter()
        .map(|(lights, buttons, _)| {
            let start = DijkstraLights::from(lights.iter().map(|_| 0).collect::<Lights>());
            let end = DijkstraLights::from(lights.clone());
            let distances =
                aoc::dijkstra::shortest_distances(&start, &end, |current: &DijkstraLights| {
                    buttons
                        .iter()
                        .map(|button| {
                            let mut next = current.lights.clone();
                            for &pos in button {
                                next[pos] = (next[pos] == 0) as u8;
                            }
                            return DijkstraState {
                                data: DijkstraLights::from(next),
                                distance: 1,
                            };
                        })
                        .collect_array_vec::<MAX_BUTTONS>()
                });
            return *distances.get(&end).unwrap();
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
