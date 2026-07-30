use aoc::string::NameToId;
use itertools::Itertools;
use rustc_hash::FxHashSet;

type Graph = Vec<Vec<usize>>;

fn parse(filename: &str) -> (Graph, Vec<usize>) {
    let mut name_to_id = NameToId::new();
    let mut graph = Graph::new();
    let mut t_ids = FxHashSet::default();
    for line in aoc::file::read_lines(filename) {
        let (a, b) = line.split_once('-').unwrap();
        let a_id = name_to_id.intern(a);
        let b_id = name_to_id.intern(b);
        graph.resize(name_to_id.new_len(graph.len()), Default::default());
        graph[a_id].push(b_id);
        graph[b_id].push(a_id);
        if a.starts_with('t') {
            t_ids.insert(a_id);
        }
        if b.starts_with('t') {
            t_ids.insert(b_id);
        }
    }
    (graph, t_ids.into_iter().collect())
}

fn solve_case1((graph, t_ids): &(Graph, Vec<usize>)) -> usize {
    let mut triangles = FxHashSet::default();
    for &t_id in t_ids {
        for [&a, &b] in graph[t_id].iter().array_combinations() {
            if !graph[a].contains(&b) {
                continue;
            }
            let mut triangle = [t_id, a, b];
            triangle.sort();
            triangles.insert(triangle);
        }
    }
    triangles.len()
}

fn main() {
    println!("Part 1");
    let example = parse("day23.example");
    aoc::expect_result!(7, solve_case1(&example));
    let input = parse("day23.input");
    aoc::expect_result!(1348, solve_case1(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
