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
            let (column, row) = heightmap.position(linear_index);
            heightmap
                .basic_neighbor_values(row, column)
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

fn main() {
    println!("Part 1");
    let example = parse("day09.example");
    assert_eq!(15, solve_case1(&example));
    let input = parse("day09.input");
    assert_eq!(580, solve_case1(&input));

    //println!("Part 2");
    //assert_eq!(1134, solve_case2(&example));
    //assert_eq!(856716, solve_case2(&input));
}
