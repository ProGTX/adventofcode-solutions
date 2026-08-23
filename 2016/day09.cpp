// https://adventofcode.com/2016/day/9

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <print>
#endif

auto parse(String const& filename) -> String {
  return String{aoc::trim(aoc::read_file(filename))};
}

template <bool Recursion>
fn decompressed_len(str compressed) -> usize {
  auto count = usize{};
  auto slice = compressed;
  loop {
    let open = slice.find('(');
    if (open == str::npos) {
      break;
    }
    count += open;
    let close = slice.find(')');
    AOC_ASSERT(close > open, "Invalid parenthesis");
    let[num_chars, num_repeats] =
        aoc::split_once<usize>(slice.substr(open + 1, close - open - 1), 'x');
    let subslice = slice.substr(close + 1, num_chars);
    let subslice_len = [&] {
      if constexpr (!Recursion) {
        return num_chars;
      } else {
        return decompressed_len<Recursion>(subslice);
      }
    }();
    count += subslice_len * num_repeats;
    slice = slice.substr(close + 1 + num_chars);
  }

  return count + slice.size();
}

static_assert(6 == decompressed_len<false>("ADVENT"));
static_assert(7 == decompressed_len<false>("A(1x5)BC"));
static_assert(9 == decompressed_len<false>("(3x3)XYZ"));
static_assert(11 == decompressed_len<false>("A(2x2)BCD(2x2)EFG"));
static_assert(6 == decompressed_len<false>("(6x1)(1x3)A"));
static_assert(9 == decompressed_len<true>("(3x3)XYZ"));
static_assert(241920 ==
              decompressed_len<true>("(27x12)(20x12)(13x14)(7x10)(1x12)A"));
static_assert(445 == decompressed_len<true>(
                         "(25x3)(3x3)ABC(2x3)XY(5x2)PQRSTX(18x9)(3x2)TWO"
                         "(5x7)SEVEN"));

fn solve_case1(str compressed) -> usize {
  return decompressed_len<false>(compressed);
}
fn solve_case2(str compressed) -> usize {
  return decompressed_len<true>(compressed);
}

int main() {
  std::println("Part 1");
  let example = parse("day09.example");
  AOC_EXPECT_RESULT(18, solve_case1(example));
  let input = parse("day09.input");
  AOC_EXPECT_RESULT(152851, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(20, solve_case2(example));
  AOC_EXPECT_RESULT(11797310782, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
