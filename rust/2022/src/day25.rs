fn parse(filename: &str) -> Vec<String> {
    aoc::file::read_lines(filename)
}

fn snafu_char_to_int(ch: char) -> i64 {
    match ch {
        '-' => -1,
        '=' => -2,
        '0'..='2' => (ch as u8 - b'0') as i64,
        _ => panic!("Invalid SNAFU character: {ch}"),
    }
}

fn int_to_snafu_char(value: i64) -> char {
    match value {
        -1 => '-',
        -2 => '=',
        0..=2 => (b'0' + value as u8) as char,
        _ => panic!("Invalid SNAFU digit: {value}"),
    }
}

fn to_decimal(snafu: &str) -> i64 {
    snafu
        .chars()
        .rev()
        .zip(std::iter::successors(Some(1i64), |&m| Some(m * 5)))
        .map(|(ch, multiplier)| snafu_char_to_int(ch) * multiplier)
        .sum()
}

fn sum_snafus(lhs: &str, rhs: &str) -> String {
    let lhs_reversed: Vec<char> = lhs.chars().rev().collect();
    let rhs_reversed: Vec<char> = rhs.chars().rev().collect();
    let max_len = lhs_reversed.len().max(rhs_reversed.len());
    let mut carry = 0i64;
    let mut output = Vec::with_capacity(max_len + 1);
    for idx in 0..=max_len {
        let lhs_digit = lhs_reversed
            .get(idx)
            .map(|&ch| snafu_char_to_int(ch))
            .unwrap_or(0);
        let rhs_digit = rhs_reversed
            .get(idx)
            .map(|&ch| snafu_char_to_int(ch))
            .unwrap_or(0);
        let digit_sum = lhs_digit + rhs_digit + carry;
        carry = (digit_sum as f64 / 5.0).round() as i64;
        output.push(int_to_snafu_char(digit_sum - carry * 5));
    }
    output.reverse();
    output
        .into_iter()
        .collect::<String>()
        .trim_start_matches('0')
        .to_string()
}

fn to_snafu(mut number: i64) -> String {
    let mut digits = Vec::new();
    while number != 0 {
        let remainder = number.rem_euclid(5);
        let (digit, carry) = match remainder {
            3 => (-2i64, 1i64),
            4 => (-1, 1),
            r => (r, 0),
        };
        digits.push(int_to_snafu_char(digit));
        number = number / 5 + carry;
    }
    if digits.is_empty() {
        "0".to_string()
    } else {
        digits.into_iter().rev().collect()
    }
}

fn solve(input: &[String]) -> String {
    input
        .iter()
        .skip(1)
        .fold(input[0].clone(), |acc, rhs| sum_snafus(&acc, rhs))
}

fn main() {
    // Unit tests
    aoc::expect_result!("1=21", sum_snafus(&to_snafu(49), &to_snafu(37)));

    aoc::expect_result!("2=-01", to_snafu(976));
    aoc::expect_result!(976_i64, to_decimal("2=-01"));
    aoc::expect_result!("1=-0-2", to_snafu(1747));
    aoc::expect_result!(1747_i64, to_decimal("1=-0-2"));
    aoc::expect_result!("12111", to_snafu(906));
    aoc::expect_result!(906_i64, to_decimal("12111"));
    aoc::expect_result!("2=0=", to_snafu(198));
    aoc::expect_result!(198_i64, to_decimal("2=0="));
    aoc::expect_result!("21", to_snafu(11));
    aoc::expect_result!(11_i64, to_decimal("21"));
    aoc::expect_result!("2=01", to_snafu(201));
    aoc::expect_result!(201_i64, to_decimal("2=01"));
    aoc::expect_result!("111", to_snafu(31));
    aoc::expect_result!(31_i64, to_decimal("111"));
    aoc::expect_result!("20012", to_snafu(1257));
    aoc::expect_result!(1257_i64, to_decimal("20012"));
    aoc::expect_result!("112", to_snafu(32));
    aoc::expect_result!(32_i64, to_decimal("112"));
    aoc::expect_result!("1=-1=", to_snafu(353));
    aoc::expect_result!(353_i64, to_decimal("1=-1="));
    aoc::expect_result!("1-12", to_snafu(107));
    aoc::expect_result!(107_i64, to_decimal("1-12"));
    aoc::expect_result!("12", to_snafu(7));
    aoc::expect_result!(7_i64, to_decimal("12"));
    aoc::expect_result!("1=", to_snafu(3));
    aoc::expect_result!(3_i64, to_decimal("1="));
    aoc::expect_result!("122", to_snafu(37));
    aoc::expect_result!(37_i64, to_decimal("122"));

    println!("Part 1");
    let example = parse("day25.example");
    aoc::expect_result!("2=-1=0", solve(&example));
    let input = parse("day25.input");
    aoc::expect_result!("2-=12=2-2-2-=0012==2", solve(&input));
}
