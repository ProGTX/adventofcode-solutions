type Segment = arrayvec::ArrayVec<u32, 7>;
type Pattern = [Segment; 10];
type Output = [Segment; 4];

fn parse_segment(s: &str) -> Segment {
    s.chars().map(|c| c as u32 - b'a' as u32).collect()
}

fn parse(filename: &str) -> (Vec<Pattern>, Vec<Output>) {
    let mut patterns = Vec::new();
    let mut outputs = Vec::new();
    for line in aoc::file::read_lines(filename) {
        let (pattern_str, output_str) = line.split_once(" | ").unwrap();
        let mut pattern_it = pattern_str.split(' ');
        patterns.push(core::array::from_fn(|_| {
            parse_segment(pattern_it.next().unwrap())
        }));
        let mut output_it = output_str.split(' ');
        outputs.push(core::array::from_fn(|_| {
            parse_segment(output_it.next().unwrap())
        }));
    }
    (patterns, outputs)
}

const SEGMENT_LENGTHS: [u32; 10] = [6, 2, 5, 5, 4, 5, 6, 3, 7, 6];

fn solve_case1((_, outputs): &(Vec<Pattern>, Vec<Output>)) -> u32 {
    const UNIQUE_SEGMENTS: [u32; 4] = [
        SEGMENT_LENGTHS[1],
        SEGMENT_LENGTHS[4],
        SEGMENT_LENGTHS[7],
        SEGMENT_LENGTHS[8],
    ];
    outputs
        .iter()
        .map(|output| {
            output
                .iter()
                .filter(|&out| UNIQUE_SEGMENTS.contains(&(out.len() as u32)))
                .count() as u32
        })
        .sum::<u32>()
}

fn main() {
    println!("Part 1");
    let example = parse("day08.example");
    assert_eq!(0, solve_case1(&example));
    let example2 = parse("day08.example2");
    assert_eq!(26, solve_case1(&example2));
    let input = parse("day08.input");
    assert_eq!(440, solve_case1(&input));

    // println!("Part 2");
    // assert_eq!(5353, solve_case2(&example));
    // assert_eq!(61229, solve_case2(&example2));
    // assert_eq!(1046281, solve_case2(&input));
}
