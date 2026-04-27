use std::collections::HashSet;

use aoc::string::NameToId;

type Terminator = std::ops::Range<usize>;
type Adjacency = Vec<Vec<usize>>;
type Input = (Adjacency, Terminator, Vec<bool>);

fn parse(filename: &str) -> Input {
    let mut name_to_id = NameToId::new();
    let mut is_big_cave = Vec::new();
    let mut connections = Adjacency::new();

    for line in aoc::file::read_lines(filename) {
        let (start, end) = line.split_once('-').unwrap();

        let start_id = name_to_id.intern(start);
        if (start_id >= is_big_cave.len()) {
            is_big_cave.push(start.chars().next().unwrap().is_uppercase());
            connections.push(Vec::new());
        }

        let end_id = name_to_id.intern(end);
        if (end_id >= is_big_cave.len()) {
            is_big_cave.push(end.chars().next().unwrap().is_uppercase());
            connections.push(Vec::new());
        }

        connections[start_id].push(end_id);
        connections[end_id].push(start_id);
    }

    return (
        connections,
        Terminator {
            start: name_to_id.expect("start"),
            end: name_to_id.expect("end"),
        },
        is_big_cave,
    );
}

fn visit_caves_dfs<const ALLOW_TWICE: bool>(
    current: usize,
    connections: &Adjacency,
    is_big_cave: &Vec<bool>,
    visited_small: &mut HashSet<usize>,
    terminator: &Terminator,
    second_visit: Option<usize>,
) -> usize {
    if (current == terminator.end) {
        return 1;
    }
    let mut inserted = false;
    if (!is_big_cave[current]) {
        if (ALLOW_TWICE && visited_small.contains(&current)) {
            // Don't insert again
        } else {
            inserted = visited_small.insert(current);
        }
    }
    let mut count = 0;
    for neighbor in &connections[current] {
        if (is_big_cave[*neighbor] || !visited_small.contains(&neighbor)) {
            count += visit_caves_dfs::<ALLOW_TWICE>(
                *neighbor,
                connections,
                is_big_cave,
                visited_small,
                terminator,
                second_visit,
            );
        } else if (ALLOW_TWICE
            && second_visit.is_none()
            && (*neighbor != terminator.start)
            && (*neighbor != terminator.end))
        {
            count += visit_caves_dfs::<ALLOW_TWICE>(
                *neighbor,
                connections,
                is_big_cave,
                visited_small,
                terminator,
                Some(*neighbor),
            );
        }
    }
    if (inserted) {
        visited_small.remove(&current);
    }
    return count;
}

fn solve_case<const ALLOW_TWICE: bool>((connections, terminator, is_big_cave): &Input) -> usize {
    let mut visited_small = HashSet::new();
    return visit_caves_dfs::<ALLOW_TWICE>(
        terminator.start,
        connections,
        is_big_cave,
        &mut visited_small,
        &terminator,
        None,
    );
}

fn main() {
    println!("Part 1");

    let example1 = parse("day12.example");
    assert_eq!(10, solve_case::<false>(&example1));
    let example2 = parse("day12.example2");
    assert_eq!(19, solve_case::<false>(&example2));
    let example3 = parse("day12.example3");
    assert_eq!(226, solve_case::<false>(&example3));
    let input = parse("day12.input");
    assert_eq!(4104, solve_case::<false>(&input));

    println!("Part 2");
    assert_eq!(36, solve_case::<true>(&example1));
    assert_eq!(103, solve_case::<true>(&example2));
    assert_eq!(3509, solve_case::<true>(&example3));
    assert_eq!(119760, solve_case::<true>(&input));
}
