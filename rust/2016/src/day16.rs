use itertools::Itertools;

/// The initial state, one bit per element
type Input = Vec<u8>;

fn parse_data(data: &str) -> Input {
    data.bytes().map(|byte| byte - b'0').collect_vec()
}

fn parse(filename: &str) -> Input {
    parse_data(aoc::file::read_string(filename).trim())
}

/// Grow the data until it fills `length` bits, then cut off the excess
///
/// One step of the dragon curve is the data so far, a single 0,
/// then the data so far reversed and with every bit flipped
fn dragon_curve(data: &[u8], length: usize) -> Vec<u8> {
    // The whole disk is allocated up front,
    // and no step is ever grown past it
    let mut result = Vec::with_capacity(length.max(data.len()));
    result.extend_from_slice(data);
    while result.len() < length {
        let old_len = result.len();
        result.push(0);
        for index in (0..old_len).rev() {
            if result.len() == length {
                break;
            }
            result.push(1 - result[index]);
        }
    }
    result.truncate(length);
    result
}

/// Repeatedly pair up the bits, keeping a 1 where the pair matches,
/// until the result has an odd length
fn checksum(mut data: Vec<u8>) -> Vec<u8> {
    // Each round is written over the front of the same buffer
    while (data.len() % 2) == 0 {
        let half = data.len() / 2;
        for index in 0..half {
            data[index] = u8::from(data[2 * index] == data[(2 * index) + 1]);
        }
        data.truncate(half);
    }
    data
}

fn solve_case<const DISK_LENGTH: usize>(data: &Input) -> String {
    // The checksum of the disk once it is filled with random-looking data
    checksum(dragon_curve(data, DISK_LENGTH))
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
    aoc::expect_result!("01100", solve_case::<20>(&example));
    aoc::expect_result!("11010011110011010", solve_case::<272>(&example));
    let input = parse("day16.input");
    aoc::expect_result!("10100011010101011", solve_case::<272>(&input));

    println!("Part 2");
    aoc::expect_result!("10111110011110111", solve_case::<35651584>(&example));
    aoc::expect_result!("01010001101011001", solve_case::<35651584>(&input));
}
