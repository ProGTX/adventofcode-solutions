struct Reindeer {
    kms: u32,
    fly: u32,
    rest: u32,
}

fn parse(filename: &str) -> Vec<Reindeer> {
    std::fs::read_to_string(filename)
        .unwrap()
        .lines()
        .map(|line| {
            let words = line.split(' ').collect::<Vec<&str>>();
            Reindeer {
                kms: words[3].parse().unwrap(),
                fly: words[6].parse().unwrap(),
                rest: words[13].parse().unwrap(),
            }
        })
        .collect()
}

fn solve_case<const SECONDS: u32>(filename: &str) -> u32 {
    parse(filename)
        .iter()
        .map(|reindeer| {
            let cycle_time = reindeer.fly + reindeer.rest;
            let num_cycles = SECONDS / cycle_time;
            let remainder = SECONDS % cycle_time;
            let distance = num_cycles * reindeer.fly * reindeer.kms
                + remainder.min(reindeer.fly) * reindeer.kms;
            distance
        })
        .max()
        .unwrap()
}

fn main() {
    println!("Part 1");
    assert_eq!(1120, solve_case::<1000>("day14.example"));
    assert_eq!(2660, solve_case::<2503>("day14.input"));
    // println!("Part 2");
    // assert_eq!(4, solve_case2("day14.example"));
    // assert_eq!(96852, solve_case2("day14.input"));
}
