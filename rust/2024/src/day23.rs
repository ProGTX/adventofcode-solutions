use aoc::string::NameToId;
use itertools::Itertools;
use rustc_hash::FxHashSet;

// Every node name is two lowercase letters, so 26*26 ids is enough for any input.
const MAX_NODES: usize = 26 * 26;

type Graph = Vec<Vec<usize>>;

// A clique candidate as a bitmask over node ids instead of a sorted Vec<usize>.
// Building and hashing one is then a handful of word ops with no heap allocation,
// which matters since solve_case2 does this millions of times.
const WORDS: usize = MAX_NODES.div_ceil(64);
type CandidateBits = [u64; WORDS];

fn set_bit(bits: &mut CandidateBits, id: usize) {
    bits[id / 64] |= 1 << (id % 64);
}

fn popcount(bits: &CandidateBits) -> u32 {
    bits.iter().map(|word| word.count_ones()).sum()
}

// Bits are set in increasing id order and words are walked low to high,
// so the result comes out sorted for free.
fn to_ids(bits: &CandidateBits) -> Vec<usize> {
    let mut ids = Vec::with_capacity(popcount(bits) as usize);
    for (word_idx, &word) in bits.iter().enumerate() {
        let mut remaining = word;
        while remaining != 0 {
            let bit = remaining.trailing_zeros() as usize;
            ids.push(word_idx * 64 + bit);
            remaining &= remaining - 1;
        }
    }
    ids
}

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
        let degree = connections.len();
        debug_assert!(degree < u32::BITS as usize, "neighbor mask needs more bits");
        for mask in 0u32..(1u32 << degree) {
            // +1 for `id` itself, needs at least 3 to matter (see part 1)
            if mask.count_ones() as usize + 1 < 3 {
                continue;
            }
            let mut bits = CandidateBits::default();
            set_bit(&mut bits, id);
            for (i, &neighbor) in connections.iter().enumerate() {
                if mask & (1 << i) != 0 {
                    set_bit(&mut bits, neighbor);
                }
            }
            unique_candidates.insert(bits);
        }
    }

    // Bucket by size instead of sorting: candidate sizes only span a small range,
    // so this is one O(n) pass instead of O(n log n) comparisons
    // each moving an 88-byte element.
    let mut by_size = Vec::new();
    for bits in unique_candidates {
        let size = popcount(&bits) as usize;
        if by_size.len() <= size {
            by_size.resize_with(size + 1, Vec::new);
        }
        by_size[size].push(bits);
    }

    let clique = by_size
        .iter()
        .rev()
        .flatten()
        .map(to_ids)
        .find(|members| {
            members.iter().all(|&member| {
                members
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
