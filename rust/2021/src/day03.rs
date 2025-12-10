type Bitset = arrayvec::ArrayVec<u8, 12>;
type Counter = arrayvec::ArrayVec<i16, 12>;

fn parse(filename: &str) -> Vec<Bitset> {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            line.chars()
                .map(|c| c.to_digit(10).unwrap() as u8)
                .collect::<Bitset>()
        })
        .collect::<Vec<_>>()
}

fn count_bits(report: &[Bitset]) -> Counter {
    let init = (0..report[0].len()).map(|_| 0).collect::<Counter>();
    report.iter().fold(init, |acc, bitset| {
        acc.iter()
            .zip(bitset)
            .map(|(count, bit)| count + (*bit as i16) - ((*bit == 0) as i16))
            .collect::<Counter>()
    })
}

fn to_decimal(bitset: &Bitset) -> u32 {
    aoc::math::binary_to_number(bitset)
}

fn solve_case1(report: &[Bitset]) -> u32 {
    let gamma = count_bits(report)
        .iter()
        .map(|count| (*count > 0) as u8)
        .collect::<Bitset>();
    let epsilon = gamma
        .iter()
        .map(|bit| (*bit == 0) as u8)
        .collect::<Bitset>();
    return to_decimal(&gamma) * to_decimal(&epsilon);
}

fn solve_case2(report: &[Bitset]) -> u32 {
    type Report = Vec<Bitset>;
    let mut oxy_gen = Report::from(report);
    let mut co2_scrubber = Report::from(report);
    for index in 0..report[0].len() {
        if (oxy_gen.len() > 1) {
            let oxy_count = count_bits(&oxy_gen);
            oxy_gen = oxy_gen
                .iter()
                .filter(|&bitset| (oxy_count[index] >= 0) == (bitset[index] > 0))
                .cloned()
                .collect::<Report>();
        }
        if (co2_scrubber.len() > 1) {
            let co2_count = count_bits(&co2_scrubber);
            co2_scrubber = co2_scrubber
                .iter()
                .filter(|&bitset| (co2_count[index] >= 0) != (bitset[index] > 0))
                .cloned()
                .collect::<Report>();
        }
    }
    return to_decimal(&oxy_gen[0]) * to_decimal(&co2_scrubber[0]);
}

fn main() {
    println!("Part 1");
    let example = parse("day03.example");
    assert_eq!(198, solve_case1(&example));
    let input = parse("day03.input");
    assert_eq!(4138664, solve_case1(&input));

    println!("Part 2");
    assert_eq!(230, solve_case2(&example));
    assert_eq!(4273224, solve_case2(&input));
}
