use aoc::string::NameToId;
use arrayvec::ArrayVec;

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

fn count_paths(
    device_map: &Vec<Outputs>,
    current: usize,
    end: usize,
    cache: &mut Vec<Option<u64>>,
) -> u64 {
    if current == end {
        // Reached the destination: one complete path found
        return 1;
    }
    if let Some(result) = cache[current] {
        return result;
    }

    // Total paths from here = sum of paths from each outgoing neighbor
    let result = device_map[current]
        .iter()
        .map(|&out| count_paths(device_map, out, end, cache))
        .sum();
    cache[current] = Some(result);
    result
}

fn search(device_map: &Vec<Outputs>, start: usize, end: usize) -> u64 {
    let mut cache = vec![None; device_map.len()];
    return count_paths(device_map, start, end, &mut cache);
}

fn solve_case1((device_map, name_to_id): &Input) -> u64 {
    search(
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

    let svr_dac = search(device_map, svr, dac);
    let dac_fft = search(device_map, dac, fft);
    let fft_out = search(device_map, fft, out);

    let svr_fft = search(device_map, svr, fft);
    let fft_dac = search(device_map, fft, dac);
    let dac_out = search(device_map, dac, out);

    return (svr_dac * dac_fft * fft_out) + (svr_fft * fft_dac * dac_out);
}

fn main() {
    println!("Part 1");
    let example = parse("day11.example");
    aoc::expect_result!(5, solve_case1(&example));
    let example2 = parse("day11.example2");
    aoc::expect_result!(8, solve_case1(&example2));
    let input = parse("day11.input");
    aoc::expect_result!(523, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(2, solve_case2(&example2));
    aoc::expect_result!(517315308154944, solve_case2(&input));
}
