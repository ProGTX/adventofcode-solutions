use itertools::Itertools;

#[derive(Default)]
struct IPv7 {
    inside: Vec<String>,
    outside: Vec<String>,
}

type Input = Vec<IPv7>;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let mut packet = IPv7::default();
            let mut by_open_bracket = line.split('[');
            packet
                .outside
                .push(by_open_bracket.next().unwrap().to_string());
            for part in by_open_bracket {
                let (inner, outer) = part.split_once(']').unwrap();
                packet.inside.push(inner.to_string());
                packet.outside.push(outer.to_string());
            }
            packet
        })
        .collect()
}

fn solve_case1(packets: &Input) -> usize {
    let abba = |s: &str| {
        s.bytes()
            .array_windows::<4>()
            .any(|w| (w[0] == w[3]) && (w[1] == w[2]) && (w[0] != w[1]))
    };
    packets
        .iter()
        .filter(|packet| {
            return true
                && packet.outside.iter().any(|s| abba(s))
                && !packet.inside.iter().any(|s| abba(s));
        })
        .count()
}

fn main() {
    println!("Part 1");
    let example = parse("day07.example");
    aoc::expect_result!(2, solve_case1(&example));
    let input = parse("day07.input");
    aoc::expect_result!(110, solve_case1(&input));

    println!("Part 2");
    //aoc::expect_result!(1337, solve_case2(&example));
    //aoc::expect_result!(1337, solve_case2(&input));
}
