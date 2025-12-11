use aoc::string::NameToId;
use arrayvec::ArrayVec;
use std::collections::HashSet;

type Outputs = ArrayVec<usize, 20>;
type Input = (Vec<Outputs>, NameToId);

fn parse(filename: &str) -> Input {
    let mut result = Vec::new();
    let mut name_to_id = NameToId::new();
    for line in aoc::file::read_lines(filename) {
        let (source, dest) = line.split_once(": ").unwrap();
        let source = name_to_id.intern(source);
        let outputs = dest
            .split(' ')
            .map(|out| name_to_id.intern(out))
            .collect::<Outputs>();
        result.resize(name_to_id.new_len(result.len()), Outputs::default());
        result[source] = outputs;
    }
    return (result, name_to_id);
}

fn search(
    unvisited: &mut Vec<(usize, HashSet<usize>)>,
    device_map: &Vec<Outputs>,
    start: usize,
    end: usize,
) -> u64 {
    println!("{} -> {}", start, end);
    unvisited.push((start, HashSet::from([start])));
    let mut num_paths = 0;
    while (!unvisited.is_empty()) {
        let (current, visited) = unvisited.pop().unwrap();
        if (current == end) {
            num_paths += 1;
            continue;
        }
        unvisited.extend(
            device_map[current]
                .iter()
                .filter(|&out| !visited.contains(out))
                .map(|&out| {
                    let mut new_visited = visited.clone();
                    new_visited.insert(out);
                    (out, new_visited)
                }),
        );
    }
    println!("  => {}", num_paths);
    return num_paths;
}

fn solve_case1((device_map, name_to_id): &Input) -> u64 {
    search(
        &mut Vec::new(),
        device_map,
        *name_to_id
            .get("you")
            .or_else(|| name_to_id.get("svr"))
            .unwrap(),
        name_to_id.expect("out"),
    )
}

fn solve_case2((device_map, name_to_id): &Input) -> u64 {
    let svr = name_to_id.expect("svr");
    let dac = name_to_id.expect("dac");
    let fft = name_to_id.expect("fft");
    let out = name_to_id.expect("out");
    println!("device_map size {}", device_map.len());
    println!("svr: {}, dac: {}, fft: {}, out: {}", svr, dac, fft, out);

    let mut unvisited = Vec::new();

    let svr_dac = search(&mut unvisited, device_map, svr, dac);
    let dac_fft = search(&mut unvisited, device_map, dac, fft);
    let fft_out = search(&mut unvisited, device_map, fft, out);

    let svr_fft = search(&mut unvisited, device_map, svr, fft);
    let fft_dac = search(&mut unvisited, device_map, fft, dac);
    let dac_out = search(&mut unvisited, device_map, dac, out);

    return (svr_dac * dac_fft * fft_out) + (svr_fft * fft_dac * dac_out);
}

fn main() {
    println!("Part 1");
    let example = parse("day11.example");
    assert_eq!(5, solve_case1(&example));
    let example2 = parse("day11.example2");
    assert_eq!(8, solve_case1(&example2));
    let input = parse("day11.input");
    assert_eq!(523, solve_case1(&input));

    println!("Part 2");
    assert_eq!(2, solve_case2(&example2));
    println!("Part 2 not done yet");
    //assert_eq!(100011612, solve_case2(&input));
}
