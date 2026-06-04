use std::collections::HashMap;

use arrayvec::ArrayVec;

const NUM_RESOURCES: usize = 4;
type Resources = [u8; NUM_RESOURCES];
type Robots = Resources;
const NUM_ROBOTS: usize = NUM_RESOURCES;
type Blueprint = [Resources; NUM_ROBOTS];
const ORE_ID: usize = 0;
const CLAY_ID: usize = 1;
const OBSIDIAN_ID: usize = 2;
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
                .collect::<ArrayVec<Resources, NUM_RESOURCES>>()
                .into_inner()
                .unwrap()
        })
        .collect()
}

/// This SearchNode is compressed into two bytes:
/// ```
/// struct SearchNode {
///     robots: [u6; 4],
///     resources: [u8; 4],
///     time_left: u8,
/// }
/// ```
#[derive(Clone, Copy, Debug, Default, Hash, PartialEq, Eq)]
#[repr(transparent)]
struct SearchNode(u64);

impl SearchNode {
    const ROBOTS_MASK: u64 = 0x3f; // 6 bits
    const RESOURCES_MASK: u64 = 0xff; // 8 bits

    const ROBOTS_SHIFT: [u32; NUM_ROBOTS] = [0, 6, 12, 18];
    const RESOURCES_SHIFT: [u32; NUM_RESOURCES] = [24, 32, 40, 48];
    const TIME_SHIFT: u32 = 56;

    fn new(robots: Robots, resources: Resources, time_left: u8) -> Self {
        let mut node = Self { 0: 0 };

        for i in 0..NUM_ROBOTS {
            node.set_robot(i, robots[i]);
            node.set_resource(i, resources[i]);
        }

        node.set_time_left(time_left);
        node
    }

    fn robots(&self, idx: usize) -> u8 {
        debug_assert!(idx < NUM_ROBOTS);
        ((self.0 >> Self::ROBOTS_SHIFT[idx]) & Self::ROBOTS_MASK) as u8
    }

    fn set_robot(&mut self, idx: usize, value: u8) {
        debug_assert!(idx < NUM_ROBOTS);
        debug_assert!(value < 64);

        let shift = Self::ROBOTS_SHIFT[idx];
        let mask = Self::ROBOTS_MASK << shift;

        self.0 &= !mask;
        self.0 |= (value as u64) << shift;
    }

    fn resources(&self, idx: usize) -> u8 {
        debug_assert!(idx < NUM_RESOURCES);
        ((self.0 >> Self::RESOURCES_SHIFT[idx]) & Self::RESOURCES_MASK) as u8
    }

    fn set_resource(&mut self, idx: usize, value: u8) {
        debug_assert!(idx < NUM_RESOURCES);

        let shift = Self::RESOURCES_SHIFT[idx];
        let mask = Self::RESOURCES_MASK << shift;

        self.0 &= !mask;
        self.0 |= (value as u64) << shift;
    }

    fn time_left(&self) -> u8 {
        (self.0 >> Self::TIME_SHIFT) as u8
    }

    fn set_time_left(&mut self, value: u8) {
        let mask = 0xffu64 << Self::TIME_SHIFT;
        self.0 &= !mask;
        self.0 |= (value as u64) << Self::TIME_SHIFT;
    }

    fn robots_array(&self) -> Robots {
        [
            self.robots(0),
            self.robots(1),
            self.robots(2),
            self.robots(3),
        ]
    }

    fn resources_array(&self) -> Resources {
        [
            self.resources(0),
            self.resources(1),
            self.resources(2),
            self.resources(3),
        ]
    }

    fn collect_resources(&self) -> Resources {
        let mut new_resources = self.resources_array();
        for robot_id in (0..NUM_ROBOTS) {
            new_resources[robot_id] += self.robots(robot_id);
        }
        new_resources
    }
}

type Cache = HashMap<SearchNode, u16>;

