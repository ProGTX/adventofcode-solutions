type Segment = aoc::closed_range::ClosedRange<i64>;

#[derive(Clone, Copy)]
struct SingleMapping {
    src: Segment,
    dst: Segment,
}

struct Input {
    seed_ints: Vec<i64>,
    mappings: Vec<Vec<SingleMapping>>,
}

fn parse(filename: &str) -> Input {
    let lines = aoc::file::read_lines(filename);
    let mut iter = lines.iter();

    let first_line = iter.next().unwrap();
    let (_, seeds_str) = first_line.split_once(':').unwrap();
    let seed_ints: Vec<i64> = seeds_str
        .split_whitespace()
        .map(|n| n.parse().unwrap())
        .collect();

    let mut mappings: Vec<Vec<SingleMapping>> = Vec::new();
    let mut mapping: Vec<SingleMapping> = Vec::new();

    for line in iter {
        if !line.starts_with(|c: char| c.is_ascii_digit()) {
            mapping.sort_unstable_by_key(|m| m.src);
            mappings.push(std::mem::take(&mut mapping));
            continue;
        }
        let mut parts = line.split_whitespace();
        let dest_start: i64 = parts.next().unwrap().parse().unwrap();
        let source_start: i64 = parts.next().unwrap().parse().unwrap();
        let range: i64 = parts.next().unwrap().parse().unwrap();
        mapping.push(SingleMapping {
            src: Segment::new(source_start, source_start + range),
            dst: Segment::new(dest_start, dest_start + range),
        });
    }
    mapping.sort_unstable_by_key(|m| m.src);
    mappings.push(mapping);

    Input {
        seed_ints,
        mappings,
    }
}

fn map_single(seed: Segment, map: SingleMapping) -> Segment {
    let offset = map.dst.begin - map.src.begin;
    Segment::new(seed.begin + offset, seed.end + offset)
}

// Diagram for the explanation of how an individual map applies to a seed range
// (x-y) seed range
// (a-b) map range
// Depending on the order, there are 4 possible scenarios
// Hyphen indicates copying the ranges as-is, plus means map is applied
// (x,a,b,y) - (x-a-1)(a+b)(b+1-y)
// (a,x,b,y) - (x+b)(b+1-y)
// (x,a,y,b) - (x-a-1)(a+y)
// (a,x,y,b) - (x+y)
fn apply_mapping(mut current_seeds: Vec<Segment>, mapping: &[SingleMapping]) -> Vec<Segment> {
    // We want all the ranges to be sorted to simplify the algorithm
    debug_assert!(
        mapping.windows(2).all(|w| w[0].src <= w[1].src),
        "mapping not presorted"
    );

    let mut next_seeds: Vec<Segment> = Vec::with_capacity(current_seeds.len());
    let mut seed_idx = 0usize;
    let mut map_idx = 0usize;

    loop {
        // find_relevant_map: find first seed range where the map applies
        loop {
            if map_idx >= mapping.len() {
                // End of mapping, add all the remaining seeds as-is
                next_seeds.extend_from_slice(&current_seeds[seed_idx..]);
                seed_idx = current_seeds.len();
                break;
            }
            if seed_idx >= current_seeds.len() {
                // Out of seeds, ignore any remaining mapping
                break;
            }
            if current_seeds[seed_idx].overlaps_with(mapping[map_idx].src) {
                // Map applies to this seed range,
                // return to perform the rest of the algorithm
                break;
            }
            if mapping[map_idx].src < current_seeds[seed_idx] {
                // Map behind seed range
                map_idx += 1;
            } else {
                // Seed range behind map, pass seed range as-is
                next_seeds.push(current_seeds[seed_idx]);
                seed_idx += 1;
            }
        }

        if map_idx >= mapping.len() || seed_idx >= current_seeds.len() {
            break;
        }

        // apply_map_to_seeds: find last seed range where the map applies,
        // for everything in between apply the map
        'apply: loop {
            if seed_idx >= current_seeds.len() {
                break;
            }
            if !current_seeds[seed_idx].overlaps_with(mapping[map_idx].src) {
                // Map doesn't apply anymore, return to outer loop
                break;
            }
            // Note that end() points to 1 beyond the last element,
            // so we decrement by one
            let a = mapping[map_idx].src.begin;
            let b = mapping[map_idx].src.end - 1;
            let x = current_seeds[seed_idx].begin;
            let y = current_seeds[seed_idx].end - 1;

            let split = if current_seeds[seed_idx].contains(mapping[map_idx].src) {
                // (x-a-1)(a+b)(b+1-y)
                if (a - x) > 1 {
                    // (x-a-1)
                    next_seeds.push(Segment::new(x, a));
                }
                // (a+b)
                next_seeds.push(map_single(mapping[map_idx].src, mapping[map_idx]));
                if (y - b) > 0 {
                    // (b+1-y)
                    current_seeds[seed_idx] = Segment::new(x, b + 2);
                    current_seeds.insert(seed_idx + 1, Segment::new(b + 1, y + 1));
                    seed_idx += 1;
                    map_idx += 1;
                    true
                } else {
                    false
                }
            } else if mapping[map_idx].src.contains(current_seeds[seed_idx]) {
                // (a,x,y,b) - (x+y)
                next_seeds.push(map_single(current_seeds[seed_idx], mapping[map_idx]));
                false
            } else if (x >= a) && (b <= y) {
                // (a,x,b,y) - (x+b)(b+1-y)
                // (x+b)
                next_seeds.push(map_single(Segment::new(x, b + 1), mapping[map_idx]));
                // (b+1-y)
                current_seeds[seed_idx] = Segment::new(x, b + 2);
                current_seeds.insert(seed_idx + 1, Segment::new(b + 1, y + 1));
                seed_idx += 1;
                map_idx += 1;
                true
            } else {
                // (x,a,y,b) - (x-a-1)(a+y)
                // (x-a-1)
                next_seeds.push(Segment::new(x, a));
                // (a+y)
                next_seeds.push(map_single(Segment::new(a, y + 1), mapping[map_idx]));
                false
            };

            if split {
                // Important to break the loop after the split
                // because the map doesn't apply anymore
                break 'apply;
            }
            seed_idx += 1;
        }
    }

    next_seeds.sort_unstable();
    next_seeds
}

fn solve_case1(input: &Input) -> i64 {
    let mut current_seeds: Vec<Segment> = input
        .seed_ints
        .chunks(2)
        .flat_map(|c| [Segment::new(c[0], c[0] + 1), Segment::new(c[1], c[1] + 1)])
        .collect();
    for mapping in &input.mappings {
        current_seeds = apply_mapping(current_seeds, mapping);
    }
    current_seeds[0].begin
}

fn solve_case2(input: &Input) -> i64 {
    let mut current_seeds: Vec<Segment> = input
        .seed_ints
        .chunks(2)
        .map(|c| Segment::new(c[0], c[0] + c[1]))
        .collect();
    for mapping in &input.mappings {
        current_seeds = apply_mapping(current_seeds, mapping);
    }
    current_seeds[0].begin
}

fn main() {
    println!("Part 1");
    let example = parse("day05.example");
    aoc::expect_result!(35, solve_case1(&example));
    let input = parse("day05.input");
    aoc::expect_result!(486613012, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(46, solve_case2(&example));
    aoc::expect_result!(56931769, solve_case2(&input));
}
