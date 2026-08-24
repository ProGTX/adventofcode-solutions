use aoc::dijkstra::{DijkstraNeighborView, DijkstraState};
use aoc::grid::{Grid, Upos};
use aoc::iter::CollectArrayVec;
use aoc::md5::md5;
use arrayvec::ArrayVec;
use std::hash::{Hash, Hasher};

/// The passcode for the vault
type Input = String;

const START: Upos = Upos::new(0, 0);
const VAULT: Upos = Upos::new(3, 3);

/// A room, plus the passcode followed by the moves taken to reach it
///
/// The path alone already determines the room,
/// so it is all that needs to be hashed
#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
struct State {
    pos: Upos,
    path: String,
}
impl Hash for State {
    fn hash<H: Hasher>(&self, hasher: &mut H) {
        self.path.hash(hasher);
    }
}

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename)[0].clone()
}

fn get_neighbors(state: &State, rooms: &Grid<u8>) -> ArrayVec<DijkstraState<State>, 4> {
    // The same hash decides all four doors of this room
    let hash = md5(state.path.as_bytes());
    rooms
        .basic_neighbor_positions(state.pos)
        .into_iter()
        .filter_map(|neighbor| {
            // The first four hex digits of the hash are the doors
            // up, down, left and right, in that order
            let (door, direction) = if (neighbor.y < state.pos.y) {
                (0, 'U')
            } else if (neighbor.y > state.pos.y) {
                (1, 'D')
            } else if (neighbor.x < state.pos.x) {
                (2, 'L')
            } else {
                (3, 'R')
            };
            // Every hash byte holds two hex digits
            let digit = (hash[door / 2] >> (4 * (1 - (door % 2)))) & 0xf;
            // Only b through f, the digits above a, open a door
            (digit > 0xa).then(|| {
                let mut path = String::with_capacity(state.path.len() + 1);
                path.push_str(&state.path);
                path.push(direction);
                State {
                    pos: neighbor,
                    path,
                }
            })
        })
        .dijkstra_uniform_neighbors()
        .collect_array_vec()
}

/// The shortest path of moves from the top-left room to the vault
fn solve_case1(passcode: &Input) -> String {
    let rooms = Grid::new(0_u8, 4, 4);
    let start = State {
        pos: START,
        path: passcode.clone(),
    };
    let distances = aoc::dijkstra::shortest_distances(
        &start,
        |state| state.pos == VAULT,
        |state| get_neighbors(state, &rooms),
    );
    // Only the moves are the answer, not the passcode they are appended to
    return distances
        .into_iter()
        .filter(|(state, _)| state.pos == VAULT)
        .min_by_key(|(_, distance)| *distance)
        .map(|(state, _)| state.path[passcode.len()..].to_string())
        .unwrap();
}

fn main() {
    println!("Part 1");
    let example = parse("day17.example");
    aoc::expect_result!("DRURDRUDDLLDLUURRDULRLDUUDDDRR", solve_case1(&example));
    let input = parse("day17.input");
    aoc::expect_result!("RLDUDRDDRR", solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
