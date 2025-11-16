// https://adventofcode.com/2015/day/14

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

struct Reindeer {
  u32 kms;
  u32 fly;
  u32 rest;
};

fn parse(String const& filename) -> Vec<Reindeer> {
  return aoc::views::read_lines(filename) |
         std::views::transform([](str line) {
           let words = aoc::split_to_array<14>(line, ' ');
           return Reindeer{
               .kms = aoc::to_number<u32>(words[3]),
               .fly = aoc::to_number<u32>(words[6]),
               .rest = aoc::to_number<u32>(words[13]),
           };
         }) |
         aoc::ranges::to<Vec<Reindeer>>();
}

template <u32 SECONDS>
fn solve_case1(String const& filename) -> u32 {
  return std::ranges::max(
      parse(filename) | std::views::transform([](Reindeer const& reindeer) {
        let cycle_time = reindeer.fly + reindeer.rest;
        let num_cycles = SECONDS / cycle_time;
        let remainder = SECONDS % cycle_time;
        let distance = num_cycles * reindeer.fly * reindeer.kms +
                       std::min(remainder, reindeer.fly) * reindeer.kms;
        return distance;
      }));
}

template <u32 SECONDS>
fn solve_case2(String const& filename) -> u32 {
  let flock = parse(filename);
  auto distances = std::array<u32, 9>{};
  auto score = std::array<u32, 9>{};
  AOC_ASSERT(flock.size() <= score.size(), "Too many reindeer");
  for (let second : std::views::iota(0u, SECONDS)) {
    // Move each reindeer
    for (let[index, reindeer] : flock | std::views::enumerate) {
      let cycle_time = reindeer.fly + reindeer.rest;
      let flying = (second % cycle_time) < reindeer.fly;
      distances[index] += static_cast<u32>(flying) * reindeer.kms;
    }
    // Award the farthest reindeer
    using index_dist_t = std::pair<usize, u32>;
    auto distances_sorted = distances |
                            std::views::enumerate |
                            aoc::ranges::to<std::vector<index_dist_t>>();
    std::ranges::sort(distances_sorted, std::ranges::greater{},
                      &index_dist_t::second);
    let farthest_dist = distances_sorted.front().second;
    for (let[index, dist] : distances_sorted) {
      if (dist == farthest_dist) {
        score[index] += 1;
      } else {
        break;
      }
    }
  }
  return std::ranges::max(score);
}

int main() {
  std::println("Part 1");
  AOC_EXPECT_RESULT(1120, solve_case1<1000>("day14.example"));
  AOC_EXPECT_RESULT(2660, solve_case1<2503>("day14.input"));
  std::println("Part 2");
  AOC_EXPECT_RESULT(689, solve_case2<1000>("day14.example"));
  AOC_EXPECT_RESULT(1256, solve_case2<2503>("day14.input"));
  AOC_RETURN_CHECK_RESULT();
}
