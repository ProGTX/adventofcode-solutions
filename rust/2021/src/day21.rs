use std::collections::{HashMap, VecDeque, hash_map::Entry};

fn parse(filename: &str) -> (u32, u32) {
    let lines = aoc::file::read_lines(filename);
    let p1 = lines[0].split(": ").nth(1).unwrap().parse().unwrap();
    let p2 = lines[1].split(": ").nth(1).unwrap().parse().unwrap();
    (p1, p2)
}

const WIN_SCORE_P1: u32 = 1000;
const WIN_SCORE_P2: u32 = 12;

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
        if (score.0 >= WIN_SCORE_P1) {
            return score.1 * rolls;
        }
        p2 = roll_dice(p2);
        score.1 += p2;
        if (score.1 >= WIN_SCORE_P1) {
            return score.0 * rolls;
        }
    }
}

#[derive(Clone, Hash, PartialEq, Eq)]
struct Universe {
    positions: (u32, u32),
    num_rolls: u32, // even indicates that it's player 1's turn
}

#[derive(Clone)]
struct UniverseScore {
    score: (u32, u32),
    num_universes: usize,
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
        num_rolls: 0,
    };

    let mut scores: HashMap<Universe, UniverseScore> = HashMap::new();
    scores.entry(universe.clone()).or_insert(UniverseScore {
        score: (0, 0),
        num_universes: 1,
    });

    let mut queue = VecDeque::new();
    queue.push_back(universe);

    let mut num_wins = (0, 0);
    let dirac = dirac_roll_frequency();
    for (roll, num_universes) in dirac.iter().enumerate() {
        println!(
            "roll {} in {} universes",
            roll + DIRAC_FREQ_OFFSET,
            num_universes
        );
    }

    while (!queue.is_empty()) {
        let universe = queue.pop_front().unwrap();
        let uni_score = scores.get(&universe).unwrap().clone();
        // Roll the dice
        for (roll, new_universes) in dirac.iter().enumerate() {
            let roll = (roll + DIRAC_FREQ_OFFSET) as u32;
            let new_universes = *new_universes as usize;

            let mut new_positions = universe.positions;
            let mut new_scores = (0, 0);

            if ((universe.num_rolls % 2) == 0) {
                // Player 1 rolled
                new_positions.0 = next_pos(universe.positions.0, roll);
                new_scores.0 = new_positions.0;
            } else {
                // Player 2 rolled
                new_positions.1 = next_pos(universe.positions.1, roll);
                new_scores.1 = new_positions.1;
            }

            let new_universe = Universe {
                positions: new_positions,
                num_rolls: universe.num_rolls + 1,
            };

            let new_uni_score = scores.entry(new_universe.clone());
            match new_uni_score {
                Entry::Vacant(e) => {
                    e.insert(UniverseScore {
                        score: (
                            uni_score.score.0 + new_scores.0,
                            uni_score.score.1 + new_scores.1,
                        ),
                        num_universes: uni_score.num_universes * new_universes,
                    });
                }
                Entry::Occupied(mut e) => {
                    let s = e.get_mut();
                    s.score.0 += new_scores.0;
                    s.score.1 += new_scores.1;
                    s.num_universes += uni_score.num_universes * new_universes;
                }
            }

            let new_uni_score = scores.get(&new_universe).unwrap();
            if (new_uni_score.score.0 >= WIN_SCORE_P2) {
                num_wins.0 += new_uni_score.num_universes;
            } else if (new_uni_score.score.1 >= WIN_SCORE_P2) {
                num_wins.1 += new_uni_score.num_universes;
            } else {
                // Add universe to explore
                queue.push_back(new_universe);
            }
        }
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
    // assert_eq!(1337, solve_case2(input));
}
