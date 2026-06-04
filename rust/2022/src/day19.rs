use std::collections::HashMap;

use arrayvec::ArrayVec;

const NUM_RESOURCES: usize = 4;
type Resources = [u8; NUM_RESOURCES];
type Robots = Resources;
const NUM_ROBOTS: usize = NUM_RESOURCES;
type Blueprint = [Resources; NUM_ROBOTS];
const GEODE_ID: usize = 3;
type Time = u8;

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
                .collect::<ArrayVec<Resources, NUM_RESOURCES>>()
                .into_inner()
                .unwrap()
        })
        .collect()
}

/// robots and time_left are packed into a u32:
/// - robots: [u6; 4] (bits 0-23)
/// - time_left: u8 (bits 24-31)
#[derive(Clone, Copy, Debug, Default, Hash, PartialEq, Eq)]
struct SearchNode {
    packed: u32,
    resources: Resources,
}

impl SearchNode {
    const ROBOTS_MASK: u32 = 0x3f; // 6 bits
    const ROBOTS_SHIFT: [u32; NUM_ROBOTS] = [0, 6, 12, 18];
    const TIME_SHIFT: u32 = 24;

    fn new(robots: Robots, resources: Resources, time_left: Time) -> Self {
        let mut node = Self {
            packed: 0,
            resources,
        };
        for i in 0..NUM_ROBOTS {
            node.set_robot(i, robots[i]);
        }
        node.set_time_left(time_left);
        node
    }

    fn robots(&self, idx: usize) -> u8 {
        debug_assert!(idx < NUM_ROBOTS);
        ((self.packed >> Self::ROBOTS_SHIFT[idx]) & Self::ROBOTS_MASK) as u8
    }

    fn set_robot(&mut self, idx: usize, value: u8) {
        debug_assert!(idx < NUM_ROBOTS);
        debug_assert!(value < 64);
        let shift = Self::ROBOTS_SHIFT[idx];
        let mask = Self::ROBOTS_MASK << shift;
        self.packed &= !mask;
        self.packed |= (value as u32) << shift;
    }

    fn time_left(&self) -> Time {
        (self.packed >> Self::TIME_SHIFT) as Time
    }

    fn set_time_left(&mut self, value: Time) {
        let mask = 0xffu32 << Self::TIME_SHIFT;
        self.packed &= !mask;
        self.packed |= (value as u32) << Self::TIME_SHIFT;
    }

    fn robots_array(&self) -> Robots {
        [
            self.robots(0),
            self.robots(1),
            self.robots(2),
            self.robots(3),
        ]
    }

    /// Collects resources using current robots,
    /// returns total new resources after collection
    fn collect_resources(&self) -> Resources {
        let mut new_resources = self.resources;
        for robot_id in 0..NUM_ROBOTS {
            new_resources[robot_id] = new_resources[robot_id].saturating_add(self.robots(robot_id));
        }
        new_resources
    }
}

type Cache = HashMap<SearchNode, u16>;

fn clamp_resources(max_resources: &Resources, resources: &Resources, time_left: Time) -> Resources {
    let mut new_resources = resources.clone();
    // Skip clamping geode resources
    for (id, max_per_minute) in max_resources[0..GEODE_ID].iter().enumerate() {
        let max_spend = *max_per_minute as u16 * time_left as u16;
        new_resources[id] = (new_resources[id] as u16).min(max_spend) as u8;
    }
    new_resources
}

/// Figure out how much time it takes to build a robot
/// and what the resources would be at the end
fn try_build_robot(
    blueprint: &Blueprint,
    max_resources: &Resources,
    search_node: &SearchNode,
    robot_id: usize,
) -> Option<(Time, Resources)> {
    // First check if we have too many robots already
    for cost_id in 0..blueprint[robot_id].len() {
        if (search_node.robots(cost_id) > max_resources[cost_id]) {
            // Too many robots of this type
            return None;
        }
    }

    // Advance time until we can build the robot
    let mut new_node = search_node.clone();
    for minute in 0..search_node.time_left() {
        let time_left = new_node.time_left();
        if (time_left == 0) {
            // Cannot build robot in time
            return None;
        }

        // Check if we can build the robot now
        let mut new_resources = new_node.resources;
        let mut could_build = true;
        for (cost_id, cost) in blueprint[robot_id].iter().enumerate() {
            if (new_node.resources[cost_id] < *cost) {
                // Can't afford robot yet
                could_build = false;
                break;
            }
            new_resources[cost_id] -= cost;
        }

        if (could_build) {
            new_node.resources = new_resources;
        }

        // Spend a minute collecting resources
        let new_resources = new_node.collect_resources();
        new_node.resources = new_resources;

        if (could_build) {
            return Some((minute + 1, new_resources));
        }
        new_node.set_time_left(time_left - 1);
    }

    // Couldn't build robot in time
    return None;
}