fn clamp_resources(max_resources: &Resources, resources: &Resources, time_left: u8) -> Resources {
    let mut new_resources = resources.clone();
    // Skip clamping geode resources
    for (id, max_per_minute) in max_resources[0..GEODE_ID].iter().enumerate() {
        let max_spend = *max_per_minute as u16 * time_left as u16;
        new_resources[id] = (new_resources[id] as u16).min(max_spend) as u8;
    }
    new_resources
}

fn try_build_robot(
    blueprint: &Blueprint,
    max_resources: &Resources,
    robots: &Robots,
    resources: &Resources,
    robot_id: usize,
) -> Option<Resources> {
    let mut new_resources = resources.clone();
    for (cost_id, cost) in blueprint[robot_id].iter().enumerate() {
        if (new_resources[cost_id] < *cost) {
            // Can't afford robot
            return None;
        }
        if (robots[cost_id] > max_resources[cost_id]) {
            // Too many robots of this type
            return None;
        }
        new_resources[cost_id] -= cost;
    }
    Some(new_resources)
}

fn max_open_geodes(
    cache: &mut Cache,
    blueprint: &Blueprint,
    max_resources: &Resources,
    search_node: SearchNode,
) -> u16 {
    let time_left = search_node.time_left();
    if (time_left == 1) {
        // Doesn't make sense to build robots in last minute
        let new_resources = search_node.collect_resources();
        return new_resources[GEODE_ID] as u16;
    }
    debug_assert_ne!(0, time_left, "Time should never reach 0");

    // Try building each type of robot with the resources available
    let robots = search_node.robots_array();
    let resources = search_node.resources_array();

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

    // Building a geode-cracking robot is always better than the alternatives
    if let Some(new_resources) =
        try_build_robot(blueprint, &max_resources, &robots, &resources, GEODE_ID)
    {
        let temp_node = SearchNode::new(robots.clone(), new_resources, time_left);
        let new_resources = temp_node.collect_resources();
        let mut new_robots = robots;
        new_robots[GEODE_ID] += 1;
        return max_open_geodes(
            cache,
            blueprint,
            max_resources,
            SearchNode::new(new_robots, new_resources, time_left - 1),
        );
    }

    let num_geodes = [ORE_ID, CLAY_ID, OBSIDIAN_ID]
        .iter()
        .filter_map(|&robot_id| {
            if let Some(new_resources) =
                try_build_robot(blueprint, &max_resources, &robots, &resources, robot_id)
            {
                let temp_node = SearchNode::new(robots.clone(), new_resources, time_left);
                let new_resources = temp_node.collect_resources();
                let mut new_robots = robots.clone();
                new_robots[robot_id] += 1;
                return Some(max_open_geodes(
                    cache,
                    blueprint,
                    max_resources,
                    SearchNode::new(new_robots, new_resources, time_left - 1),
                ));
            }
            return None;
        })
        .max()
        .unwrap_or(0)
        .max({
            // Try not building any robot
            let new_resources = search_node.collect_resources();
            max_open_geodes(
                cache,
                blueprint,
                max_resources,
                SearchNode::new(robots, new_resources, time_left - 1),
            )
        });
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
                &max_resources(&blueprint),
                SearchNode::new([1, 0, 0, 0], Resources::default(), 24),
            );
            println!("   Blueprint {} produces {} geodes", id, num_geodes);
            let score = (id as u16 + 1) * num_geodes;
            score
        })
        .sum()
}

fn solve_case2(blueprints: &[Blueprint]) -> u16 {
    let mut cache = Cache::new();
    blueprints
        .iter()
        .take(3)
        .enumerate()
        .map(|(id, blueprint)| {
            cache.clear();
            let num_geodes = max_open_geodes(
                &mut cache,
                blueprint,
                &max_resources(&blueprint),
                SearchNode::new([1, 0, 0, 0], Resources::default(), 32),
            );
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
