type Point = aoc::point::Point<i64>;

const CORRECTION: i64 = 10_000_000_000_000;

#[derive(Clone, Copy, Default)]
struct ClawMachine {
    a: Point,
    b: Point,
    prize: Point,
}

impl ClawMachine {
    fn lowest_cost(&self) -> i64 {
        // prize = c
        // A*ax + B*bx = cx
        // A*ay + B*by = cy
        // Solve A and B
        let b_num = self.a.x * self.prize.y - self.a.y * self.prize.x;
        let b_den = self.a.x * self.b.y - self.a.y * self.b.x;
        if b_den == 0 || (b_num % b_den) != 0 {
            return 0;
        }
        let b = b_num / b_den;

        let a_num = self.prize.y - b * self.b.y;
        let a_den = self.a.y;
        if a_den == 0 || (a_num % a_den) != 0 {
            return 0;
        }
        let a = a_num / a_den;

        a * 3 + b
    }
}

fn parse_point(line: &str) -> Point {
    let mut numbers = line
        .split(|c: char| !c.is_ascii_digit())
        .filter(|s| !s.is_empty())
        .map(|s| s.parse::<i64>().unwrap());
    Point::new(numbers.next().unwrap(), numbers.next().unwrap())
}

fn parse(filename: &str) -> Vec<ClawMachine> {
    let mut machines = vec![ClawMachine::default()];

    for line in aoc::file::read_lines(filename) {
        if line.is_empty() {
            machines.push(ClawMachine::default());
            continue;
        }

        let current = machines.last_mut().unwrap();
        if line.starts_with("Prize") {
            current.prize = parse_point(&line);
        } else if line.starts_with("Button A") {
            current.a = parse_point(&line);
        } else if line.starts_with("Button B") {
            current.b = parse_point(&line);
        } else {
            unreachable!("Parsing failure");
        }
    }

    machines
}

fn solve_case<const CORRECT_CONVERSION: bool>(machines: &[ClawMachine]) -> i64 {
    machines
        .iter()
        .map(|machine| {
            let mut machine = *machine;
            if CORRECT_CONVERSION {
                machine.prize += Point::new(CORRECTION, CORRECTION);
            }
            machine.lowest_cost()
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day13.example");
    aoc::expect_result!(480, solve_case::<false>(&example));
    let input = parse("day13.input");
    aoc::expect_result!(38714, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(459236326669 + 416082282239, solve_case::<true>(&example));
    aoc::expect_result!(74015623345775, solve_case::<true>(&input));
}
