use arrayvec::ArrayVec;
use std::collections::hash_map::Entry;
use std::collections::{HashMap, HashSet};

type ReplacementsT = HashMap<String, ArrayVec<String, 10>>;

fn parse(filename: &str) -> (ReplacementsT, String) {
    let mut replacements = ReplacementsT::new();
    let mut last = false;
    for line in aoc::file::read_lines(filename) {
        if (last) {
            return (replacements, line);
        }
        if (line.is_empty()) {
            last = true;
            continue;
        }
        let (needle, replace) = line.split_once(" => ").unwrap();
        let needle = needle.to_string();
        let replace = replace.to_string();
        match replacements.entry(needle) {
            Entry::Vacant(e) => e.insert(ArrayVec::new()).push(replace),
            Entry::Occupied(mut e) => e.get_mut().push(replace),
        }
    }
    unreachable!();
}

fn solve_case1(parse_input: &(ReplacementsT, String)) -> usize {
    let (replacements, molecule) = parse_input;
    let mut generated = HashSet::<String>::new();
    for (needle, replaces) in replacements {
        for (index, _) in molecule.match_indices(needle) {
            for replace in replaces {
                let mut result = String::new();
                result.push_str(&molecule[..index]);
                result.push_str(&replace);
                result.push_str(&molecule[index + needle.len()..]);
                generated.insert(result);
            }
        }
    }
    generated.len()
}

fn main() {
    println!("Part 1");
    let example = parse("day19.example");
    aoc::expect_result!(4, solve_case1(&example));
    let input = parse("day19.input");
    aoc::expect_result!(576, solve_case1(&input));

    //println!("Part 2");
    //aoc::expect_result!(3, solve_case2(&example));
    //aoc::expect_result!(1257, solve_case2(&input));
}
