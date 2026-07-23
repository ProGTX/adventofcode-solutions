use aoc::{
    algorithm::flood_fill,
    closed_range::ClosedRange,
    direction::Direction,
    grid::{BASIC_NEIGHBOR_DIFFS, ConfigInput, Grid, Ipos, Upos},
};
use itertools::Itertools;
use rustc_hash::{FxHashMap, FxHashSet};

type Garden = Grid<char>;
type Region = FxHashSet<usize>;
type Side = ClosedRange<Upos>;
const PADDING: char = '#';

fn get_regions(garden: &Garden) -> Vec<Region> {
    let get_region = |(start)| {
        flood_fill(&start, |current| {
            let current_upos = garden.position(*current);
            let current_ipos = Ipos::new(current_upos.x as isize, current_upos.y as isize);
            let current_value = garden.get(current_ipos.y as usize, current_ipos.x as usize);
            BASIC_NEIGHBOR_DIFFS.iter().filter_map(move |diff| {
                let neighbor_pos = current_ipos + *diff;
                let neighbor = Upos::new(neighbor_pos.x as usize, neighbor_pos.y as usize);
                (garden.get(neighbor.y, neighbor.x) == current_value)
                    .then(|| garden.linear_from_pos(neighbor))
            })
        })
    };

    let mut regions = Vec::new();
    regions.push(get_region(garden.linear_index(1, 1)));
    for (linear_index, plant) in garden.data.iter().enumerate() {
        if ((*plant == PADDING) || regions.iter().any(|region| region.contains(&linear_index))) {
            continue;
        }
        regions.push(get_region(linear_index));
    }
    return regions;
}

fn parse(filename: &str) -> (Garden, Vec<Region>) {
    let garden = Garden::from_file_config(
        filename,
        ConfigInput {
            padding: Some(PADDING),
            ..Default::default()
        },
    )
    .0;
    let regions = get_regions(&garden);
    (garden, regions)
}

fn perimeter(garden: &Garden, region: &Region) -> Vec<Side> {
    let mut occurence = FxHashMap::<Side, u32>::default();
    for index in region {
        // We need to convert a single plant into 4 sides of a square
        let current = garden.position(*index);
        for side in [
            // Define the corners clockwise
            current.clone(),
            Upos::new(current.x + 1, current.y),
            Upos::new(current.x + 1, current.y + 1),
            Upos::new(current.x, current.y + 1),
            current, // Needs to be circular
        ]
        .array_windows::<2>()
        {
            *occurence.entry(ClosedRange::new(side[0], side[1])).or_default() += 1;
        }
    }
    return occurence
        .into_iter()
        .filter_map(|(side, count)| (count == 1).then(|| side))
        .collect_vec();
}

fn solve_case1((garden, regions): &(Garden, Vec<Region>)) -> usize {
    regions
        .iter()
        .map(|region| region.len() * perimeter(garden, region).len())
        .sum()
}

/// Which side of a fence segment the region sits on,
/// i.e. which of the segment's two flanking cells is part of `region`.
fn fence_direction(garden: &Garden, region: &Region, side: &Side) -> Direction {
    let (from, to) = (*side).into();
    if from.y == to.y {
        let below = garden.linear_index(from.y, from.x);
        if region.contains(&below) {
            Direction::North
        } else {
            Direction::South
        }
    } else {
        let right = garden.linear_index(from.y, from.x);
        if region.contains(&right) {
            Direction::West
        } else {
            Direction::East
        }
    }
}

/// Merges fence segments into sides: segments in the same direction,
/// collinear (sharing a row for North/South, a column for East/West),
/// and consecutive along that line, belong to the same straight side.
fn count_sides(garden: &Garden, region: &Region, fence: &[Side]) -> usize {
    let mut groups = FxHashMap::<_, Vec<usize>>::default();
    // Bucket each segment by its direction and the line it lies on,
    // recording its position along that line.
    for &side in fence {
        let from = side.begin;
        let direction = fence_direction(garden, region, &side);
        let (line, position) = match direction {
            Direction::North | Direction::South => (from.y, from.x),
            Direction::East | Direction::West => (from.x, from.y),
            _ => unreachable!("perimeter fence segments are always axis-aligned"),
        };
        groups.entry((direction, line)).or_default().push(position);
    }

    // Within each bucket, count runs of consecutive positions:
    // each gap starts a new side, so a side is one run.
    groups
        .into_values()
        .map(|mut positions| {
            positions.sort_unstable();
            1 + positions
                .iter()
                .tuple_windows()
                .filter(|&(a, b)| *b != *a + 1)
                .count()
        })
        .sum()
}

fn solve_case2((garden, regions): &(Garden, Vec<Region>)) -> usize {
    regions
        .iter()
        .map(|region| {
            let fence = perimeter(garden, region);
            region.len() * count_sides(garden, region, &fence)
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day12.example");
    aoc::expect_result!(140, solve_case1(&example));
    let example2 = parse("day12.example2");
    aoc::expect_result!(772, solve_case1(&example2));
    let example3 = parse("day12.example3");
    aoc::expect_result!(1930, solve_case1(&example3));
    let example4 = parse("day12.example4");
    aoc::expect_result!(692, solve_case1(&example4));
    let example5 = parse("day12.example5");
    aoc::expect_result!(1184, solve_case1(&example5));
    let input = parse("day12.input");
    aoc::expect_result!(1450422, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(80, solve_case2(&example));
    aoc::expect_result!(436, solve_case2(&example2));
    aoc::expect_result!(1206, solve_case2(&example3));
    aoc::expect_result!(236, solve_case2(&example4));
    aoc::expect_result!(368, solve_case2(&example5));
    aoc::expect_result!(906606, solve_case2(&input));
}
