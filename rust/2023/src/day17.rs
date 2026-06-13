use aoc::dijkstra::{DijkstraState, shortest_distances};
use aoc::direction::Direction;
use aoc::grid::Grid;
use aoc::point::Point;
use arrayvec::ArrayVec;

type Pos = Point<i32>;
type Input = Grid<u32>;

fn parse(filename: &str) -> Input {
    aoc::grid::from_file(filename)
}

#[derive(Clone, Copy, Debug, Hash, PartialEq, Eq, PartialOrd, Ord)]
struct Node {
    pos: Pos,
    direction: Pos,
    consecutive: u8,
}

fn least_heat_loss<const MIN: u8, const MAX: u8>(city_block: &Input) -> u32 {
    let end_pos = Pos::new(
        city_block.num_columns as i32 - 1,
        city_block.num_rows as i32 - 1,
    );

    let get_neighbors = |current: &Node| {
        let mut neighbors = ArrayVec::<DijkstraState<Node>, 3>::new();
        let mut add_neighbor = |direction: Pos, consecutive: u8| {
            let pos = current.pos + direction;
            if !city_block.in_bounds_signed(pos.y as isize, pos.x as isize) {
                return;
            }
            neighbors.push(DijkstraState {
                data: Node {
                    pos,
                    direction,
                    consecutive,
                },
                distance: *city_block.get(pos.y as usize, pos.x as usize),
            });
        };

        if current.consecutive < MAX {
            // Continue straight
            add_neighbor(current.direction, current.consecutive + 1);
        }
        if current.consecutive >= MIN {
            // Turn left or right
            let direction = Pos::new(current.direction.y, current.direction.x);
            add_neighbor(direction, 1);
            add_neighbor(Pos::new(-direction.x, -direction.y), 1);
        }
        neighbors
    };

    let end_reached = |node: &Node| node.pos == end_pos && node.consecutive >= MIN;

    [Direction::East, Direction::South]
        .into_iter()
        .flat_map(|direction| {
            let start = Node {
                pos: Pos::default(),
                direction: direction.diff(),
                consecutive: 0,
            };
            let distances = shortest_distances(&start, end_reached, get_neighbors);
            distances
                .into_iter()
                .filter(|(node, _)| end_reached(node))
                .map(|(_, distance)| distance)
                .min()
        })
        .min()
        .unwrap()
}

fn main() {
    println!("Part 1");
    let example = parse("day17.example");
    aoc::expect_result!(102, least_heat_loss::<1, 3>(&example));
    let input = parse("day17.input");
    aoc::expect_result!(967, least_heat_loss::<1, 3>(&input));

    println!("Part 2");
    aoc::expect_result!(94, least_heat_loss::<4, 10>(&example));
    let example2 = parse("day17.example2");
    aoc::expect_result!(71, least_heat_loss::<4, 10>(&example2));
    aoc::expect_result!(1101, least_heat_loss::<4, 10>(&input));
}
