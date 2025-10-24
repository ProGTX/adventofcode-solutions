use std::collections::HashMap;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn deliveries<const NUM_SANTAS: usize>(directions: &str) -> i32 {
    let mut santas = [(0, 0); NUM_SANTAS];
    let mut visited: [HashMap<(i32, i32), usize>; NUM_SANTAS] = std::array::from_fn(|_| {
        let mut map = HashMap::new();
        map.insert((0, 0), 1);
        map
    });
    let mut santa_id = 0;
    for c in directions.chars() {
        let (dx, dy) = match c {
            '<' => (-1, 0),
            '>' => (1, 0),
            '^' => (0, -1),
            'v' => (0, 1),
            _ => (0, 0),
        };
        santas[santa_id].0 += dx;
        santas[santa_id].1 += dy;
        *visited[santa_id].entry(santas[santa_id]).or_default() += 1;
        santa_id = (santa_id + 1) % NUM_SANTAS;
    }
    if NUM_SANTAS > 1 {
        let (left, right) = visited.split_at_mut(1);
        left[0].extend(std::mem::take(&mut right[0]));
    }
    visited[0].len() as i32
}

fn solve_case1(filename: &str) -> std::io::Result<i32> {
    let file = File::open(filename)?;
    let mut line = String::new();
    BufReader::new(file).read_line(&mut line)?;
    Ok(deliveries::<1>(&line))
}

fn solve_case2(filename: &str) -> std::io::Result<i32> {
    let file = File::open(filename)?;
    let mut line = String::new();
    BufReader::new(file).read_line(&mut line)?;
    Ok(deliveries::<2>(&line))
}

fn main() {
    println!("Part 1");
    assert_eq!(4, solve_case1("day03.example").unwrap());
    assert_eq!(2565, solve_case1("day03.input").unwrap());
    println!("Part 2");
    assert_eq!(3, solve_case2("day03.example").unwrap());
    assert_eq!(2639, solve_case2("day03.input").unwrap());
}
