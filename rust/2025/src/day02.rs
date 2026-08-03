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

/// Writes n's decimal digits into buf and returns the slice holding them.
///
/// Both parts format every id in every range, and String would heap-allocate
/// and free on each one where this reuses a stack buffer.
fn digits(buf: &mut [u8; 20], mut n: u64) -> &[u8] {
    let mut first = buf.len();
    loop {
        first -= 1;
        buf[first] = b'0' + (n % 10) as u8;
        n /= 10;
        if n == 0 {
            break;
        }
    }
    &buf[first..]
}

fn solve_case1(ranges: &[Range]) -> u64 {
    ranges
        .iter()
        .map(|range| {
            range
                .clone()
                .into_iter()
                .filter(|&id| {
                    let mut buf = [0u8; 20];
                    let s = digits(&mut buf, id);
                    let half = s.len() / 2;
                    return (s.len() % 2 == 0) && (s[..half] == s[half..]);
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
                    let mut buf = [0u8; 20];
                    let s = digits(&mut buf, id);
                    let size = s.len();
                    let divisors = &all_divisors[size];
                    // Skip 1
                    return divisors[1..].iter().any(|&divisor| {
                        let mut chunks = s.chunks(size / (divisor as usize));
                        let first = chunks.next().unwrap();
                        return chunks.all(|chunk| chunk == first);
                    });
                })
                .sum::<u64>()
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day02.example");
    aoc::expect_result!(1227775554, solve_case1(&example));
    let input = parse("day02.input");
    aoc::expect_result!(30323879646, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(4174379265, solve_case2(&example));
    aoc::expect_result!(43872163557, solve_case2(&input));
}
