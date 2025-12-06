use itertools::Itertools;

fn parse(filename: &str) -> Vec<String> {
    aoc::file::read_lines(filename)
}

const OPENING_BRACES: &str = "([{<";
const CLOSING_BRACES: &str = ")]}>";
const ILLEGAL_SCORES: [usize; 4] = [3, 57, 1197, 25137];

fn get_score<const AUTOCOMPLETE: bool>(navigation_line: &String) -> usize {
    debug_assert!(navigation_line.len() > 0, "Not handling empty strings");
    debug_assert!(
        CLOSING_BRACES
            .find(navigation_line.chars().next().unwrap())
            .is_none(),
        "Shouldn't start with a closing brace"
    );
    let mut closing_braces_stack = Vec::new();
    for c in navigation_line.chars() {
        let pos = CLOSING_BRACES.find(c);
        if (pos.is_some()) {
            if (c == *closing_braces_stack.last().unwrap()) {
                closing_braces_stack.pop();
            } else if (!AUTOCOMPLETE) {
                return ILLEGAL_SCORES[pos.unwrap()];
            } else {
                return 0;
            }
        } else {
            let pos = OPENING_BRACES.find(c).unwrap();
            closing_braces_stack.push(CLOSING_BRACES.chars().skip(pos).next().unwrap());
        }
    }
    let mut score = 0;
    if (AUTOCOMPLETE) {
        for &brace in closing_braces_stack.iter().rev() {
            score *= 5;
            score += CLOSING_BRACES.find(brace).unwrap() + 1;
        }
    }
    return score;
}

fn solve_case1(navigation_lines: &[String]) -> usize {
    navigation_lines.iter().map(get_score::<false>).sum()
}

fn solve_case2(navigation_lines: &[String]) -> usize {
    let scores = navigation_lines
        .iter()
        .map(get_score::<true>)
        .sorted_unstable()
        .collect::<Vec<_>>();
    let offset = scores.iter().position(|&v| v != 0).unwrap();
    scores[offset + (scores.len() - offset) / 2]
}

fn main() {
    println!("Part 1");
    let example = parse("day10.example");
    assert_eq!(26397, solve_case1(&example));
    let input = parse("day10.input");
    assert_eq!(319233, solve_case1(&input));

    println!("Part 2");
    assert_eq!(288957, solve_case2(&example));
    assert_eq!(1118976874, solve_case2(&input));
}
