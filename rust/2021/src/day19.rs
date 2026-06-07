use aoc::nd_point::{NDPoint, distance_manhattan};
use std::collections::{HashMap, HashSet};

type Point3D = NDPoint<i32, 3>;
type Scanner = Vec<Point3D>;

fn parse(filename: &str) -> Vec<Scanner> {
    let mut scanners = Vec::new();
    let mut current = Vec::new();
    for line in aoc::file::read_lines(filename) {
        if line.starts_with("---") {
            current = Vec::new();
        } else if line.is_empty() {
            scanners.push(current.clone());
        } else {
            let mut parts = line.splitn(3, ',');
            current.push(Point3D {
                data: [
                    parts.next().unwrap().parse().unwrap(),
                    parts.next().unwrap().parse().unwrap(),
                    parts.next().unwrap().parse().unwrap(),
                ],
            });
        }
    }
    scanners.push(current); // last scanner has no trailing blank line
    scanners
}

fn rotations(scanner: &Scanner) -> impl Iterator<Item = Scanner> + '_ {
    // 6 axis permutations times 4 sign combos where det(rotation) = +1
    let transforms: [fn(Point3D) -> Point3D; 24] = [
        // permutation (x,y,z), det=+1 -> signs product must be +1
        |Point3D { data: [x, y, z] }| Point3D { data: [x, y, z] },
        |Point3D { data: [x, y, z] }| Point3D { data: [x, -y, -z] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-x, y, -z] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-x, -y, z] },
        // permutation (x,z,y), det=-1 -> signs product must be -1
        |Point3D { data: [x, y, z] }| Point3D { data: [x, z, -y] },
        |Point3D { data: [x, y, z] }| Point3D { data: [x, -z, y] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-x, z, y] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-x, -z, -y] },
        // permutation (y,x,z), det=-1 -> signs product must be -1
        |Point3D { data: [x, y, z] }| Point3D { data: [y, x, -z] },
        |Point3D { data: [x, y, z] }| Point3D { data: [y, -x, z] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-y, x, z] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-y, -x, -z] },
        // permutation (y,z,x), det=+1 -> signs product must be +1
        |Point3D { data: [x, y, z] }| Point3D { data: [y, z, x] },
        |Point3D { data: [x, y, z] }| Point3D { data: [y, -z, -x] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-y, z, -x] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-y, -z, x] },
        // permutation (z,x,y), det=+1 -> signs product must be +1
        |Point3D { data: [x, y, z] }| Point3D { data: [z, x, y] },
        |Point3D { data: [x, y, z] }| Point3D { data: [z, -x, -y] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-z, x, -y] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-z, -x, y] },
        // permutation (z,y,x), det=-1 -> signs product must be -1
        |Point3D { data: [x, y, z] }| Point3D { data: [z, y, -x] },
        |Point3D { data: [x, y, z] }| Point3D { data: [z, -y, x] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-z, y, x] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-z, -y, -x] },
    ];
    transforms
        .into_iter()
        .map(|f| scanner.iter().map(|&p| f(p)).collect())
}

// Try to align `scanner` against the already-known beacon cloud.
// For each of the 24 rotations, count how often each candidate translation
// offset (known_beacon - rotated_beacon) appears.
// If any offset is seen >=12 times, at least 12 beacons coincide,
// so the scanners overlap.
// Return the rotated+translated beacons in world coordinates
// and the scanner's world-space position,
// or None if no match found.
fn try_align(all_beacons: &HashSet<Point3D>, scanner: &Scanner) -> Option<(Scanner, Point3D)> {
    for rotated in rotations(scanner) {
        let mut offset_counts: HashMap<Point3D, usize> = HashMap::new();
        for &a in all_beacons {
            for &b in &rotated {
                *offset_counts.entry(a - b).or_insert(0) += 1;
            }
        }
        if let Some((&offset, _)) = //
            offset_counts.iter().find(|(_, c)| **c >= 12)
        {
            return Some((rotated.iter().map(|&p| p + offset).collect(), offset));
        }
    }
    None
}

// Repeatedly try to align each unaligned scanner against the growing beacon cloud.
// Each successful match merges that scanner's beacons into the cloud
// and removes it from the unaligned set.
// Scanners that can't yet match (no direct overlap with scanner 0)
// will eventually match once an intermediate scanner has been merged.
// All unaligned scanners are tried in parallel each round via scoped threads.
fn align_all(scanners: &[Scanner]) -> (HashSet<Point3D>, Vec<Point3D>) {
    let mut all_beacons: HashSet<Point3D> = scanners[0].iter().copied().collect();
    let mut scanner_positions: Vec<Point3D> = vec![Point3D { data: [0, 0, 0] }];
    let mut unaligned: Vec<usize> = (1..scanners.len()).collect();

    let parallelism = std::thread::available_parallelism()
        .map(|n| n.get())
        .unwrap_or(4);
    while !unaligned.is_empty() {
        let beacons = &all_beacons;
        let result = std::thread::scope(|scope| {
            for chunk in unaligned.chunks(parallelism) {
                let handles: Vec<_> = chunk
                    .iter()
                    .map(|&s| scope.spawn(move || try_align(beacons, &scanners[s]).map(|r| (s, r))))
                    .collect();
                if let Some(r) = //
                    handles.into_iter().find_map(|h| h.join().unwrap())
                {
                    return Some(r);
                }
            }
            None
        });
        let (s, (aligned, pos)) = result.expect("no scanner aligned this round — bug");
        all_beacons.extend(aligned);
        scanner_positions.push(pos);
        unaligned.retain(|&i| i != s);
    }

    (all_beacons, scanner_positions)
}

fn solve_case1(scanners: &[Scanner]) -> usize {
    align_all(scanners).0.len()
}

fn solve_case2(scanners: &[Scanner]) -> u32 {
    let scanners = align_all(scanners).1;
    let n = scanners.len();
    (0..n)
        .flat_map(|i| {
            return (0..n).filter(move |j| i < *j).map(move |j| (i, j));
        })
        .map(|(i, j)| distance_manhattan(scanners[i], scanners[j]) as u32)
        .max()
        .unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day19.example");
    aoc::expect_result!(79, solve_case1(&example));
    let input = parse("day19.input");
    aoc::expect_result!(372, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(3621, solve_case2(&example));
    aoc::expect_result!(12241, solve_case2(&input));
}
