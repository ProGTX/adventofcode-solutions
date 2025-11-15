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

fn solve_case1<const SECONDS: u32>(filename: &str) -> u32 {
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

fn solve_case2<const SECONDS: u32>(filename: &str) -> u32 {
    let flock = parse(filename);
    let mut distances = [0u32; 9];
    let mut score = [0u32; 9];
    debug_assert!(flock.len() <= score.len(), "Too many reindeer");
    for second in 0..SECONDS {
        // Move each reindeer
        for (index, reindeer) in flock.iter().enumerate() {
            let cycle_time = reindeer.fly + reindeer.rest;
            let flying = (second % cycle_time) < reindeer.fly;
            distances[index] += (flying as u32) * reindeer.kms;
        }
        // Award the farthest reindeer
        let mut distances_sorted = distances.iter().enumerate().collect::<Vec<_>>();
        distances_sorted.sort_by_key(|(_, dist)| **dist);
        let farthest_dist = distances_sorted.last().unwrap().1;
        for (index, dist) in distances_sorted.iter().rev() {
            if (*dist == farthest_dist) {
                score[*index] += 1;
            } else {
                break;
            }
        }
    }
    return *score.iter().max().unwrap();
}

fn main() {
    println!("Part 1");
    assert_eq!(1120, solve_case1::<1000>("day14.example"));
    assert_eq!(2660, solve_case1::<2503>("day14.input"));
    println!("Part 2");
    assert_eq!(689, solve_case2::<1000>("day14.example"));
    assert_eq!(1256, solve_case2::<2503>("day14.input"));
}
