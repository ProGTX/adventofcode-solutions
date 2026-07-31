use crate::dijkstra::{DijkstraNeighborView, DijkstraState};
use rustc_hash::{FxHashMap, FxHashSet};
use std::collections::VecDeque;
use std::collections::hash_map::Entry;
use std::hash::Hash;
use std::ops::{Add, Mul};

/// Generic depth-first search with memoization, using a caller-owned cache.
///
/// `get_neighbors` expands a state into a range of successor states,
/// each paired with a weight (`DijkstraState::distance`).
/// A state-dependent leaf value can be encoded as a single weighted edge
/// to an already-`end_reached` state.
/// A state's value is 1 if `end_reached(state)`,
/// otherwise it's neighbors' `(weight * value)` terms
/// folded together with `combine`, left to right
/// (0 if there are none, i.e. a dead end).
/// Returns the value computed for `start_state`.
/// Every state visited along the way is also cached.
pub fn dfs_with_cache<T, V, EndReachedF, NeighborsF, NeighborIter, CombineF>(
    cache: &mut FxHashMap<T, V>,
    start_state: T,
    end_reached: EndReachedF,
    get_neighbors: NeighborsF,
    combine: CombineF,
) -> V
where
    T: Clone + Eq + Hash,
    V: Copy + From<u32> + Mul<Output = V>,
    EndReachedF: Fn(&T) -> bool,
    NeighborIter: IntoIterator<Item = DijkstraState<T>>,
    NeighborsF: Fn(&T) -> NeighborIter,
    CombineF: Fn(V, V) -> V,
{
    fn search<T, V, EndReachedF, NeighborsF, NeighborIter, CombineF>(
        state: T,
        cache: &mut FxHashMap<T, V>,
        end_reached: &EndReachedF,
        get_neighbors: &NeighborsF,
        combine: &CombineF,
    ) -> V
    where
        T: Clone + Eq + Hash,
        V: Copy + From<u32> + Mul<Output = V>,
        EndReachedF: Fn(&T) -> bool,
        NeighborIter: IntoIterator<Item = DijkstraState<T>>,
        NeighborsF: Fn(&T) -> NeighborIter,
        CombineF: Fn(V, V) -> V,
    {
        if let Some(&value) = cache.get(&state) {
            return value;
        }
        let result = if end_reached(&state) {
            V::from(1)
        } else {
            let mut acc = V::from(0);
            for neighbor in get_neighbors(&state) {
                let weighted = V::from(neighbor.distance)
                    * search(neighbor.data, cache, end_reached, get_neighbors, combine);
                acc = combine(acc, weighted);
            }
            acc
        };
        cache.insert(state, result);
        result
    }

    search(start_state, cache, &end_reached, &get_neighbors, &combine)
}

/// `dfs` that allocates its own cache  and returns it in full,
/// keyed by every state visited, including `start_state`.
pub fn dfs<T, V, EndReachedF, NeighborsF, NeighborIter, CombineF>(
    start_state: T,
    end_reached: EndReachedF,
    get_neighbors: NeighborsF,
    combine: CombineF,
) -> FxHashMap<T, V>
where
    T: Clone + Eq + Hash,
    V: Copy + From<u32> + Mul<Output = V>,
    EndReachedF: Fn(&T) -> bool,
    NeighborIter: IntoIterator<Item = DijkstraState<T>>,
    NeighborsF: Fn(&T) -> NeighborIter,
    CombineF: Fn(V, V) -> V,
{
    let mut cache = FxHashMap::default();
    dfs_with_cache(&mut cache, start_state, end_reached, get_neighbors, combine);
    cache
}

/// `dfs_with_cache` for the common case:
/// every edge has weight 1 and neighbors are combined by addition.
/// A state's value is 1 if `end_reached(state)`,
/// otherwise the sum of its neighbors' values (0 for a dead end).
pub fn dfs_uniform_with_cache<T, V, EndReachedF, NeighborsF, NeighborIter>(
    cache: &mut FxHashMap<T, V>,
    start_state: T,
    end_reached: EndReachedF,
    get_neighbors: NeighborsF,
) -> V
where
    T: Clone + Eq + Hash,
    V: Copy + From<u32> + Mul<Output = V> + Add<Output = V>,
    EndReachedF: Fn(&T) -> bool,
    NeighborIter: IntoIterator<Item = T>,
    NeighborsF: Fn(&T) -> NeighborIter,
{
    dfs_with_cache(
        cache,
        start_state,
        end_reached,
        |state| {
            get_neighbors(state)
                .into_iter()
                .dijkstra_uniform_neighbors()
        },
        |a, b| a + b,
    )
}

/// `dfs_uniform_with_cache` that allocates its own cache.
pub fn dfs_uniform<T, V, EndReachedF, NeighborsF, NeighborIter>(
    start_state: T,
    end_reached: EndReachedF,
    get_neighbors: NeighborsF,
) -> FxHashMap<T, V>
where
    T: Clone + Eq + Hash,
    V: Copy + From<u32> + Mul<Output = V> + Add<Output = V>,
    EndReachedF: Fn(&T) -> bool,
    NeighborIter: IntoIterator<Item = T>,
    NeighborsF: Fn(&T) -> NeighborIter,
{
    let mut cache = FxHashMap::default();
    dfs_uniform_with_cache(&mut cache, start_state, end_reached, get_neighbors);
    cache
}

