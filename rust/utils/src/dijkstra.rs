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

pub fn shortest_distance_bidirectional_astar<
    T,
    GetForwardNeighborsF,
    GetBackwardNeighborsF,
    ForwardNeighborIter,
    BackwardNeighborIter,
    ForwardHeuristicF,
    BackwardHeuristicF,
>(
    start: &T,
    end: &T,
    get_forward_neighbors: GetForwardNeighborsF,
    get_backward_neighbors: GetBackwardNeighborsF,
    forward_heuristic: ForwardHeuristicF,
    backward_heuristic: BackwardHeuristicF,
) -> Option<u32>
where
    T: Clone + Ord + Hash,
    ForwardNeighborIter: IntoIterator<Item = DijkstraState<T>>,
    BackwardNeighborIter: IntoIterator<Item = DijkstraState<T>>,
    GetForwardNeighborsF: Fn(&T) -> ForwardNeighborIter,
    GetBackwardNeighborsF: Fn(&T) -> BackwardNeighborIter,
    ForwardHeuristicF: Fn(&T) -> u32,
    BackwardHeuristicF: Fn(&T) -> u32,
{
    if start == end {
        return Some(0);
    }

    let mut forward_distances: HashMap<T, u32> = HashMap::new();
    let mut backward_distances: HashMap<T, u32> = HashMap::new();
    let mut forward_unvisited: BinaryHeap<(Reverse<u32>, u32, T)> = BinaryHeap::new();
    let mut backward_unvisited: BinaryHeap<(Reverse<u32>, u32, T)> = BinaryHeap::new();
    let mut best_distance: Option<u32> = None;

    forward_distances.insert(start.clone(), 0);
    backward_distances.insert(end.clone(), 0);
    forward_unvisited.push((Reverse(forward_heuristic(start)), 0, start.clone()));
    backward_unvisited.push((Reverse(backward_heuristic(end)), 0, end.clone()));

    while !forward_unvisited.is_empty() && !backward_unvisited.is_empty() {
        while let Some((_, g, data)) = forward_unvisited.peek() {
            if *g <= *forward_distances.get(data).unwrap() {
                break;
            }
            forward_unvisited.pop();
        }
        while let Some((_, g, data)) = backward_unvisited.peek() {
            if *g <= *backward_distances.get(data).unwrap() {
                break;
            }
            backward_unvisited.pop();
        }

        if forward_unvisited.is_empty() || backward_unvisited.is_empty() {
            break;
        }

        let forward_f = forward_unvisited.peek().unwrap().0.0;
        let backward_f = backward_unvisited.peek().unwrap().0.0;
        if let Some(distance) = best_distance {
            if forward_f >= distance && backward_f >= distance {
                break;
            }
        }

        if forward_f <= backward_f {
            let (_, g, data) = forward_unvisited.pop().unwrap();
            for neighbor in get_forward_neighbors(&data) {
                let next_g = g + neighbor.distance;
                match forward_distances.entry(neighbor.data.clone()) {
                    Entry::Vacant(e) => {
                        e.insert(next_g);
                    }
                    Entry::Occupied(mut e) => {
                        if next_g >= *e.get() {
                            continue;
                        }
                        *e.get_mut() = next_g;
                    }
                }

                let next_f = next_g + forward_heuristic(&neighbor.data);
                forward_unvisited.push((Reverse(next_f), next_g, neighbor.data.clone()));

                if let Some(backward_g) = backward_distances.get(&neighbor.data) {
                    let total_distance = next_g + backward_g;
                    if best_distance.is_none_or(|distance| total_distance < distance) {
                        best_distance = Some(total_distance);
                    }
                }
            }
        } else {
            let (_, g, data) = backward_unvisited.pop().unwrap();
            for neighbor in get_backward_neighbors(&data) {
                let next_g = g + neighbor.distance;
                match backward_distances.entry(neighbor.data.clone()) {
                    Entry::Vacant(e) => {
                        e.insert(next_g);
                    }
                    Entry::Occupied(mut e) => {
                        if next_g >= *e.get() {
                            continue;
                        }
                        *e.get_mut() = next_g;
                    }
                }

                let next_f = next_g + backward_heuristic(&neighbor.data);
                backward_unvisited.push((Reverse(next_f), next_g, neighbor.data.clone()));

                if let Some(forward_g) = forward_distances.get(&neighbor.data) {
                    let total_distance = next_g + forward_g;
                    if best_distance.is_none_or(|distance| total_distance < distance) {
                        best_distance = Some(total_distance);
                    }
                }
            }
        }
    }

    best_distance
}

pub fn shortest_distance_bidirectional<
    T,
    GetForwardNeighborsF,
    GetBackwardNeighborsF,
    ForwardNeighborIter,
    BackwardNeighborIter,
>(
    start: &T,
    end: &T,
    get_forward_neighbors: GetForwardNeighborsF,
    get_backward_neighbors: GetBackwardNeighborsF,
) -> Option<u32>
where
    T: Clone + Ord + Hash,
    ForwardNeighborIter: IntoIterator<Item = DijkstraState<T>>,
    BackwardNeighborIter: IntoIterator<Item = DijkstraState<T>>,
    GetForwardNeighborsF: Fn(&T) -> ForwardNeighborIter,
    GetBackwardNeighborsF: Fn(&T) -> BackwardNeighborIter,
{
    shortest_distance_bidirectional_astar(
        start,
        end,
        get_forward_neighbors,
        get_backward_neighbors,
        |_| 0,
        |_| 0,
    )
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
