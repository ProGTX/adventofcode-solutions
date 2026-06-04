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

#[derive(Clone, Copy, Debug, Default, Hash, PartialEq, Eq)]
struct SearchNode {
    bits: u64,
}

impl SearchNode {
    const A_BITS: u64 = 0x3f; // 6 bits
    const B_BITS: u64 = 0x7f; // 7 bits; 

    const A_SHIFT: [u32; 4] = [0, 6, 12, 18];
    const B_SHIFT: [u32; 4] = [24, 31, 38, 45];
    const TIME_SHIFT: u32 = 52;

    fn new(robots: Robots, resources: Resources, time_left: u8) -> Self {
        let mut node = Self { bits: 0 };

        for i in 0..4 {
            node.set_robot(i, robots[i]);
            node.set_resource(i, resources[i]);
        }

        node.set_time_left(time_left);
        node
    }

    fn robots(&self, idx: usize) -> u8 {
        assert!(idx < 4);
        ((self.bits >> Self::A_SHIFT[idx]) & Self::A_BITS) as u8
    }

    fn set_robot(&mut self, idx: usize, value: u8) {
        assert!(idx < 4);
        assert!(value < 64);

        let shift = Self::A_SHIFT[idx];
        let mask = Self::A_BITS << shift;

        self.bits &= !mask;
        self.bits |= (value as u64) << shift;
    }

    fn resources(&self, idx: usize) -> u8 {
        assert!(idx < 4);
        ((self.bits >> Self::B_SHIFT[idx]) & Self::B_BITS) as u8
    }

    fn set_resource(&mut self, idx: usize, value: u8) {
        assert!(idx < 4);
        assert!(value < 128);

        let shift = Self::B_SHIFT[idx];
        let mask = Self::B_BITS << shift;

        self.bits &= !mask;
        self.bits |= (value as u64) << shift;
    }

    fn time_left(&self) -> u8 {
        (self.bits >> Self::TIME_SHIFT) as u8
    }

    fn set_time_left(&mut self, value: u8) {
        let mask = 0xffu64 << Self::TIME_SHIFT;
        self.bits &= !mask;
        self.bits |= (value as u64) << Self::TIME_SHIFT;
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

fn max_open_geodes(cache: &mut Cache, blueprint: &Blueprint, search_node: SearchNode) -> u16 {
    let time_left = search_node.time_left();
    if (time_left == 1) {
        // Doesn't make sense to build robots in last minute
        let new_resources = search_node.collect_resources();
        return new_resources[GEODE_ID] as u16;
    }
    debug_assert_ne!(0, time_left, "Time should never reach 0");
    if let Some(&cached_result) = cache.get(&search_node) {
        return cached_result;
    }
    // Try building each type of robot with the resources available
    let robots = search_node.robots_array();
    let resources = search_node.resources_array();
    let num_geodes = (0..NUM_ROBOTS)
        .filter_map(|robot_id| {
            if let Some(new_resources) = try_build_robot(blueprint, &resources, robot_id) {
                let temp_node = SearchNode::new(robots.clone(), new_resources, time_left);
                let new_resources = temp_node.collect_resources();
                let mut new_robots = robots.clone();
                new_robots[robot_id] += 1;
                return Some(max_open_geodes(
                    cache,
                    blueprint,
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
                SearchNode::new(robots, new_resources, time_left - 1),
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
                SearchNode::new([1, 0, 0, 0], Resources::default(), 34),
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
    aoc::expect_result!(1337, solve_case2(&input));
}
