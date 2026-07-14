use arrayvec::ArrayVec;
use rustc_hash::FxHashMap;
use std::thread;

#[derive(Clone)]
struct Record {
    springs: String,
    groups: Vec<u8>,
}

fn parse(filename: &str) -> Vec<Record> {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let (springs, groups) = line.split_once(" ").unwrap();
            Record {
                springs: springs.to_owned(),
                groups: groups.split(',').map(|g| g.parse().unwrap()).collect(),
            }
        })
        .collect()
}

const DAMAGED: char = '#';
const OPERATIONAL: char = '.';
const UNKNOWN: char = '?';

#[derive(Clone, Default, Debug, Hash, PartialEq, Eq)]
struct SearchState {
    springs: String,
    groups: Vec<u8>,
    damaged_before: u8,
}

fn arrangement_neighbors(state: &SearchState) -> ArrayVec<SearchState, 2> {
    let mut neighbors = ArrayVec::new();
    let mut current_spring = state.springs.chars();
    match current_spring.next() {
        Some(DAMAGED) => {
            let rest = current_spring.as_str();
            let extra_damaged = rest.chars().take_while(|&c| c == DAMAGED).count();
            let total_damaged = state.damaged_before + 1 + extra_damaged as u8;
            if (extra_damaged + 1) == state.springs.len() {
                // End of search, success if last group equals our count
                if (state.groups.len() == 1) && (*state.groups.last().unwrap() == total_damaged) {
                    neighbors.push(SearchState::default());
                }
            } else {
                neighbors.push(SearchState {
                    springs: rest[extra_damaged..].to_owned(),
                    groups: state.groups.clone(),
                    damaged_before: total_damaged,
                });
            }
        }
        Some(OPERATIONAL) => {
            let mut new_groups = state.groups.clone();
            if state.damaged_before > 0 {
                // A damaged group located before current spring
                if state.groups.is_empty() || (state.groups[0] != state.damaged_before) {
                    // Invalid group count
                    return neighbors;
                }
                // Close the group
                new_groups = state.groups[1..].to_owned();
            }
            let rest = current_spring.as_str();
            let skip = rest.chars().take_while(|&c| c == OPERATIONAL).count();
            neighbors.push(SearchState {
                springs: rest[skip..].to_owned(),
                groups: new_groups,
                damaged_before: 0,
            });
        }
        Some(UNKNOWN) => {
            // Two options to explore
            let mut state1 = state.clone();
            state1.springs.replace_range(0..1, &DAMAGED.to_string());
            let mut state2 = state.clone();
            state2.springs.replace_range(0..1, &OPERATIONAL.to_string());
            neighbors.push(state1);
            neighbors.push(state2);
        }
        None => {
            // End of search, handled by end_reached
        }
        _ => unreachable!("Invalid value"),
    }
    neighbors
}

fn count_arrangements<const FACTOR: usize>(records: &[Record]) -> u64 {
    let mut cache = FxHashMap::<_, u64>::default();
    records
        .iter()
        .map(|record| {
            let start = SearchState {
                springs: vec![record.springs.as_str(); FACTOR].join("?"),
                groups: record.groups.repeat(FACTOR),
                damaged_before: 0,
            };
            cache.clear();
            aoc::algorithm::dfs_uniform_with_cache(
                &mut cache,
                start,
                |state: &SearchState| state.springs.is_empty() && state.groups.is_empty(),
                arrangement_neighbors,
            )
        })
        .sum()
}

fn solve_case<const FACTOR: usize>(records: &[Record]) -> u64 {
    let num_threads = thread::available_parallelism().map_or(1, |n| n.get());
    let chunk_size = records.len().div_ceil(num_threads).max(1);

    thread::scope(|scope| {
        records
            .chunks(chunk_size)
            .map(|chunk| scope.spawn(move || count_arrangements::<FACTOR>(chunk)))
            .collect::<Vec<_>>()
            .into_iter()
            .map(|handle| handle.join().unwrap())
            .sum()
    })
}

fn main() {
    println!("Part 1");
    let example = parse("day12.example");
    aoc::expect_result!(21, solve_case::<1>(&example));
    let input = parse("day12.input");
    aoc::expect_result!(7379, solve_case::<1>(&input));

    println!("Part 2");
    aoc::expect_result!(206, solve_case::<2>(&example));
    aoc::expect_result!(2612, solve_case::<3>(&example));
    aoc::expect_result!(36308, solve_case::<4>(&example));
    aoc::expect_result!(525152, solve_case::<5>(&example));
    aoc::expect_result!(450228, solve_case::<2>(&input));
    aoc::expect_result!(83673283, solve_case::<3>(&input));
    aoc::expect_result!(23356098881, solve_case::<4>(&input));
    aoc::expect_result!(7732028747925, solve_case::<5>(&input));
}
