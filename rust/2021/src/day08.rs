type Display = arrayvec::ArrayVec<u32, 7>;
type Pattern = [Display; 10];
type Output = [Display; 4];
type Possibility = std::collections::HashSet<u32>;

fn parse(filename: &str) -> (Vec<Pattern>, Vec<Output>) {
    let mut patterns = Vec::new();
    let mut outputs = Vec::new();
    let parse_display =
        |s: &str| -> Display { s.chars().map(|c| c as u32 - b'a' as u32).collect() };
    for line in aoc::file::read_lines(filename) {
        let (pattern_str, output_str) = line.split_once(" | ").unwrap();
        let mut pattern_it = pattern_str.split(' ');
        patterns.push(core::array::from_fn(|_| {
            parse_display(pattern_it.next().unwrap())
        }));
        let mut output_it = output_str.split(' ');
        outputs.push(core::array::from_fn(|_| {
            parse_display(output_it.next().unwrap())
        }));
    }
    (patterns, outputs)
}

const DISPLAY_LENGTHS: [u32; 10] = [6, 2, 5, 5, 4, 5, 6, 3, 7, 6];
const UNIQUE_DISPLAYS: [u32; 4] = [
    DISPLAY_LENGTHS[1],
    DISPLAY_LENGTHS[4],
    DISPLAY_LENGTHS[7],
    DISPLAY_LENGTHS[8],
];

fn solve_case1((_, outputs): &(Vec<Pattern>, Vec<Output>)) -> u32 {
    outputs
        .iter()
        .map(|output_list| {
            output_list
                .iter()
                .filter(|&out| UNIQUE_DISPLAYS.contains(&(out.len() as u32)))
                .count() as u32
        })
        .sum()
}

fn to_display(arr: &[u32]) -> Display {
    arr.iter().map(|&v| v).collect()
}

fn to_possibility(display: &Display) -> Possibility {
    display.iter().map(|&v| v).collect()
}

fn solve_case2((patterns, outputs): &(Vec<Pattern>, Vec<Output>)) -> u32 {
    let display_mappings = [
        to_display(&[0, 1, 2, 4, 5, 6]),    // 0
        to_display(&[2, 5]),                // 1
        to_display(&[0, 2, 3, 4, 6]),       // 2
        to_display(&[0, 2, 3, 5, 6]),       // 3
        to_display(&[1, 2, 3, 5]),          // 4
        to_display(&[0, 1, 3, 5, 6]),       // 5
        to_display(&[0, 1, 3, 4, 5, 6]),    // 6
        to_display(&[0, 2, 5]),             // 7
        to_display(&[0, 1, 2, 3, 4, 5, 6]), // 8
        to_display(&[0, 1, 2, 3, 5, 6]),    // 9
    ];
    // Indicates possible mappings for each size of display
    let mut size_options = core::array::from_fn::<_, 8, _>(|_| Possibility::default());
    for mapping in display_mappings {
        size_options[mapping.len()] = size_options[mapping.len()]
            .union(&to_possibility(&mapping))
            .map(|&v| v)
            .collect::<Possibility>();
    }
    let size_options = size_options;
    patterns
        .iter()
        .zip(outputs.iter())
        .map(|(pattern_list, output_list)| {
            let mut display_options =
                core::array::from_fn::<_, 7, _>(|_| Possibility::from([0, 1, 2, 3, 4, 5, 6]));
            for pattern in pattern_list {
                for &segment in &size_options[pattern.len()] {
                    display_options[segment as usize] = display_options[segment as usize]
                        .intersection(&to_possibility(pattern))
                        .map(|&v| v)
                        .collect::<Possibility>();
                }
            }
            println!("Options: ");
            for (index, opt) in display_options.iter().enumerate() {
                println!("  {}: {:?}", index, opt);
            }
            0
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day08.example");
    assert_eq!(0, solve_case1(&example));
    let example2 = parse("day08.example2");
    assert_eq!(26, solve_case1(&example2));
    let input = parse("day08.input");
    assert_eq!(440, solve_case1(&input));

    println!("Part 2");
    assert_eq!(5353, solve_case2(&example));
    assert_eq!(61229, solve_case2(&example2));
    assert_eq!(1046281, solve_case2(&input));
}
