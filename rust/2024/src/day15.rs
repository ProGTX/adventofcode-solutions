use aoc::direction::Direction;
use aoc::grid::{Grid, Ipos};
use rustc_hash::FxHashSet;

const WALL: char = '#';
const EMPTY: char = '.';
const BOX: char = 'O';
const BOX_LEFT: char = '[';
const BOX_RIGHT: char = ']';
const ROBOT: char = '@';

const UP: char = '^';
const DOWN: char = 'v';
const LEFT: char = '<';
const RIGHT: char = '>';

struct Input {
    map: Grid<char>,
    robot_pos: Ipos,
    directions: Vec<Direction>,
}

fn to_facing(c: char) -> Direction {
    match c {
        '^' => Direction::North,
        'v' => Direction::South,
        '<' => Direction::West,
        '>' => Direction::East,
        _ => unreachable!("Error parsing directions"),
    }
}

fn direction_diff(direction: Direction) -> Ipos {
    let diff = direction.diff();
    Ipos::new(diff.x as isize, diff.y as isize)
}

fn parse_directions(directions: &str) -> Vec<Direction> {
    directions.chars().map(to_facing).collect()
}

fn parse(filename: &str) -> Input {
    let mut map_lines: Vec<String> = Vec::new();
    let mut robot_pos = Ipos::new(0, 0);
    let mut directions: Vec<Direction> = Vec::new();

    for line in aoc::file::read_lines(filename) {
        if line.is_empty() {
            continue;
        }
        if [UP, DOWN, LEFT, RIGHT].contains(&line.chars().next().unwrap()) {
            directions.extend(parse_directions(&line));
        } else {
            if let Some(robot_x) = line.find(ROBOT) {
                robot_pos = Ipos::new(robot_x as isize, map_lines.len() as isize);
            }
            map_lines.push(line);
        }
    }

    let num_columns = map_lines[0].len();
    let num_rows = map_lines.len();
    let data: Vec<char> = map_lines.iter().flat_map(|line| line.chars()).collect();
    let map = Grid::from_vec(data, num_rows, num_columns);

    Input {
        map,
        robot_pos,
        directions,
    }
}

fn check_unreachable_cell<const WIDE: bool>(c: char) {
    if !WIDE || (c != BOX_LEFT && c != BOX_RIGHT) {
        unreachable!("Invalid cell: {c:?}");
    }
}

/// Every box is identified by the position of its left ('[') cell,
/// even when we only have a reference to its right (']') half
/// That keeps `boxes` deduplicated below
/// (two adjacent boxes could otherwise be recorded once
/// via their left cell and once via their right cell)
fn box_left_of(map: &Grid<char>, pos: Ipos) -> Ipos {
    if *map.get(pos.y as usize, pos.x as usize) == BOX_LEFT {
        pos
    } else {
        pos - Ipos::new(1, 0)
    }
}

