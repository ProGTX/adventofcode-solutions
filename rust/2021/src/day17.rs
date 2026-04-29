use aoc::point::Point;

type Input = (Point<i32>, Point<i32>);

fn parse(filename: &str) -> Input {
    let s = aoc::file::read_string(filename);
    let (xs, ys) = s
        .trim()
        .strip_prefix("target area: ")
        .unwrap()
        .split_once(", ")
        .unwrap();
    let x = xs.strip_prefix("x=").unwrap().split_once("..").unwrap();
    let y = ys.strip_prefix("y=").unwrap().split_once("..").unwrap();
    let rectangle = (
        // Invert the y so that we represent two opposite corners of a rectangle
        Point::new(x.0.parse().unwrap(), y.1.parse().unwrap()),
        Point::new(x.1.parse().unwrap(), y.0.parse().unwrap()),
    );
    // These assertions are used when solving the problem
    assert!(rectangle.0.x <= rectangle.1.x);
    assert!(rectangle.0.x > 0);
    assert!(rectangle.0.y >= rectangle.1.y);
    assert!(rectangle.0.y < 0);
    return rectangle;
}

fn solve_case1((_, bottom_right): &Input) -> i32 {
    let max_dy = -bottom_right.y - 1;
    return (max_dy * (max_dy + 1)) / 2;
}

fn hits_target(mut dx: i32, mut dy: i32, top_left: &Point<i32>, bottom_right: &Point<i32>) -> bool {
    let (mut x, mut y) = (0, 0);
    loop {
        x += dx;
        y += dy;
        dx = (dx - 1).max(0);
        dy -= 1;
        if (true
            && (x >= top_left.x)
            && (x <= bottom_right.x)
            && (y >= bottom_right.y)
            && (y <= top_left.y))
        {
            return true;
        }
        if ((y < bottom_right.y) || (x > bottom_right.x)) {
            return false;
        }
    }
}

fn solve_case2((top_left, bottom_right): &Input) -> i32 {
    let min_dx = {
        let mut dx = 0;
        while ((dx * (dx + 1)) / 2 < top_left.x) {
            dx += 1;
        }
        dx
    };
    let max_dx = bottom_right.x;
    let min_dy = bottom_right.y;
    let max_dy = -bottom_right.y - 1; // From part 1
    (min_dx..=max_dx)
        .flat_map(|dx| (min_dy..=max_dy).map(move |dy| (dx, dy)))
        .filter(|&(dx, dy)| hits_target(dx, dy, top_left, bottom_right))
        .count() as i32
}

fn main() {
    println!("Part 1");
    let example = parse("day17.example");
    assert_eq!(45, solve_case1(&example));
    let input = parse("day17.input");
    assert_eq!(5565, solve_case1(&input));

    println!("Part 2");
    assert_eq!(112, solve_case2(&example));
    assert_eq!(2118, solve_case2(&input));
}
