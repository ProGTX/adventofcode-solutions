// https://adventofcode.com/2024/day/23

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <bit>
#include <print>
#include <thread>
#include <unordered_set>

// Every node name is two lowercase letters,
// so 26*26 ids is enough for any input.
constexpr usize max_nodes = 26 * 26;

using Graph = Vec<Vec<i32>>;

struct Input {
  Graph graph;
  Vec<String> names;
};

struct Triangle {
  std::array<i32, 3> ids;
  constexpr bool operator==(Triangle const&) const = default;
};
template <>
struct std::hash<Triangle> : aoc::packed_hash {};
static_assert(aoc::hashable<Triangle>);

// A clique candidate as a bitmask over node ids instead of a sorted Vec<i32>.
// Building and hashing one is then a handful of word ops with no heap alloc,
// which matters since solve_case2 does this millions of times.
constexpr usize candidate_words = (max_nodes + 63) / 64;
struct CandidateBits {
  std::array<u64, candidate_words> words{};
  constexpr bool operator==(CandidateBits const&) const = default;
};
template <>
struct std::hash<CandidateBits> : aoc::packed_hash {};
static_assert(aoc::hashable<CandidateBits>);

fn set_bit(CandidateBits& bits, i32 id) {
  bits.words[static_cast<usize>(id) / 64] |=
      (u64{1} << (static_cast<usize>(id) % 64));
}

fn popcount(CandidateBits const& bits) -> i32 {
  auto count = i32{0};
  for (let word : bits.words) {
    count += std::popcount(word);
  }
  return count;
}

// Bits are set in increasing id order and words are walked low to high,
// so the result comes out sorted for free.
fn to_ids(CandidateBits const& bits) -> Vec<i32> {
  auto ids = Vec<i32>{};
  ids.reserve(static_cast<usize>(popcount(bits)));
  for (usize word_idx = 0; word_idx < bits.words.size(); ++word_idx) {
    auto remaining = bits.words[word_idx];
    while (remaining != 0) {
      let bit = std::countr_zero(remaining);
      ids.push_back(static_cast<i32>(word_idx * 64 + static_cast<usize>(bit)));
      remaining &= (remaining - 1);
    }
  }
  return ids;
}

auto parse(String const& filename) -> Input {
  auto name_to_id = aoc::name_to_id{};
  auto input = Input{};
  for (str line : aoc::views::read_lines(filename)) {
    let[a, b] = aoc::split_once(line, '-');
    let a_id = static_cast<i32>(name_to_id.intern(a));
    let b_id = static_cast<i32>(name_to_id.intern(b));
    let new_size = name_to_id.new_size(input.graph.size());
    input.graph.resize(new_size);
    input.names.resize(new_size);
    input.graph[static_cast<usize>(a_id)].push_back(b_id);
    input.graph[static_cast<usize>(b_id)].push_back(a_id);
    input.names[static_cast<usize>(a_id)] = String{a};
    input.names[static_cast<usize>(b_id)] = String{b};
  }
  return input;
}

fn solve_case1(Input const& input) -> i32 {
  auto triangles = std::unordered_set<Triangle>{};
  for (usize t_id = 0; t_id < input.names.size(); ++t_id) {
    if (!input.names[t_id].starts_with('t')) {
      continue;
    }
    let& connections = input.graph[t_id];
    for (usize i = 0; i < connections.size(); ++i) {
      for (usize j = i + 1; j < connections.size(); ++j) {
        let a = connections[i];
        let b = connections[j];
        if (!stdr::contains(input.graph[static_cast<usize>(a)], b)) {
          continue;
        }
        auto triangle = std::array{static_cast<i32>(t_id), a, b};
        stdr::sort(triangle);
        triangles.insert(Triangle{triangle});
      }
    }
  }
  return static_cast<i32>(triangles.size());
}

// All subsets of one node's neighbors, plus the node itself,
// as candidate bitmasks.
fn candidates_for_node(i32 id, Vec<i32> const& connections,
                       std::unordered_set<CandidateBits>& out) {
  let degree = connections.size();
  AOC_ASSERT(degree < 32, "neighbor mask needs more bits");
  for (u32 mask = 0; mask < (u32{1} << degree); ++mask) {
    // +1 for `id` itself, needs at least 3 to matter (see part 1)
    if (static_cast<usize>(std::popcount(mask)) + 1 < 3) {
      continue;
    }
    auto bits = CandidateBits{};
    set_bit(bits, id);
    for (usize i = 0; i < connections.size(); ++i) {
      if ((mask & (u32{1} << i)) != 0) {
        set_bit(bits, connections[i]);
      }
    }
    out.insert(bits);
  }
}

fn solve_case2(Input const& input) -> String {
  constexpr usize min_parallelism = 8;
  let num_threads =
      std::max(min_parallelism, usize{std::thread::hardware_concurrency()});
  let chunk_size = (input.graph.size() + num_threads - 1) / num_threads;

  // Every node's neighbor-subset generation is independent of every other
  // node's, so the id range is split across threads,
  // each deduping into its own local set.
  auto local_sets = Vec<std::unordered_set<CandidateBits>>(num_threads);
  {
    auto threads = Vec<std::jthread>{};
    threads.reserve(num_threads);
    for (usize t = 0; t < num_threads; ++t) {
      let start_id = t * chunk_size;
      let end_id = std::min(start_id + chunk_size, input.graph.size());
      threads.emplace_back([&, start_id, end_id, t] {
        for (auto id = start_id; id < end_id; ++id) {
          candidates_for_node(static_cast<i32>(id), input.graph[id],
                              local_sets[t]);
        }
      });
    }
  } // all threads join here

  // Bucket by size instead of sorting: candidate sizes only span a small
  // range, so this is one O(n) pass instead of O(n log n) comparisons each
  // moving an 88-byte element.
  // This also skips merging the per-thread sets into one:
  // the same clique candidate can come out of more than one thread
  // (once per member, and members can land in different chunks),
  // but a duplicate bitmask just becomes a harmless repeat in the same
  // bucket, since the search below only needs one match and stops at the first.
  auto by_size = Vec<Vec<CandidateBits>>{};
  for (let& set : local_sets) {
    for (let& bits : set) {
      let size = static_cast<usize>(popcount(bits));
      if (by_size.size() <= size) {
        by_size.resize(size + 1);
      }
      by_size[size].push_back(bits);
    }
  }

  auto clique = Vec<i32>{};
  for (auto size_it = by_size.rbegin();
       (size_it != by_size.rend()) && clique.empty(); ++size_it) {
    for (let& bits : *size_it) {
      auto members = to_ids(bits);
      let all_connected = stdr::all_of(members, [&](i32 member) {
        return stdr::all_of(members, [&](i32 other) {
          return (other == member) ||
                 stdr::contains(input.graph[static_cast<usize>(member)], other);
        });
      });
      if (all_connected) {
        clique = std::move(members);
        break;
      }
    }
  }

  auto party = clique |
               stdv::transform([&](i32 id) {
                 return input.names[static_cast<usize>(id)];
               }) |
               aoc::collect_vec<String>();
  stdr::sort(party);
  return aoc::ranges::join(party, ',');
}

int main() {
  std::println("Part 1");
  let example = parse("day23.example");
  AOC_EXPECT_RESULT(7, solve_case1(example));
  let input = parse("day23.input");
  AOC_EXPECT_RESULT(1348, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT("co,de,ka,ta", solve_case2(example));
  AOC_EXPECT_RESULT("am,bv,ea,gh,is,iy,ml,nj,nl,no,om,tj,yv",
                    solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
