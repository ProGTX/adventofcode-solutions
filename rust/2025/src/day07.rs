use std::collections::HashSet;

type Manifolds = aoc::grid::Grid<char>;
type Upos = aoc::grid::Upos;
const WALL: char = '#';
const EMPTY: char = '.';
const SPLITTER: char = '^';

fn parse(filename: &str) -> (Manifolds, Upos) {
    let (manifolds, config) = Manifolds::from_file_config(
        filename,
        aoc::grid::ConfigInput {
            padding: Some(WALL),
            start_char: Some('S'),
            end_char: None,
        },
    );
    (manifolds, config.start_pos.unwrap())
}

fn solve_case1((manifolds, start_pos): &(Manifolds, Upos)) -> usize {
    let mut beams = HashSet::new();
    beams.insert(*start_pos);
    let mut num_splits = 0;
    while (!beams.is_empty()) {
        let mut next_beams = HashSet::new();
        for beam_pos_orig in beams {
            let beam_pos = beam_pos_orig + Upos::new(0, 1);
            let value = manifolds.get(beam_pos.y, beam_pos.x);
            match *value {
                EMPTY => {
                    next_beams.insert(beam_pos);
                }
                SPLITTER => {
                    num_splits += 1;
                    next_beams.insert(Upos::new(beam_pos.x - 1, beam_pos.y));
                    next_beams.insert(beam_pos + Upos::new(1, 0));
                }
                WALL => {
                    // Do nothing
                }
                _ => unreachable!("Invalid value in the manifold"),
            };
        }
        beams = next_beams;
    }
    return num_splits;
}

fn main() {
    println!("Part 1");
    let example = parse("day07.example");
    assert_eq!(21, solve_case1(&example));
    let input = parse("day07.input");
    assert_eq!(1560, solve_case1(&input));

    // println!("Part 2");
    // assert_eq!(40, solve_case2(&example));
    // assert_eq!(9876636978528, solve_case2(&input));
}
