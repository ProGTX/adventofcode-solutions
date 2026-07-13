use rustc_hash::FxHashSet;

const NUM_REPEAT: i32 = 2000;
const PATTERN_SIZE: usize = 4;
const MAX_DIFF_ABS: i32 = 9;
const DIFF_OPTS: i32 = 2 * MAX_DIFF_ABS + 1;

fn parse(filename: &str) -> Vec<i32> {
    aoc::file::read_numbers::<i32>(filename)
}

fn mix(lhs: i64, rhs: i64) -> i64 {
    lhs ^ rhs
}

fn prune(secret: i64) -> i32 {
    (secret % 16777216) as i32
}

fn next_secret(secret: i32) -> i32 {
    let secret = prune(mix(secret as i64, secret as i64 * 64));
    let secret = prune(mix(secret as i64, secret as i64 / 32));
    let secret = prune(mix(secret as i64, secret as i64 * 2048));
    secret
}

fn next_secret_repeat(secret: i32) -> i32 {
    let mut secret = secret;
    for _ in 0..NUM_REPEAT {
        secret = next_secret(secret);
    }
    secret
}

fn sum_secrets(secrets: &[i32]) -> i64 {
    secrets.iter().map(|&s| next_secret_repeat(s) as i64).sum()
}

fn get_pattern(diffs: &[i32; PATTERN_SIZE]) -> i32 {
    (diffs[0] + MAX_DIFF_ABS) * (DIFF_OPTS * DIFF_OPTS * DIFF_OPTS)
        + (diffs[1] + MAX_DIFF_ABS) * (DIFF_OPTS * DIFF_OPTS)
        + (diffs[2] + MAX_DIFF_ABS) * DIFF_OPTS
        + (diffs[3] + MAX_DIFF_ABS)
}

fn get_value_total() -> Vec<i32> {
    let max_pattern = get_pattern(&[9, 9, 9, 9]);
    vec![0; (max_pattern + 1) as usize]
}

fn rotate_left(diffs: &mut [i32; PATTERN_SIZE]) {
    let first = diffs[0];
    for i in 0..PATTERN_SIZE - 1 {
        diffs[i] = diffs[i + 1];
    }
    diffs[PATTERN_SIZE - 1] = first;
}

// See https://www.reddit.com/r/adventofcode/comments/1hk15et/comment/m3asuqa/
// for inspiration
fn most_bananas(secrets: &[i32]) -> i32 {
    let mut buyer_has_pattern = FxHashSet::default();
    let mut value_total = get_value_total();

    for &starting_secret in secrets {
        buyer_has_pattern.clear();
        let mut diffs = [0i32; PATTERN_SIZE];
        let mut secret = starting_secret;
        let mut previous = secret % 10;
        for i in 0..NUM_REPEAT {
            secret = next_secret(secret);
            let current = secret % 10;
            diffs[PATTERN_SIZE - 1] = current - previous;
            if i >= (PATTERN_SIZE as i32 - 1) {
                let pattern = get_pattern(&diffs);
                if buyer_has_pattern.insert(pattern) {
                    value_total[pattern as usize] += current;
                }
            }
            rotate_left(&mut diffs);
            previous = current;
        }
    }
    *value_total.iter().max().unwrap()
}

fn solve_case1(buyer_starters: &[i32]) -> i64 {
    sum_secrets(buyer_starters)
}

fn solve_case2(buyer_starters: &[i32]) -> i64 {
    most_bananas(buyer_starters) as i64
}

fn main() {
    println!("Asserts");
    assert_eq!(15887950, next_secret(123));
    assert_eq!(16495136, next_secret(15887950));
    assert_eq!(527345, next_secret(16495136));
    assert_eq!(704524, next_secret(527345));
    assert_eq!(1553684, next_secret(704524));
    assert_eq!(12683156, next_secret(1553684));
    assert_eq!(11100544, next_secret(12683156));
    assert_eq!(12249484, next_secret(11100544));
    assert_eq!(7753432, next_secret(12249484));
    assert_eq!(5908254, next_secret(7753432));

    assert_eq!(8685429, next_secret_repeat(1));
    assert_eq!(4700978, next_secret_repeat(10));
    assert_eq!(15273692, next_secret_repeat(100));
    assert_eq!(8667524, next_secret_repeat(2024));

    println!("Part 1");
    let example = parse("day22.example");
    aoc::expect_result!(37327623, solve_case1(&example));
    let input = parse("day22.input");
    aoc::expect_result!(20215960478i64, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(9, most_bananas(&[123]));
    aoc::expect_result!(23, most_bananas(&[1, 2, 3, 2024]));
    aoc::expect_result!(24, solve_case2(&example));
    aoc::expect_result!(2221, solve_case2(&input));
}
