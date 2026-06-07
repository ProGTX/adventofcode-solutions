use aoc::grid::Grid;
use aoc::string::NameToId;
use itertools::Itertools;

#[derive(Clone, Default)]
struct Link {
    to_id: usize,
    happiness: i32,
}

type Connections = Vec<Vec<Link>>;

fn parse(filename: &str) -> Connections {
    let mut name_to_id = NameToId::new();
    let mut connections = Connections::new();
    for line in aoc::file::read_lines(filename) {
        // "Alice would gain 54 happiness units by sitting next to Bob."
        let parts: Vec<&str> = line.split(' ').collect();
        let from_id = name_to_id.intern(parts[0]);
        let to_dot = parts[10];
        let to_id = name_to_id.intern(&to_dot[..to_dot.len() - 1]);
        connections.resize(name_to_id.new_len(connections.len()), Default::default());
        let value: i32 = parts[3].parse().unwrap();
        let happiness = if parts[2] == "gain" { value } else { -value };
        connections[from_id].push(Link { to_id, happiness });
    }
    connections
}

fn adjacency_matrix(connections: &Connections) -> Grid<i32> {
    let n = connections.len();
    let mut matrix = Grid::new(0i32, n, n);
    for (from_id, links) in connections.iter().enumerate() {
        for link in links {
            matrix.modify(link.happiness, from_id, link.to_id);
        }
    }
    matrix
}

fn seating_happiness(seating: &[usize], matrix: &Grid<i32>) -> i32 {
    let n = seating.len();
    (0..n)
        .map(|i| {
            let a = seating[i];
            let b = seating[(i + 1) % n];
            matrix.get(a, b) + matrix.get(b, a)
        })
        .sum()
}

fn max_happiness(matrix: &Grid<i32>) -> i32 {
    let n = matrix.num_rows;
    (1..n)
        .permutations(n - 1)
        .map(|perm| {
            let seating: Vec<usize> = std::iter::once(0).chain(perm).collect();
            seating_happiness(&seating, matrix)
        })
        .max()
        .unwrap_or(0)
}

fn solve_case1(filename: &str) -> i32 {
    let connections = parse(filename);
    max_happiness(&adjacency_matrix(&connections))
}

fn solve_case2(filename: &str) -> i32 {
    let connections = parse(filename);
    let n = connections.len();
    let base = adjacency_matrix(&connections);
    let n2 = n + 1;
    let mut matrix = Grid::new(0i32, n2, n2);
    for row in 0..n {
        for col in 0..n {
            matrix.modify(*base.get(row, col), row, col);
        }
    }
    max_happiness(&matrix)
}

fn main() {
    println!("Part 1");
    aoc::expect_result!(330, solve_case1("day13.example"));
    aoc::expect_result!(664, solve_case1("day13.input"));

    println!("Part 2");
    aoc::expect_result!(286, solve_case2("day13.example"));
    aoc::expect_result!(640, solve_case2("day13.input"));
}
