use std::collections::{HashMap, HashSet};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
struct Point3D {
    x: i32,
    y: i32,
    z: i32,
}

impl Point3D {
    fn new(x: i32, y: i32, z: i32) -> Self {
        Point3D { x, y, z }
    }
}

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
                x: parts.next().unwrap().parse().unwrap(),
                y: parts.next().unwrap().parse().unwrap(),
                z: parts.next().unwrap().parse().unwrap(),
            });
        }
    }
    scanners.push(current); // last scanner has no trailing blank line
    scanners
}

fn rotations(scanner: &Scanner) -> impl Iterator<Item = Scanner> + '_ {
    // 6 axis permutations × 4 sign combos where det(rotation) = +1
    let transforms: [fn(Point3D) -> Point3D; 24] = [
        // permutation (x,y,z), det=+1 -> signs product must be +1
        |Point3D { x, y, z }| Point3D::new(x, y, z),
        |Point3D { x, y, z }| Point3D::new(x, y, -z),
        |Point3D { x, y, z }| Point3D::new(x, -y, z),
        |Point3D { x, y, z }| Point3D::new(x, -y, -z),
        // permutation (x,z,y), det=-1 -> signs product must be -1
        |Point3D { x, y, z }| Point3D::new(-x, z, y),
        |Point3D { x, y, z }| Point3D::new(-x, -z, y),
        |Point3D { x, y, z }| Point3D::new(-x, z, -y),
        |Point3D { x, y, z }| Point3D::new(-x, -z, -y),
        // permutation (y,x,z), det=-1 -> signs product must be -1
        |Point3D { x, y, z }| Point3D::new(-y, x, z),
        |Point3D { x, y, z }| Point3D::new(y, -x, z),
        |Point3D { x, y, z }| Point3D::new(y, x, -z),
        |Point3D { x, y, z }| Point3D::new(-y, -x, -z),
        // permutation (y,z,x), det=+1 -> signs product must be +1
        |Point3D { x, y, z }| Point3D::new(y, z, x),
        |Point3D { x, y, z }| Point3D::new(y, -z, x),
        |Point3D { x, y, z }| Point3D::new(-y, z, x),
        |Point3D { x, y, z }| Point3D::new(-y, -z, x),
        // permutation (z,x,y), det=+1 -> signs product must be +1
        |Point3D { x, y, z }| Point3D::new(z, x, y),
        |Point3D { x, y, z }| Point3D::new(z, -x, y),
        |Point3D { x, y, z }| Point3D::new(-z, x, y),
        |Point3D { x, y, z }| Point3D::new(-z, -x, y),
        // permutation (z,y,x), det=-1 -> signs product must be -1
        |Point3D { x, y, z }| Point3D::new(-z, y, x),
        |Point3D { x, y, z }| Point3D::new(z, y, -x),
        |Point3D { x, y, z }| Point3D::new(z, -y, -x),
        |Point3D { x, y, z }| Point3D::new(-z, -y, x),
    ];
    transforms
        .into_iter()
        .map(|f| scanner.iter().map(|&p| f(p)).collect())
}

fn solve_case1(scanners: &[Scanner]) -> usize {
    let mut offset_frequency = HashMap::new();
    let mut all_beacons = HashSet::from_iter(scanners[0]);
    return all_beacons.len();
}

fn solve_case2(scanners: &[Scanner]) -> usize {
    // TODO: Implement Part 2
    0
}

fn main() {
    println!("UNSOLVED");

    println!("Part 1");
    let example = parse("day19.example");
    assert_eq!(79, solve_case1(&example));
    let input = parse("day19.input");
    // assert_eq!(XXX, solve_case1(&input));

    println!("Part 2");
    // assert_eq!(XXX, solve_case2(&example));
    // assert_eq!(XXX, solve_case2(&input));
}
