use std::collections::HashMap;

fn solve_case(filename: &str) -> u32 {
    let mut generous_sue = HashMap::new();
    generous_sue.insert("children", 3);
    generous_sue.insert("cats", 7);
    generous_sue.insert("samoyeds", 2);
    generous_sue.insert("pomeranians", 3);
    generous_sue.insert("akitas", 0);
    generous_sue.insert("vizslas", 0);
    generous_sue.insert("goldfish", 5);
    generous_sue.insert("trees", 3);
    generous_sue.insert("cars", 2);
    generous_sue.insert("perfumes", 1);
    let generous_sue = generous_sue;

    for (index, line) in std::fs::read_to_string(filename)
        .unwrap()
        .lines()
        .enumerate()
    {
        let (_, info) = line.split_once(": ").unwrap();
        let split_info = info.split(", ");
        let matches = split_info.into_iter().all(|info| {
            let (key, value) = info.split_once(": ").unwrap();
            return *generous_sue.get(key).unwrap() == value.parse::<u32>().unwrap();
        });
        if (matches) {
            return (index + 1) as u32;
        }
    }
    unreachable!("Aunt not found!");
}

fn main() {
    println!("Part 1");
    assert_eq!(373, solve_case("day16.input"));
    // println!("Part 2");
    // assert_eq!(1766400, solve_case::<true>("day16.input"));
}
