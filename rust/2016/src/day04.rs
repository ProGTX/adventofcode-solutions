use aoc::iter::CollectArrayVec;
use itertools::Itertools;

struct Room {
    name: String,
    sector: usize,
    checksum: [char; 5],
}

// This char chosen because it appears before 'a'
const SEPARATOR: char = '`';

type Input = Vec<Room>;

fn parse(filename: &str) -> Input {
    let sep_str = format!("{}", SEPARATOR);
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let parts = line.split('-').collect::<Vec<_>>();
            let num_parts = parts.len();
            let name = parts.iter().take(num_parts - 1).join(&sep_str);
            let tail = parts
                .iter()
                .skip(num_parts - 1)
                .next()
                .unwrap()
                .split_once('[')
                .unwrap();
            Room {
                name: name,
                sector: tail.0.parse().unwrap(),
                checksum: (&tail.1[..(tail.1.len() - 1)])
                    .chars()
                    .collect_array_vec::<5>()
                    .into_inner()
                    .unwrap(),
            }
        })
        .collect()
}

const NUM_LETTERS: usize = 26;

fn solve_case1(rooms: &Input) -> usize {
    rooms
        .iter()
        .filter_map(|room| {
            let mut frequency_map = <[usize; NUM_LETTERS]>::default();
            for letter in room.name.bytes() {
                if letter == (SEPARATOR as u8) {
                    continue;
                }
                frequency_map[(letter - b'a') as usize] += 1;
            }
            let mut sorted = frequency_map
                .iter()
                .enumerate()
                .collect_array_vec::<NUM_LETTERS>();
            sorted.sort_by(|(lhs_id, lhs), (rhs_id, rhs)| rhs.cmp(lhs).then(lhs_id.cmp(rhs_id)));
            sorted
                .into_iter()
                .zip(room.checksum.iter())
                .all(|((letter_id, _), check)| *check == ((letter_id as u8 + b'a') as char))
                .then_some(room.sector)
        })
        .sum()
}

fn solve_case2(rooms: &Input) -> usize {
    for room in rooms {
        let real_name = room
            .name
            .bytes()
            .map(|letter| {
                if letter == (SEPARATOR as u8) {
                    ' '
                } else {
                    ((((letter - b'a') as usize + room.sector) % NUM_LETTERS) as u8 + b'a') as char
                }
            })
            .collect::<String>();
        if real_name == "northpole object storage" {
            return room.sector;
        }
    }
    0
}

fn main() {
    println!("Part 1");
    let example = parse("day04.example");
    aoc::expect_result!(1514, solve_case1(&example));
    let input = parse("day04.input");
    aoc::expect_result!(173787, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(0, solve_case2(&example));
    aoc::expect_result!(548, solve_case2(&input));
}
