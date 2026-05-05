use aoc::nd_point::NDPoint;

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

#[allow(dead_code)]
fn solve_case1(_data: &[Scanner]) -> usize {
    // TODO: Implement Part 1
    0
}

#[allow(dead_code)]
fn solve_case2(_data: &[Scanner]) -> usize {
    // TODO: Implement Part 2
    0
}

fn main() {
    println!("UNSOLVED");

    println!("Part 1");
    let _example = parse("day19.example");
    // assert_eq!(79, solve_case1(&_example));
    let _input = parse("day19.input");
    // assert_eq!(XXX, solve_case1(&input));

    println!("Part 2");
    // assert_eq!(XXX, solve_case2(&example));
    // assert_eq!(XXX, solve_case2(&input));
}
