fn solve_case<const TARGET: u32, const PART2: bool>() -> u32 {
    let multiplier = if PART2 { 11u32 } else { 10u32 };
    let size = (TARGET / multiplier) as usize;
    let mut presents = vec![0u32; size + 1];
    for elf in 1..=size {
        let last = if PART2 { (elf * 50).min(size) } else { size };
        let mut house = elf;
        while house <= last {
            presents[house] += elf as u32 * multiplier;
            house += elf;
        }
    }
    presents
        .iter()
        .enumerate()
        .skip(1)
        .find(|(_, p)| **p >= TARGET)
        .map(|(i, _)| i as u32)
        .unwrap()
}

fn main() {
    println!("Part 1");
    aoc::expect_result!(8, solve_case::<150, false>());
    aoc::expect_result!(831600, solve_case::<36000000, false>());

    println!("Part 2");
    aoc::expect_result!(8, solve_case::<150, true>());
    aoc::expect_result!(884520, solve_case::<36000000, true>());
}
