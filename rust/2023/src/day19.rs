const START_NAME: &str = "in";
const NO_COMPARISON: usize = 4;

#[derive(Clone, Copy)]
struct Part {
    xmas: [i32; 4],
}
impl Part {
    fn sum(&self) -> i32 {
        self.xmas.iter().sum()
    }
}

#[derive(Clone, Copy)]
enum Destination {
    Accepted,
    Rejected,
    Workflow(usize),
}

#[derive(Clone, Copy)]
struct Rule {
    destination: Destination,
    member: usize,
    number: i32,
}

type Workflow = Vec<Rule>;

struct Input {
    parts: Vec<Part>,
    workflows: Vec<Workflow>,
    start: usize,
}

fn parse_destination(dest_str: &str, name_to_id: &mut aoc::string::NameToId) -> Destination {
    match dest_str {
        "A" => Destination::Accepted,
        "R" => Destination::Rejected,
        _ => Destination::Workflow(name_to_id.intern(dest_str)),
    }
}

fn parse_rule(rule_str: &str, name_to_id: &mut aoc::string::NameToId) -> Rule {
    // a<2006:qkq
    // rfg
    let bytes = rule_str.as_bytes();
    if rule_str.len() == 1 || (bytes[1] != b'<' && bytes[1] != b'>') {
        return Rule {
            destination: parse_destination(rule_str, name_to_id),
            member: NO_COMPARISON,
            number: 0,
        };
    }
    let member = match bytes[0] {
        b'x' => 0,
        b'm' => 1,
        b'a' => 2,
        b's' => 3,
        _ => unreachable!("Invalid part member"),
    };
    let less_than = bytes[1] == b'<';
    let (number_str, dest_str) = rule_str[2..].split_once(':').unwrap();
    let number: i32 = number_str.parse().unwrap();
    Rule {
        destination: parse_destination(dest_str, name_to_id),
        member,
        number: if less_than { -number } else { number },
    }
}

fn parse(filename: &str) -> Input {
    let lines = aoc::file::read_lines(filename);

    let mut name_to_id = aoc::string::NameToId::new();
    let start = name_to_id.intern(START_NAME);

    let mut workflows: Vec<Workflow> = Vec::new();
    let mut parts: Vec<Part> = Vec::new();
    let mut parsing_parts = false;

    for line in &lines {
        if line.is_empty() {
            parsing_parts = true;
            continue;
        }
        if !parsing_parts {
            // px{a<2006:qkq,m>2090:A,rfg}
            let (name, rules_str) = line.split_once('{').unwrap();
            let rules_str = &rules_str[..rules_str.len() - 1];
            let workflow: Workflow = rules_str
                .split(',')
                .map(|rule_str| parse_rule(rule_str, &mut name_to_id))
                .collect();
            let id = name_to_id.intern(name);
            workflows.resize(name_to_id.new_len(workflows.len()), Vec::new());
            workflows[id] = workflow;
        } else {
            // {x=787,m=2655,a=1222,s=2876}
            let nums: Vec<i32> = line[1..line.len() - 1]
                .split(',')
                .map(|n| n[2..].parse().unwrap())
                .collect();
            parts.push(Part {
                xmas: [nums[0], nums[1], nums[2], nums[3]],
            });
        }
    }

    Input {
        parts,
        workflows,
        start,
    }
}

fn rule_matches(part: &Part, rule: &Rule) -> bool {
    if rule.member == NO_COMPARISON {
        return true;
    }
    let value = part.xmas[rule.member];
    if rule.number > 0 {
        value > rule.number
    } else {
        value < -rule.number
    }
}

fn check_part(part: &Part, workflows: &[Workflow], start: usize) -> bool {
    let mut current = start;
    loop {
        for rule in &workflows[current] {
            if !rule_matches(part, rule) {
                continue;
            }
            match rule.destination {
                Destination::Accepted => return true,
                Destination::Rejected => return false,
                Destination::Workflow(id) => {
                    current = id;
                    break;
                }
            }
        }
    }
}

fn solve_case1(input: &Input) -> i32 {
    input
        .parts
        .iter()
        .filter(|part| check_part(part, &input.workflows, input.start))
        .map(Part::sum)
        .sum()
}

#[derive(Clone, Copy)]
struct Range {
    start: i32,
    end: i32,
}

impl Range {
    fn size(self) -> i64 {
        (self.end - self.start + 1) as i64
    }
}

fn range_product(ranges: &[Range; 4]) -> i64 {
    ranges.iter().map(|range| range.size()).product()
}

fn count_accepted(workflows: &[Workflow], workflow_id: usize, mut ranges: [Range; 4]) -> i64 {
    let mut total = 0i64;
    for rule in &workflows[workflow_id] {
        let mut matching = ranges;
        let mut has_match = true;
        if rule.member != NO_COMPARISON {
            if rule.number > 0 {
                matching[rule.member].start = matching[rule.member].start.max(rule.number + 1);
                ranges[rule.member].end = ranges[rule.member].end.min(rule.number);
            } else {
                matching[rule.member].end = matching[rule.member].end.min(-rule.number - 1);
                ranges[rule.member].start = ranges[rule.member].start.max(-rule.number);
            }
            has_match = matching[rule.member].start <= matching[rule.member].end;
        }

        if has_match {
            match rule.destination {
                Destination::Accepted => total += range_product(&matching),
                Destination::Rejected => {}
                Destination::Workflow(id) => total += count_accepted(workflows, id, matching),
            }
        }

        if rule.member == NO_COMPARISON || ranges[rule.member].start > ranges[rule.member].end {
            break;
        }
    }
    total
}

fn solve_case2(input: &Input) -> i64 {
    let full_range = Range {
        start: 1,
        end: 4000,
    };
    count_accepted(&input.workflows, input.start, [full_range; 4])
}

fn main() {
    println!("Part 1");
    let example = parse("day19.example");
    aoc::expect_result!(19114, solve_case1(&example));
    let input = parse("day19.input");
    aoc::expect_result!(509597, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(167409079868000, solve_case2(&example));
    aoc::expect_result!(143219569011526, solve_case2(&input));
}
