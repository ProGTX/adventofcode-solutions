struct Card {
    winning: Vec<i32>,
    actual: Vec<i32>,
}

fn parse(filename: &str) -> Vec<Card> {
    aoc::file::read_lines(filename)
        .into_iter()
        .map(|line| {
            let (_, numbers) = line.split_once(':').unwrap();
            let (winning_str, actual_str) = numbers.split_once('|').unwrap();
            // Cards need to be sorted in order for intersection to work
            let mut winning: Vec<i32> = winning_str
                .split_whitespace()
                .map(|n| n.parse().unwrap())
                .collect();
            let mut actual: Vec<i32> = actual_str
                .split_whitespace()
                .map(|n| n.parse().unwrap())
                .collect();
            winning.sort_unstable();
            actual.sort_unstable();
            Card { winning, actual }
        })
        .collect()
}

fn num_matching(card: &Card) -> usize {
    let (mut wi, mut ai) = (0, 0);
    let mut count = 0;
    while wi < card.winning.len() && ai < card.actual.len() {
        match card.winning[wi].cmp(&card.actual[ai]) {
            std::cmp::Ordering::Less => wi += 1,
            std::cmp::Ordering::Greater => ai += 1,
            std::cmp::Ordering::Equal => {
                count += 1;
                wi += 1;
                ai += 1;
            }
        }
    }
    count
}

fn card_value(card: &Card) -> i32 {
    let matching = num_matching(card);
    if matching == 0 {
        return 0;
    }
    1 << (matching - 1)
}

fn solve_case1(cards: &[Card]) -> i32 {
    cards.iter().map(card_value).sum()
}

fn solve_case2(cards: &[Card]) -> i32 {
    let mut num_scratchcards = vec![0i32; 1];
    let mut sum = 0i32;

    for (id, card) in cards.iter().enumerate() {
        // Add the original card
        num_scratchcards[id] += 1;
        let current_num = num_scratchcards[id];
        sum += current_num;

        let matching = num_matching(card);
        // Create copies of next cards
        let required_size = id + matching + 2;
        if num_scratchcards.len() < required_size {
            num_scratchcards.resize(required_size, 0);
        }
        for i in 1..=matching {
            num_scratchcards[id + i] += current_num;
        }
    }

    sum
}

fn main() {
    println!("Part 1");
    let example = parse("day04.example");
    aoc::expect_result!(13, solve_case1(&example));
    let input = parse("day04.input");
    aoc::expect_result!(32609, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(30, solve_case2(&example));
    aoc::expect_result!(14624680, solve_case2(&input));
}
