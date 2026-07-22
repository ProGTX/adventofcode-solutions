use aoc::algorithm::flood_fill;
use std::collections::HashSet;

type Droplet = [i32; 3];
type Input = HashSet<Droplet>;

const DELTAS: [Droplet; 6] = [
    [0, 0, 1],
    [0, 1, 0],
    [1, 0, 0],
    [0, 0, -1],
    [0, -1, 0],
    [-1, 0, 0],
];

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
        .into_iter()
        .map(|line| {
            let v: Vec<i32> = line.split(',').map(|n| n.parse().unwrap()).collect();
            [v[0], v[1], v[2]]
        })
        .collect()
}

fn add_area(a: Droplet, b: Droplet) -> Droplet {
    [a[0] + b[0], a[1] + b[1], a[2] + b[2]]
}

fn solve_case1(droplets: &Input) -> u32 {
    droplets
        .iter()
        .flat_map(|&droplet| {
            DELTAS.map(|delta| {
                return add_area(droplet, delta);
            })
        })
        .filter(|neighbor| !droplets.contains(neighbor))
        .count() as u32
}

fn solve_case2(droplets: &Input) -> u32 {
    // Find bounding box extended by 1 in each direction,
    // so the fill can flow around the entire exterior of the droplet cluster
    let mut min_pt = [i32::MAX; 3];
    let mut max_pt = [i32::MIN; 3];
    for droplet in droplets {
        for i in 0..3 {
            min_pt[i] = min_pt[i].min(droplet[i]);
            max_pt[i] = max_pt[i].max(droplet[i]);
        }
    }
    for i in 0..3 {
        min_pt[i] -= 1;
        max_pt[i] += 1;
    }

    let in_bounds = |pos: &Droplet| -> bool {
        return (0..3).all(|i| pos[i] >= min_pt[i] && pos[i] <= max_pt[i]);
    };

    // BFS from corner of bounding box outward
    let exterior = flood_fill(&min_pt, |&current| {
        DELTAS.into_iter().filter_map(move |delta| {
            let neighbor = add_area(current, delta);
            (in_bounds(&neighbor) && !droplets.contains(&neighbor)).then_some(neighbor)
        })
    });

    // Each time a neighbor is a droplet face, count it as exterior surface
    exterior
        .into_iter()
        .flat_map(|current| DELTAS.map(|delta| add_area(current, delta)))
        .filter(|neighbor| droplets.contains(neighbor))
        .count() as u32
}

fn main() {
    println!("Part 1");
    let example = parse("day18.example");
    aoc::expect_result!(10, solve_case1(&example));
    let example2 = parse("day18.example2");
    aoc::expect_result!(64, solve_case1(&example2));
    let input = parse("day18.input");
    aoc::expect_result!(4310, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(58, solve_case2(&example2));
    aoc::expect_result!(2466, solve_case2(&input));
}
