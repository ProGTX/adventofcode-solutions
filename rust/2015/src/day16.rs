use std::collections::HashMap;

fn solve_case<const RANGES: bool>(filename: &str) -> u32 {
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

    for (index, line) in std::fs::read_to_string(filename)
        .unwrap()
        .lines()
        .enumerate()
    {
        let (_, info) = line.split_once(": ").unwrap();
        let split_info = info.split(", ");
        let matches = split_info.into_iter().all(|info| {
            let (key, value) = info.split_once(": ").unwrap();
            let required = *generous_sue.get(key).unwrap();
            let actual = value.parse::<u32>().unwrap();
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
    assert_eq!(373, solve_case::<false>("day16.input"));
    println!("Part 2");
    assert_eq!(260, solve_case::<true>("day16.input"));
}
