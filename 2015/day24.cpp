// https://adventofcode.com/2015/day/24

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>
#include <span>

auto parse(String const& filename) -> Vec<u32> {
  auto packages =
      aoc::views::read_numbers<u32>(filename) | aoc::collect_vec<u32>();
  stdr::reverse(packages);
  return packages;
}

// TODO: aoc::static_vector<u32, 16>
using Group = Vec<u32>;

fn qe(std::span<const u32> group) -> u64 {
  return stdr::fold_left(group, u64{1}, std::multiplies{});
}

template <u32 NUM_GROUPS>
fn solve_case(std::span<const u32> packages) -> u64 {
  // Assume all packages have unique weights and are sorted in decreasing order
  let total_weight = aoc::ranges::accumulate(packages, 0u);
  AOC_ASSERT((total_weight % NUM_GROUPS) == 0, "Invalid package configuration");
  let target_group_weight = total_weight / NUM_GROUPS;
  auto num_packages = 0uz;
  auto weight = 0u;
  let size = packages.size();
  while (num_packages < size) {
    weight += packages[num_packages];
    if (weight > target_group_weight) {
      break;
    }
    num_packages += 1;
  }
  auto possible = Vec<Group>{};
  for (let index : Range{num_packages - 1, size}) {
    possible = packages |
               aoc::views::combinations(index + 1) |
               stdv::filter([&](let& combo) {
                 return aoc::ranges::dot_product(packages, combo) ==
                        target_group_weight;
               }) |
               stdv::transform([&](let& combo) {
                 let neighbor = aoc::binary_select_from_combination<Group>(
                     packages, combo);
                 return neighbor;
               }) |
               aoc::collect_vec<Group>();
    if (!possible.empty()) {
      break;
    }
  }
  stdr::sort(possible, [](Group const& lhs, Group const& rhs) {
    if (lhs.size() == rhs.size()) {
      return qe(lhs) < qe(rhs);
    }
    return lhs.size() < rhs.size();
  });
  return qe(possible[0]);
}

int main() {
  std::println("Part 1");
  let example = parse("day24.example");
  AOC_EXPECT_RESULT(99, solve_case<3>(example));
  let input = parse("day24.input");
  AOC_EXPECT_RESULT(10723906903, solve_case<3>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(44, solve_case<4>(example));
  AOC_EXPECT_RESULT(74850409, solve_case<4>(input));

  AOC_RETURN_CHECK_RESULT();
}
