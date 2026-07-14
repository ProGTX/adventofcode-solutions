// https://adventofcode.com/2023/day/12

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <future>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

struct Record {
  String springs;
  Vec<u8> groups;
};

using Input = Vec<Record>;

fn parse(String const& filename) -> Input {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) {
           auto [springs, groups_str] = aoc::split_once<String>(line, ' ');
           return Record{std::move(springs),
                         aoc::split_to_vec<u8>(groups_str, ',')};
         }) |
         aoc::collect_vec<Record>();
}

constexpr inline char operational = '.';
constexpr inline char damaged = '#';
constexpr inline char unknown = '?';

struct SearchState {
  String springs;
  Vec<u8> groups;
  u8 damaged_before;

  constexpr bool operator==(SearchState const&) const = default;
};

template <>
struct std::hash<SearchState> {
  size_t operator()(SearchState const& state) const {
    auto combine = aoc::hash_combine{};
    combine(std::hash<String>{}(state.springs));
    for (u8 group : state.groups) {
      combine(std::hash<u8>{}(group));
    }
    combine(std::hash<u8>{}(state.damaged_before));
    return combine.seed;
  }
};

using Neighbors = aoc::static_vector<SearchState, 2>;
using Cache = std::unordered_map<SearchState, u64>;

fn arrangement_neighbors(SearchState const& state) -> Neighbors {
  auto neighbors = Neighbors{};
  if (state.springs.empty()) {
    // End of search
    return neighbors;
  }
  {
    switch (state.springs.front()) {
      case damaged: {
        let rest = str{state.springs}.substr(1);
        let extra_damaged = static_cast<u8>(stdr::distance(
            rest | stdv::take_while([](char c) { return c == damaged; })));
        let total_damaged =
            static_cast<u8>(state.damaged_before + 1 + extra_damaged);
        if ((extra_damaged + 1) == state.springs.size()) {
          // End of search, success if last group equals our count
          if ((state.groups.size() == 1) &&
              (state.groups.back() == total_damaged)) {
            neighbors.emplace_back();
          }
        } else {
          neighbors.emplace_back(
              SearchState{.springs = String{rest.substr(extra_damaged)},
                          .groups = state.groups,
                          .damaged_before = total_damaged});
        }
        break;
      }
      case operational: {
        auto new_groups = Vec<u8>{};
        if (state.damaged_before > 0) {
          if (state.groups.empty() ||
              (state.groups.front() != state.damaged_before)) {
            // Invalid group count
            break;
          }
          // Close the group
          new_groups = Vec<u8>(state.groups.begin() + 1, state.groups.end());
        } else {
          new_groups = state.groups;
        }
        let rest = str{state.springs}.substr(1);
        let skip = static_cast<usize>(stdr::distance(
            rest | stdv::take_while([](char c) { return c == operational; })));
        neighbors.emplace_back(SearchState{.springs = String{rest.substr(skip)},
                                           .groups = std::move(new_groups),
                                           .damaged_before = 0});
        break;
      }
      case unknown: {
        // Two options to explore
        auto state1 = state;
        state1.springs.front() = damaged;
        auto state2 = state;
        state2.springs.front() = operational;
        neighbors.emplace_back(std::move(state1));
        neighbors.emplace_back(std::move(state2));
        break;
      }
      default:
        AOC_UNREACHABLE("Invalid value");
    }
  }
  return neighbors;
}

template <usize factor>
fn count_arrangements(std::span<Record const> records) -> u64 {
  auto cache = Cache{};
  return aoc::ranges::accumulate(
      records | stdv::transform([&](Record const& record) {
        let start =
            SearchState{.springs = aoc::ranges::join(
                            stdv::repeat(record.springs, factor), unknown),
                        .groups = stdv::repeat(record.groups, factor) |
                                  stdv::join |
                                  aoc::collect_vec<u8>(),
                        .damaged_before = 0};
        cache.clear();
        return aoc::dfs_uniform(
            cache, start,
            [](SearchState const& state) {
              return state.springs.empty() && state.groups.empty();
            },
            arrangement_neighbors);
      }),
      u64{0});
}

template <usize factor>
fn solve_case(Input const& records) -> u64 {
  let num_threads = std::max<usize>(std::thread::hardware_concurrency(), 1uz);
  let chunk_size =
      std::max((records.size() + num_threads - 1) / num_threads, 1uz);

  auto futures = Vec<std::future<u64>>{};
  for (let chunk_index : Range{0uz, num_threads}) {
    let offset = chunk_index * chunk_size;
    if (offset >= records.size()) {
      break;
    }
    let count = std::min(chunk_size, records.size() - offset);
    futures.push_back(std::async(std::launch::async, [&records, offset, count] {
      return count_arrangements<factor>(
          std::span{records}.subspan(offset, count));
    }));
  }
  return aoc::ranges::accumulate(
      futures | stdv::transform([](auto& f) { return f.get(); }), u64{0});
}

int main() {
  std::println("Part 1");
  let example = parse("day12.example");
  AOC_EXPECT_RESULT(21, (solve_case<1>(example)));
  let input = parse("day12.input");
  AOC_EXPECT_RESULT(7379, (solve_case<1>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(206, (solve_case<2>(example)));
  AOC_EXPECT_RESULT(2612, (solve_case<3>(example)));
  AOC_EXPECT_RESULT(36308, (solve_case<4>(example)));
  AOC_EXPECT_RESULT(525152, (solve_case<5>(example)));
  AOC_EXPECT_RESULT(450228, (solve_case<2>(input)));
  AOC_EXPECT_RESULT(83673283, (solve_case<3>(input)));
  AOC_EXPECT_RESULT(23356098881, (solve_case<4>(input)));
  AOC_EXPECT_RESULT(7732028747925, (solve_case<5>(input)));

  AOC_RETURN_CHECK_RESULT();
}
