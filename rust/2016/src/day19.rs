use itertools::Itertools;
use std::mem::swap;

/// The number of elves in the circle
type Input = usize;

fn parse(filename: &str) -> Input {
    aoc::file::read_string(filename).trim().parse().unwrap()
}

/// The elf that ends up with all of the presents
fn solve_case1(num_elves: &Input) -> usize {
    let mut elves = (1..num_elves + 1).collect_vec();
    let mut new_elves = Vec::with_capacity(*num_elves);
    while (elves.len() > 1) {
        let current_len = elves.len();
        new_elves.clear();
        // An odd circle ends with its last elf taking from its first,
        // so the round after it starts one elf further along
        let mut i = if ((current_len % 2) == 0) { 0 } else { 2 };
        while (i < current_len) {
            new_elves.push(elves[i]);
            i += 2;
        }

        swap(&mut elves, &mut new_elves);
    }
    return elves[0];
}

fn main() {
    println!("Part 1");
    let example = parse("day19.example");
    aoc::expect_result!(3, solve_case1(&example));
    let input = parse("day19.input");
    aoc::expect_result!(1830117, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
