use itertools::Itertools;

#[derive(Clone)]
enum Operation {
    Add(u64),
    Mul(u64),
    Square,
}

#[derive(Clone)]
struct Monkey {
    items: Vec<u64>,
    operation: Operation,
    test: (u64, usize, usize),
}

impl Monkey {
    fn inspect(&self, old: u64) -> u64 {
        match self.operation {
            Operation::Add(add) => old + add,
            Operation::Mul(mul) => old * mul,
            Operation::Square => old * old,
        }
    }
}

fn parse(filename: &str) -> Vec<Monkey> {
    let split_once = |it: &mut std::str::Lines<'_>, delimiter| {
        it.next()
            .unwrap()
            .split_once(delimiter)
            .unwrap()
            .1
            .to_string()
    };
    aoc::file::read_string(filename)
        .trim()
        .split("\n\n")
        .map(|monkey| {
            let lines = monkey.lines();
            let mut line_it = lines.into_iter();
            line_it.next();
            let items = split_once(&mut line_it, ": ");
            let operation = split_once(&mut line_it, " = ");
            let (_, op, rhs) = operation.split_whitespace().tuples().next().unwrap();
            let divisor = split_once(&mut line_it, "by ");
            let true_monkey = split_once(&mut line_it, "monkey ");
            let false_monkey = split_once(&mut line_it, "monkey ");
            Monkey {
                items: items.split(", ").map(|s| s.parse().unwrap()).collect(),
                operation: match op {
                    "+" => Operation::Add(rhs.parse().unwrap()),
                    "*" => {
                        if (rhs == "old") {
                            Operation::Square
                        } else {
                            Operation::Mul(rhs.parse().unwrap())
                        }
                    }
                    _ => unreachable!("Invalid operation"),
                },
                test: (
                    divisor.parse().unwrap(),
                    true_monkey.parse().unwrap(),
                    false_monkey.parse().unwrap(),
                ),
            }
        })
        .collect_vec()
}

fn solve_case<const NUM_ROUNDS: usize, const RELIEF: u64>(monkeys: &Vec<Monkey>) -> u64 {
    let supermodulo = if (RELIEF == 1) {
        monkeys.iter().map(|monkey| monkey.test.0).product::<u64>()
    } else {
        u64::MAX
    };
    let mut monkeys = monkeys.clone();
    let num_monkeys = monkeys.len();
    let mut num_inspected = vec![0; num_monkeys];
    for _ in 0..NUM_ROUNDS {
        for monkey_id in 0..num_monkeys {
            let items = monkeys[monkey_id].items.clone();
            for worry_level in items {
                let worry_level = monkeys[monkey_id].inspect(worry_level % supermodulo) / RELIEF;
                let next_index = if ((worry_level % monkeys[monkey_id].test.0) == 0) {
                    monkeys[monkey_id].test.1
                } else {
                    monkeys[monkey_id].test.2
                };
                monkeys[next_index].items.push(worry_level);
            }
            num_inspected[monkey_id] += monkeys[monkey_id].items.len();
            monkeys[monkey_id].items.clear();
        }
    }
    num_inspected.sort_unstable_by(|a, b| b.cmp(a));
    (num_inspected[0] as u64) * (num_inspected[1] as u64)
}

fn main() {
    println!("Part 1");
    let example = parse("day11.example");
    assert_eq!(10605, solve_case::<20, 3>(&example));
    let input = parse("day11.input");
    assert_eq!(98280, solve_case::<20, 3>(&input));

    println!("Part 2");
    assert_eq!(2713310158, solve_case::<10000, 1>(&example));
    assert_eq!(17673687232, solve_case::<10000, 1>(&input));
}
