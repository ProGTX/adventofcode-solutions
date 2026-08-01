use aoc::{
    dijkstra::{self, DijkstraNeighborView},
    string::NameToId,
};
use rustc_hash::FxHashMap;

type Graph = Vec<Vec<usize>>;

/// A directed edge, since flow always runs one way.
/// An undirected connection is two arcs, one per direction.
#[derive(Clone, Copy, Debug, Hash, PartialEq, Eq)]
struct GraphArc {
    from: usize,
    to: usize,
}
impl GraphArc {
    const fn new(from: usize, to: usize) -> Self {
        Self { from, to }
    }
    const fn reversed(self) -> Self {
        Self::new(self.to, self.from)
    }
}

/// How many remaining units of flow each arc can still carry
type Residual = FxHashMap<GraphArc, i32>;

/// The puzzle states the two groups are held together by exactly three wires
const CUT_SIZE: usize = 3;

fn parse(filename: &str) -> Graph {
    let mut name_to_id = NameToId::new();
    let mut graph = Graph::new();
    for line in aoc::file::read_lines(filename) {
        let (name, connections) = line.split_once(':').unwrap();
        let id = name_to_id.intern(name.trim());
        for connection in connections.split_whitespace() {
            let connection_id = name_to_id.intern(connection);
            let new_len = name_to_id.new_len(graph.len());
            graph.resize(new_len, Default::default());
            // A connection is listed on one side only, but goes both ways
            graph[id].push(connection_id);
            graph[connection_id].push(id);
        }
    }
    graph
}

/// The neighbors still reachable from `node` without a saturated arc
fn open_neighbors(graph: &Graph, residual: &Residual, node: usize) -> Vec<usize> {
    graph[node]
        .iter()
        .copied()
        .filter(|&next| residual[&GraphArc::new(node, next)] > 0)
        .collect()
}

/// Edmonds–Karp algorithm.
/// The largest number of edge-disjoint paths between `source` and `sink`,
/// which by Menger's theorem is the smallest number of wires separating them.
/// Gives up as soon as the count exceeds `CUT_SIZE`,
/// since then the two are on the same side and the exact number is of no use.
/// The residual is left as the search found it,
/// so a returned `CUT_SIZE` leaves exactly the arcs of the cut saturated.
fn max_flow(graph: &Graph, residual: &mut Residual, source: usize, sink: usize) -> usize {
    let mut flow = 0;
    while flow <= CUT_SIZE {
        let mut predecessors = FxHashMap::default();
        // Every arc counts as one step,
        // so this is a BFS for the shortest augmenting path,
        // as Edmonds-Karp prescribes
        let distances = dijkstra::shortest_distances_with_predecessors(
            &source,
            |node| *node == sink,
            |node| {
                open_neighbors(graph, residual, *node)
                    .into_iter()
                    .dijkstra_uniform_neighbors()
            },
            &mut predecessors,
        );
        if !distances.contains_key(&sink) {
            break;
        }
        // `get_path` is in end-to-start order, and leaves out the start node
        let mut path = dijkstra::get_path(&predecessors, &sink);
        path.push(source);
        path.reverse();
        for step in path.windows(2) {
            let arc = GraphArc::new(step[0], step[1]);
            // Freeing up the arc in the opposite direction
            // is what lets a later path undo this one's choice
            *residual.get_mut(&arc).unwrap() -= 1;
            *residual.get_mut(&arc.reversed()).unwrap() += 1;
        }
        flow += 1;
    }
    flow
}

fn solve_case(graph: &Graph) -> usize {
    // `parse` put every connection into both nodes' neighbor lists,
    // so one arc per neighbor entry covers both directions of every connection,
    // each starting out unused
    let full_residual: Residual = graph
        .iter()
        .enumerate()
        .flat_map(|(node, neighbors)| {
            neighbors
                .iter()
                .map(move |&next| (GraphArc::new(node, next), 1))
        })
        .collect();

    // Any node will do as the source,
    // but the sink has to end up on the other side of the cut,
    // which only shows in its flow
    let source = 0;
    for sink in (0..graph.len()).filter(|&sink| sink != source) {
        let mut residual = full_residual.clone();
        if max_flow(graph, &mut residual, source, sink) != CUT_SIZE {
            continue;
        }
        // With the cut saturated, the source can no longer reach past it
        let group =
            aoc::algorithm::flood_fill(&source, |node| open_neighbors(graph, &residual, *node));
        return group.len() * (graph.len() - group.len());
    }
    unreachable!("No cut of {CUT_SIZE} wires found");
}

fn main() {
    println!("Part 1");
    let example = parse("day25.example");
    aoc::expect_result!(54, solve_case(&example));
    let input = parse("day25.input");
    aoc::expect_result!(601344, solve_case(&input));

    println!("Part 2");
    aoc::return_incomplete();
}
