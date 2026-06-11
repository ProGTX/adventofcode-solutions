type Pattern = Vec<String>;
type Input = Vec<Pattern>;

fn parse(filename: &str) -> Input {
    let mut result = Vec::new();
    let mut pattern = Pattern::new();
    for line in aoc::file::read_lines(filename) {
        if line.is_empty() {
            result.push(std::mem::take(&mut pattern));
        } else {
            pattern.push(line);
        }
    }
    result.push(pattern);
    result
}

const ROCK: u8 = b'#';

// We convert a line to a number by assuming a binary representation,
// where ash is 0 and rock is 1
// But we always add an extra 1 bit at the end to ensure a fixed end bit
fn transform_to_number(line: &str) -> u32 {
    let mut num = 0u32;
    for (i, &c) in line.as_bytes().iter().enumerate() {
        if c == ROCK {
            num |= 1 << i;
        }
    }
    num + (1 << line.len())
}

fn transformed_number_size(number: u32) -> u32 {
    // Important to reduce by one because of the extra guard bit
    u32::BITS - number.leading_zeros() - 1
}

fn to_numbers(pattern: &[String]) -> Vec<u32> {
    pattern
        .iter()
        .map(|line| transform_to_number(line))
        .collect()
}

fn transpose(pattern: &[String]) -> Vec<String> {
    let number_size = pattern[0].len();
    (0..number_size)
        .map(|column| {
            pattern
                .iter()
                .map(|row| row.as_bytes()[column] as char)
                .collect()
        })
        .collect()
}

fn check_index(number: u32, index: u32) -> bool {
    if index == 0 {
        return true;
    }
    let number_size = transformed_number_size(number);
    let sub_size = index.min(number_size - index);
    let mut mask_lhs = 1u32 << (index - 1);
    let mut mask_rhs = mask_lhs << 1;
    for _ in 0..sub_size {
        if (number & mask_lhs != 0) != (number & mask_rhs != 0) {
            return false;
        }
        mask_lhs >>= 1;
        mask_rhs <<= 1;
    }
    true
}

fn reflection_points(number: u32) -> Vec<u32> {
    let number_size = transformed_number_size(number);
    let mut refl_points = vec![0u32; number_size as usize];
    for index in 1..number_size {
        if check_index(number, index) {
            refl_points[index as usize] = 1;
        }
    }
    refl_points
}

fn reflection_counts(pattern: &[u32]) -> Vec<u32> {
    let number_size = transformed_number_size(pattern[0]);
    let mut refl_points = vec![0u32; number_size as usize];
    for &number in pattern {
        let local_refl_points = reflection_points(number);
        for i in 0..number_size as usize {
            refl_points[i] += local_refl_points[i];
        }
    }
    refl_points
}

fn reflection_point(pattern: &[u32]) -> i32 {
    debug_assert!(!pattern.is_empty(), "Cannot work without lines");
    let number_size = transformed_number_size(pattern[0]);
    debug_assert!(number_size > 0, "Cannot work with empty lines");

    let refl_points = reflection_counts(pattern);

    let mut refl_map: Vec<(u32, u32)> = (0..number_size)
        .map(|i| (i, refl_points[i as usize]))
        .collect();
    refl_map.sort_unstable_by(|a, b| b.1.cmp(&a.1));
    if (refl_map[0].1 as usize) < pattern.len() {
        return 0;
    }
    // In some cases two matches can be found
    // In that case select the one that's closer to the center
    if refl_map[0].1 == refl_map[1].1 {
        let center = number_size / 2;
        let dist_lhs = (refl_map[0].0 as i32 - center as i32).abs();
        let dist_rhs = (refl_map[1].0 as i32 - center as i32).abs();
        return if dist_lhs < dist_rhs {
            refl_map[0].0 as i32
        } else {
            refl_map[1].0 as i32
        };
    }
    refl_map[0].0 as i32
}

// Unlike reflection_point, this doesn't apply
// the "closer to center" tie-break:
// for unsmudging we need to know about *any* reflection point
// other than ignore_p, even if the original one is still also a full match
// and would otherwise win the tie-break.
fn find_new_reflection_point(pattern: &[u32], ignore_p: i32) -> i32 {
    let refl_points = reflection_counts(pattern);
    for (index, &count) in refl_points.iter().enumerate().skip(1) {
        if (count as usize == pattern.len()) && (index as i32 != ignore_p) {
            return index as i32;
        }
    }
    0
}

fn unsmudge_case(pattern: &mut [u32], ignore_p: i32) -> i32 {
    debug_assert!(!pattern.is_empty(), "Cannot work without lines");
    let number_size = transformed_number_size(pattern[0]);
    debug_assert!(number_size > 0, "Cannot work with empty lines");
    for row in 0..pattern.len() {
        for column in 0..number_size {
            pattern[row] ^= 1 << column;
            let p = find_new_reflection_point(pattern, ignore_p);
            pattern[row] ^= 1 << column;
            if p > 0 {
                return p;
            }
        }
    }
    0
}

fn check_pattern<const UNSMUDGE: bool>(pattern: &[String]) -> i32 {
    let mut pattern_numbers = to_numbers(pattern);
    let mut transposed_numbers = to_numbers(&transpose(pattern));

    let p1 = reflection_point(&pattern_numbers);
    let p1t = 100 * reflection_point(&transposed_numbers);

    if !UNSMUDGE {
        p1 + p1t
    } else {
        let p2 = unsmudge_case(&mut pattern_numbers, p1);
        let p2t = 100 * unsmudge_case(&mut transposed_numbers, p1t / 100);
        p2 + p2t
    }
}

fn solve_case<const UNSMUDGE: bool>(input: &Input) -> i32 {
    input
        .iter()
        .map(|pattern| check_pattern::<UNSMUDGE>(pattern))
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day13.example");
    aoc::expect_result!(405, solve_case::<false>(&example));
    let example2 = parse("day13.example2");
    aoc::expect_result!(712, solve_case::<false>(&example2));
    let input = parse("day13.input");
    aoc::expect_result!(31265, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(400, solve_case::<true>(&example));
    aoc::expect_result!(1415, solve_case::<true>(&example2));
    aoc::expect_result!(39359, solve_case::<true>(&input));
}