/// Computes the length of the longest path from `start`
/// to every node reachable from it (the critical path).
///
/// The portion of the graph reachable from `start` must be a DAG.
/// If it contains a cycle, this function does not terminate.
pub fn critical_distances<T, GetEndF, GetNeighborsF, NeighborIter>(
    start: &T,
    is_end: GetEndF,
    get_neighbors: GetNeighborsF,
) -> FxHashMap<T, u32>
where
    T: Clone + Ord + Hash,
    GetEndF: Fn(&T) -> bool,
    NeighborIter: IntoIterator<Item = DijkstraState<T>>,
    GetNeighborsF: Fn(&T) -> NeighborIter,
{
    // Discover every reachable node via DFS,
    // recording each one when it finishes (post-order).
    // Reversing post-order gives a topological order:
    // for every edge u -> v, u comes before v.
    // Nodes beyond `is_end` are not expanded,
    // mirroring Dijkstra's early exit.
    let mut visited: FxHashSet<T> = FxHashSet::default();
    let mut post_order: Vec<T> = Vec::new();
    let mut stack: Vec<(T, bool)> = vec![(start.clone(), false)];
    visited.insert(start.clone());

    while let Some((node, finished)) = stack.pop() {
        if finished {
            post_order.push(node);
            continue;
        }
        if is_end(&node) {
            post_order.push(node);
            continue;
        }
        stack.push((node.clone(), true));
        for neighbor in get_neighbors(&node) {
            if visited.insert(neighbor.data.clone()) {
                stack.push((neighbor.data, false));
            }
        }
    }

    // Relax edges in topological order,
    // so that by the time a node is processed,
    // its own distance from `start` is already final.
    // All predecessors of `is_end` are guaranteed
    // to precede it in this order,
    // so its distance is settled when we reach it and we can stop.
    let mut distances: FxHashMap<T, u32> = FxHashMap::default();
    distances.insert(start.clone(), 0);

    for node in post_order.into_iter().rev() {
        let Some(&distance) = distances.get(&node) else {
            continue;
        };
        for neighbor in get_neighbors(&node) {
            let next_distance = distance + neighbor.distance;
            match distances.entry(neighbor.data) {
                Entry::Vacant(e) => {
                    e.insert(next_distance);
                }
                Entry::Occupied(mut e) => {
                    if next_distance > *e.get() {
                        *e.get_mut() = next_distance;
                    }
                }
            }
        }
        if is_end(&node) {
            break;
        }
    }

    return distances;
}

/// Longest simple path from `start` in a weighted graph
/// whose nodes are indexed `0..num_nodes`.
///
/// `get_neighbors` expands a node into its successors,
/// each paired with an edge weight (`DijkstraState::distance`).
/// `end_reached` takes a node and how many nodes the path visits
/// including that one, so both "this is the exit"
/// and "every node has been visited" are expressible.
/// The search stops there, and returns the heaviest such path,
/// or None if no path reaches one without revisiting a node.
///
/// No memoization: a node's value depends on which nodes the path already used.
/// Longest path is NP-hard once the graph has cycles,
/// so this is an exhaustive walk with backtracking.
pub fn longest_simple_path<EndReachedF, NeighborsF, NeighborIter>(
    num_nodes: usize,
    start: usize,
    end_reached: EndReachedF,
    get_neighbors: NeighborsF,
) -> Option<u32>
where
    EndReachedF: Fn(usize, usize) -> bool,
    NeighborIter: IntoIterator<Item = DijkstraState<usize>>,
    NeighborsF: Fn(usize) -> NeighborIter,
{
    fn search<EndReachedF, NeighborsF, NeighborIter>(
        node: usize,
        num_visited: usize,
        visited: &mut [bool],
        end_reached: &EndReachedF,
        get_neighbors: &NeighborsF,
    ) -> Option<u32>
    where
        EndReachedF: Fn(usize, usize) -> bool,
        NeighborIter: IntoIterator<Item = DijkstraState<usize>>,
        NeighborsF: Fn(usize) -> NeighborIter,
    {
        if end_reached(node, num_visited) {
            return Some(0);
        }
        visited[node] = true;
        let mut longest = None;
        for neighbor in get_neighbors(node) {
            if visited[neighbor.data] {
                continue;
            }
            if let Some(rest) = search(
                neighbor.data,
                num_visited + 1,
                visited,
                end_reached,
                get_neighbors,
            ) {
                longest = longest.max(Some(rest + neighbor.distance));
            }
        }
        visited[node] = false;
        longest
    }

    let mut visited = vec![false; num_nodes];
    search(start, 1, &mut visited, &end_reached, &get_neighbors)
}

/// Breadth-first flood fill over unweighted, undirected connectivity.
///
/// `get_neighbors` expands a state into its neighboring states.
/// There's no `end_reached`/early exit, unlike `dijkstra::shortest_distances`:
/// a flood fill's whole point is to reach everything connected to `start`,
/// so `get_neighbors` is expected to encode the stopping condition itself
/// (e.g. by not yielding neighbors that fail some predicate).
/// Returns every state reachable from `start`, including `start` itself.
pub fn flood_fill<T, GetNeighborsF, NeighborIter>(
    start: &T,
    get_neighbors: GetNeighborsF,
) -> FxHashSet<T>
where
    T: Clone + Eq + Hash,
    NeighborIter: IntoIterator<Item = T>,
    GetNeighborsF: Fn(&T) -> NeighborIter,
{
    let mut visited = FxHashSet::default();
    let mut queue = VecDeque::new();
    visited.insert(start.clone());
    queue.push_back(start.clone());
    while let Some(node) = queue.pop_front() {
        for neighbor in get_neighbors(&node) {
            if visited.insert(neighbor.clone()) {
                queue.push_back(neighbor);
            }
        }
    }
    visited
}
