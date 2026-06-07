fn solve_case1<const TARGET: u32>() -> u32 {
    debug_assert!(TARGET > 10);
    for house in 2.. {
        let presents = 10 * aoc::math::divisors(house).iter().sum::<u32>();
        if (presents >= TARGET) {
            return house;
        }
    }
    unreachable!();
}

fn num_presents(house_id: u32) -> u32 {
    11 * aoc::math::divisors(house_id)
        .iter()
        .filter(|&divisor| house_id <= 50 * divisor)
        .sum::<u32>()
}

fn solve_case2<const TARGET: u32>() -> u32 {
    for house in 2.. {
        if num_presents(house) >= TARGET {
            return house;
        }
    }
    unreachable!();
}

fn main() {
    println!("Part 1");
    aoc::expect_result!(8, solve_case1::<150>());
    aoc::expect_result!(831600, solve_case1::<36000000>());

    println!("Part 2");
    aoc::expect_result!(8, solve_case2::<150>());
    aoc::expect_result!(884520, solve_case2::<36000000>());
}
