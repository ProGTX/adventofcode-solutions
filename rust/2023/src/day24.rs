use aoc::nd_point::NDPoint;

type Vec3 = NDPoint<f64, 3>;

struct Hailstone {
    position: Vec3,
    velocity: Vec3,
}

fn parse_vec3(s: &str) -> Vec3 {
    let mut data = [0.0f64; 3];
    for (i, n) in s.split(',').enumerate() {
        data[i] = n.trim().parse().unwrap();
    }
    Vec3 { data }
}

fn parse(filename: &str) -> Vec<Hailstone> {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let (pos_str, vel_str) = line.split_once('@').unwrap();
            Hailstone {
                position: parse_vec3(pos_str),
                velocity: parse_vec3(vel_str),
            }
        })
        .collect()
}

/// Returns the XY intersection point if the paths cross in the future
fn intersect_xy(h1: &Hailstone, h2: &Hailstone) -> Option<Vec3> {
    // We want to find time_h1 and time_h2 such that (in XY):
    //   h1.position + time_h1 * h1.velocity = h2.position + time_h2 * h2.velocity
    //
    // Rearranging into a 2x2 linear system Ax = b:
    //   [ h1.vel.x  -h2.vel.x ] [ time_h1 ]   [ h2.pos.x - h1.pos.x ]
    //   [ h1.vel.y  -h2.vel.y ] [ time_h2 ] = [ h2.pos.y - h1.pos.y ]
    //
    // Solved via Cramer's rule.
    let determinant = h2.velocity.x() * h1.velocity.y() - h1.velocity.x() * h2.velocity.y();
    if determinant == 0.0 {
        // Parallel paths, no unique intersection
        return None;
    }

    let pos_diff = h2.position - h1.position;

    let time_h1 = (-pos_diff.x() * h2.velocity.y() + h2.velocity.x() * pos_diff.y()) / determinant;
    let time_h2 = (h1.velocity.x() * pos_diff.y() - h1.velocity.y() * pos_diff.x()) / determinant;

    if time_h1 < 0.0 || time_h2 < 0.0 {
        // Paths crossed in the past for one or both hailstones
        return None;
    }

    // Evaluate h1's position at time_h1 to get the intersection point
    Some(h1.position + h1.velocity.scale(time_h1))
}

fn solve_case1<const MIN: u64, const MAX: u64>(hailstones: &[Hailstone]) -> usize {
    let (min, max) = (MIN as f64, MAX as f64);
    let mut count = 0;
    for i in 0..hailstones.len() {
        for j in (i + 1)..hailstones.len() {
            if let Some(p) = intersect_xy(&hailstones[i], &hailstones[j]) {
                count += ((min..=max).contains(p.x()) && (min..=max).contains(p.y())) as usize;
            }
        }
    }
    count
}

fn main() {
    println!("Part 1");
    let example = parse("day24.example");
    aoc::expect_result!(2, solve_case1::<7, 27>(&example));
    let input = parse("day24.input");
    aoc::expect_result!(
        21785,
        solve_case1::<200000000000000, 400000000000000>(&input)
    );

    aoc::return_incomplete();
}