fn push_wide_boxes_vertical(map: &mut Grid<char>, new_pos: Ipos, diff: Ipos) -> bool {
    // A wide box's two cells can each be blocked by a different box above/below,
    // so the stack to push is a tree, not a single line.
    // First, breadth-first collect every box in that tree without mutating,
    // failing fast if any of them is blocked by a wall.

    // `new_pos` is whichever of the two cells the robot bumped into
    let root = box_left_of(map, new_pos);
    let mut boxes: FxHashSet<Ipos> = FxHashSet::default();
    boxes.insert(root);
    let mut frontier = vec![root];

    // BFS over layers: frontier holds the boxes discovered in the previous layer
    // whose neighbors (in the push direction) haven't been examined yet
    // `next_frontier` accumulates newly discovered boxes for the following layer
    // The loop ends once a layer discovers nothing new
    while !frontier.is_empty() {
        let mut next_frontier = Vec::new();
        for &box_pos in &frontier {
            // Check the cell directly ahead (in the push direction)
            // of each half of this box
            // The left half and the right half can each be blocked by a different box
            // so both must be inspected separately
            for cell in [box_pos, box_pos + Ipos::new(1, 0)] {
                let ahead = cell + diff;
                match *map.get(ahead.y as usize, ahead.x as usize) {
                    WALL => {
                        // Can't move stack of boxes into wall
                        return false;
                    }
                    BOX_LEFT | BOX_RIGHT => {
                        // Found another box in the way
                        let ahead_box_pos = box_left_of(map, ahead);
                        if boxes.insert(ahead_box_pos) {
                            // If box not already queued
                            // (it may have been reached from a sibling box's other half,
                            // or from a previous layer),
                            // add it so its own neighbors get checked next layer
                            next_frontier.push(ahead_box_pos);
                        }
                    }
                    EMPTY => {
                        // Nothing blocking this half of the box,
                        // no further boxes to add to the tree along this path
                    }
                    _ => unreachable!("Invalid cell"),
                }
            }
        }
        frontier = next_frontier;
    }

    // Every box in the tree has room to move by `diff`
    // A box's new position can coincide with another box's old position
    // so clear every old cell across the entire tree before writing any new cell
    // Otherwise a box moving into a cell could be immediately stepped on
    // by a not-yet-processed box vacating it
    for &box_pos in &boxes {
        *map.get_mut(box_pos.y as usize, box_pos.x as usize) = EMPTY;
        *map.get_mut(box_pos.y as usize, (box_pos.x + 1) as usize) = EMPTY;
    }
    for &box_pos in &boxes {
        let new_box_pos = box_pos + diff;
        *map.get_mut(new_box_pos.y as usize, new_box_pos.x as usize) = BOX_LEFT;
        *map.get_mut(new_box_pos.y as usize, (new_box_pos.x + 1) as usize) = BOX_RIGHT;
    }

    // Boxes can be moved and have been,
    // but the caller is still responsible for moving the robot itself
    return true;
}

fn push_box_stack_simple<const WIDE: bool>(map: &mut Grid<char>, robot_pos: &mut Ipos, diff: Ipos) {
    // Try to move stack of boxes by finding an empty space behind them
    let new_pos = *robot_pos + diff;
    let mut behind_boxes_pos = new_pos;
    loop {
        behind_boxes_pos += diff;
        let behind_boxes_value = *map.get(behind_boxes_pos.y as usize, behind_boxes_pos.x as usize);
        match behind_boxes_value {
            WALL => {
                // Can't move stack of boxes into wall
                break;
            }
            BOX => {
                // Continue searching stack of boxes
            }
            EMPTY => {
                if !WIDE {
                    // Move entire stack, but this can be achieved with just two steps
                    *map.get_mut(behind_boxes_pos.y as usize, behind_boxes_pos.x as usize) = BOX;
                    *map.get_mut(new_pos.y as usize, new_pos.x as usize) = ROBOT;
                    *map.get_mut(robot_pos.y as usize, robot_pos.x as usize) = EMPTY;
                } else {
                    // Wide boxes alternate '[' / ']', so we need to rotate
                    // the whole stack (plus the robot) by one step instead.
                    // This path is only reached via horizontal movement,
                    // so everything lies in one row.
                    assert_eq!(diff.y, 0, "Cannot use this function for vertical movement");
                    let robot_idx = map.linear_index(robot_pos.y as usize, robot_pos.x as usize);
                    let empty_idx =
                        map.linear_index(behind_boxes_pos.y as usize, behind_boxes_pos.x as usize);
                    if diff.x > 0 {
                        map.data[robot_idx..=empty_idx].rotate_right(1);
                    } else {
                        map.data[empty_idx..=robot_idx].rotate_left(1);
                    }
                }
                *robot_pos = new_pos;
                break;
            }
            c => {
                check_unreachable_cell::<WIDE>(c);
                // If cell is valid (part 2), continue search
            }
        }
    }
}

