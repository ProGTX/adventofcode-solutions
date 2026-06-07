struct RegionInfo {
    width: usize,
    height: usize,
    counts: Vec<usize>,
}

struct Input {
    shape_areas: Vec<usize>,
    regions: Vec<RegionInfo>,
}

fn parse(filename: &str) -> Input {
    let lines = aoc::file::read_lines(filename);
    let mut shape_areas = Vec::new();
    let mut regions = Vec::new();
    let mut current_shape_idx = 0_usize;

    for line in &lines {
        if let Some(idx_str) = line.strip_suffix(':') {
            if let Ok(idx) = idx_str.parse::<usize>() {
                current_shape_idx = idx;
                shape_areas.resize(idx + 1, 0);
                shape_areas[idx] = 0;
            }
        } else if line.contains('#') {
            shape_areas[current_shape_idx] += line.chars().filter(|&c| c == '#').count();
        } else if let Some((dims, counts_str)) = line.split_once(':') {
            if let Some((w_str, h_str)) = dims.split_once('x') {
                if let (Ok(width), Ok(height)) = (w_str.parse::<usize>(), h_str.parse::<usize>()) {
                    let counts = counts_str
                        .split_whitespace()
                        .map(|s| s.parse::<usize>().unwrap())
                        .collect();
                    regions.push(RegionInfo {
                        width,
                        height,
                        counts,
                    });
                }
            }
        }
    }

    Input {
        shape_areas,
        regions,
    }
}

fn can_fit_into_region(shape_areas: &[usize], region: &RegionInfo) -> bool {
    let presents_area: usize = region
        .counts
        .iter()
        .enumerate()
        .map(|(i, &count)| count * shape_areas[i])
        .sum();
    presents_area <= region.width * region.height
}

fn solve_case1(input: &Input) -> usize {
    input
        .regions
        .iter()
        .filter(|region| can_fit_into_region(&input.shape_areas, region))
        .count()
}

fn main() {
    println!("Part 1");
    let input = parse("day12.input");
    aoc::expect_result!(510, solve_case1(&input));
}
