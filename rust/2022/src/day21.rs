type Ratio = aoc::math::Ratio<i64>;

#[derive(Clone, Copy, PartialEq, Eq)]
enum Op {
    Value,
    Add,
    Sub,
    Mul,
    Div,
}

impl Op {
    fn apply(self, lhs: Ratio, rhs: Ratio) -> Ratio {
        match self {
            Op::Value | Op::Add => lhs + rhs,
            Op::Sub => lhs - rhs,
            Op::Mul => lhs * rhs,
            Op::Div => lhs / rhs,
        }
    }

    fn inverse(self) -> Self {
        match self {
            Op::Add | Op::Value => Op::Sub,
            Op::Sub => Op::Add,
            Op::Mul => Op::Div,
            Op::Div => Op::Mul,
        }
    }

    fn is_noncommutative(self) -> bool {
        matches!(self, Op::Sub | Op::Div)
    }
}

struct ParsedMonkey {
    name_lhs: String,
    name_rhs: String,
    value_lhs: Option<Ratio>,
    value_rhs: Option<Ratio>,
    op: Op,
}

type ParsedMonkeys = Vec<(String, ParsedMonkey)>;

fn parse(filename: &str) -> ParsedMonkeys {
    aoc::file::read_lines(filename)
        .into_iter()
        .map(|line| {
            let (name, rest) = line.split_once(':').unwrap();
            let parts: Vec<&str> = rest[1..].split(' ').collect();
            let pm = if parts.len() == 1 {
                let value = Ratio::integer(parts[0].parse::<i64>().unwrap());
                ParsedMonkey {
                    name_lhs: String::new(),
                    name_rhs: String::new(),
                    value_lhs: Some(value),
                    value_rhs: Some(Ratio::integer(0)),
                    op: Op::Value,
                }
            } else {
                ParsedMonkey {
                    name_lhs: parts[0].to_string(),
                    name_rhs: parts[2].to_string(),
                    value_lhs: None,
                    value_rhs: None,
                    op: match parts[1] {
                        "+" => Op::Add,
                        "-" => Op::Sub,
                        "*" => Op::Mul,
                        "/" => Op::Div,
                        _ => unreachable!(),
                    },
                }
            };
            (name.to_string(), pm)
        })
        .collect()
}

#[derive(Clone)]
struct Monkey {
    index_lhs: usize,
    index_rhs: usize,
    value_lhs: Option<Ratio>,
    value_rhs: Option<Ratio>,
    op: Op,
}

impl Monkey {
    fn value(lhs: Ratio, rhs: Ratio) -> Self {
        Self {
            index_lhs: usize::MAX,
            index_rhs: usize::MAX,
            value_lhs: Some(lhs),
            value_rhs: Some(rhs),
            op: Op::Add,
        }
    }

    fn is_ready(&self) -> bool {
        self.value_lhs.is_some() && self.value_rhs.is_some()
    }

    fn apply(&self) -> Ratio {
        self.op
            .apply(self.value_lhs.unwrap(), self.value_rhs.unwrap())
    }
}

fn transform_monkeys(parsed: &ParsedMonkeys) -> (Vec<Monkey>, usize, usize) {
    let mut monkeys = Vec::new();
    let mut root_index = 0;
    let mut human_index = 0;
    for (index, (name, pm)) in parsed.iter().enumerate() {
        if name == "root" {
            root_index = index;
        }
        if name == "humn" {
            human_index = index;
        }

        if pm.op == Op::Value {
            monkeys.push(Monkey::value(pm.value_lhs.unwrap(), pm.value_rhs.unwrap()));
            continue;
        }

        let index_lhs = parsed.iter().position(|(n, _)| n == &pm.name_lhs).unwrap();
        let index_rhs = parsed.iter().position(|(n, _)| n == &pm.name_rhs).unwrap();
        monkeys.push(Monkey {
            index_lhs,
            index_rhs,
            value_lhs: pm.value_lhs,
            value_rhs: pm.value_rhs,
            op: pm.op,
        });
    }
    (monkeys, root_index, human_index)
}

fn get_yelled_number(monkeys: &mut Vec<Monkey>, current: usize) -> Ratio {
    if monkeys[current].is_ready() {
        return monkeys[current].apply();
    }
    let index_lhs = monkeys[current].index_lhs;
    let index_rhs = monkeys[current].index_rhs;
    let lhs = get_yelled_number(monkeys, index_lhs);
    let rhs = get_yelled_number(monkeys, index_rhs);
    monkeys[current].value_lhs = Some(lhs);
    monkeys[current].value_rhs = Some(rhs);
    monkeys[current].apply()
}

fn solve_case1(parsed: &ParsedMonkeys) -> i64 {
    let (mut monkeys, root_index, _) = transform_monkeys(parsed);
    get_yelled_number(&mut monkeys, root_index).as_integer()
}

fn solve_case2(parsed: &ParsedMonkeys) -> i64 {
    let (mut monkeys, root_index, human_index) = transform_monkeys(parsed);

    let mut current_index = human_index;
    loop {
        let current_name = &parsed[current_index].0;
        let parent_index = parsed
            .iter()
            .position(|(_, pm)| pm.name_lhs == *current_name || pm.name_rhs == *current_name)
            .unwrap();

        let parent_op = monkeys[parent_index].op;
        let current_was_left = monkeys[parent_index].index_lhs == current_index;
        let sibling_index = if current_was_left {
            monkeys[parent_index].index_rhs
        } else {
            monkeys[parent_index].index_lhs
        };

        monkeys[current_index].value_lhs = None;
        monkeys[current_index].value_rhs = None;

        if parent_index == root_index {
            monkeys[parent_index] = Monkey::value(Ratio::integer(0), Ratio::integer(0));
            monkeys[current_index].index_lhs = parent_index;
            monkeys[current_index].index_rhs = sibling_index;
            monkeys[current_index].op = Op::Add;
            break;
        }

        // For non-commutative ops where current was on the right:
        //   parent = sibling - current -> current = sibling - parent
        //   parent = sibling / current -> current = sibling / parent
        // Swap operand order and use the original op instead of its inverse
        let swap_operands = !current_was_left && parent_op.is_noncommutative();
        monkeys[current_index].index_lhs = if swap_operands {
            sibling_index
        } else {
            parent_index
        };
        monkeys[current_index].index_rhs = if swap_operands {
            parent_index
        } else {
            sibling_index
        };
        monkeys[current_index].op = if swap_operands {
            parent_op
        } else {
            parent_op.inverse()
        };

        current_index = parent_index;
    }

    get_yelled_number(&mut monkeys, human_index).as_integer()
}

fn main() {
    println!("Part 1");
    let example = parse("day21.example");
    aoc::expect_result!(152, solve_case1(&example));
    let input = parse("day21.input");
    aoc::expect_result!(291425799367130_i64, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(301, solve_case2(&example));
    aoc::expect_result!(3219579395609_i64, solve_case2(&input));
}
