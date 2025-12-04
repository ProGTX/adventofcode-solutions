type PaperRolls = aoc::grid::Grid<char>;

fn parse(filename: &str) -> PaperRolls {
    aoc::grid::Grid::from_file(filename)
}

const PAPER: char = '@';
const EMPTY: char = '.';

fn accessible_indexes(paper_rolls: &PaperRolls) -> Vec<usize> {
    paper_rolls
        .data
        .iter()
        .enumerate()
        .filter(|(linear_index, current)| {
            (**current == PAPER)
                && (4 > paper_rolls
                    .all_neighbor_values(paper_rolls.position(*linear_index))
                    .iter()
                    .filter(|&v| *v == PAPER)
                    .count())
        })
        .map(|(index, _)| index)
        .collect()
}

fn solve_case1(paper_rolls: &PaperRolls) -> u32 {
    accessible_indexes(paper_rolls).len() as u32
}

fn solve_case2(input: &PaperRolls) -> u32 {
    let mut paper_rolls = input.clone();
    let mut indexes = accessible_indexes(&paper_rolls);
    let mut total = 0;
    while (!indexes.is_empty()) {
        total += indexes.len() as u32;
        for pos in indexes.iter().map(|index| input.position(*index)) {
            paper_rolls.modify(EMPTY, pos.y, pos.x);
        }
        indexes = accessible_indexes(&paper_rolls);
    }
    total
}

fn main() {
    println!("Part 1");
    let example = parse("day04.example");
    assert_eq!(13, solve_case1(&example));
    let input = parse("day04.input");
    assert_eq!(1553, solve_case1(&input));

    println!("Part 2");
    assert_eq!(43, solve_case2(&example));
    assert_eq!(8442, solve_case2(&input));
}
