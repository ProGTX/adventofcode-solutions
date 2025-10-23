use std::fs::File;
use std::io::{self, BufRead, BufReader};

fn solve_case1(filename: &str) -> io::Result<i32> {
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let mut sum = 0;
    for line in reader.lines() {
        let boxdims: Vec<i32> = line?
            .split('x')
            .map(|s| s.parse::<i32>().unwrap())
            .collect();
        let sides = [
            boxdims[0] * boxdims[1],
            boxdims[0] * boxdims[2],
            boxdims[1] * boxdims[2],
        ];
        sum += sides.iter().map(|s| s * 2).sum::<i32>();
        sum += sides.iter().min().unwrap();
    }
    Ok(sum)
}

fn solve_case2(filename: &str) -> io::Result<i32> {
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let mut ribbon = 0;
    for line in reader.lines() {
        let mut boxdims: Vec<i32> = line?
            .split('x')
            .map(|s| s.parse::<i32>().unwrap())
            .collect();
        boxdims.sort();
        ribbon += 2 * (boxdims[0] + boxdims[1]);
        ribbon += boxdims.iter().product::<i32>();
    }
    Ok(ribbon)
}

fn main() {
    println!("Part 1");
    assert_eq!((58 + 43), solve_case1("day02.example").unwrap());
    assert_eq!(1606483, solve_case1("day02.input").unwrap());
    println!("Part 2");
    assert_eq!((34 + 14), solve_case2("day02.example").unwrap());
    assert_eq!(3842356, solve_case2("day02.input").unwrap());
}
