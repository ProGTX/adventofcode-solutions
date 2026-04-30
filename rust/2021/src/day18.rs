// Snailfish number
#[derive(Debug, Clone)]
enum SFN {
    Number(u8),
    Pair(Box<SFN>, Box<SFN>),
}
impl SFN {
    fn number(n: u8) -> Self {
        SFN::Number(n)
    }
    fn pair(left: SFN, right: SFN) -> Self {
        SFN::Pair(Box::new(left), Box::new(right))
    }
}

// Recursive descent parser
fn parse_sfn_inner(s: &str) -> (SFN, &str) {
    if (s.starts_with('[')) {
        let (left, rest) = parse_sfn_inner(&s[1..]);
        let (right, rest) = parse_sfn_inner(&rest[1..]); // skip ','
        (SFN::pair(left, right), &rest[1..]) // skip ']'
    } else {
        let end = s.find(|c: char| !c.is_ascii_digit()).unwrap_or(s.len());
        (SFN::number(s[..end].parse().unwrap()), &s[end..])
    }
}
fn parse_sfn(s: &str) -> SFN {
    parse_sfn_inner(s).0
}

fn parse(filename: &str) -> Vec<SFN> {
    aoc::file::read_lines(filename)
        .iter()
        .map(|s| parse_sfn(s))
        .collect()
}

#[derive(Debug, Default)]
struct ExplodeResult {
    left: Option<u8>,
    right: Option<u8>,
    exploded: bool,
}

fn explode(sfn: &mut SFN, level: usize) -> ExplodeResult {
    match sfn {
        SFN::Number(_) => {
            return ExplodeResult::default();
        }
        SFN::Pair(left, right) => {
            if (level > 4) {
                match (left.as_ref(), right.as_ref()) {
                    (SFN::Number(left_num), SFN::Number(right_num)) => {
                        return ExplodeResult {
                            left: Some(*left_num),
                            right: Some(*right_num),
                            exploded: true,
                        };
                    }
                    _ => unreachable!("Exploding pairs will always consist of two regular numbers"),
                }
            } else {
                let left_result = explode(left, level + 1);

                let right_result = explode(right, level + 1);
            }
        }
    }
    return ExplodeResult::default();
}

fn split(sfn: &mut SFN) -> bool {
    match sfn {
        SFN::Number(num) => {
            let num = *num;
            if (num >= 10) {
                *sfn = SFN::pair(SFN::number(num / 2), SFN::number(num / 2 + num % 2));
                return true;
            }
        }
        SFN::Pair(left, right) => {
            if (split(left)) {
                return true;
            }
            if (split(right)) {
                return true;
            }
        }
    }
    return false;
}

fn reduce(sfn: &mut SFN) -> &mut SFN {
    loop {
        while (explode(sfn, 1).exploded) {}
        if !split(sfn) {
            break;
        }
    }
    sfn
}

fn add(lhs: &SFN, rhs: &SFN) -> SFN {
    reduce(&mut SFN::pair(lhs.clone(), rhs.clone())).clone()
}

fn magnitude(sfn: &SFN) -> u64 {
    match sfn {
        SFN::Number(num) => *num as u64,
        SFN::Pair(left, right) => 3 * magnitude(left) + 2 * magnitude(right),
    }
}

fn solve_case1(numbers: &[SFN]) -> u64 {
    magnitude(
        &numbers[1..]
            .iter()
            .fold(numbers[0].clone(), |acc: SFN, rhs| add(&acc, rhs)),
    )
}

fn solve_case2(numbers: &[SFN]) -> u64 {
    // TODO: Implement Part 2
    0
}

fn main() {
    println!("Part 1");

    let example = parse("day18.example");
    assert_eq!(4140, solve_case1(&example));
    let input = parse("day18.input");
    assert_eq!(1337, solve_case1(&input));

    println!("Part 2");
    // assert_eq!(XXX, solve_case2(&example));
    // assert_eq!(XXX, solve_case2(&input));
}
