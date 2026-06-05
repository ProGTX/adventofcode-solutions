use aoc::direction::*;
use aoc::grid::Grid;
use aoc::nd_point::NDPoint;
use aoc::point::Point;

type Pos = Point<i32>;
type Vec3 = NDPoint<i32, 3>;

const EMPTY: char = ' ';
const TILE: char = '.';
const WALL: char = '#';

const TURN_CLOCKWISE: i32 = -1;
const TURN_COUNTERCLOCKWISE: i32 = -2;

struct Input {
    map: Grid<char>,
    steps: Vec<i32>,
}

fn parse(filename: &str) -> Input {
    let mut steps = Vec::new();
    let mut raw_map: Vec<String> = Vec::new();
    let mut parsing_steps = false;

    for line in std::fs::read_to_string(filename).unwrap().lines() {
        if line.is_empty() {
            parsing_steps = true;
            continue;
        }
        if !parsing_steps {
            raw_map.push(line.to_string());
        } else {
            let mut start = 0;
            let mut size = 0;
            for (i, c) in line.char_indices() {
                match c {
                    'R' | 'L' => {
                        if size > 0 {
                            steps.push(line[start..start + size].parse().unwrap());
                        }
                        steps.push(if c == 'R' {
                            TURN_CLOCKWISE
                        } else {
                            TURN_COUNTERCLOCKWISE
                        });
                        start = i + 1;
                        size = 0;
                    }
                    _ => size += 1,
                }
            }
            if size > 0 {
                steps.push(line[start..start + size].parse().unwrap());
            }
        }
    }

    let max_width = raw_map.iter().map(|l| l.len()).max().unwrap_or(0);
    let map_width = max_width + 2;
    let map_height = raw_map.len() + 2;

    let mut map_data = Vec::with_capacity(map_width * map_height);
    map_data.extend(std::iter::repeat(EMPTY).take(map_width));
    for line in &raw_map {
        map_data.push(EMPTY);
        map_data.extend(line.chars());
        map_data.extend(std::iter::repeat(EMPTY).take(map_width - 1 - line.len()));
    }
    map_data.extend(std::iter::repeat(EMPTY).take(map_width));

    Input {
        map: Grid::from_vec(map_data, map_height, map_width),
        steps,
    }
}

fn find_start(map: &Grid<char>) -> Arrow {
    for x in 1..map.num_columns {
        if *map.get(1, x) == TILE {
            return Arrow {
                pos: Pos::new(x as i32, 1),
                dir: Direction::East,
            };
        }
    }
    panic!("no starting tile found");
}

fn execute_steps(
    steps: &[i32],
    start: Arrow,
    mut step_fn: impl FnMut(&mut Arrow) -> bool,
) -> Arrow {
    let mut current = start;
    for &step in steps {
        if step == TURN_CLOCKWISE {
            current.dir = current.dir.clockwise();
        } else if step == TURN_COUNTERCLOCKWISE {
            current.dir = current.dir.counterclockwise();
        } else {
            for _ in 0..step {
                if !step_fn(&mut current) {
                    break;
                }
            }
        }
    }
    current
}

fn password(arrow: Arrow) -> i32 {
    1000 * arrow.pos.y + 4 * arrow.pos.x + arrow.dir as i32
}

fn solve_case1(input: &Input) -> i32 {
    let Input { map, steps } = input;
    let start = find_start(map);

    let final_arrow = execute_steps(steps, start, |current| {
        let diff = current.dir.diff();
        let mut next_pos = current.pos + diff;
        if *map.get(next_pos.y as usize, next_pos.x as usize) == EMPTY {
            // Flat wrap: step back from current position to find the far edge
            let mut wrapped = current.pos;
            while *map.get((wrapped.y - diff.y) as usize, (wrapped.x - diff.x) as usize) != EMPTY {
                wrapped = wrapped - diff;
            }
            next_pos = wrapped;
        }
        if *map.get(next_pos.y as usize, next_pos.x as usize) == WALL {
            return false;
        }
        current.pos = next_pos;
        return true;
    });

    return password(final_arrow);
}

#[derive(Clone, Copy, PartialEq, Eq, Hash, PartialOrd, Ord)]
struct FaceInfo {
    pos: Pos,
    normal: Vec3,
    right: Vec3,
    down: Vec3,
}

fn new_vec3(x: i32, y: i32, z: i32) -> Vec3 {
    Vec3 { data: [x, y, z] }
}

