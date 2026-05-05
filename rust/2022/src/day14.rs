use aoc::grid::Grid;

type Point = aoc::point::Point<i32>;
type RockLine = [Point; 2];

const EMPTY: char = '.';
const ROCK: char = '#';
const SAND_START: char = '+';
const SAND: char = 'o';

fn parse_rock_lines(line: &str) -> Vec<RockLine> {
    let points: Vec<Point> = line
        .split(" -> ")
        .map(|s| Point::parse(s.trim(), ",").unwrap())
        .collect();
    points.windows(2).map(|w| [w[0], w[1]]).collect()
}

fn parse(filename: &str) -> Vec<RockLine> {
    aoc::file::read_lines(filename)
        .iter()
        .flat_map(|line| parse_rock_lines(line))
        .collect()
}

fn make_cave_map<const WITH_GROUND: bool>(input: &[RockLine]) -> (Grid<char>, Point) {
    let mut rock_lines: Vec<RockLine> = input.to_vec();
    let sand_start = Point::new(500, 0);

    let (mut min_x, mut max_x, mut min_y, mut max_y) = rock_lines
        .iter()
        .flat_map(|rl| rl.iter().copied())
        .chain([sand_start])
        .fold(
            (i32::MAX, i32::MIN, i32::MAX, i32::MIN),
            |(mn_x, mx_x, mn_y, mx_y), p| {
                (mn_x.min(p.x), mx_x.max(p.x), mn_y.min(p.y), mx_y.max(p.y))
            },
        );

    if WITH_GROUND {
        let height = max_y - min_y + 2;
        let ground = [
            sand_start + Point::new(-height, height),
            sand_start + Point::new(height, height),
        ];
        min_x = min_x.min(ground[0].x).min(ground[1].x);
        max_x = max_x.max(ground[0].x).max(ground[1].x);
        min_y = min_y.min(ground[0].y).min(ground[1].y);
        max_y = max_y.max(ground[0].y).max(ground[1].y);
        rock_lines.push(ground);
    }

    let min_point = Point::new(min_x, min_y);
    let num_rows = (max_y - min_y + 1) as usize;
    let num_cols = (max_x - min_x + 1) as usize;
    let mut cave_map = Grid::new(EMPTY, num_rows, num_cols);

    let adjust = |p: Point| p - min_point;

    for rl in &rock_lines {
        let diff = rl[1] - rl[0];
        let step = Point::new(diff.x.signum(), diff.y.signum());
        let mut pos = rl[0];
        while pos != rl[1] {
            let a = adjust(pos);
            cave_map.modify(ROCK, a.y as usize, a.x as usize);
            pos = pos + step;
        }
        let a = adjust(rl[1]);
        cave_map.modify(ROCK, a.y as usize, a.x as usize);
    }

    let adj_start = adjust(sand_start);
    cave_map.modify(SAND_START, adj_start.y as usize, adj_start.x as usize);

    (cave_map, adj_start)
}

fn try_insert_grain(cave_map: &mut Grid<char>, start_row: i32, start_col: i32) -> bool {
    let is_empty = |v: char| v == EMPTY || v == SAND_START;

    let mut row = start_row;
    let mut col = start_col;
    let mut within_bounds = true;

    'sim: while within_bounds {
        for (dr, dc) in [(1i32, 0i32), (1, -1), (1, 1)] {
            let nr = row + dr;
            let nc = col + dc;
            if !cave_map.in_bounds_signed(nr as isize, nc as isize) {
                row = nr;
                col = nc;
                within_bounds = false;
                continue 'sim;
            }
            if is_empty(*cave_map.get(nr as usize, nc as usize)) {
                row = nr;
                col = nc;
                continue 'sim;
            }
        }
        within_bounds = false;
    }

    if cave_map.in_bounds_signed(row as isize, col as isize)
        && is_empty(*cave_map.get(row as usize, col as usize))
    {
        cave_map.modify(SAND, row as usize, col as usize);
        true
    } else {
        false
    }
}

fn solve_case<const WITH_GROUND: bool>(input: &[RockLine]) -> i32 {
    let (mut cave_map, adjusted_start) = make_cave_map::<WITH_GROUND>(input);
    let mut num_grains = 0i32;
    let index_increase = cave_map.num_columns;

    loop {
        let mut sand_success = false;
        let start_idx = cave_map.linear_index(adjusted_start.y as usize, adjusted_start.x as usize);
        let mut previous_index = start_idx;
        let mut linear_index = previous_index + index_increase;

        while linear_index < cave_map.data.len() {
            let value = cave_map.data[linear_index];
            debug_assert!(value != SAND_START);
            if value == ROCK || value == SAND {
                let row = (previous_index / index_increase) as i32;
                let col = (previous_index % index_increase) as i32;
                sand_success = try_insert_grain(&mut cave_map, row, col);
                break;
            }
            previous_index = linear_index;
            linear_index += index_increase;
        }

        if !sand_success {
            break;
        }
        num_grains += 1;
    }

    num_grains
}

fn main() {
    println!("Part 1");
    let example = parse("day14.example");
    assert_eq!(24, solve_case::<false>(&example));
    let input = parse("day14.input");
    assert_eq!(655, solve_case::<false>(&input));

    println!("Part 2");
    assert_eq!(93, solve_case::<true>(&example));
    assert_eq!(26484, solve_case::<true>(&input));
}
