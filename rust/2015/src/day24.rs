use itertools::Itertools;

fn parse(filename: &str) -> Vec<u32> {
    let mut packages = aoc::file::read_numbers::<u32>(filename);
    packages.reverse();
    packages
}

type Group = Vec<u32>;

fn qe(group: &Group) -> u64 {
    group.iter().map(|w| *w as u64).product::<u64>()
}

fn solve_case<const NUM_GROUPS: u32>(packages: &[u32]) -> u64 {
    // Assume all packages have unique weights and are sorted in decreasing order
    let total_weight = packages.iter().sum::<u32>();
    debug_assert!(
        (total_weight % NUM_GROUPS) == 0,
        "Invalid package configuration"
    );
    let target_group_weight = total_weight / NUM_GROUPS;
    let mut num_packages = 0;
    let mut weight = 0;
    let size = packages.len();
    while (num_packages < size) {
        weight += packages[num_packages];
        if (weight > target_group_weight) {
            break;
        }
        num_packages += 1;
    }
    let mut possible = Vec::<Group>::new();
    for index in num_packages - 1..size {
        let neighbors = packages
            .iter()
            .combinations(index + 1)
            .filter(|combo| combo.iter().map(|w| **w).sum::<u32>() == target_group_weight)
            .map(|combo| {
                let mut combo = combo.iter().map(|w| **w).collect::<Vec<_>>();
                combo.sort();
                combo
            })
            .collect::<Vec<_>>();
        if (!neighbors.is_empty()) {
            possible.extend(neighbors);
            break;
        }
    }
    possible.sort_by(|lhs, rhs| lhs.len().cmp(&rhs.len()).then(qe(lhs).cmp(&qe(rhs))));
    return qe(&possible[0]);
}

fn main() {
    println!("Part 1");
    let example = parse("day24.example");
    assert_eq!(99, solve_case::<3>(&example));
    let input = parse("day24.input");
    assert_eq!(10723906903, solve_case::<3>(&input));

    println!("Part 2");
    assert_eq!(44, solve_case::<4>(&example));
    assert_eq!(74850409, solve_case::<4>(&input));
}