fn move_robot<const WIDE: bool>(
    mut map: Grid<char>,
    mut robot_pos: Ipos,
    directions: &[Direction],
) -> Grid<char> {
    assert_eq!(
        ROBOT,
        *map.get(robot_pos.y as usize, robot_pos.x as usize),
        "Robot not located where expected"
    );

    for &direction in directions {
        let diff = direction_diff(direction);
        let new_pos = robot_pos + diff;
        let new_pos_value = *map.get(new_pos.y as usize, new_pos.x as usize);

        match new_pos_value {
            WALL => {
                // Can't move into wall
            }
            EMPTY => {
                // Move into empty space
                *map.get_mut(new_pos.y as usize, new_pos.x as usize) = ROBOT;
                *map.get_mut(robot_pos.y as usize, robot_pos.x as usize) = EMPTY;
                robot_pos = new_pos;
            }
            BOX => {
                push_box_stack_simple::<WIDE>(&mut map, &mut robot_pos, diff);
            }
            c => {
                check_unreachable_cell::<WIDE>(c);
                if WIDE {
                    if diff.y == 0 {
                        push_box_stack_simple::<WIDE>(&mut map, &mut robot_pos, diff);
                    } else if push_wide_boxes_vertical(&mut map, new_pos, diff) {
                        *map.get_mut(new_pos.y as usize, new_pos.x as usize) = ROBOT;
                        *map.get_mut(robot_pos.y as usize, robot_pos.x as usize) = EMPTY;
                        robot_pos = new_pos;
                    }
                }
            }
        }
    }
    map
}

fn sum_coordinates<const WIDE: bool>(map: &Grid<char>) -> i64 {
    let target = if WIDE { BOX_LEFT } else { BOX };
    map.data
        .iter()
        .enumerate()
        .filter(|&(_, &c)| c == target)
        .map(|(i, _)| {
            let pos = map.position(i);
            100 * pos.y as i64 + pos.x as i64
        })
        .sum()
}

fn test_map() -> Grid<char> {
    Grid::from_vec("########...O..#......".chars().collect(), 3, 7)
}

fn widen_cell(c: char) -> [char; 2] {
    match c {
        WALL => [WALL, WALL],
        BOX => [BOX_LEFT, BOX_RIGHT],
        EMPTY => [EMPTY, EMPTY],
        ROBOT => [ROBOT, EMPTY],
        _ => unreachable!("Invalid cell"),
    }
}

fn widen_map(map: &Grid<char>) -> Grid<char> {
    let data: Vec<char> = map.data.iter().flat_map(|&c| widen_cell(c)).collect();
    Grid::from_vec(data, map.num_rows, map.num_columns * 2)
}

fn solve_case<const WIDE: bool>(input: &Input) -> i64 {
    let map = if WIDE {
        widen_map(&input.map)
    } else {
        input.map.clone()
    };
    let robot_pos = input.robot_pos * Ipos::new(1 + WIDE as isize, 1);
    sum_coordinates::<WIDE>(&move_robot::<WIDE>(map, robot_pos, &input.directions))
}

fn main() {
    println!("Asserts");
    assert_eq!(104, sum_coordinates::<false>(&test_map()));
    let widened = widen_map(&test_map());
    let expected_widened: String =
        "##############".to_string() + "##......[]...." + "##............";
    assert_eq!(expected_widened, widened.data.iter().collect::<String>());
    assert_eq!(108, sum_coordinates::<true>(&widened));

    println!("Part 1");
    let example = parse("day15.example");
    aoc::expect_result!(10092, solve_case::<false>(&example));
    let example2 = parse("day15.example2");
    aoc::expect_result!(2028, solve_case::<false>(&example2));
    let example3 = parse("day15.example3");
    aoc::expect_result!(908, solve_case::<false>(&example3));
    let input = parse("day15.input");
    aoc::expect_result!(1514353, solve_case::<false>(&input));

    println!("Part 2");
    aoc::expect_result!(9021, solve_case::<true>(&example));
    aoc::expect_result!(1751, solve_case::<true>(&example2));
    aoc::expect_result!(618, solve_case::<true>(&example3));
    aoc::expect_result!(1533076, solve_case::<true>(&input));
}
