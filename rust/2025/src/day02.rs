use itertools::Itertools;

type Range = std::ops::Range<u64>;

fn parse(filename: &str) -> Vec<Range> {
    type Point = aoc::point::Point<u64>;
    let mut ranges = aoc::file::read_string(filename)
        .trim()
        .split(',')
        .map(|range| {
            let point = Point::parse(range, "-").unwrap();
            Range {
                start: point.x,
                end: (point.y + 1),
            }
        })
        .collect::<Vec<_>>();
    ranges.sort_by_key(|r| r.start);
    ranges
}

fn solve_case1(ranges: &[Range]) -> u64 {
    ranges
        .iter()
        .map(|range| {
            range
                .clone()
                .into_iter()
                .filter(|id| {
                    let s = id.to_string();
                    let half = s.len() / 2;
                    return (s.len() % 2 == 0) && (&s[..half] == &s[half..]);
                })
                .sum::<u64>()
        })
        .sum()
}

fn solve_case2(ranges: &[Range]) -> u64 {
    ranges
        .iter()
        .map(|range| {
            range
                .clone()
                .into_iter()
                .filter(|&id| {
                    if (id < 11) {
                        return false;
                    }
                    let s = id.to_string();
                    let size = s.len();
                    let divisors = aoc::math::divisors(size as u64);
                    // Skip 1
                    return divisors[1..].iter().any(|&divisor| {
                        let chunks = s.chars().chunks(size / (divisor as usize));
                        let mut chunks_iter = chunks.into_iter();
                        let first = chunks_iter.next().unwrap().collect::<String>();
                        return chunks_iter.all(|chunk| chunk.collect::<String>() == first);
                    });
                })
                .sum::<u64>()
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day02.example");
    assert_eq!(1227775554, solve_case1(&example));
    let input = parse("day02.input");
    assert_eq!(30323879646, solve_case1(&input));

    println!("Part 2");
    assert_eq!(4174379265, solve_case2(&example));
    assert_eq!(43872163557, solve_case2(&input));
}
