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

const LEFT: bool = false;
const RIGHT: bool = true;

fn add_closest<const DIRECTION: bool>(sfn: &mut SFN, explosive: u8) -> bool {
    match sfn {
        SFN::Number(num) => {
            *num += explosive;
            return true;
        }
        SFN::Pair(left, right) => {
            if (DIRECTION == LEFT) {
                if (add_closest::<DIRECTION>(left, explosive)
                    || add_closest::<DIRECTION>(right, explosive))
                {
                    return true;
                }
            } else {
                if (add_closest::<DIRECTION>(right, explosive)
                    || add_closest::<DIRECTION>(left, explosive))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

fn explode(sfn: &mut SFN, level: usize) -> ExplodeResult {
    match sfn {
        SFN::Number(_) => {
            return ExplodeResult::default();
        }
        SFN::Pair(left, right) => {
            if (level > 4) {
                // Explode the pair
                match (left.as_ref(), right.as_ref()) {
                    (SFN::Number(left_num), SFN::Number(right_num)) => {
                        let lnum = Some(*left_num);
                        let rnum = Some(*right_num);
                        *sfn = SFN::Number(0);
                        return ExplodeResult {
                            left: lnum,
                            right: rnum,
                            exploded: true,
                        };
                    }
                    _ => unreachable!("Exploding pairs will always consist of two regular numbers"),
                }
            } else {
                // Try to explode subtrees and propagate the values

                let left_result = explode(left, level + 1);
                if (left_result.exploded) {
                    if let Some(rnum) = left_result.right {
                        // Try to consume the number by adding it
                        // to the left-most number on the right
                        if (add_closest::<LEFT>(right, rnum)) {
                            return ExplodeResult {
                                left: left_result.left,
                                right: None,
                                exploded: true,
                            };
                        }
                    };
                    // Fallback to just return same value
                    return left_result;
                }
                drop(left_result);

                let right_result = explode(right, level + 1);
                if (right_result.exploded) {
                    if let Some(lnum) = right_result.left {
                        // Try to consume the number by adding it
                        // to the right-most number on the left
                        if (add_closest::<RIGHT>(left, lnum)) {
                            return ExplodeResult {
                                left: None,
                                right: right_result.right,
                                exploded: true,
                            };
                        }
                    };
                    // Fallback to just return same value
                    return right_result;
                }
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

fn magnitude(sfn: &SFN) -> u32 {
    match sfn {
        SFN::Number(num) => *num as u32,
        SFN::Pair(left, right) => 3 * magnitude(left) + 2 * magnitude(right),
    }
}

fn solve_case1(numbers: &[SFN]) -> u32 {
    magnitude(
        &numbers[1..]
            .iter()
            .fold(numbers[0].clone(), |acc: SFN, rhs| add(&acc, rhs)),
    )
}

fn solve_case2(numbers: &[SFN]) -> u32 {
    let n = numbers.len();
    (0..n)
        .flat_map(|i| (0..n).filter(move |&j| j != i).map(move |j| (i, j)))
        .map(|(i, j)| magnitude(&add(&numbers[i], &numbers[j])))
        .max()
        .unwrap()
}

fn main() {
    println!("Part 1");

    assert_eq!(143, magnitude(&parse_sfn("[[1,2],[[3,4],5]]")));
    assert_eq!(
        1384,
        magnitude(&parse_sfn("[[[[0,7],4],[[7,8],[6,0]]],[8,1]]"))
    );
    assert_eq!(445, magnitude(&parse_sfn("[[[[1,1],[2,2]],[3,3]],[4,4]]")));
    assert_eq!(791, magnitude(&parse_sfn("[[[[3,0],[5,3]],[4,4]],[5,5]]")));
    assert_eq!(1137, magnitude(&parse_sfn("[[[[5,0],[7,4]],[5,5]],[6,6]]")));
    assert_eq!(
        3488,
        magnitude(&parse_sfn(
            "[[[[8,7],[7,7]],[[8,6],[7,7]]],[[[0,7],[6,6]],[8,7]]]"
        ))
    );

    let example = parse("day18.example");
    assert_eq!(4140, solve_case1(&example));
    let input = parse("day18.input");
    assert_eq!(4207, solve_case1(&input));

    println!("Part 2");
    assert_eq!(3993, solve_case2(&example));
    assert_eq!(4635, solve_case2(&input));
}
