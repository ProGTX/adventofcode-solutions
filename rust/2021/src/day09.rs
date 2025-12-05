use itertools::Itertools;
use std::collections::HashSet;

type Heightmap = aoc::grid::Grid<u32>;
type LowPoints = Vec<(usize, u32)>;

fn parse(filename: &str) -> Heightmap {
    aoc::grid::from_file(filename)
}

fn get_low_points(heightmap: &Heightmap) -> LowPoints {
    heightmap
        .data
        .iter()
        .enumerate()
        .filter(|&(linear_index, current)| {
            heightmap
                .basic_neighbor_values(heightmap.position(linear_index))
                .iter()
                .all(|neighbor| current < neighbor)
        })
        .map(|(linear_index, &height)| (linear_index, height))
        .collect()
}

fn solve_case1(heightmap: &Heightmap) -> u32 {
    get_low_points(heightmap)
        .iter()
        .map(|(_, height)| 1 + height)
        .sum()
}

fn solve_case2(heightmap: &Heightmap) -> u32 {
    get_low_points(heightmap)
        .iter()
        .map(|(origin, _)| {
            let mut visited = HashSet::new();
            let mut unvisited = Vec::new();
            unvisited.push(*origin);
            while (!unvisited.is_empty()) {
                let index = unvisited.pop().unwrap();
                if (!visited.insert(index)) {
                    continue;
                }
                let pos = heightmap.position(index);
                let current = heightmap.get(pos.y, pos.x);
                for neighbor in heightmap.basic_neighbor_positions(pos) {
                    let neighbor_index = heightmap.linear_index(neighbor.y, neighbor.x);
                    let neighbor = heightmap.get(neighbor.y, neighbor.x);
                    if ((*neighbor != 9)
                        && (neighbor > current)
                        && (!visited.contains(&neighbor_index)))
                    {
                        unvisited.push(neighbor_index);
                    }
                }
            }
            visited.len() as u32
        })
        .sorted_unstable()
        .rev()
        .take(3)
        .product()
}

fn main() {
    println!("Part 1");
    let example = parse("day09.example");
    assert_eq!(15, solve_case1(&example));
    let input = parse("day09.input");
    assert_eq!(580, solve_case1(&input));

    println!("Part 2");
    assert_eq!(1134, solve_case2(&example));
    assert_eq!(856716, solve_case2(&input));
}
