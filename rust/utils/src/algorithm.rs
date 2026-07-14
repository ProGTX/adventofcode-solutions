use crate::dijkstra::{DijkstraNeighborView, DijkstraState};
use rustc_hash::FxHashMap;
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
