fn to_numbers(s: &str) -> Vec<u32> {
    s.bytes().map(|c| (c - b'a') as u32).collect::<Vec<u32>>()
}

fn to_string(numbers: &Vec<u32>) -> String {
    numbers
        .iter()
        .map(|&n| char::from_u32(n + (b'a' as u32)).unwrap())
        .collect()
}

fn solve_case(filename: &str) -> String {
    let numbers = to_numbers(std::fs::read_to_string(filename).unwrap().trim());
    to_string(&numbers)
}

fn main() {
    println!("Part 1");
    assert_eq!("ghjaabcc", solve_case("day11.example"));
    assert_eq!("!hxbxwxba", solve_case("day11.input"));
    // println!("Part 2");
    // assert_eq!(3369156, solve_case::<50>("day11.example"));
    // assert_eq!(5103798, solve_case::<50>("day11.input"));
}
