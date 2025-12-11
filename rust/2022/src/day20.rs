fn parse(filename: &str) -> Vec<i64> {
    aoc::file::read_numbers(filename)
}

fn solve_case<const NUM_MIXES: usize, const DECRYPTION_KEY: i64>(numbers: &[i64]) -> i64 {
    let numbers = numbers
        .iter()
        .map(|num| num * DECRYPTION_KEY)
        .enumerate()
        .collect::<Vec<_>>();
    let mut mixed = numbers.clone();

    for _ in 0..NUM_MIXES {
        for &(orig_index, orig_value) in numbers.iter() {
            if (orig_value == 0) {
                continue;
            }
            let current_ipos = mixed
                .iter()
                .position(|(index, _)| *index == orig_index)
                .unwrap() as i64;
            let next_ipos = if (orig_value == 0) {
                current_ipos
            } else {
                // Very important to subtract one
                (current_ipos + orig_value).rem_euclid(mixed.len() as i64 - 1)
            };
            let distance = next_ipos - current_ipos;
            let current_pos = current_ipos as usize;
            let next_pos = next_ipos as usize;
            if (distance < 0) {
                // shift_right
                mixed.copy_within(next_pos..current_pos, next_pos + 1);
            } else {
                // shift_left
                mixed.copy_within(current_pos + 1..next_pos + 1, current_pos);
            }
            mixed[next_ipos as usize] = (orig_index, orig_value);
        }
    }

    let mut pos = mixed.iter().position(|(_, value)| *value == 0).unwrap();
    return (0..3)
        .map(|_| {
            pos = (pos + 1000) % mixed.len();
            return mixed[pos].1;
        })
        .sum();
}

fn main() {
    println!("Part 1");
    let example = parse("day20.example");
    assert_eq!(3, solve_case::<1, 1>(&example));
    let input = parse("day20.input");
    assert_eq!(1591, solve_case::<1, 1>(&input));

    println!("Part 2");
    assert_eq!(1623178306, solve_case::<10, 811589153>(&example));
    assert_eq!(14579387544492, solve_case::<10, 811589153>(&input));
}
