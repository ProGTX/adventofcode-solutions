use std::cmp::Ordering;
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

pub fn shortest_distances<T, F, I>(start: &T, end: &T, get_neighbors: F) -> HashMap<T, u32>
where
    T: Clone + Ord + Hash,
    I: IntoIterator<Item = DijkstraState<T>>,
    F: Fn(&T) -> I,
{
    let mut distances = HashMap::new();
    let mut unvisited = BinaryHeap::new();
    distances.insert(start.clone(), 0_u32);
    unvisited.push(DijkstraState {
        distance: 0,
        data: start.clone(),
    });
    while (!unvisited.is_empty()) {
        let current = unvisited.pop().unwrap();
        if (current.data == *end) {
            break;
        }
        if (current.distance > *distances.get(&current.data).unwrap()) {
            continue;
        }
        for mut neighbor in get_neighbors(&current.data) {
            let next_distance = current.distance + neighbor.distance;
            match distances.entry(neighbor.data.clone()) {
                Entry::Vacant(e) => {
                    e.insert(next_distance);
                    neighbor.distance = next_distance;
                    unvisited.push(neighbor);
                }
                Entry::Occupied(mut e) => {
                    if (next_distance < *e.get()) {
                        *e.get_mut() = next_distance;
                        neighbor.distance = next_distance;
                        unvisited.push(neighbor);
                    }
                }
            }
        }
    }
    return distances;
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

impl<I: Iterator> DijkstraNeighborView for I {}
