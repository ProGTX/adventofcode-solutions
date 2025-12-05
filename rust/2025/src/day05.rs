type Range = std::ops::Range<u64>;
type Input = (Vec<Range>, Vec<u64>);

fn parse(filename: &str) -> Input {
    type Point = aoc::point::Point<u64>;
    let lines = aoc::file::read_lines(filename);
    let mut empty_line_index = 0;

    let mut ranges = lines
        .iter()
        .take_while(|&line| !line.is_empty())
        .map(|range| {
            let point = Point::parse(range, "-").unwrap();
            empty_line_index += 1;
            Range {
                start: point.x,
                end: (point.y + 1),
            }
        })
        .collect::<Vec<_>>();
    ranges.sort_by_key(|r| r.start);

    // Remove overlap from ranges
    // Needed by part 2, but also helps part 1 be faster
    ranges = {
        let mut ranges2 = Vec::new();
        let mut last_end = 0;
        for r in ranges.iter_mut() {
            if (last_end >= r.end) {
                continue;
            }
            if (last_end > r.start) {
                r.start = last_end;
            }
            last_end = r.end;
            ranges2.push(r.clone());
        }
        ranges2
    };

    let mut ids = lines
        .iter()
        .skip(empty_line_index + 1)
        .map(|id| id.parse::<u64>().unwrap())
        .collect::<Vec<_>>();
    ids.sort();

    (ranges, ids)
}

fn solve_case1((ranges, ids): &Input) -> usize {
    ids.iter()
        .filter(|&&id| {
            ranges
                .iter()
                .any(|range| (id >= range.start) && (id < range.end))
        })
        .count()
}

fn solve_case2((ranges, _): &Input) -> usize {
    ranges
        .iter()
        .map(|range| (range.end - range.start) as usize)
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day05.example");
    assert_eq!(3, solve_case1(&example));
    let input = parse("day05.input");
    assert_eq!(821, solve_case1(&input));

    println!("Part 2");
    assert_eq!(14, solve_case2(&example));
    assert_eq!(344771884978261, solve_case2(&input));
}
