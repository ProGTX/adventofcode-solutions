// https://adventofcode.com/2016/day/15

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <algorithm>
#include <print>
#endif

struct Disc {
  usize num_pos;
  usize pos;
};

using Input = Vec<Disc>;

/// The disc that part 2 adds below the input's discs
constexpr Disc EXTRA_DISC_VALUE{
    .num_pos = 11,
    .pos = 0,
};

auto parse(String const& filename) -> Input {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) {
           // Disc #D has N positions; at time=0, it is at position P.
           let words = aoc::split(line, ' ');
           return Disc{
               .num_pos = aoc::to_number<usize>(words[3]),
               .pos = aoc::to_number<usize>(aoc::rtrim(words[11], ".")),
           };
         }) |
         aoc::collect_vec<Disc>();
}

template <bool EXTRA_DISC>
fn solve_case(Input discs) -> usize {
  if constexpr (EXTRA_DISC) {
    // Part 2 adds one more disc below the ones from the input
    discs.push_back(EXTRA_DISC_VALUE);
  }
  // The first time to press the button so that the capsule
  // falls through every disc's slot
  return *stdr::find_if(Range{0uz}, [&](usize time) {
    // The capsule reaches disc `id` one second per disc after the press
    return stdr::all_of(discs | stdv::enumerate, [=](let& entry) {
      let[index, disc] = entry;
      return ((time + static_cast<usize>(index) + 1 + disc.pos) %
              disc.num_pos) == 0;
    });
  });
}

int main() {
  std::println("Part 1");
  let example = parse("day15.example");
  AOC_EXPECT_RESULT(5, solve_case<false>(example));
  let input = parse("day15.input");
  AOC_EXPECT_RESULT(317371, solve_case<false>(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(85, solve_case<true>(example));
  AOC_EXPECT_RESULT(2080951, solve_case<true>(input));

  AOC_RETURN_CHECK_RESULT();
}
