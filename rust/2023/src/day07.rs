type Input = Vec<(String, i32)>;

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)
        .into_iter()
        .map(|line| {
            let (hand_str, bid_str) = line.split_once(' ').unwrap();
            (hand_str.to_string(), bid_str.parse().unwrap())
        })
        .collect()
}

const JOKER_CARD_NUMBER: i32 = 1;
const ACE_CARD_NUMBER: usize = 14;

fn card_to_number<const JOKER: bool>(c: char) -> i32 {
    if c.is_ascii_digit() {
        let number = c as i32 - '0' as i32;
        debug_assert!(
            number > JOKER_CARD_NUMBER && number < 10,
            "Invalid card number"
        );
        return number;
    }
    match c {
        'T' => 10,
        'J' => {
            if JOKER {
                JOKER_CARD_NUMBER
            } else {
                11
            }
        }
        'Q' => 12,
        'K' => 13,
        'A' => ACE_CARD_NUMBER as i32,
        _ => panic!("Invalid card number"),
    }
}

#[derive(PartialEq, Eq, PartialOrd, Ord, Clone, Copy)]
enum HandKind {
    HighCard,
    OnePair,
    TwoPair,
    ThreeKind,
    FullHouse,
    FourKind,
    FiveKind,
}

#[derive(Clone, Copy, PartialEq, Eq)]
struct Hand<const JOKER: bool> {
    cards: [i32; 5],
}

impl<const JOKER: bool> Hand<JOKER> {
    fn new(cards_str: &str) -> Self {
        let mut cards = [0i32; 5];
        for (i, c) in cards_str.chars().enumerate() {
            cards[i] = card_to_number::<JOKER>(c);
        }
        Hand { cards }
    }

    fn kind(&self) -> HandKind {
        let mut card_map = [0i32; ACE_CARD_NUMBER];
        let mut num_jokers = 0i32;
        for &c in &self.cards {
            if c == JOKER_CARD_NUMBER {
                if JOKER {
                    num_jokers += 1;
                }
            } else {
                // Minus 2 to exclude 0 and 1
                card_map[(c - 2) as usize] += 1;
            }
        }
        card_map.sort_unstable_by(|a, b| b.cmp(a));
        if JOKER {
            card_map[0] += num_jokers;
        }
        match card_map[0] {
            5 => HandKind::FiveKind,
            4 => HandKind::FourKind,
            3 => {
                if card_map[1] == 2 {
                    HandKind::FullHouse
                } else {
                    HandKind::ThreeKind
                }
            }
            2 => {
                if card_map[1] == 2 {
                    HandKind::TwoPair
                } else {
                    HandKind::OnePair
                }
            }
            _ => HandKind::HighCard,
        }
    }
}

impl<const JOKER: bool> Ord for Hand<JOKER> {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.kind()
            .cmp(&other.kind())
            .then_with(|| self.cards.cmp(&other.cards))
    }
}

impl<const JOKER: bool> PartialOrd for Hand<JOKER> {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

fn total_winnings<const JOKER: bool>(mut bids: Vec<(Hand<JOKER>, i32)>) -> i32 {
    bids.sort_unstable();
    bids.iter()
        .enumerate()
        .map(|(i, (_, bid))| (i as i32 + 1) * *bid)
        .sum()
}

fn solve_case<const JOKER: bool>(input: &Input) -> i32 {
    let bids = input
        .iter()
        .map(|(hand_str, bid)| (Hand::<JOKER>::new(hand_str), *bid))
        .collect();
    total_winnings(bids)
}

fn main() {
    println!("Part 1");
    let example = parse("day07.example");
    aoc::expect_result!(6440, solve_case::<false>(&example));
    let input = parse("day07.input");
    aoc::expect_result!(249638405, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(5905, solve_case::<true>(&example));
    aoc::expect_result!(249776650, solve_case::<true>(&input));
}
