use itertools::Itertools;

struct Disc {
    num_pos: usize,
    pos: usize,
}

type Input = Vec<Disc>;

/// The disc that part 2 adds below the input's discs
const EXTRA_DISC_VALUE: Disc = Disc {
    num_pos: 11,
    pos: 0,
};

fn parse(filename: &str) -> Input {
    return aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            // Disc #D has N positions; at time=0, it is at position P.
            let words = line.split_whitespace().collect_vec();
            Disc {
                num_pos: words[3].parse().unwrap(),
                pos: words[11].trim_end_matches('.').parse().unwrap(),
            }
        })
        .collect_vec();
}

fn solve_case<const EXTRA_DISC: bool>(discs: &Input) -> usize {
    // The first time to press the button so that the capsule
    // falls through every disc's slot
    (0..)
        .find(|time| {
            // The capsule reaches disc `id` one second per disc after the press
            discs
                .iter()
                // Part 2 adds one more disc below the ones from the input
                .chain(EXTRA_DISC.then_some(&EXTRA_DISC_VALUE))
                .enumerate()
                .all(|(index, disc)| ((time + index + 1 + disc.pos) % disc.num_pos) == 0)
        })
        .unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day15.example");
    aoc::expect_result!(5, solve_case::<false>(&example));
    let input = parse("day15.input");
    aoc::expect_result!(317371, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(85, solve_case::<true>(&example));
    aoc::expect_result!(2080951, solve_case::<true>(&input));
}
