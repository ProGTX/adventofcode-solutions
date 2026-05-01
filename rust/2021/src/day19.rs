use aoc::nd_point::NDPoint;
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
        |Point3D { data: [x, y, z] }| Point3D { data: [x, y, -z] },
        |Point3D { data: [x, y, z] }| Point3D { data: [x, -y, z] },
        |Point3D { data: [x, y, z] }| Point3D { data: [x, -y, -z] },
        // permutation (x,z,y), det=-1 -> signs product must be -1
        |Point3D { data: [x, y, z] }| Point3D { data: [-x, z, y] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-x, -z, y] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-x, z, -y] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-x, -z, -y] },
        // permutation (y,x,z), det=-1 -> signs product must be -1
        |Point3D { data: [x, y, z] }| Point3D { data: [-y, x, z] },
        |Point3D { data: [x, y, z] }| Point3D { data: [y, -x, z] },
        |Point3D { data: [x, y, z] }| Point3D { data: [y, x, -z] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-y, -x, -z] },
        // permutation (y,z,x), det=+1 -> signs product must be +1
        |Point3D { data: [x, y, z] }| Point3D { data: [y, z, x] },
        |Point3D { data: [x, y, z] }| Point3D { data: [y, -z, x] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-y, z, x] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-y, -z, x] },
        // permutation (z,x,y), det=+1 -> signs product must be +1
        |Point3D { data: [x, y, z] }| Point3D { data: [z, x, y] },
        |Point3D { data: [x, y, z] }| Point3D { data: [z, -x, y] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-z, x, y] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-z, -x, y] },
        // permutation (z,y,x), det=-1 -> signs product must be -1
        |Point3D { data: [x, y, z] }| Point3D { data: [-z, y, x] },
        |Point3D { data: [x, y, z] }| Point3D { data: [z, y, -x] },
        |Point3D { data: [x, y, z] }| Point3D { data: [z, -y, -x] },
        |Point3D { data: [x, y, z] }| Point3D { data: [-z, -y, x] },
    ];
    transforms
        .into_iter()
        .map(|f| scanner.iter().map(|&p| f(p)).collect())
}

fn solve_case1(scanners: &[Scanner]) -> usize {
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
    // aoc::expect_result!(XXX, solve_case2(&example));
    // aoc::expect_result!(XXX, solve_case2(&input));
}