fn solve_case2(input: &Input, cube_side: i32) -> i32 {
    let Input { map, steps } = input;
    let num_rows = map.num_rows as i32;
    let num_columns = map.num_columns as i32;

    // BFS from the first face, assigning 3D orientations to all 6 cube faces
    let start_face = {
        let mut found = None;
        'outer: for fy in (1..num_rows - 1).step_by(cube_side as usize) {
            for fx in (1..num_columns - 1).step_by(cube_side as usize) {
                if *map.get(fy as usize, fx as usize) != EMPTY {
                    found = Some(Pos::new(fx, fy));
                    break 'outer;
                }
            }
        }
        found.unwrap()
    };

    // Crossing a 2D edge transforms the orientation:
    //   east:  { face.right,  -face.normal,  face.down   }
    //   west:  { -face.right,  face.normal,  face.down   }
    //   south: { face.down,    face.right,  -face.normal }
    //   north: { -face.down,   face.right,   face.normal }
    let faces_map = aoc::dijkstra::shortest_distances(
        &FaceInfo {
            pos: start_face,
            normal: new_vec3(0, 0, 1),
            right: new_vec3(1, 0, 0),
            down: new_vec3(0, 1, 0),
        },
        |_| false,
        |face| {
            let mut neighbors = Vec::new();
            for dir in BASIC_DIRECTIONS {
                let diff = dir.diff();
                let new_pos = Pos::new(
                    (diff.x * cube_side) + face.pos.x,
                    (diff.y * cube_side) + face.pos.y,
                );
                if (false
                    || new_pos.x < 1
                    || new_pos.y < 1
                    || new_pos.x >= num_columns - 1
                    || new_pos.y >= num_rows - 1)
                {
                    continue;
                }
                if *map.get(new_pos.y as usize, new_pos.x as usize) == EMPTY {
                    continue;
                }
                let neighbor = match dir {
                    Direction::East => FaceInfo {
                        pos: new_pos,
                        normal: face.right,
                        right: -face.normal,
                        down: face.down,
                    },
                    Direction::West => FaceInfo {
                        pos: new_pos,
                        normal: -face.right,
                        right: face.normal,
                        down: face.down,
                    },
                    Direction::South => FaceInfo {
                        pos: new_pos,
                        normal: face.down,
                        right: face.right,
                        down: -face.normal,
                    },
                    Direction::North => FaceInfo {
                        pos: new_pos,
                        normal: -face.down,
                        right: face.right,
                        down: face.normal,
                    },
                };
                neighbors.push(aoc::dijkstra::DijkstraState {
                    data: neighbor,
                    distance: 1,
                });
            }
            neighbors
        },
    );

    let faces = faces_map.keys().copied().collect::<Vec<_>>();

    // Given an arrow stepping off a face edge into empty space,
    // compute the cube-wrapped destination arrow.
    // Works purely from the 3D orientations (normal/right/down)
    // assigned to each face during the BFS above.
    let cube_jump = |arrow: Arrow| -> Arrow {
        let src = *faces
            .iter()
            .find(|f| {
                return true
                    && arrow.pos.x >= f.pos.x
                    && arrow.pos.x < f.pos.x + cube_side
                    && arrow.pos.y >= f.pos.y
                    && arrow.pos.y < f.pos.y + cube_side;
            })
            .unwrap();

        // Moving east/west/south/north on the 2D map corresponds to moving in the
        // src.right / -src.right / src.down / -src.down directions in 3D.
        // The destination face is the one whose normal points that way.
        let dest_normal = match arrow.dir {
            Direction::East => src.right,
            Direction::West => -src.right,
            Direction::South => src.down,
            Direction::North => -src.down,
        };
        let dest = *faces.iter().find(|f| f.normal == dest_normal).unwrap();

        // Folding across the shared edge, the src face's outward normal becomes
        // the in-plane arrival direction on the destination face (reversed)
        let arrival_dir = -src.normal;
        let (
            new_facing, //
            entry_coord,
            y_is_fixed,
            dest_along,
        ) = if (arrival_dir == dest.right) {
            (Direction::East, 0_i32, false, dest.down)
        } else if arrival_dir == -dest.right {
            (Direction::West, cube_side - 1, false, dest.down)
        } else if arrival_dir == dest.down {
            (Direction::South, 0_i32, true, dest.right)
        } else {
            (Direction::North, cube_side - 1, true, dest.right)
        };

        // Measure how far along the src entry edge the arrow sits,
        // then map it onto the destination edge.
        // Reverse when the two edge axes are antiparallel.
        let is_ew = matches!(arrow.dir, Direction::East | Direction::West);
        let src_along = if is_ew { src.down } else { src.right };
        let src_offset = if is_ew {
            arrow.pos.y - src.pos.y
        } else {
            arrow.pos.x - src.pos.x
        };
        let dest_offset = if src_along == -dest_along {
            cube_side - 1 - src_offset
        } else {
            src_offset
        };

        let x_in_dst = if y_is_fixed { dest_offset } else { entry_coord };
        let y_in_dst = if y_is_fixed { entry_coord } else { dest_offset };
        Arrow {
            pos: dest.pos + Pos::new(x_in_dst, y_in_dst),
            dir: new_facing,
        }
    };

    let start = find_start(map);

    let final_arrow = execute_steps(steps, start, |current| {
        let next_pos = current.pos + current.dir.diff();
        let (
            final_pos, //
            final_dir,
        ) = if *map.get(next_pos.y as usize, next_pos.x as usize) == EMPTY {
            let jumped = cube_jump(*current);
            (jumped.pos, jumped.dir)
        } else {
            (next_pos, current.dir)
        };
        if *map.get(final_pos.y as usize, final_pos.x as usize) == WALL {
            return false;
        }
        current.pos = final_pos;
        current.dir = final_dir;
        return true;
    });

    return password(final_arrow);
}

fn main() {
    println!("Part 1");
    let example = parse("day22.example");
    aoc::expect_result!(6032, solve_case1(&example));
    let input = parse("day22.input");
    aoc::expect_result!(97356, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(5031, solve_case2(&example, 4));
    aoc::expect_result!(120175, solve_case2(&input, 50));
}
