use core::range::RangeInclusive;

#[derive(Clone)]
struct Cuboid {
    x: RangeInclusive<i32>,
    y: RangeInclusive<i32>,
    z: RangeInclusive<i32>,
    sign: i32,
}
impl Cuboid {
    fn size(&self) -> usize {
        1 * (self.x.last - self.x.start + 1) as usize
            * (self.y.last - self.y.start + 1) as usize
            * (self.z.last - self.z.start + 1) as usize
    }
}

type Input = Vec<Cuboid>;

fn parse(filename: &str) -> Input {
    let mut cuboids = Vec::new();

    for line in aoc::file::read_lines(filename) {
        let (state, coords) = line.split_once(' ').unwrap();
        let sign = if state == "on" { 1 } else { -1 };

        let parse_range = |s: &str| -> RangeInclusive<i32> {
            let (_, nums) = s.split_once('=').unwrap();
            let (start, end) = nums.split_once("..").unwrap();
            RangeInclusive {
                start: start.parse().unwrap(),
                last: end.parse().unwrap(),
            }
        };

        let mut parts = coords.split(',');
        cuboids.push(Cuboid {
            x: parse_range(parts.next().unwrap()),
            y: parse_range(parts.next().unwrap()),
            z: parse_range(parts.next().unwrap()),
            sign,
        });
    }

    cuboids
}

fn intersect(a: &Cuboid, b: &Cuboid) -> Option<Cuboid> {
    let x = RangeInclusive {
        start: a.x.start.max(b.x.start),
        last: a.x.last.min(b.x.last),
    };
    let y = RangeInclusive {
        start: a.y.start.max(b.y.start),
        last: a.y.last.min(b.y.last),
    };
    let z = RangeInclusive {
        start: a.z.start.max(b.z.start),
        last: a.z.last.min(b.z.last),
    };
    (x.start <= x.last && y.start <= y.last && z.start <= z.last).then_some(Cuboid {
        x,
        y,
        z,
        sign: -a.sign,
    })
}

fn clamp(c: &Cuboid, lo: i32, hi: i32) -> Option<Cuboid> {
    let x = RangeInclusive {
        start: c.x.start.max(lo),
        last: c.x.last.min(hi),
    };
    let y = RangeInclusive {
        start: c.y.start.max(lo),
        last: c.y.last.min(hi),
    };
    let z = RangeInclusive {
        start: c.z.start.max(lo),
        last: c.z.last.min(hi),
    };
    (x.start <= x.last && y.start <= y.last && z.start <= z.last).then_some(Cuboid {
        x,
        y,
        z,
        sign: c.sign,
    })
}

fn reboot(reboot_steps: &[Cuboid]) -> u64 {
    let mut cuboids: Vec<Cuboid> = Vec::new();
    for step in reboot_steps {
        let mut new_cuboids = cuboids.clone();
        for c in &cuboids {
            if let Some(i) = intersect(c, step) {
                new_cuboids.push(i);
            }
        }
        if step.sign > 0 {
            new_cuboids.push(step.clone());
        }
        cuboids = new_cuboids;
    }
    cuboids
        .iter()
        .map(|c| c.size() as i64 * c.sign as i64)
        .sum::<i64>() as u64
}

fn solve_case1(reboot_steps: &[Cuboid]) -> u64 {
    let clamped: Vec<Cuboid> = reboot_steps
        .iter()
        .filter_map(|s| clamp(s, -50, 50))
        .collect();
    reboot(&clamped)
}

fn solve_case2(reboot_steps: &[Cuboid]) -> u64 {
    reboot(reboot_steps)
}

fn main() {
    println!("Part 1");
    let example = parse("day22.example");
    aoc::expect_result!(39, solve_case1(&example));
    let example2 = parse("day22.example2");
    aoc::expect_result!(590784, solve_case1(&example2));
    let example3 = parse("day22.example3");
    aoc::expect_result!(474140, solve_case1(&example3));
    let input = parse("day22.input");
    aoc::expect_result!(596989, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(39, solve_case2(&example));
    aoc::expect_result!(39769202357779, solve_case2(&example2));
    aoc::expect_result!(2758514936282235, solve_case2(&example3));
    aoc::expect_result!(1160011199157381, solve_case2(&input));
}
