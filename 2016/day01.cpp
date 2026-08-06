// https://adventofcode.com/2016/day/1

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <print>
#endif

using Arrow = aoc::arrow_type<int>;

enum class Rotation {
  Clockwise,
  Anticlockwise,
};

using Instruction = std::pair<Rotation, u32>;

auto parse(String const& filename) -> Vec<Instruction> {
  let text = aoc::read_file(filename);
  return aoc::split_to_vec<str>(aoc::trim(text), ", ") |
         stdv::transform([](str instruction) -> Instruction {
           let rotation = [&] {
             if (instruction.starts_with('R')) {
               return Rotation::Clockwise;
             }
             if (instruction.starts_with('L')) {
               return Rotation::Anticlockwise;
             }
             AOC_UNREACHABLE("Unknown rotation");
           }();
           let blocks = aoc::to_number<u32>(instruction.substr(1));
           return Instruction{rotation, blocks};
         }) |
         aoc::collect_vec<Instruction>();
}

template <bool DetectCycle>
fn solve_case(Vec<Instruction> const& instructions) -> i32 {
  auto arrow = Arrow{point{}, aoc::north};

  auto visited = aoc::hash_set<point>{};
  if constexpr (DetectCycle) {
    visited.insert(arrow.position);
  }

  for (let& [ rotation, blocks ] : instructions) {
    arrow.direction = (rotation == Rotation::Clockwise)
                          ? aoc::clockwise_basic(arrow.direction)
                          : aoc::anticlockwise_basic(arrow.direction);
    if constexpr (DetectCycle) {
      // The first location visited twice can be mid-leg, so check every block
      for (let _ : aoc::views::indices(blocks)) {
        arrow.position += aoc::get_diff<int>(arrow.direction);
        if (!visited.insert(arrow.position).second) {
          return distance_manhattan(point{}, arrow.position);
        }
      }
    } else {
      arrow.position +=
          aoc::get_diff<int>(arrow.direction) * static_cast<i32>(blocks);
    }
  }

  return distance_manhattan(point{}, arrow.position);
}

int main() {
  std::println("Part 1");
  let example = parse("day01.example");
  AOC_EXPECT_RESULT(12, solve_case<false>(example));
  let example2 = parse("day01.example2");
  AOC_EXPECT_RESULT(8, solve_case<false>(example2));
  let input = parse("day01.input");
  AOC_EXPECT_RESULT(279, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(12, solve_case<true>(example));
  AOC_EXPECT_RESULT(4, solve_case<true>(example2));
  AOC_EXPECT_RESULT(163, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
