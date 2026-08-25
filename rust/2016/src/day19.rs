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

/// A circle of elves as a doubly linked list,
/// each node living at its own index for the whole run
struct Elf {
    id: usize,
    prev: usize,
    next: usize,
}

/// The elf that ends up with all of the presents,
/// when each elf takes from the one across the circle
fn solve_case2(num_elves: &Input) -> usize {
    let num_elves = *num_elves;
    let mut elves = (0..num_elves)
        .map(|index| Elf {
            id: index + 1,
            prev: (index + num_elves - 1) % num_elves,
            next: (index + 1) % num_elves,
        })
        .collect_vec();
    let mut current = 0;
    // The elf directly across the circle, `num_left / 2` elves ahead
    let mut across = num_elves / 2;
    let mut num_left = num_elves;
    while (num_left > 1) {
        let (before, after) = (elves[across].prev, elves[across].next);
        elves[after].prev = before;
        elves[before].next = after;
        // Walking the whole half circle again every time would be quadratic,
        // so the elf across is carried along instead:
        // losing one elf pulls it one step closer,
        // and the circle shrinking to an even size pushes it one step away
        across = elves[across].next;
        if ((num_left % 2) != 0) {
            across = elves[across].next;
        }
        num_left -= 1;
        current = elves[current].next;
    }
    return elves[current].id;
}

fn main() {
    println!("Part 1");
    let example = parse("day19.example");
    aoc::expect_result!(3, solve_case1(&example));
    let input = parse("day19.input");
    aoc::expect_result!(1830117, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(2, solve_case2(&example));
    aoc::expect_result!(1417887, solve_case2(&input));
}
