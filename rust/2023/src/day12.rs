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

#[derive(Clone, Default, Debug)]
struct SearchState {
    springs: String,
    groups: Vec<u8>,
    damaged_before: u8,
}

fn search_arrangements(state: SearchState) -> u64 {
    let mut current_spring = state.springs.chars();
    match current_spring.next() {
        Some(DAMAGED) => {
            let rest = current_spring.as_str();
            let extra_damaged = rest.chars().take_while(|&c| c == DAMAGED).count();
            let total_damaged = state.damaged_before + 1 + extra_damaged as u8;
            if (extra_damaged + 1) == state.springs.len() {
                // End of search, success if last group equals our count
                return ((state.groups.len() == 1)
                    && (*state.groups.last().unwrap() == total_damaged))
                    as u64;
            }
            return search_arrangements(SearchState {
                springs: rest[extra_damaged..].to_owned(),
                groups: state.groups,
                damaged_before: total_damaged,
            });
        }
        Some(OPERATIONAL) => {
            let mut new_state = SearchState::default();
            if state.damaged_before > 0 {
                // A damaged group located before current spring
                if state.groups.is_empty() || (state.groups[0] != state.damaged_before) {
                    // Invalid group count
                    return 0;
                }
                // Close the group
                new_state.groups = state.groups[1..].to_owned();
            } else {
                new_state.groups = state.groups;
            }
            let rest = current_spring.as_str();
            let skip = rest.chars().take_while(|&c| c == OPERATIONAL).count();
            new_state.springs = rest[skip..].to_owned();
            return search_arrangements(new_state);
        }
        Some(UNKNOWN) => {
            // Two options to explore
            let mut new_state1 = state.clone();
            new_state1.springs.replace_range(0..1, &DAMAGED.to_string());
            let mut new_state2 = state;
            new_state2
                .springs
                .replace_range(0..1, &OPERATIONAL.to_string());
            return search_arrangements(new_state1) + (search_arrangements(new_state2));
        }
        None => {
            // End of search, success if no groups left
            return state.groups.is_empty() as u64;
        }
        _ => unreachable!("Invalid value"),
    }
}

fn num_arrangements(record: &Record) -> u64 {
    search_arrangements(SearchState {
        springs: record.springs.clone(),
        groups: record.groups.clone(),
        damaged_before: 0,
    })
}

fn solve_case1(records: &[Record]) -> u64 {
    records.iter().map(num_arrangements).sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day12.example");
    aoc::expect_result!(21, solve_case1(&example));
    let input = parse("day12.input");
    aoc::expect_result!(7379, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
