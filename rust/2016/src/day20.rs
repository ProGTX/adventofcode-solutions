use aoc::closed_range::ClosedRange;
use itertools::Itertools;

type BlockedRange = ClosedRange<u32>;

/// The blocked ranges of IPs, ordered by where they begin
type Input = Vec<BlockedRange>;

fn parse(filename: &str) -> Input {
    return aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            // BEGIN-END
            let (begin, end) = line.split_once('-').unwrap();
            BlockedRange::new(begin.parse().unwrap(), end.parse().unwrap())
        })
        .sorted_unstable()
        .collect_vec();
}

/// The lowest IP that is not blocked by any of the ranges,
/// or how many IPs in the whole u32 range the ranges leave allowed
fn solve_case<const COUNT_ALLOWED: bool>(blacklist: &Input) -> u32 {
    assert_eq!(
        0, blacklist[0].begin,
        "Algorithm assumes there is nothing before first range"
    );
    let mut num_allowed = 0;
    // The lowest IP not yet known to be blocked
    let mut lowest = 0;
    for range in blacklist {
        // Sorted by where they begin, so anything between the ranges is a gap
        if (range.begin > lowest) {
            // The first gap already holds the lowest allowed IP
            if (!COUNT_ALLOWED) {
                return lowest;
            }
            num_allowed += range.begin - lowest;
        }
        // A range reaching the top of the address space blocks all the rest
        let Some(next) = range.end.checked_add(1) else {
            return num_allowed;
        };
        // Ranges already passed can reach further than this one does
        lowest = lowest.max(next);
    }
    // Everything above the last blocked range is allowed
    if (!COUNT_ALLOWED) {
        return lowest;
    }
    return num_allowed + (u32::MAX - lowest) + 1;
}

fn main() {
    println!("Part 1");
    let example = parse("day20.example");
    aoc::expect_result!(3, solve_case::<false>(&example));
    let input = parse("day20.input");
    aoc::expect_result!(4793564, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(4294967288, solve_case::<true>(&example));
    aoc::expect_result!(146, solve_case::<true>(&input));
}
