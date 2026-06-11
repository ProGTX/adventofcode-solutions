type Steps = Vec<String>;

fn parse(filename: &str) -> Steps {
    aoc::file::read_lines(filename)
        .last()
        .unwrap()
        .split(',')
        .map(|s| s.to_string())
        .collect()
}

// Determine the ASCII code for the current character of the string.
// Increase the current value by the ASCII code you just determined.
// Set the current value to itself multiplied by 17.
// Set the current value to the remainder of dividing itself by 256.
fn hash_alg(s: &str) -> i32 {
    s.bytes().fold(0i32, |current_value, ascii| {
        ((current_value + ascii as i32) * 17) % 256
    })
}

fn sum_steps(steps: &Steps) -> i32 {
    steps.iter().map(|s| hash_alg(s)).sum()
}

fn solve_case1(steps: &Steps) -> i32 {
    sum_steps(steps)
}

type Lens = (String, i32);
type LensBox = Vec<Lens>;
type Boxes = Vec<LensBox>;

fn split_label(s: &str) -> Lens {
    if let Some(name) = s.strip_suffix('-') {
        return (name.to_string(), -1);
    }
    let (name, number) = s.split_once('=').unwrap();
    (name.to_string(), number.parse().unwrap())
}

fn sort_boxes(instructions: &[Lens]) -> Boxes {
    let mut boxes: Boxes = vec![LensBox::new(); 256];

    for (name, number) in instructions {
        let current_box = &mut boxes[hash_alg(name) as usize];
        match current_box.iter().position(|(n, _)| n == name) {
            Some(pos) => {
                if *number < 0 {
                    current_box.remove(pos);
                } else {
                    current_box[pos] = (name.clone(), *number);
                }
            }
            None if *number >= 0 => current_box.push((name.clone(), *number)),
            None => {
                // Trying to remove from an empty box, do nothing
            }
        }
    }

    boxes
}

fn focusing_power(b: &LensBox) -> i32 {
    b.iter()
        .enumerate()
        .map(|(i, lens)| (i as i32 + 1) * lens.1)
        .sum()
}

fn sum_boxes(boxes: &Boxes) -> i32 {
    boxes
        .iter()
        .enumerate()
        .map(|(i, b)| (i as i32 + 1) * focusing_power(b))
        .sum()
}

fn solve_case2(steps: &Steps) -> i32 {
    let instructions: Vec<Lens> = steps.iter().map(|s| split_label(s)).collect();
    sum_boxes(&sort_boxes(&instructions))
}

fn main() {
    // Asserts
    assert_eq!(52, hash_alg("HASH"));
    let test_case: Steps = [
        "rn=1", "cm-", "qp=3", "cm=2", "qp-", "pc=4", "ot=9", "ab=5", "pc-", "pc=6", "ot=7",
    ]
    .iter()
    .map(|s| s.to_string())
    .collect();
    assert_eq!(1320, sum_steps(&test_case));

    println!("Part 1");
    let example = parse("day15.example");
    aoc::expect_result!(1320, solve_case1(&example));
    let input = parse("day15.input");
    aoc::expect_result!(510273, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(145, solve_case2(&example));
    aoc::expect_result!(212449, solve_case2(&input));
}
