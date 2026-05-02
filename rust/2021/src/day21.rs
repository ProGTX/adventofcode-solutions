fn parse(filename: &str) -> (u32, u32) {
    let lines = aoc::file::read_lines(filename);
    let p1 = lines[0].split(": ").nth(1).unwrap().parse().unwrap();
    let p2 = lines[1].split(": ").nth(1).unwrap().parse().unwrap();
    (p1, p2)
}

#[derive(Default)]
struct DeterministicDice {
    current: u32,
}
impl DeterministicDice {
    fn roll(&mut self) -> u32 {
        let result = self.current % 100 + 1;
        self.current = result;
        result
    }
}

fn solve_case1((mut p1, mut p2): (u32, u32)) -> u32 {
    let mut score = (0, 0);
    let mut rolls = 0;
    let mut dice = DeterministicDice::default();
    let mut roll_dice = |pos| {
        let rolled = dice.roll() + dice.roll() + dice.roll();
        rolls += 3;
        return (pos + rolled - 1) % 10 + 1;
    };
    loop {
        p1 = roll_dice(p1);
        score.0 += p1;
        if (score.0 >= 1000) {
            return score.1 * rolls;
        }
        p2 = roll_dice(p2);
        score.1 += p2;
        if (score.1 >= 1000) {
            return score.0 * rolls;
        }
    }
}

fn solve_case2((p1, p2): (u32, u32)) -> u32 {
    // TODO: Implement Part 2
    0
}

fn main() {
    println!("Part 1");
    let example = parse("day21.example");
    assert_eq!(739785, solve_case1(example));
    let input = parse("day21.input");
    assert_eq!(805932, solve_case1(input));

    println!("Part 2");
    // assert_eq!(XXX, solve_case2(&example));
    // assert_eq!(XXX, solve_case2(&input));
}