fn max_open_geodes_cached(
    cache: &mut Cache,
    current_best: &mut u16,
    blueprint: &Blueprint,
    max_resources: &Resources,
    search_node: SearchNode,
) -> u16 {
    let time_left = search_node.time_left();
    let current_geodes = search_node.resources[GEODE_ID] as u16;
    if (time_left == 0) {
        return current_geodes;
    }

    // Try building each type of robot with the resources available
    let robots = search_node.robots_array();
    let resources = search_node.resources;

    // If we have more resources than we can use,
    // then the cache node is functionally equivalent to the one with the largest resources
    let cache_node = SearchNode::new(
        robots.clone(),
        clamp_resources(max_resources, &resources, time_left),
        time_left,
    );

    if let Some(&cached_result) = cache.get(&cache_node) {
        return cached_result;
    }

    // Evaluate the potential whether it's even worth exploring
    let default_until_end =
        current_geodes + (search_node.robots(GEODE_ID) as u16) * (time_left as u16);
    *current_best = (*current_best).max(default_until_end);
    let max_potential_geodes = {
        let time = time_left as u16;
        current_geodes + ((robots[GEODE_ID] as u16) * time) + (time * (time - 1)) / 2
    };

    // Try building each type of robot, starting with geode-cracking robots
    let num_geodes = (0..NUM_ROBOTS)
        .rev()
        .filter_map(|robot_id| {
            if (*current_best > max_potential_geodes) {
                // Not worth continuing this search
                return None;
            }
            if let Some((time_to_build, new_resources)) =
                try_build_robot(blueprint, &max_resources, &search_node, robot_id)
            {
                // Build the robot (we already know the final resources)
                // and investigate further

                let mut new_node =
                    SearchNode::new(robots.clone(), new_resources, time_left - time_to_build);
                new_node.set_robot(robot_id, new_node.robots(robot_id) + 1);
                let variant_geodes =
                    max_open_geodes_cached(cache, current_best, blueprint, max_resources, new_node);
                *current_best = (*current_best).max(variant_geodes);
                return Some(variant_geodes);
            }
            return None;
        })
        .max()
        .unwrap_or(default_until_end);
    cache.insert(cache_node, num_geodes);
    return num_geodes;
}

fn max_resources(blueprint: &Blueprint) -> Resources {
    let mut resources = Resources::default();
    for robot_costs in blueprint {
        for (i, &cost) in robot_costs.iter().enumerate() {
            resources[i] = resources[i].max(cost);
        }
    }
    // We always want more geode-cracking robots
    resources[GEODE_ID] = u8::MAX;
    resources
}

fn max_open_geodes(blueprint: &Blueprint, time_left: Time) -> u16 {
    let mut cache = Cache::new();
    let mut current_best = 0;
    max_open_geodes_cached(
        &mut cache,
        &mut current_best,
        blueprint,
        &max_resources(&blueprint),
        SearchNode::new([1, 0, 0, 0], Resources::default(), time_left),
    )
}

fn solve_case1(blueprints: &[Blueprint]) -> u16 {
    blueprints
        .iter()
        .enumerate()
        .map(|(id, blueprint)| {
            let num_geodes = max_open_geodes(blueprint, 24);
            println!("   Blueprint {} produces {} geodes", id, num_geodes);
            let score = (id as u16 + 1) * num_geodes;
            score
        })
        .sum()
}

fn solve_case2(blueprints: &[Blueprint]) -> u16 {
    blueprints
        .iter()
        .take(3)
        .enumerate()
        .map(|(id, blueprint)| {
            let num_geodes = max_open_geodes(blueprint, 32);
            println!("   Blueprint {} produces {} geodes", id, num_geodes);
            num_geodes
        })
        .product()
}

fn main() {
    println!("Part 1");
    let example = parse("day19.example");
    aoc::expect_result!(33, solve_case1(&example));
    let input = parse("day19.input");
    aoc::expect_result!(1395, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(56 * 62, solve_case2(&example));
    aoc::expect_result!(2700, solve_case2(&input));
}
