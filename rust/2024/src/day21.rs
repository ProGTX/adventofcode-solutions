use aoc::{
    dijkstra::DijkstraNeighborView,
    grid::{ConfigInput, Grid},
};
use arrayvec::ArrayVec;

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

#[derive(Clone, Copy, Debug, Hash, PartialEq, Eq, PartialOrd, Ord)]
struct SearchState<const NUM_ROBOTS: usize> {
    // All robots but the last one are on directional keypads,
    // the last one is on the numeric keypad
    robot_pos: [usize; NUM_ROBOTS],
    human_key: char,
}

fn solve_case<const NUM_ROBOTS: usize>(input: &Input) -> u64 {
    let keypad_grid = |keypad: &str| {
        let lines: Vec<_> = keypad.lines().map(str::to_string).collect();
        Grid::from_lines_config(&lines, ConfigInput::default()).0
    };
    let numeric_keypad = keypad_grid(NUMERIC_KEYPAD);
    let directional_keypad = keypad_grid(DIRECTIONAL_KEYPAD);

    let num_directional_robots = NUM_ROBOTS - 1;

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

    let human_press =
        |state: &SearchState<NUM_ROBOTS>, key: char| -> Option<SearchState<NUM_ROBOTS>> {
            let mut new_state = *state;
            new_state.human_key = key;
            if key != ACTION {
                // Anything but ACTION moves the first robot's arm directly
                new_state.robot_pos[0] = move_arm(&directional_keypad, state.robot_pos[0], key)?;
                return Some(new_state);
            }
            // ACTION activates the key the first robot points at,
            // cascading inwards past every robot that is itself resting on ACTION
            let depth = state.robot_pos[..num_directional_robots]
                .iter()
                .take_while(|&&index| directional_keypad.data[index] == ACTION)
                .count();
            if depth == num_directional_robots {
                // Every directional robot forwarded the press,
                // so the last robot types the numeric key it points at
                return Some(new_state);
            }
            // The first robot not resting on ACTION holds a direction,
            // which moves the arm of the robot one level further in
            let direction = directional_keypad.data[state.robot_pos[depth]];
            let inner_keypad = if depth + 1 < num_directional_robots {
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

    let neighbors = |state: &SearchState<NUM_ROBOTS>| {
        directional_keypad
            .data
            .iter()
            .copied()
            .skip(1)
            .filter_map(|human_key| human_press(state, human_key))
            .collect::<ArrayVec<SearchState<NUM_ROBOTS>, 5>>()
            .into_iter()
            .dijkstra_uniform_neighbors()
    };

    let indef_of_key =
        |keypad: &Grid<char>, key| keypad.data.iter().position(|v| *v == key).unwrap();
    let dir_action_index = indef_of_key(&directional_keypad, ACTION);

    // Typing a code is one search per key,
    // each one starting where the previous one left the arms
    // Every directional robot rests on ACTION, the last one is on the given numeric key
    let arms_on = |numeric_key| {
        let mut robot_pos = [dir_action_index; NUM_ROBOTS];
        robot_pos[num_directional_robots] = indef_of_key(&numeric_keypad, numeric_key);
        robot_pos
    };

    let sequence_length = |code: &str| {
        let mut state = SearchState {
            robot_pos: arms_on(ACTION),
            human_key: KEYPAD_GAP,
        };
        let mut length = 0_u64;
        for key in code.chars() {
            let end = SearchState {
                robot_pos: arms_on(key),
                human_key: ACTION,
            };
            let distances =
                aoc::dijkstra::shortest_distances(&state, |state| *state == end, &neighbors);
            length += distances[&end] as u64;
            // The start's `human_key` only marks "nothing pressed yet",
            // keeping it distinct from `end` when a key gets typed twice in a row
            state = SearchState {
                human_key: KEYPAD_GAP,
                ..end
            };
        }
        length
    };

    // The code without its trailing ACTION, so `029A` counts as 29
    let numeric = |code: &str| {
        code.trim_end_matches(|key: char| !key.is_ascii_digit())
            .parse::<usize>()
            .unwrap()
    };

    return input
        .iter()
        .map(|code| sequence_length(code) * numeric(code) as u64)
        .sum();
}

fn main() {
    println!("Part 1");
    let example = parse("day21.example");
    aoc::expect_result!(126384, solve_case::<{ 2 + 1 }>(&example));
    let input = parse("day21.input");
    aoc::expect_result!(231564, solve_case::<{ 2 + 1 }>(&input));

    println!("Part 2");
    // This is correct, but way too slow
    aoc::return_incomplete();
    aoc::expect_result!(1337, solve_case::<{ 25 + 1 }>(&example));
    aoc::expect_result!(1337, solve_case::<{ 25 + 1 }>(&input));
}
