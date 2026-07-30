use aoc::string::NameToId;
use itertools::Itertools;
use rustc_hash::FxHashSet;

type Graph = Vec<Vec<usize>>;

fn parse(filename: &str) -> (Graph, Vec<String>) {
    let mut name_to_id = NameToId::new();
    let mut graph = Graph::new();
    let mut names = Vec::new();
    for line in aoc::file::read_lines(filename) {
        let (a, b) = line.split_once('-').unwrap();
        let a_id = name_to_id.intern(a);
        let b_id = name_to_id.intern(b);
        let new_len = name_to_id.new_len(graph.len());
        graph.resize(new_len, Default::default());
        names.resize(new_len, String::new());
        graph[a_id].push(b_id);
        graph[b_id].push(a_id);
        names[a_id] = a.to_string();
        names[b_id] = b.to_string();
    }
    (graph, names)
}

fn solve_case1((graph, names): &(Graph, Vec<String>)) -> usize {
    let t_ids = names
        .iter()
        .enumerate()
        .filter(|(_, name)| name.starts_with('t'))
        .map(|(id, _)| id);

    let mut triangles = FxHashSet::default();
    for t_id in t_ids {
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

fn solve_case2((graph, names): &(Graph, Vec<String>)) -> String {
    let mut unique_candidates = FxHashSet::default();
    for (id, connections) in graph.iter().enumerate() {
        for subset in connections.iter().copied().powerset() {
            // +1 for `id` itself, needs at least 3 to matter (see part 1)
            if subset.len() + 1 < 3 {
                continue;
            }
            let mut candidate = subset;
            candidate.push(id);
            candidate.sort();
            unique_candidates.insert(candidate);
        }
    }

    let mut candidates = unique_candidates.into_iter().collect_vec();
    candidates.sort_by_key(|candidate| std::cmp::Reverse(candidate.len()));

    let clique = candidates
        .iter()
        .find(|candidate| {
            candidate.iter().all(|&member| {
                candidate
                    .iter()
                    .all(|&other| (other == member) || graph[member].contains(&other))
            })
        })
        .unwrap();

    let mut party = clique.iter().map(|&id| names[id].as_str()).collect_vec();
    party.sort();
    party.join(",")
}

fn main() {
    println!("Part 1");
    let example = parse("day23.example");
    aoc::expect_result!(7, solve_case1(&example));
    let input = parse("day23.input");
    aoc::expect_result!(1348, solve_case1(&input));

    println!("Part 2");
    aoc::expect_result!("co,de,ka,ta", solve_case2(&example));
    aoc::expect_result!(
        "am,bv,ea,gh,is,iy,ml,nj,nl,no,om,tj,yv",
        solve_case2(&input)
    );
}
