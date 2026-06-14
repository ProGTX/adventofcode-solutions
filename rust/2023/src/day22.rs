use aoc::closed_range::ClosedRange;
use aoc::nd_point::NDPoint;

type Coord = NDPoint<i32, 3>;
type Brick = ClosedRange<Coord>;

fn parse_coord(s: &str) -> Coord {
    let mut data = [0; 3];
    for (i, n) in s.split(',').enumerate() {
        data[i] = n.parse().unwrap();
    }
    Coord { data }
}

fn parse(filename: &str) -> Vec<Brick> {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let (first, last) = line.split_once('~').unwrap();
            ClosedRange::new(parse_coord(first), parse_coord(last))
        })
        .collect()
}

// Returns whether two bricks overlap when viewed from above
fn overlaps_xy(a: &Brick, b: &Brick) -> bool {
    a.begin.data[0] <= b.end.data[0]
        && a.end.data[0] >= b.begin.data[0]
        && a.begin.data[1] <= b.end.data[1]
        && a.end.data[1] >= b.begin.data[1]
}

// Drops each brick straight down until it rests on the floor
// or on top of an already-settled brick below it.
// Bricks are processed lowest-first,
// so by the time a brick is settled,
// every brick that could support it is already in its final position.
fn fall_down(bricks: &mut Vec<Brick>) {
    // Sorts bricks in increasing order by their lowest z component
    bricks.sort_by_key(|brick| brick.begin.data[2]);

    for i in 0..bricks.len() {
        let brick = bricks[i];
        // The brick keeps its height as it falls
        let height = brick.end.data[2] - brick.begin.data[2];

        // Find the highest surface among the already-settled bricks
        // below this one that it would land on, or the floor
        let mut new_begin_z = 1;
        for other in &bricks[..i] {
            if overlaps_xy(&brick, other) {
                new_begin_z = new_begin_z.max(other.end.data[2] + 1);
            }
        }

        // Settle the brick directly on top of that surface
        bricks[i].begin.data[2] = new_begin_z;
        bricks[i].end.data[2] = new_begin_z + height;
    }
}

// supports[i] lists the bricks resting directly on top of brick i
// supported_by[i] lists the bricks directly below brick i
fn get_supports(bricks: &[Brick]) -> (Vec<Vec<usize>>, Vec<Vec<usize>>) {
    let mut supports = vec![Vec::new(); bricks.len()];
    let mut supported_by = vec![Vec::new(); bricks.len()];

    for (i, brick) in bricks.iter().enumerate() {
        for (j, other) in bricks.iter().enumerate() {
            if i != j && other.begin.data[2] == brick.end.data[2] + 1 && overlaps_xy(brick, other) {
                supports[i].push(j);
                supported_by[j].push(i);
            }
        }
    }

    (supports, supported_by)
}

fn solve_case1(bricks: &Vec<Brick>) -> usize {
    let mut bricks = bricks.clone();
    fall_down(&mut bricks);
    let (supports, supported_by) = get_supports(&bricks);

    supports
        .iter()
        .filter(|above| above.iter().all(|&j| supported_by[j].len() > 1))
        .count()
}

fn solve_case2(bricks: &Vec<Brick>) -> usize {
    let mut bricks = bricks.clone();
    fall_down(&mut bricks);
    let (supports, supported_by) = get_supports(&bricks);

    // Simulate disintegrating each brick in turn
    // and count the chain reaction it triggers
    (0..bricks.len())
        .map(|i| {
            // Bricks known to have fallen, starting with brick i itself
            let mut fallen = vec![false; bricks.len()];
            fallen[i] = true;
            // Candidates to check, seeded with everything resting on i
            let mut queue = supports[i].clone();
            let mut count = 0;

            while let Some(j) = queue.pop() {
                // Skip bricks already counted, or ones still held up
                // by a support that hasn't fallen (yet).
                // The latter get re-queued and rechecked
                // once that support falls.
                if fallen[j] || !supported_by[j].iter().all(|&k| fallen[k]) {
                    continue;
                }
                fallen[j] = true;
                count += 1;
                // j fell, so bricks resting on it might fall too
                queue.extend(&supports[j]);
            }

            count
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day22.example");
    aoc::expect_result!(5, solve_case1(&example));
    let example2 = parse("day22.example2");
    aoc::expect_result!(3, solve_case1(&example2));
    let example3 = parse("day22.example3");
    aoc::expect_result!(2, solve_case1(&example3));
    let input = parse("day22.input");
    aoc::expect_result!(465, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(7, solve_case2(&example));
    aoc::expect_result!(1, solve_case2(&example2));
    aoc::expect_result!(3, solve_case2(&example3));
    aoc::expect_result!(79042, solve_case2(&input));
}
