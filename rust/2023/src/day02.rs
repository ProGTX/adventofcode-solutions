fn parse(filename: &str) -> Vec<Vec<String>> {
    aoc::file::read_lines(filename)
        .into_iter()
        .map(|line| {
            let (_, game_str) = line.split_once(':').unwrap();
            game_str.split(';').map(str::to_string).collect()
        })
        .collect()
}

#[derive(Clone, Copy, PartialEq)]
struct CubeConfig {
    red: i32,
    green: i32,
    blue: i32,
}
impl CubeConfig {
    fn contains(self, other: CubeConfig) -> bool {
        self.red >= other.red && self.green >= other.green && self.blue >= other.blue
    }

    fn max(self, other: CubeConfig) -> CubeConfig {
        CubeConfig {
            red: self.red.max(other.red),
            green: self.green.max(other.green),
            blue: self.blue.max(other.blue),
        }
    }

    fn power(self) -> i32 {
        self.red * self.green * self.blue
    }
}

const CONFIG1: CubeConfig = CubeConfig {
    red: 12,
    green: 13,
    blue: 14,
};

/// Returns 1 if game possible, 0 if not (part 1)
/// Returns > 1 if config == {0,0,0} (part 2)
fn cube_power<S: AsRef<str>>(config: Option<CubeConfig>, rounds: &[S]) -> i32 {
    let mut max_config = CubeConfig {
        red: 1,
        green: 1,
        blue: 1,
    };
    for round in rounds {
        let mut current = CubeConfig {
            red: 0,
            green: 0,
            blue: 0,
        };
        for cube_str in round.as_ref().split(',') {
            let cube_str = cube_str.trim();
            if cube_str.is_empty() {
                continue;
            }
            let (number_str, color) = cube_str.split_once(' ').unwrap();
            let number: i32 = number_str.parse().unwrap();
            match color {
                "red" => current.red = number,
                "green" => current.green = number,
                "blue" => current.blue = number,
                _ => unreachable!("Invalid color"),
            }
            if let Some(cfg) = config {
                if !cfg.contains(current) {
                    return 0;
                }
            } else {
                max_config = max_config.max(current);
            }
        }
    }
    max_config.power()
}

fn solve_case1(games: &[Vec<String>]) -> i32 {
    games
        .iter()
        .enumerate()
        .map(|(id, rounds)| cube_power(Some(CONFIG1), rounds) * (id as i32 + 1))
        .sum()
}

fn solve_case2(games: &[Vec<String>]) -> i32 {
    games.iter().map(|rounds| cube_power(None, rounds)).sum()
}

fn main() {
    println!("Asserts");
    aoc::expect_result!(
        1,
        cube_power(
            Some(CONFIG1),
            &["3 blue, 4 red", "1 red, 2 green, 6 blue", "2 green"]
        )
    );
    aoc::expect_result!(
        0,
        cube_power(
            Some(CONFIG1),
            &[
                "8 green, 6 blue, 20 red",
                "5 blue, 4 red, 13 green",
                "5 green, 1 red"
            ]
        )
    );
    aoc::expect_result!(
        48,
        cube_power(
            None,
            &["3 blue, 4 red", "1 red, 2 green, 6 blue", "2 green"]
        )
    );
    aoc::expect_result!(
        1560,
        cube_power(
            None,
            &[
                "8 green, 6 blue, 20 red",
                "5 blue, 4 red, 13 green",
                "5 green, 1 red"
            ]
        )
    );

    println!("Part 1");
    let example = parse("day02.example");
    aoc::expect_result!(8, solve_case1(&example));
    let input = parse("day02.input");
    aoc::expect_result!(2600, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!(2286, solve_case2(&example));
    aoc::expect_result!(86036, solve_case2(&input));
}
