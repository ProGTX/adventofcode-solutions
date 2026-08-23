// https://adventofcode.com/2016/day/11

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <array>
#include <print>
#include <span>
#include <utility>
#endif

/// A floor holds one bit per element's chip, and one per its generator.
using Floor = u16;
constexpr usize GENERATOR_SHIFT = 8;
constexpr usize NUM_FLOORS = 4;

/// Every chip and every generator a single floor could hold
constexpr usize MAX_ITEMS = 2 * GENERATOR_SHIFT;
constexpr usize MAX_PAIRS = (MAX_ITEMS * (MAX_ITEMS - 1)) / 2;

using Floors = std::array<Floor, NUM_FLOORS>;

struct State {
  Floors floors;
  usize elevator;

  constexpr bool operator==(State const&) const = default;
  constexpr auto operator<=>(State const&) const = default;
};
template <>
struct std::hash<State> : aoc::packed_hash {};
static_assert(aoc::hashable<State>);

/// The parsed floors, and how many elements they have room for
struct Input {
  Floors floors;
  usize num_elements;
};

auto parse(String const& filename) -> Input {
  auto floors = Floors{};
  auto elements = aoc::name_to_id{};

  for (let[floor, line] : aoc::read_lines(filename) | stdv::enumerate) {
    let words = aoc::split_to_vec<str>(line, ' ', [](let& word) {
      return aoc::trim(aoc::construct_string<str>(word), ",.");
    });
    for (let[i, word] : words | stdv::enumerate) {
      // <element> generator | <element>-compatible microchip
      auto name = str{};
      auto chip = false;
      if (word == "generator") {
        name = words[i - 1];
      } else if (word == "microchip") {
        constexpr let suffix = str{"-compatible"};
        name = words[i - 1];
        AOC_ASSERT(name.ends_with(suffix), "Chip without an element");
        name.remove_suffix(suffix.size());
        chip = true;
      } else {
        continue;
      }
      let id = elements.intern(name);
      floors[floor] |= 1u << (id + (chip ? 0 : GENERATOR_SHIFT));
    }
  }

  // Two extra elements, for the parts added in part 2
  let num_elements = elements.new_size(0) + 2;
  AOC_ASSERT((num_elements <= GENERATOR_SHIFT), "Too many elements");

  return Input{floors, num_elements};
}

/// A chip is fried unless it sits with its own generator,
/// or the floor holds no generators at all.
fn is_valid(Floor floor) -> bool {
  let generators = floor >> GENERATOR_SHIFT;
  return (generators == 0) || ((floor & ~generators & 0xff) == 0);
}

/// Elements are interchangeable, so a state is only its multiset of
/// (chip floor, generator floor) pairs.
/// Relabelling the elements into sorted order picks one representative
/// per class of equivalent states.
///
/// https://www.reddit.com/r/adventofcode/comments/5hoia9/comment/db1v1ws/
fn canonicalize(Floors floors) -> Floors {
  auto elements =
      aoc::static_vector<std::pair<usize, usize>, GENERATOR_SHIFT>{};
  for (let id : aoc::views::indices(GENERATOR_SHIFT)) {
    let holds = [](usize bit) {
      return [bit](Floor items) { return (items & (1u << bit)) != 0; };
    };
    let chip = aoc::ranges::position(floors, true, holds(id));
    let generator =
        aoc::ranges::position(floors, true, holds(id + GENERATOR_SHIFT));
    if (chip.has_value() && generator.has_value()) {
      elements.emplace_back(*chip, *generator);
    }
  }
  stdr::sort(elements);

  auto canonical = Floors{};
  for (let[id, element] : elements | stdv::enumerate) {
    let[chip, generator] = element;
    canonical[chip] |= 1u << id;
    canonical[generator] |= 1u << (id + GENERATOR_SHIFT);
  }
  return canonical;
}

template <bool ExtraParts>
fn solve_case(Input const& input) -> u32 {
  // The extra parts start on the first floor, otherwise their bits stay unused
  auto start_floors = input.floors;
  if constexpr (ExtraParts) {
    for (let id : Range{input.num_elements - 2, input.num_elements}) {
      start_floors[0] |= (1u << id) | (1u << (id + GENERATOR_SHIFT));
    }
  }
  let start = State{canonicalize(start_floors), 0};
  // Everything ends up on the top floor
  let all_items =
      start_floors[0] | start_floors[1] | start_floors[2] | start_floors[3];
  let end = State{canonicalize(Floors{0, 0, 0, static_cast<Floor>(all_items)}),
                  NUM_FLOORS - 1};

  let get_neighbors = [&](State const& current) {
    let floor = current.elevator;
    // Each item on this floor, as a mask of the one bit that represents it
    auto items = aoc::static_vector<Floor, MAX_ITEMS>{};
    // Clear the lowest set bit each round, leaving the items still to be listed
    for (auto left = current.floors[floor]; left != 0; left &= (left - 1)) {
      // The lowest set bit, i.e. one chip or one generator
      items.push_back(static_cast<Floor>(left & -left));
    }

    // Every pair of items that could be taken along together
    auto pairs = aoc::static_vector<Floor, MAX_PAIRS>{};
    for (let[i, first] : items | stdv::enumerate) {
      for (let second : items | stdv::drop(i + 1)) {
        pairs.push_back(first | second);
      }
    }

    auto neighbors = Vec<aoc::dijkstra_neighbor_t<State>>{};
    // The elevator goes one floor at a time, and going below the first
    // wraps the unsigned floor id past the top one
    for (let next : {floor - 1, floor + 1}) {
      if (next >= NUM_FLOORS) {
        continue;
      }
      // Carrying a pair up is never worse than carrying a single item,
      // and bringing a single item down is never worse than bringing a pair,
      // so only fall back to the other kind
      // when the preferred one has no valid move
      let groups = (next > floor) ? std::array{std::span<const Floor>{pairs},
                                               std::span<const Floor>{items}}
                                  : std::array{std::span<const Floor>{items},
                                               std::span<const Floor>{pairs}};
      let before = neighbors.size();
      for (let group : groups) {
        for (let moved : group) {
          auto floors = current.floors;
          floors[floor] &= ~moved;
          floors[next] |= moved;
          if (is_valid(floors[floor]) && is_valid(floors[next])) {
            neighbors.emplace_back(State{canonicalize(floors), next}, 1);
          }
        }
        if (neighbors.size() > before) {
          break;
        }
      }
    }

    return neighbors;
  };

  let distances = aoc::shortest_distances_dijkstra(
      start, [&](State const& current) { return current == end; },
      get_neighbors);

  return static_cast<u32>(distances.at(end));
}

int main() {
  std::println("Part 1");
  let example = parse("day11.example");
  AOC_EXPECT_RESULT(11, solve_case<false>(example));
  let input = parse("day11.input");
  AOC_EXPECT_RESULT(31, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(55, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
