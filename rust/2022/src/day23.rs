use aoc::direction::{ALL_SKY_DIRECTIONS, Direction};
use aoc::point::Point;
use std::collections::{BTreeMap, HashMap};

type Pos = Point<i32>;

const PROPOSAL_DIRS: [[Direction; 3]; 4] = [
    [Direction::North, Direction::Northeast, Direction::Northwest],
    [Direction::South, Direction::Southwest, Direction::Southeast],
    [Direction::West, Direction::Southwest, Direction::Northwest],
    [Direction::East, Direction::Northeast, Direction::Southeast],
];

fn parse(filename: &str) -> Vec<Pos> {
    let mut elves = Vec::new();
    for (row, line) in aoc::file::read_lines(filename).iter().enumerate() {
        for (col, ch) in line.chars().enumerate() {
            if ch == '#' {
                elves.push(Pos::new(col as i32, row as i32));
            }
        }
    }
    elves
}

fn simulate(elves: &mut Vec<Pos>, num_rounds: i32) -> i32 {
    // current position -> proposed position
    let mut sim: BTreeMap<Pos, Pos> = elves.iter().map(|&e| (e, e)).collect();
    let mut proposals = PROPOSAL_DIRS;

    let is_empty = |sim: &BTreeMap<Pos, Pos>, pos: Pos, dirs: &[Direction]| -> bool {
        dirs.iter().all(|d| !sim.contains_key(&(pos + d.diff())))
    };

    let mut round = 0;
    while round < num_rounds {
        // Propose: collect separately so we can read sim while computing
        let new_proposals = sim
            .keys()
            .map(|&current| {
                let mut proposed = current;
                if !is_empty(&sim, current, &ALL_SKY_DIRECTIONS) {
                    for dirs in &proposals {
                        if is_empty(&sim, current, dirs) {
                            proposed = current + dirs[0].diff();
                            break;
                        }
                    }
                }
                (current, proposed)
            })
            .collect::<Vec<_>>();

        if new_proposals.iter().all(|&(c, p)| c == p) {
            break;
        }

        // Count how many elves propose each destination
        let mut counts: HashMap<Pos, i32> = HashMap::new();
        for &(current, proposed) in &new_proposals {
            if current != proposed {
                *counts.entry(proposed).or_insert(0) += 1;
            }
        }

        // Execute: move elves with unique proposals, keep others in place
        sim = new_proposals
            .into_iter()
            .map(|(current, proposed)| {
                let dest = if counts.get(&proposed).copied().unwrap_or(0) == 1 {
                    proposed
                } else {
                    current
                };
                (dest, dest)
            })
            .collect();

        proposals.rotate_left(1);
        round += 1;
    }

    *elves = sim.into_keys().collect();
    round + 1
}

fn solve_case1(elves: &[Pos]) -> i32 {
    let mut elves = elves.to_vec();
    let _ = simulate(&mut elves, 10);
    let min_x = elves.iter().map(|e| e.x).min().unwrap();
    let max_x = elves.iter().map(|e| e.x).max().unwrap();
    let min_y = elves.iter().map(|e| e.y).min().unwrap();
    let max_y = elves.iter().map(|e| e.y).max().unwrap();
    (max_x - min_x + 1) * (max_y - min_y + 1) - elves.len() as i32
}

fn solve_case2(elves: &[Pos]) -> i32 {
    let mut elves = elves.to_vec();
    simulate(&mut elves, i32::MAX)
}

fn main() {
    println!("Part 1");
    let example2 = parse("day23.example2");
    aoc::expect_result!(25, solve_case1(&example2));
    let example = parse("day23.example");
    aoc::expect_result!(110, solve_case1(&example));
    let input = parse("day23.input");
    aoc::expect_result!(3906, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(4, solve_case2(&example2));
    aoc::expect_result!(20, solve_case2(&example));
    aoc::expect_result!(895, solve_case2(&input));
}
