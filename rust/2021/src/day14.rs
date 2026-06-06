use std::collections::HashMap;

type Polymer = String;
type Rules = HashMap<(char, char), char>;
type Input = (Polymer, Rules);

fn parse(filename: &str) -> Input {
    let mut polymer = String::new();
    let mut rules = HashMap::new();
    let mut parsing_rules = false;
    for line in aoc::file::read_lines(filename) {
        if line.is_empty() {
            parsing_rules = true;
        } else if parsing_rules {
            let (pair, insert) = line.split_once(" -> ").unwrap();
            let mut pair_chars = pair.chars();
            let a = pair_chars.next().unwrap();
            let b = pair_chars.next().unwrap();
            let c = insert.chars().next().unwrap();
            rules.insert((a, b), c);
        } else {
            polymer = line;
        }
    }
    (polymer, rules)
}

type Pairs = HashMap<(char, char), usize>;

fn apply_rules(pairs: &Pairs, rules: &Rules) -> Pairs {
    let mut new_pairs = Pairs::new();
    for (rule_pair, insert) in rules {
        match pairs.get(rule_pair) {
            None => {}
            Some(count) => {
                *new_pairs.entry((rule_pair.0, *insert)).or_insert(0) += count;
                *new_pairs.entry((*insert, rule_pair.1)).or_insert(0) += count;
            }
        }
    }
    return new_pairs;
}

fn solve_case<const STEPS: usize>((polymer, rules): &Input) -> usize {
    let chars: Vec<char> = polymer.chars().collect();
    let mut pairs = Pairs::new();
    for w in chars.windows(2) {
        *pairs.entry((w[0], w[1])).or_insert(0) += 1;
    }
    for _ in 0..STEPS {
        pairs = apply_rules(&pairs, rules);
    }
    let mut counts = [0usize; 26];
    for ((a, _), count) in &pairs {
        counts[*a as usize - 'A' as usize] += count;
    }
    counts[*chars.last().unwrap() as usize - 'A' as usize] += 1;
    let max = counts.iter().copied().max().unwrap();
    let min = counts.iter().copied().filter(|&c| c > 0).min().unwrap();
    return max - min;
}

fn main() {
    println!("Part 1");
    let example = parse("day14.example");
    aoc::expect_result!(1588, solve_case::<10>(&example));
    let input = parse("day14.input");
    aoc::expect_result!(2703, solve_case::<10>(&input));

    println!("Part 2");
    aoc::expect_result!(2188189693529, solve_case::<40>(&example));
    aoc::expect_result!(2984946368465, solve_case::<40>(&input));
}
