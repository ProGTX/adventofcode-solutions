use std::collections::HashMap;

use arrayvec::ArrayVec;

const NUM_RESOURCES: usize = 4;
type Resources = [u8; NUM_RESOURCES];
type Robots = Resources;
const NUM_ROBOTS: usize = NUM_RESOURCES;
type Blueprint = [Resources; NUM_ROBOTS];
const GEODE_ID: usize = 3;

fn parse(filename: &str) -> Vec<Blueprint> {
    aoc::file::read_lines(filename)
        .iter()
        .map(|line| {
            line.split(" Each ")
                .skip(1)
                .map(|robots| {
                    let parts = robots.split_whitespace().collect::<Vec<_>>();
                    let parse = |pos: usize| parts[pos].parse::<u8>().unwrap();
                    match parts[0] {
                        "ore" => [parse(3), 0, 0, 0],
                        "clay" => [parse(3), 0, 0, 0],
                        "obsidian" => [parse(3), parse(6), 0, 0],
                        "geode" => [parse(3), 0, parse(6), 0],
                        _ => unreachable!("Invalid robot type"),
                    }
                })
                .collect::<ArrayVec<Resources, 4>>()
                .into_inner()
                .unwrap()
        })
        .collect()
}

fn try_build_robot(
    blueprint: &Blueprint,
    resources: &Resources,
    robot_id: usize,
) -> Option<Resources> {
    let mut new_resources = resources.clone();
    for (cost_id, cost) in blueprint[robot_id].iter().enumerate() {
        if (new_resources[cost_id] < *cost) {
            return None;
        }
        new_resources[cost_id] -= cost;
    }
    Some(new_resources)
}

fn collect_resources(robots: &Robots, resources: &Resources) -> Resources {
    let mut new_resources = resources.clone();
    for robot_id in (0..NUM_ROBOTS) {
        new_resources[robot_id] += robots[robot_id];
    }
    new_resources
}

#[derive(Debug, Eq, Hash, PartialEq)]
struct SearchNode {
    robots: Robots,
    resources: Resources,
    time_left: u8,
}

type Cache = HashMap<SearchNode, u16>;

fn max_open_geodes(cache: &mut Cache, blueprint: &Blueprint, search_node: SearchNode) -> u16 {
    if (search_node.time_left == 1) {
        // Doesn't make sense to build robots in last minute
        let new_resources = collect_resources(&search_node.robots, &search_node.resources);
        return new_resources[GEODE_ID] as u16;
    }
    debug_assert_ne!(0, search_node.time_left, "Time should never reach 0");
    if let Some(&cached_result) = cache.get(&search_node) {
        return cached_result;
    }
    // Try building each type of robot with the resources available
    let num_geodes = (0..NUM_ROBOTS)
        .filter_map(|robot_id| {
            if let Some(new_resources) =
                try_build_robot(blueprint, &search_node.resources, robot_id)
            {
                let new_resources = collect_resources(&search_node.robots, &new_resources);
                let mut new_robots = search_node.robots.clone();
                new_robots[robot_id] += 1;
                return Some(max_open_geodes(
                    cache,
                    blueprint,
                    SearchNode {
                        robots: new_robots,
                        resources: new_resources,
                        time_left: search_node.time_left - 1,
                    },
                ));
            }
            return None;
        })
        .max()
        .unwrap_or(0)
        .max({
            // Try not building any robot
            let new_resources = collect_resources(&search_node.robots, &search_node.resources);
            max_open_geodes(
                cache,
                blueprint,
                SearchNode {
                    robots: search_node.robots,
                    resources: new_resources,
                    time_left: search_node.time_left - 1,
                },
            )
        });
    cache.insert(search_node, num_geodes);
    return num_geodes;
}

fn solve_case1(blueprints: &[Blueprint]) -> u16 {
    let mut cache = Cache::new();
    blueprints
        .iter()
        .enumerate()
        .map(|(id, blueprint)| {
            cache.clear();
            let num_geodes = max_open_geodes(
                &mut cache,
                blueprint,
                SearchNode {
                    robots: [1, 0, 0, 0],
                    resources: Resources::default(),
                    time_left: 24,
                },
            );
            println!("   Blueprint {} produces {} geodes", id, num_geodes);
            let score = (id as u16 + 1) * num_geodes;
            score
        })
        .sum()
}

fn main() {
    println!("Part 1");
    let example = parse("day19.example");
    aoc::expect_result!(33, solve_case1(&example));
    let input = parse("day19.input");
    aoc::expect_result!(1395, solve_case1(&input));

    // println!("Part 2");
    // aoc::expect_result!(56 * 62, solve_case2(&example));
    // aoc::expect_result!(1337, solve_case2(&input));
}
