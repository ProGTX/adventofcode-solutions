struct Range {
    lo: i32,
    hi: i32,
}
impl Range {
    fn contains(&self, other: &Range) -> bool {
        self.lo <= other.lo && other.hi <= self.hi
    }
    fn overlaps_with(&self, other: &Range) -> bool {
        self.lo <= other.hi && other.lo <= self.hi
    }
}

fn parse_range(s: &str) -> Range {
    let (lo, hi) = s.split_once('-').unwrap();
    Range {
        lo: lo.parse().unwrap(),
        hi: hi.parse().unwrap(),
    }
}

type Input = Vec<(Range, Range)>;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
        .into_iter()
        .map(|line| {
            let (a, b) = line.split_once(',').unwrap();
            (parse_range(a), parse_range(b))
        })
        .collect()
}

fn solve_case1(input: &Input) -> usize {
    input
        .iter()
        .filter(|(a, b)| a.contains(b) || b.contains(a))
        .count()
}

fn solve_case2(input: &Input) -> usize {
    input.iter().filter(|(a, b)| a.overlaps_with(b)).count()
}

fn main() {
    println!("Part 1");
    let example = parse("day04.example");
    assert_eq!(2, solve_case1(&example));
    let input = parse("day04.input");
    assert_eq!(582, solve_case1(&input));

    println!("Part 2");
    assert_eq!(4, solve_case2(&example));
    assert_eq!(893, solve_case2(&input));
}
