use std::collections::HashMap;

// Each aunt is described by exactly three known compounds
type Sue = Vec<(String, u32)>;

fn parse(filename: &str) -> Vec<Sue> {
    std::fs::read_to_string(filename)
        .unwrap()
        .lines()
        .map(|line| {
            let (_, info) = line.split_once(": ").unwrap();
            info.split(", ")
                .map(|info| {
                    let (key, value) = info.split_once(": ").unwrap();
                    (key.to_string(), value.parse::<u32>().unwrap())
                })
                .collect()
        })
        .collect()
}

fn solve_case<const RANGES: bool>(sues: &[Sue]) -> u32 {
    let generous_sue = HashMap::from([
        ("children", 3),
        ("cats", 7),
        ("samoyeds", 2),
        ("pomeranians", 3),
        ("akitas", 0),
        ("vizslas", 0),
        ("goldfish", 5),
        ("trees", 3),
        ("cars", 2),
        ("perfumes", 1),
    ]);

    for (index, sue) in sues.iter().enumerate() {
        let matches = sue.iter().all(|(key, value)| {
            let key = key.as_str();
            let required = *generous_sue.get(key).unwrap();
            let actual = *value;
            if (!RANGES) {
                return actual == required;
            } else {
                return match key {
                    "cats" | "trees" => actual > required,
                    "pomeranians" | "goldfish" => actual < required,
                    _ => actual == required,
                };
            }
        });
        if (matches) {
            return (index + 1) as u32;
        }
    }
    unreachable!("Aunt not found!");
}

fn main() {
    println!("Part 1");
    let input = parse("day16.input");
    aoc::expect_result!(373, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(260, solve_case::<true>(&input));
}
