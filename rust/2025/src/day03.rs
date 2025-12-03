use arrayvec::ArrayVec;

type Batteries = aoc::grid::Grid<u64>;

fn parse(filename: &str) -> Batteries {
    aoc::grid::from_file(filename)
}

fn index_of_max(row: &[u64]) -> usize {
    // We need to ensure that if two elements compare equal, take the first one
    row.iter()
        .enumerate()
        .max_by(|(index1, value1), (index2, value2)| value1.cmp(value2).then(index2.cmp(index1)))
        .unwrap()
        .0
}

fn solve_case<const TURN_NUM: usize>(batteries: &Batteries) -> u64 {
    (0..batteries.num_rows)
        .map(|row| {
            let current = batteries.row(row);
            let size = current.len();
            let mut indexes = ArrayVec::<usize, TURN_NUM>::new();
            indexes.push(index_of_max(&current[..size - (TURN_NUM - 1)]));
            for tail in (0..(TURN_NUM - 1)).rev() {
                let offset = indexes.last().unwrap();
                indexes.push(index_of_max(&current[offset + 1..size - tail]) + offset + 1);
            }
            indexes
                .iter()
                .rev()
                .fold((0, 1), |(acc, multiplier), &index| {
                    (acc + current[index] * multiplier, multiplier * 10)
                })
                .0
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day03.example");
    assert_eq!(357, solve_case::<2>(&example));
    let input = parse("day03.input");
    assert_eq!(17166, solve_case::<2>(&input));

    println!("Part 2");
    assert_eq!(3121910778619, solve_case::<12>(&example));
    assert_eq!(169077317650774, solve_case::<12>(&input));
}
