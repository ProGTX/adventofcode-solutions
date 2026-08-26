use aoc::grid::Grid;
use itertools::Itertools;

#[derive(Clone, Copy, Default)]
struct Node {
    size: usize,
    used: usize,
}

/// The nodes of the storage cluster, by their x and y coordinates
type Input = Grid<Node>;

fn parse(filename: &str) -> Input {
    // /dev/grid/node-xX-yY    SIZET   USEDT   AVAILT   USE%
    let nodes = aoc::file::read_lines(filename)
        .iter()
        // The two lines of the df header come first
        .skip(2)
        .map(|line| {
            let words = line.split_whitespace().collect_vec();
            let (x, y) = words[0]
                .trim_start_matches("/dev/grid/node-x")
                .split_once("-y")
                .unwrap();
            let value = |word: usize| {
                return words[word].trim_end_matches('T').parse().unwrap();
            };
            (
                x.parse::<usize>().unwrap(),
                y.parse::<usize>().unwrap(),
                Node {
                    size: value(1),
                    used: value(2),
                },
            )
        })
        .collect_vec();
    let num_columns = nodes.iter().map(|(x, _, _)| x).max().unwrap() + 1;
    let num_rows = nodes.iter().map(|(_, y, _)| y).max().unwrap() + 1;
    // The nodes are listed by column, so each one goes to its own place
    let mut cluster = Grid::new(Node::default(), num_rows, num_columns);
    for (x, y, node) in nodes {
        *cluster.get_mut(y, x) = node;
    }
    return cluster;
}

/// The number of viable pairs of nodes
fn solve_case1(cluster: &Input) -> usize {
    let mut num_viable_pairs = 0;
    let network = (0..cluster.num_columns).cartesian_product(0..cluster.num_rows);
    for (x_a, y_a) in network.clone() {
        let a = cluster.get(y_a, x_a);
        if (a.used == 0) {
            // Node A must not be empty
            continue;
        }
        for (x_b, y_b) in network.clone() {
            if (x_a, y_a) == (x_b, y_b) {
                // Nodes A and B most not be the same node
                continue;
            }
            let b = cluster.get(y_b, x_b);
            // The data on node A would fit on node B
            num_viable_pairs += (a.used <= (b.size - b.used)) as usize;
        }
    }
    return num_viable_pairs;
}

fn main() {
    println!("Part 1");
    let input = parse("day22.input");
    aoc::expect_result!(993, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
