use std::collections::HashSet;

type EnergyLevels = aoc::grid::Grid<u32>;

fn parse(filename: &str) -> EnergyLevels {
    aoc::grid::from_file(filename)
}

fn solve_case<const NUM_STEPS: usize>(energy_levels: &EnergyLevels) -> usize {
    let mut current_levels = energy_levels.clone();
    let mut sum = 0;
    for step in (0..NUM_STEPS) {
        let mut unvisited = Vec::new();
        let mut visited = HashSet::new();

        // Increase level by 1
        for (index, energy) in current_levels.data.iter_mut().enumerate() {
            *energy += 1;
            if (*energy > 9) {
                unvisited.push(index);
            }
        }

        // Flash
        while (!unvisited.is_empty()) {
            let current_index = unvisited.pop().unwrap();
            if (!visited.insert(current_index)) {
                continue;
            }
            for neighbor_pos in
                current_levels.all_neighbor_positions(current_levels.position(current_index))
            {
                let index = current_levels.linear_from_pos(neighbor_pos);
                let value = current_levels.get_mut(neighbor_pos.y, neighbor_pos.x);
                *value += 1;
                if ((*value > 9) && !visited.contains(&index)) {
                    unvisited.push(index);
                }
            }
        }

        // Reset flashed
        for &index in &visited {
            current_levels.data[index] = 0;
        }

        if (NUM_STEPS < usize::MAX) {
            sum += visited.len();
        } else if (visited.len() == current_levels.data.len()) {
            return step + 1;
        }
    }
    return sum;
}

fn main() {
    println!("Part 1");
    let example = parse("day11.example");
    assert_eq!(9, solve_case::<2>(&example));
    let example2 = parse("day11.example2");
    assert_eq!(1656, solve_case::<100>(&example2));
    let input = parse("day11.input");
    assert_eq!(1694, solve_case::<100>(&input));

    println!("Part 2");
    assert_eq!(6, solve_case::<{ usize::MAX }>(&example));
    assert_eq!(195, solve_case::<{ usize::MAX }>(&example2));
    assert_eq!(346, solve_case::<{ usize::MAX }>(&input));
}
