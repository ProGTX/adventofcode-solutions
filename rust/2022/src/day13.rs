use itertools::Itertools;
use std::cmp::Ordering;

type PacketList = Vec<Packet>;

#[derive(Clone, Debug, PartialEq, Eq)]
enum Packet {
    Integer(u32),
    List(PacketList),
}
impl Ord for Packet {
    fn cmp(&self, other: &Self) -> Ordering {
        match (self, other) {
            (Packet::Integer(left), Packet::Integer(right)) => left.cmp(right),
            (Packet::List(left), Packet::List(right)) => left
                .iter()
                .zip(right)
                .fold(Ordering::Equal, |acc, (l, r)| acc.then(l.cmp(r)))
                .then(left.len().cmp(&right.len())),
            (Packet::Integer(left), Packet::List(_)) => {
                Packet::List(PacketList::from([Packet::Integer(*left)])).cmp(other)
            }
            (Packet::List(_), Packet::Integer(right)) => {
                self.cmp(&Packet::List(PacketList::from([Packet::Integer(*right)])))
            }
        }
    }
}
impl PartialOrd for Packet {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

type Input = Vec<(Packet, Packet)>;

fn parse_list_inner(s: &str) -> Packet {
    let mut list_stack = Vec::<PacketList>::new();
    list_stack.push(PacketList::new());
    let mut current_number = None;
    let push_number = |list_stack: &mut PacketList, current_number: &mut Option<u32>| {
        if let Some(num) = *current_number {
            list_stack.push(Packet::Integer(num));
        }
        *current_number = None;
    };
    for c in s.chars() {
        match c {
            '[' => {
                push_number(&mut list_stack.last_mut().unwrap(), &mut current_number);
                list_stack.push(PacketList::new());
            }
            ']' => {
                push_number(&mut list_stack.last_mut().unwrap(), &mut current_number);
                let last = list_stack.pop().unwrap();
                list_stack.last_mut().unwrap().push(Packet::List(last));
            }
            ',' => {
                push_number(&mut list_stack.last_mut().unwrap(), &mut current_number);
            }
            digit if (digit.is_digit(10)) => {
                let digit = digit.to_digit(10).unwrap();
                if let Some(num) = &mut current_number {
                    *num = *num * 10 + digit;
                } else {
                    current_number = Some(digit);
                }
            }
            _ => unreachable!("Invalid character in input"),
        };
    }
    push_number(&mut list_stack.last_mut().unwrap(), &mut current_number);
    return Packet::List(list_stack.last_mut().unwrap().to_vec());
}

fn parse(filename: &str) -> Input {
    aoc::file::read_string(filename)
        .trim()
        .split("\n\n")
        .map(|pair| {
            let (first, second) = pair.split_once('\n').unwrap();
            return (
                parse_list_inner(&first[1..first.len() - 1]),
                parse_list_inner(&second[1..second.len() - 1]),
            );
        })
        .collect()
}

fn solve_case1(input: &Input) -> usize {
    input
        .iter()
        .enumerate()
        .filter(|(_, (first, second))| first < second)
        .map(|(index, _)| index + 1)
        .sum()
}

fn solve_case2(input: &Input) -> usize {
    let mut packets = input
        .iter()
        .flat_map(|(first, second)| [first, second])
        .cloned()
        .collect_vec();
    let p2 = parse_list_inner("[2]");
    let p6 = parse_list_inner("[6]");
    packets.push(p2.clone());
    packets.push(p6.clone());
    packets.sort();
    return (packets.iter().position(|p| *p == p2).unwrap() + 1)
        * (packets.iter().position(|p| *p == p6).unwrap() + 1);
}

fn main() {
    println!("Part 1");
    let example = parse("day13.example");
    assert_eq!(13, solve_case1(&example));
    let input = parse("day13.input");
    assert_eq!(5198, solve_case1(&input));

    println!("Part 2");
    assert_eq!(140, solve_case2(&example));
    assert_eq!(22344, solve_case2(&input));
}
