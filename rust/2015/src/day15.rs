#[derive(Default, Debug)]
struct Cookie {
    capacity: i32,
    durability: i32,
    flavor: i32,
    texture: i32,
    calories: u32,
}

fn parse_property(property: &str) -> i32 {
    property.split_once(' ').unwrap().1.parse::<i32>().unwrap()
}

fn parse(filename: &str) -> Vec<Cookie> {
    std::fs::read_to_string(filename)
        .unwrap()
        .lines()
        .map(|line| {
            let properties = line.split(", ").collect::<Vec<&str>>();
            Cookie {
                capacity: parse_property(properties[0].split_once(": ").unwrap().1),
                durability: parse_property(properties[1]),
                flavor: parse_property(properties[2]),
                texture: parse_property(properties[3]),
                calories: parse_property(properties[4]) as u32,
            }
        })
        .collect()
}

fn score_cookie<const KCAL_500: bool>(cookies: &[Cookie], teaspoon_stack: &[u32]) -> u32 {
    let mut result = Cookie::default();
    for (index, teaspoon_u32) in teaspoon_stack.iter().enumerate() {
        let teaspoon = *teaspoon_u32 as i32;
        result.capacity += cookies[index].capacity * teaspoon;
        result.durability += cookies[index].durability * teaspoon;
        result.flavor += cookies[index].flavor * teaspoon;
        result.texture += cookies[index].texture * teaspoon;
        if (KCAL_500) {
            result.calories += cookies[index].calories * teaspoon_u32;
        }
    }
    if (KCAL_500) {
        if (result.calories != 500) {
            return 0;
        }
    }
    return [
        result.capacity.max(0) as u32,
        result.durability.max(0) as u32,
        result.flavor.max(0) as u32,
        result.texture.max(0) as u32,
    ]
    .iter()
    .product();
}

fn solve_case<const KCAL_500: bool>(filename: &str) -> u32 {
    const LIMIT: u32 = 100;
    let cookies = parse(filename);
    let mut max_score = 0;
    let mut teaspoon_stack = Vec::<u32>::new();
    teaspoon_stack.push(0);
    let mut teaspoons = 0;
    while (!teaspoon_stack.is_empty()) {
        if (teaspoons > LIMIT) {
            let last_teaspoon = teaspoon_stack.pop().unwrap();
            teaspoons -= last_teaspoon;
            match teaspoon_stack.last_mut() {
                Some(last_teaspoon) => {
                    *last_teaspoon += 1;
                    teaspoons += 1;
                    continue;
                }
                None => {
                    break;
                }
            }
        }
        if (teaspoon_stack.len() == (cookies.len() - 1)) {
            teaspoon_stack.push(LIMIT - teaspoons);
            let score = score_cookie::<KCAL_500>(&cookies, &teaspoon_stack);
            max_score = max_score.max(score);
            teaspoon_stack.pop();
            *teaspoon_stack.last_mut().unwrap() += 1;
            teaspoons += 1;
            continue;
        }
        teaspoon_stack.push(0);
    }
    return max_score;
}

fn main() {
    println!("Part 1");
    assert_eq!(62842880, solve_case::<false>("day15.example"));
    assert_eq!(21367368, solve_case::<false>("day15.input"));
    println!("Part 2");
    assert_eq!(57600000, solve_case::<true>("day15.example"));
    assert_eq!(1766400, solve_case::<true>("day15.input"));
}
