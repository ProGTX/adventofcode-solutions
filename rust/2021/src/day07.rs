fn parse(filename: &str) -> Vec<i32> {
    let mut positions = aoc::file::read_string(filename)
        .trim()
        .split(',')
        .map(|crab| crab.parse().unwrap())
        .collect::<Vec<_>>();
    positions.sort();
    positions
}

fn solve_case<const PROGRESSIVE_COST: bool>(positions: &[i32]) -> i32 {
    // This could potentially be optimized based on the sorting
    // because it guarantees a local minimum is also a global one
    (positions[0]..positions[positions.len() - 1] + 1)
        .map(|(move_to)| {
            positions
                .iter()
                .map(|&crab| {
                    let distance = (crab - move_to).abs();
                    if (!PROGRESSIVE_COST) {
                        distance
                    } else {
                        (distance * (distance + 1)) / 2
                    }
                })
                .sum()
        })
        .min()
        .unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day07.example");
    assert_eq!(37, solve_case::<false>(&example));
    let input = parse("day07.input");
    assert_eq!(355592, solve_case::<false>(&input));

    println!("Part 2");
    assert_eq!(168, solve_case::<true>(&example));
    assert_eq!(101618069, solve_case::<true>(&input));
}
