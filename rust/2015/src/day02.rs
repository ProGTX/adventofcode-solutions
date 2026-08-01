use std::fs::File;
use std::io::{self, BufRead, BufReader};

fn parse(filename: &str) -> io::Result<Vec<Vec<i32>>> {
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let mut boxes = Vec::new();
    for line in reader.lines() {
        boxes.push(
            line?
                .split('x')
                .map(|s| s.parse::<i32>().unwrap())
                .collect::<Vec<i32>>(),
        );
    }
    Ok(boxes)
}

fn solve_case1(boxes: &[Vec<i32>]) -> i32 {
    let mut sum = 0;
    for boxdims in boxes {
        let sides = [
            boxdims[0] * boxdims[1],
            boxdims[0] * boxdims[2],
            boxdims[1] * boxdims[2],
        ];
        sum += sides.iter().map(|s| s * 2).sum::<i32>();
        sum += sides.iter().min().unwrap();
    }
    sum
}

fn solve_case2(boxes: &[Vec<i32>]) -> i32 {
    let mut ribbon = 0;
    for boxdims in boxes {
        let mut boxdims = boxdims.clone();
        boxdims.sort();
        ribbon += 2 * (boxdims[0] + boxdims[1]);
        ribbon += boxdims.iter().product::<i32>();
    }
    ribbon
}

fn main() {
    println!("Part 1");
    let example = parse("day02.example").unwrap();
    aoc::expect_result!((58 + 43), solve_case1(&example));
    let input = parse("day02.input").unwrap();
    aoc::expect_result!(1606483, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!((34 + 14), solve_case2(&example));
    aoc::expect_result!(3842356, solve_case2(&input));
}
