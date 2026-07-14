use aoc::dijkstra::DijkstraState;
use itertools::Itertools;
use std::cell::Cell;

type Input = (Vec<String>, Vec<String>);

fn parse(filename: &str) -> Input {
    let lines = aoc::file::read_lines(filename);
    let patterns = lines[0].split(',').map(|p| p.trim().to_string()).collect();
    let designs = lines[1..]
        .iter()
        .filter(|line| !line.is_empty())
        .cloned()
        .collect();
    (designs, patterns)
}

fn num_possible_designs_reachable(design: &str, patterns: &[String]) -> i64 {
    let end_found = Cell::new(false);
    aoc::dijkstra::shortest_distances(
        &String::new(),
        |current: &String| {
            if current.len() == design.len() {
                end_found.set(true);
            }
            end_found.get()
        },
        |current: &String| {
            patterns
                .iter()
                .filter_map(|pattern| {
                    let neighbor = format!("{current}{pattern}");
                    design.starts_with(neighbor.as_str()).then(|| {
                        let distance = (design.len() - neighbor.len() + 1) as u32;
                        DijkstraState {
                            data: neighbor,
                            distance,
                        }
                    })
                })
                .collect::<Vec<_>>()
        },
    );
    end_found.get() as i64
}

fn num_possible_designs_all(design: &str, patterns: &[String]) -> i64 {
    let start = design.to_string();
    let counts = aoc::algorithm::dfs_uniform(
        start.clone(),
        |remaining: &String| remaining.is_empty(),
        |remaining: &String| {
            patterns
                .iter()
                .filter(|pattern| remaining.starts_with(pattern.as_str()))
                .map(|pattern| remaining[pattern.len()..].to_string())
                .collect_vec()
        },
    );
    counts[&start]
}

fn solve_case<const ALL_OPTIONS: bool>((designs, patterns): &Input) -> i64 {
    designs
        .iter()
        .map(|design| {
            if ALL_OPTIONS {
                num_possible_designs_all(design, patterns)
            } else {
                num_possible_designs_reachable(design, patterns)
            }
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day19.example");
    aoc::expect_result!(6, solve_case::<false>(&example));
    let input = parse("day19.input");
    aoc::expect_result!(317, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(16, solve_case::<true>(&example));
    aoc::expect_result!(883443544805484, solve_case::<true>(&input));
}
