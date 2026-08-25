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

/// The lowest IP that is not blocked by any of the ranges
fn solve_case1(blacklist: &Input) -> u32 {
    assert_eq!(
        0, blacklist[0].begin,
        "Algorithm assumes there is nothing before first range"
    );
    let mut lowest = 0;
    for range in blacklist {
        // The ranges are sorted by where they begin,
        // so once one starts past the lowest IP still allowed,
        // no later range can cover it either
        if (range.begin > lowest) {
            break;
        }
        // Ranges already passed can reach further than this one does
        lowest = lowest.max(range.end + 1);
    }
    return lowest;
}

fn main() {
    println!("Part 1");
    let example = parse("day20.example");
    aoc::expect_result!(3, solve_case1(&example));
    let input = parse("day20.input");
    aoc::expect_result!(4793564, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
