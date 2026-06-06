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

fn main() {
    println!("Part 1");
    aoc::expect_result!(8, solve_case1::<150>());
    aoc::expect_result!(831600, solve_case1::<36000000>());

    //println!("Part 2");
    //aoc::expect_result!(3, solve_case2());
    //aoc::expect_result!(1257, solve_case2());
}
