use std::collections::HashMap;

fn parse(filename: &str) -> (u32, u32) {
    let lines = aoc::file::read_lines(filename);
    let p1 = lines[0].split(": ").nth(1).unwrap().parse().unwrap();
    let p2 = lines[1].split(": ").nth(1).unwrap().parse().unwrap();
    (p1, p2)
}

const WIN_SCORE_PART1: u32 = 1000;
const WIN_SCORE_PART2: u32 = 21;

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

fn next_pos(pos: u32, roll: u32) -> u32 {
    (pos + roll - 1) % 10 + 1
}

fn solve_case1((mut p1, mut p2): (u32, u32)) -> u32 {
    let mut score = (0, 0);
    let mut rolls = 0;
    let mut dice = DeterministicDice::default();
    let mut roll_dice = |pos| {
        let rolled = dice.roll() + dice.roll() + dice.roll();
        rolls += 3;
        return next_pos(pos, rolled);
    };
    loop {
        p1 = roll_dice(p1);
        score.0 += p1;
        if (score.0 >= WIN_SCORE_PART1) {
            return score.1 * rolls;
        }
        p2 = roll_dice(p2);
        score.1 += p2;
        if (score.1 >= WIN_SCORE_PART1) {
            return score.0 * rolls;
        }
    }
}

#[derive(Clone, Hash, PartialEq, Eq)]
struct Universe {
    positions: (u32, u32),
    score: (u32, u32),
    is_player_one: bool,
}

fn dirac_rolls() -> impl Iterator<Item = u32> {
    (1..=3).flat_map(|a| (1..=3).flat_map(move |b| (1..=3).map(move |c| a + b + c)))
}

const DIRAC_FREQ_OFFSET: usize = 3;

fn dirac_roll_frequency() -> [u32; 7] {
    let mut freq = [0; 7];
    for roll in dirac_rolls() {
        freq[roll as usize - DIRAC_FREQ_OFFSET] += 1;
    }
    freq
}

fn solve_case2((p1, p2): (u32, u32)) -> usize {
    let universe = Universe {
        positions: (p1, p2),
        score: (0, 0),
        is_player_one: true,
    };

    let mut multiverse: HashMap<Universe, usize> = HashMap::new();
    multiverse.insert(universe, 1);

    let mut num_wins = (0, 0);
    let dirac = dirac_roll_frequency();

    while (!multiverse.is_empty()) {
        let mut new_multiverse = HashMap::new();
        for (universe, uni_count) in multiverse {
            // Roll the dice
            for (roll, num_rolled_universes) in dirac.iter().enumerate() {
                let roll: u32 = (roll + DIRAC_FREQ_OFFSET) as u32;
                let new_uni_count = uni_count * (*num_rolled_universes as usize);

                let mut new_universe = universe.clone();
                new_universe.is_player_one = !universe.is_player_one;

                if (universe.is_player_one) {
                    // Player 1 rolled
                    new_universe.positions.0 = next_pos(universe.positions.0, roll);
                    new_universe.score.0 += new_universe.positions.0;

                    if (new_universe.score.0 >= WIN_SCORE_PART2) {
                        num_wins.0 += new_uni_count;
                        continue;
                    }
                } else {
                    // Player 2 rolled
                    new_universe.positions.1 = next_pos(universe.positions.1, roll);
                    new_universe.score.1 += new_universe.positions.1;

                    if (new_universe.score.1 >= WIN_SCORE_PART2) {
                        num_wins.1 += new_uni_count;
                        continue;
                    }
                }

                // Nobody won, explore this universe further
                *new_multiverse.entry(new_universe).or_default() += new_uni_count;
            }
        }

        multiverse = new_multiverse;
    }

    return num_wins.0.max(num_wins.1);
}

fn main() {
    println!("Part 1");
    let example = parse("day21.example");
    assert_eq!(739785, solve_case1(example));
    let input = parse("day21.input");
    assert_eq!(805932, solve_case1(input));

    println!("Part 2");
    assert_eq!(444356092776315, solve_case2(example));
    assert_eq!(133029050096658, solve_case2(input));
}
