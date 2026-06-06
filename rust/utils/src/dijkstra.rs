use std::cmp::Ordering;
use std::cmp::Reverse;
use std::collections::BinaryHeap;
use std::collections::HashMap;
use std::collections::hash_map::Entry;
use std::hash::Hash;

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct DijkstraState<T> {
    pub data: T,
    pub distance: u32,
}
// https://doc.rust-lang.org/std/collections/binary_heap/index.html
impl<T: Ord> Ord for DijkstraState<T> {
    fn cmp(&self, other: &Self) -> Ordering {
        other
            .distance
            .cmp(&self.distance)
            .then_with(|| self.data.cmp(&other.data))
    }
}
impl<T: Ord> PartialOrd for DijkstraState<T> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

pub fn shortest_distances_astar<T, GetEndF, GetNeighborsF, NeighborIter, HeuristicF>(
    start: &T,
    is_end: GetEndF,
    get_neighbors: GetNeighborsF,
    heuristic: HeuristicF,
) -> HashMap<T, u32>
where
    T: Clone + Ord + Hash,
    GetEndF: Fn(&T) -> bool,
    NeighborIter: IntoIterator<Item = DijkstraState<T>>,
    GetNeighborsF: Fn(&T) -> NeighborIter,
    HeuristicF: Fn(&T) -> u32,
{
    let mut distances: HashMap<T, u32> = HashMap::new();
    // Heap entries: (Reverse(f), g, data) — sorted by f = g + h, min-first.
    let mut unvisited: BinaryHeap<(Reverse<u32>, u32, T)> = BinaryHeap::new();
    distances.insert(start.clone(), 0_u32);
    unvisited.push((Reverse(heuristic(start)), 0, start.clone()));
    while let Some((_, g, data)) = unvisited.pop() {
        if (is_end(&data)) {
            break;
        }
        if (g > *distances.get(&data).unwrap()) {
            continue;
        }
        for neighbor in get_neighbors(&data) {
            let next_g = g + neighbor.distance;
            match distances.entry(neighbor.data.clone()) {
                Entry::Vacant(e) => {
                    e.insert(next_g);
                    let next_f = next_g + heuristic(&neighbor.data);
                    unvisited.push((Reverse(next_f), next_g, neighbor.data));
                }
                Entry::Occupied(mut e) => {
                    if (next_g < *e.get()) {
                        *e.get_mut() = next_g;
                        let next_f = next_g + heuristic(&neighbor.data);
                        unvisited.push((Reverse(next_f), next_g, neighbor.data));
                    }
                }
            }
        }
    }
    return distances;
}

pub fn shortest_distances<T, GetEndF, GetNeighborsF, NeighborIter>(
    start: &T,
    is_end: GetEndF,
    get_neighbors: GetNeighborsF,
) -> HashMap<T, u32>
where
    T: Clone + Ord + Hash,
    GetEndF: Fn(&T) -> bool,
    NeighborIter: IntoIterator<Item = DijkstraState<T>>,
    GetNeighborsF: Fn(&T) -> NeighborIter,
{
    shortest_distances_astar(start, is_end, get_neighbors, |_| 0)
}

pub trait DijkstraNeighborView: Iterator {
    fn dijkstra_uniform_neighbors(
        self,
    ) -> std::iter::Map<Self, fn(Self::Item) -> DijkstraState<Self::Item>>
    where
        Self: Sized,
    {
        self.map(|data| DijkstraState { data, distance: 1 })
    }
}

impl<NeighborIter: Iterator> DijkstraNeighborView for NeighborIter {}
