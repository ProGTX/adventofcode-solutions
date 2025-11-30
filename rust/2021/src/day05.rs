use aoc::point::Point;

type Segments = Vec<(Point<i32>, Point<i32>)>;

fn parse(filename: &str) -> (Segments, Point<i32>) {
    let mut dimensions = Point::default();
    let segments = aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let (begin, end) = line.split_once(" -> ").unwrap();
            let segment = (
                Point::parse(begin, ",").unwrap(),
                Point::parse(end, ",").unwrap(),
            );
            dimensions = dimensions.max(&segment.0).max(&segment.1);
            segment
        })
        .collect::<Vec<_>>();
    (segments, dimensions + Point { x: 1, y: 1 })
}

fn solve_case<const DIAGONAL: bool>((segments, dimensions): &(Segments, Point<i32>)) -> usize {
    let mut intersections =
        aoc::grid::Grid::<u32>::new(0, dimensions.y as usize, dimensions.x as usize);
    for (begin, end) in segments {
        if ((begin.x != end.x) && (begin.y != end.y)) {
            if (!DIAGONAL) {
                continue;
            }
        }
        let diff = *end - *begin;
        let direction = diff.safe_divide(diff.abs(), 0);
        let mut pos = begin.clone();
        while (pos != *end) {
            let current = intersections.get_mut(pos.y as usize, pos.x as usize);
            *current += 1;
            pos += direction;
        }
        // Once more for the end point
        let current = intersections.get_mut(pos.y as usize, pos.x as usize);
        *current += 1;
    }
    intersections.data.iter().filter(|&&v| v > 1).count()
}

fn main() {
    println!("Part 1");
    let example = parse("day05.example");
    assert_eq!(5, solve_case::<false>(&example));
    let input = parse("day05.input");
    assert_eq!(6461, solve_case::<false>(&input));

    println!("Part 2");
    assert_eq!(12, solve_case::<true>(&example));
    assert_eq!(18065, solve_case::<true>(&input));
}
