use aoc::{
    dijkstra::DijkstraNeighborView,
    grid::{ConfigInput, Grid},
};
use arrayvec::ArrayVec;
use rustc_hash::FxHashMap;

type Input = [String; 5];

fn parse(filename: &str) -> Input {
    aoc::file::read_lines(filename).try_into().unwrap()
}

const NUMERIC_KEYPAD: &str = "\
789
456
123
X0A";
const DIRECTIONAL_KEYPAD: &str = "\
X^A
<v>";
const KEYPAD_GAP: char = 'X';
const ACTION: char = 'A';

const NUM_DIRECTIONAL_ROBOTS: usize = 2;

#[derive(Clone, Copy, Debug, Hash, PartialEq, Eq, PartialOrd, Ord)]
struct SearchState {
    // Two robots on directional keypads, third on numeric one
    robot_pos: [usize; NUM_DIRECTIONAL_ROBOTS + 1],
    human_key: char,
}

fn solve_case1(input: &Input) -> u64 {
    let keypad_grid = |keypad: &str| {
        let lines: Vec<_> = keypad.lines().map(str::to_string).collect();
        Grid::from_lines_config(&lines, ConfigInput::default()).0
    };
    let numeric_keypad = keypad_grid(NUMERIC_KEYPAD);
    let directional_keypad = keypad_grid(DIRECTIONAL_KEYPAD);

    // Moves an arm one step, or `None` if that leaves the keypad or hits the gap
    let move_arm = |keypad: &Grid<char>, index: usize, direction: char| {
        let pos = keypad.position(index);
        let (row_diff, col_diff): (isize, isize) = match direction {
            '^' => (-1, 0),
            'v' => (1, 0),
            '<' => (0, -1),
            '>' => (0, 1),
            _ => unreachable!("Not a movement key: {direction}"),
        };
        let (row, col) = (pos.y as isize + row_diff, pos.x as isize + col_diff);
        if !keypad.in_bounds_signed(row, col) {
            return None;
        }
        let new_index = keypad.linear_index(row as usize, col as usize);
        (keypad.data[new_index] != KEYPAD_GAP).then_some(new_index)
    };

    let human_press = |state: &SearchState, key: char| -> Option<SearchState> {
        let mut new_state = *state;
        new_state.human_key = key;
        if key != ACTION {
            // Anything but ACTION moves the first robot's arm directly
            new_state.robot_pos[0] = move_arm(&directional_keypad, state.robot_pos[0], key)?;
            return Some(new_state);
        }
        // ACTION activates the key the first robot points at,
        // cascading inwards past every robot that is itself resting on ACTION
        let depth = state.robot_pos[..NUM_DIRECTIONAL_ROBOTS]
            .iter()
            .take_while(|&&index| directional_keypad.data[index] == ACTION)
            .count();
        if depth == NUM_DIRECTIONAL_ROBOTS {
            // Every directional robot forwarded the press,
            // so the last robot types the numeric key it points at
            return Some(new_state);
        }
        // The first robot not resting on ACTION holds a direction,
        // which moves the arm of the robot one level further in
        let direction = directional_keypad.data[state.robot_pos[depth]];
        let inner_keypad = if depth + 1 < NUM_DIRECTIONAL_ROBOTS {
            &directional_keypad
        } else {
            &numeric_keypad
        };
        new_state.robot_pos[depth + 1] =
            move_arm(inner_keypad, state.robot_pos[depth + 1], direction)?;
        Some(new_state)
    };

    // This assert allows us to just skip(1) in search
    debug_assert_eq!(KEYPAD_GAP, directional_keypad.data[0]);

    let neighbors = |state: &SearchState| {
        directional_keypad
            .data
            .iter()
            .copied()
            .skip(1)
            .filter_map(|human_key| human_press(state, human_key))
            .collect::<ArrayVec<SearchState, 5>>()
            .into_iter()
            .dijkstra_uniform_neighbors()
    };

    let indef_of_key =
        |keypad: &Grid<char>, key| keypad.data.iter().position(|v| *v == key).unwrap();
    let dir_action_index = indef_of_key(&directional_keypad, ACTION);

    // Typing a code is one search per key,
    // each one starting where the previous one left the arms
    let shortest_sequence = |code: &str| {
        let mut state = SearchState {
            robot_pos: [
                dir_action_index,
                dir_action_index,
                indef_of_key(&numeric_keypad, ACTION),
            ],
            human_key: KEYPAD_GAP,
        };
        let mut human_keys = String::new();
        for key in code.chars() {
            let end = SearchState {
                robot_pos: [
                    dir_action_index,
                    dir_action_index,
                    indef_of_key(&numeric_keypad, key),
                ],
                human_key: ACTION,
            };
            let mut predecessors = FxHashMap::default();
            aoc::dijkstra::shortest_distances_with_predecessors(
                &state,
                |state| *state == end,
                &neighbors,
                &mut predecessors,
            );
            // `get_path` is in end-to-start order, and leaves out the start state
            let path = aoc::dijkstra::get_path(&predecessors, &end);
            human_keys.extend(path.iter().rev().map(|state| state.human_key));
            // The start's `human_key` only marks "nothing pressed yet",
            // keeping it distinct from `end` when a key gets typed twice in a row
            state = SearchState {
                human_key: KEYPAD_GAP,
                ..end
            };
        }
        human_keys
    };

    // The code without its trailing ACTION, so `029A` counts as 29
    let numeric = |code: &str| {
        code.trim_end_matches(|key: char| !key.is_ascii_digit())
            .parse::<usize>()
            .unwrap()
    };

    return input
        .iter()
        .map(|code| (shortest_sequence(code).len() * numeric(code)) as u64)
        .sum();
}

fn main() {
    println!("Part 1");
    let example = parse("day21.example");
    aoc::expect_result!(126384, solve_case1(&example));
    let input = parse("day21.input");
    aoc::expect_result!(231564, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
