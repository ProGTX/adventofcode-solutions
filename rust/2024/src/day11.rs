use rustc_hash::FxHashMap;

type BlinkKey = (u64, u64);
type BlinkCache = FxHashMap<BlinkKey, u64>;

fn parse(filename: &str) -> Vec<u64> {
    aoc::file::read_lines(filename)
        .last()
        .unwrap()
        .split(' ')
        .map(|s| s.parse().unwrap())
        .collect()
}

fn num_digits(n: u64) -> u32 {
    if n >= 10 { num_digits(n / 10) + 1 } else { 1 }
}

fn pown(x: u64, p: u32) -> u64 {
    if p == 0 {
        return 1;
    }
    let mut x = x;
    let mut p = p;
    let mut result: u64 = 1;
    loop {
        if p & 0x1 == 1 {
            result *= x;
        }
        p >>= 1;
        if p == 0 {
            break;
        }
        x *= x;
    }
    result
}

fn blink(stone: u64, blink_counter: u64, blink_cache: &mut BlinkCache) -> u64 {
    if blink_counter == 0 {
        return 1;
    }
    let cache_key: BlinkKey = (stone, blink_counter);
    if let Some(&num_stones) = blink_cache.get(&cache_key) {
        return num_stones;
    }
    let num_stones = if stone == 0 {
        blink(1, blink_counter - 1, blink_cache)
    } else {
        let digits = num_digits(stone);
        if digits % 2 == 0 {
            // The stone is replaced by two stones
            let divider = pown(10, digits / 2);
            // The left half of the digits are engraved on the new left stone
            // The right half of the digits are engraved on the new right stone
            blink(stone / divider, blink_counter - 1, blink_cache)
                + blink(stone % divider, blink_counter - 1, blink_cache)
        } else {
            blink(stone * 2024, blink_counter - 1, blink_cache)
        }
    };
    blink_cache.insert(cache_key, num_stones);
    num_stones
}

fn solve_case<const TIMES: u64>(stones: &[u64]) -> u64 {
    // We need to use dynamic programming to speed up the calculation
    let mut blink_cache = BlinkCache::default();
    let mut num_stones = 0;
    for &stone in stones {
        num_stones += blink(stone, TIMES, &mut blink_cache);
    }
    num_stones
}

fn main() {
    println!("Part 1");
    let example = parse("day11.example");
    aoc::expect_result!(55312, solve_case::<25>(&example));
    let input = parse("day11.input");
    aoc::expect_result!(191690, solve_case::<25>(&input));

    println!("Part 2");
    aoc::expect_result!(65601038650482, solve_case::<75>(&example));
    aoc::expect_result!(228651922369703, solve_case::<75>(&input));
}
