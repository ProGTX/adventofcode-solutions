use std::fs::File;
use std::io::{self, BufRead, BufReader};

fn solve_case<const PART2: bool>(filename: &str) -> io::Result<i32> {
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

fn main() {
    println!("Part 1");
    assert_eq!((58 + 43), solve_case::<false>("day02.example").unwrap());
    assert_eq!(1606483, solve_case::<false>("day02.input").unwrap());
    // println!("Part 2");
    // assert_eq!(5, solve_case::<true>("day02.example").unwrap());
    // assert_eq!(1797, solve_case::<true>("day02.input").unwrap());
}
