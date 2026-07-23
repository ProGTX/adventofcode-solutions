use aoc::grid::Ipos;

struct Robot {
    position: Ipos,
    velocity: Ipos,
}

fn parse(filename: &str) -> Vec<Robot> {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            let (pos, vel) = line.split_once(' ').unwrap();
            let position = Ipos::parse(&pos[2..], ",").unwrap();
            let velocity = Ipos::parse(&vel[2..], ",").unwrap();
            Robot { position, velocity }
        })
        .collect()
}

fn move_robot<const GRID_WIDTH: isize, const GRID_HEIGHT: isize, const SECONDS: isize>(
    robot: &Robot,
) -> Ipos {
    let grid_size = Ipos::new(GRID_WIDTH, GRID_HEIGHT);
    (robot.position + (grid_size + robot.velocity).scale(SECONDS)) % grid_size
}

fn move_all_robots<const GRID_WIDTH: isize, const GRID_HEIGHT: isize, const SECONDS: isize>(
    mut robots: Vec<Robot>,
) -> Vec<Robot> {
    for robot in &mut robots {
        robot.position = move_robot::<GRID_WIDTH, GRID_HEIGHT, SECONDS>(robot);
    }
    robots
}

fn safety_factor<const GRID_WIDTH: isize, const GRID_HEIGHT: isize>(robots: &[Robot]) -> i64 {
    let half = Ipos::new(GRID_WIDTH / 2, GRID_HEIGHT / 2);
    // The fifth quadrant is the one we ignore
    let mut quadrants = [0i64; 5];
    for robot in robots {
        let quadrant_id: usize = if (robot.position.x == half.x) || (robot.position.y == half.y) {
            // Ignore this robot
            4
        } else {
            let quadrant = Ipos::new(
                robot.position.x / (half.x + 1),
                robot.position.y / (half.y + 1),
            );
            (2 * quadrant.y + quadrant.x) as usize
        };
        quadrants[quadrant_id] += 1;
    }
    quadrants[..4].iter().product()
}

fn solve_case<const GRID_WIDTH: isize, const GRID_HEIGHT: isize, const SECONDS: isize>(
    robots: Vec<Robot>,
) -> i64 {
    safety_factor::<GRID_WIDTH, GRID_HEIGHT>(&move_all_robots::<GRID_WIDTH, GRID_HEIGHT, SECONDS>(
        robots,
    ))
}

fn main() {
    println!("Asserts");
    let robot = Robot {
        position: Ipos::new(2, 4),
        velocity: Ipos::new(2, -3),
    };
    assert_eq!(Ipos::new(4, 1), move_robot::<11, 7, 1>(&robot));
    assert_eq!(Ipos::new(6, 5), move_robot::<11, 7, 2>(&robot));

    println!("Part 1");
    let example = parse("day14.example");
    aoc::expect_result!(12, solve_case::<11, 7, 100>(example));
    let input = parse("day14.input");
    aoc::expect_result!(221655456, solve_case::<101, 103, 100>(input));

    println!("Part 2");
    aoc::return_incomplete();
}
