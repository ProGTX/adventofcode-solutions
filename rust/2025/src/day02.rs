use arrayvec::ArrayVec;

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
    let all_divisors = {
        type DivisorStorage = ArrayVec<u64, 7>;
        let mut divisors = ArrayVec::<DivisorStorage, 11>::new();
        divisors.push(DivisorStorage::new()); // 0
        divisors.last_mut().unwrap().push(1);
        for div in 1..divisors.capacity() {
            divisors.push(
                aoc::math::divisors(div as u64)
                    .iter()
                    .map(|&v| v)
                    .collect::<DivisorStorage>(),
            );
        }
        divisors
    };
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
                    let divisors = &all_divisors[size];
                    // Skip 1
                    return divisors[1..].iter().any(|&divisor| {
                        let chunks = s.as_bytes().chunks(size / (divisor as usize));
                        let mut chunks_it = chunks.into_iter();
                        let first = str::from_utf8(chunks_it.next().unwrap()).unwrap();
                        return chunks_it.all(|chunk| {
                            let current = str::from_utf8(chunk).unwrap();
                            current == first
                        });
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
