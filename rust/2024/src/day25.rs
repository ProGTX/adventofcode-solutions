const KEYHOLE_ROWS: usize = 7;
const KEYHOLE_COLUMNS: usize = 5;
const MAX_HEIGHT: i32 = KEYHOLE_ROWS as i32 - 2;

const ACTIVE: char = '#';
const INACTIVE: char = '.';
const ALL_FILLED: &str = "#####";

type KeyholeStorage = [i32; KEYHOLE_COLUMNS];

struct Schematics {
    keys: Vec<KeyholeStorage>,
    locks: Vec<KeyholeStorage>,
}

fn parse(filename: &str) -> Schematics {
    let lines: Vec<String> = aoc::file::read_lines(filename)
        .into_iter()
        .filter(|line| !line.is_empty())
        .collect();

    let mut keys = Vec::new();
    let mut locks = Vec::new();

    for block in lines.chunks(KEYHOLE_ROWS) {
        let is_lock = block[0] == ALL_FILLED;
        let mut heights: KeyholeStorage = if is_lock {
            [0; KEYHOLE_COLUMNS]
        } else {
            [MAX_HEIGHT; KEYHOLE_COLUMNS]
        };

        for line in &block[1..] {
            for (i, c) in line.chars().enumerate() {
                if is_lock {
                    if c == ACTIVE {
                        heights[i] += 1;
                    }
                } else if c == INACTIVE {
                    heights[i] -= 1;
                }
            }
        }

        if is_lock {
            locks.push(heights);
        } else {
            keys.push(heights);
        }
    }

    Schematics { keys, locks }
}

fn overlaps(key: &KeyholeStorage, lock: &KeyholeStorage) -> bool {
    key.iter()
        .zip(lock.iter())
        .any(|(&key_value, &lock_value)| (key_value + lock_value) > MAX_HEIGHT)
}

fn count_fitting(keys: &[KeyholeStorage], locks: &[KeyholeStorage]) -> i32 {
    keys.iter()
        .map(|key| locks.iter().filter(|lock| !overlaps(key, lock)).count() as i32)
        .sum()
}

fn solve_case(schematics: &Schematics) -> i32 {
    count_fitting(&schematics.keys, &schematics.locks)
}

fn main() {
    println!("Part 1");
    let example = parse("day25.example");
    aoc::expect_result!(3, solve_case(&example));
    let input = parse("day25.input");
    aoc::expect_result!(3338, solve_case(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
