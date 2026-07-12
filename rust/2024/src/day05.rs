// For each page we store a list of pages that must come before it and after it.
#[derive(Default, Clone)]
struct OrderingRule {
    before: Vec<i32>,
    after: Vec<i32>,
}

struct Input {
    rules: Vec<OrderingRule>,
    updates: Vec<Vec<i32>>,
}

fn parse(filename: &str) -> Input {
    let lines = aoc::file::read_lines(filename);
    let separator = lines.iter().position(|line| line.is_empty()).unwrap();

    // We use the vector as a hashmap, so we need to use enough memory
    // for all the pages.
    let rules =
        lines[..separator]
            .iter()
            .fold(vec![OrderingRule::default(); 100], |mut rules, line| {
                let mut parts = line.split('|');
                let before: i32 = parts.next().unwrap().parse().unwrap();
                let after: i32 = parts.next().unwrap().parse().unwrap();
                rules[before as usize].after.push(after);
                rules[after as usize].before.push(before);
                rules
            });

    let updates: Vec<Vec<i32>> = lines[separator + 1..]
        .iter()
        .map(|line| line.split(',').map(|s| s.parse().unwrap()).collect())
        .collect();

    Input { rules, updates }
}

fn index_of_correct_update(rules: &[OrderingRule], update: &[i32]) -> i32 {
    let is_ordered = (0..update.len()).all(|u| {
        let page = update[u] as usize;
        // Check that pages before are not in the rules list of pages after.
        let before_ok = update[..u]
            .iter()
            .all(|earlier| !rules[page].after.contains(earlier));
        // Check that pages after are not in the rules list of pages before.
        let after_ok = update[u + 1..]
            .iter()
            .all(|later| !rules[page].before.contains(later));
        before_ok && after_ok
    });
    if is_ordered {
        (update.len() / 2) as i32
    } else {
        -1
    }
}

// Orders the update and returns the middle element.
fn order_update(rules: &[OrderingRule], update: &[i32]) -> i32 {
    let mut ordered_update = update.to_vec();
    ordered_update.sort_by(|&lhs, &rhs| {
        if rules[rhs as usize].before.contains(&lhs) {
            std::cmp::Ordering::Less
        } else {
            std::cmp::Ordering::Greater
        }
    });
    ordered_update[ordered_update.len() / 2]
}

fn solve_case<const ORDER: bool>(input: &Input) -> i32 {
    input
        .updates
        .iter()
        .map(|update| {
            let index = index_of_correct_update(&input.rules, update);
            if !ORDER {
                if index >= 0 {
                    update[index as usize]
                } else {
                    0
                }
            } else if index < 0 {
                order_update(&input.rules, update)
            } else {
                0
            }
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example2 = parse("day05.example2");
    aoc::expect_result!(61, solve_case::<false>(&example2));
    let example3 = parse("day05.example3");
    aoc::expect_result!(0, solve_case::<false>(&example3));
    let example = parse("day05.example");
    aoc::expect_result!(143, solve_case::<false>(&example));
    let input = parse("day05.input");
    aoc::expect_result!(6034, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(0, solve_case::<true>(&example2));
    aoc::expect_result!(47, solve_case::<true>(&example3));
    aoc::expect_result!(123, solve_case::<true>(&example));
    aoc::expect_result!(6305, solve_case::<true>(&input));
}
