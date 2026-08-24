use itertools::Itertools;

/// The initial state (one bit per element),
/// and the length of the disk to fill
type Input = (Vec<u8>, usize);

fn parse_data(data: &str) -> Vec<u8> {
    data.bytes().map(|byte| byte - b'0').collect_vec()
}

fn parse(filename: &str) -> Input {
    let data = parse_data(aoc::file::read_string(filename).trim());
    // The example fills a much smaller disk than the real input
    let length = if data.len() <= 5 { 20 } else { 272 };
    (data, length)
}

/// Grow the data until it fills `length` bits, then cut off the excess
///
/// One step of the dragon curve is the data so far, a single 0,
/// then the data so far reversed and with every bit flipped
fn dragon_curve(data: &[u8], length: usize) -> Vec<u8> {
    let mut result = data.to_vec();
    while result.len() < length {
        result.reserve(result.len() + 1);
        result.push(0);
        for index in (0..(result.len() - 1)).rev() {
            result.push(1 - result[index]);
        }
    }
    result.truncate(length);
    result
}

/// Repeatedly pair up the bits, keeping a 1 where the pair matches,
/// until the result has an odd length
fn checksum(data: &[u8]) -> Vec<u8> {
    let mut result = data.to_vec();
    while (result.len() % 2) == 0 {
        result = result
            .chunks_exact(2)
            .map(|pair| u8::from(pair[0] == pair[1]))
            .collect_vec();
    }
    result
}

fn solve_case1((data, disk_length): &Input) -> String {
    // The checksum of the disk once it is filled with random-looking data
    checksum(&dragon_curve(data, *disk_length))
        .into_iter()
        .map(|b| (b + b'0') as char)
        .collect()
}

fn main() {
    println!("Asserts");
    assert_eq!(vec![1], parse_data("1"));
    assert_eq!(vec![1, 0, 0], dragon_curve(&parse_data("1"), 3));
    assert_eq!(vec![0, 0, 1], dragon_curve(&parse_data("0"), 3));
    assert_eq!(
        vec![1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0],
        dragon_curve(&parse_data("11111"), 11)
    );
    assert_eq!(
        vec![
            1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0
        ],
        dragon_curve(&parse_data("111100001010"), 25)
    );

    println!("Part 1");
    let example = parse("day16.example");
    aoc::expect_result!("01100", solve_case1(&example));
    let input = parse("day16.input");
    aoc::expect_result!("10100011010101011", solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
