use itertools::Itertools;

type Boxes = Vec<(i64, i64, i64)>;

fn parse(filename: &str) -> Boxes {
    let parse = |s: &str| s.parse::<i64>().unwrap();
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let coords = line.split(',').collect::<Vec<_>>();
            (parse(coords[0]), parse(coords[1]), parse(coords[2]))
        })
        .collect()
}

struct DistanceEntry {
    distance: u64,
    from: usize,
    to: usize,
}

fn solve_case1<const NUM_CONNECTIONS: usize>(boxes: &Boxes) -> usize {
    let mut distances = boxes
        .iter()
        .enumerate()
        .cartesian_product(boxes.iter().enumerate())
        .filter(|((index_p, _), (index_q, _))| index_p < index_q)
        .map(
            |(((index_p, &(p1, p2, p3))), (index_q, &(q1, q2, q3)))| DistanceEntry {
                distance: ((p1 - q1).pow(2) as u64
                    + (p2 - q2).pow(2) as u64
                    + (p3 - q3).pow(2) as u64),
                from: index_p,
                to: index_q,
            },
        )
        .collect::<Vec<_>>();
    distances.sort_unstable_by_key(|de| de.distance);
    // In the beginning each box is its own circuit
    let mut circuit_map = (0..boxes.len()).collect::<Vec<_>>();
    for de in &distances[0..NUM_CONNECTIONS] {
        let from = circuit_map[de.from];
        let to = circuit_map[de.to];
        if (from == to) {
            // Already in same circuit, do nothing
            continue;
        }
        // Join circuits by renaming old circuit names to new ones
        let old_name = to.max(from);
        let new_name = to.min(from);
        for current in circuit_map.iter_mut() {
            if (current == &old_name) {
                *current = new_name;
            }
        }
    }
    // Sum up circuit sizes
    let mut circuit_sizes = vec![0; boxes.len()];
    for circuit in circuit_map {
        circuit_sizes[circuit] += 1;
    }
    circuit_sizes.sort_unstable();
    circuit_sizes.iter().rev().take(3).product()
}

fn main() {
    println!("Part 1");
    let example = parse("day08.example");
    assert_eq!(40, solve_case1::<10>(&example));
    let input = parse("day08.input");
    assert_eq!(102816, solve_case1::<1000>(&input));

    // println!("Part 2");
    // assert_eq!(25272, solve_case2(&example));
    // assert_eq!(9876636978528, solve_case2(&input));
}
