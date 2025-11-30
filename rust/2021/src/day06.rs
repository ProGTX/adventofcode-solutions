type School = [u64; 7];

fn parse(filename: &str) -> School {
    let mut fish = School::default();
    for fish_timer in aoc::file::read_string(filename)
        .trim()
        .split(',')
        .map(|num| num.parse::<u64>().unwrap())
    {
        fish[fish_timer as usize] += 1
    }
    fish
}

fn solve_case<const NUM_DAYS: u32>(school: &School) -> u64 {
    let mut brood = [0, 0];
    let mut fish = school.clone();

    for _ in 0..NUM_DAYS {
        let num_new_brood = fish.first().unwrap().clone();
        fish.rotate_left(1);

        let num_new_matures = brood.first().unwrap().clone();
        brood = [brood[1], num_new_brood];

        *fish.last_mut().unwrap() += num_new_matures;
    }

    return fish.iter().sum::<u64>() + brood.iter().sum::<u64>();
}

fn main() {
    println!("Part 1");
    let example = parse("day06.example");
    assert_eq!(26, solve_case::<18>(&example));
    assert_eq!(5934, solve_case::<80>(&example));
    let input = parse("day06.input");
    assert_eq!(371379, solve_case::<80>(&input));

    println!("Part 2");
    assert_eq!(26984457539, solve_case::<256>(&example));
    assert_eq!(1674303997472, solve_case::<256>(&input));
}
