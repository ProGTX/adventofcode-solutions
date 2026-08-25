// https://adventofcode.com/2016/day/19

#include "../common/common.h"
#include "../common/rust.h"

#ifndef AOC_IMPORT_STD
#include <print>
#include <utility>
#endif

/// The number of elves in the circle
using Input = usize;

auto parse(String const& filename) -> Input {
  return aoc::to_number<Input>(aoc::trim(aoc::read_file(filename)));
}

/// The elf that ends up with all of the presents
fn solve_case1(Input const& num_elves) -> usize {
  auto elves = Range{1uz, num_elves + 1} | aoc::collect_vec<usize>();
  auto new_elves = Vec<usize>{};
  new_elves.reserve(num_elves);
  while (elves.size() > 1) {
    let current_len = elves.size();
    new_elves.clear();
    // An odd circle ends with its last elf taking from its first,
    // so the round after it starts one elf further along
    auto i = ((current_len % 2) == 0) ? 0uz : 2uz;
    while (i < current_len) {
      new_elves.push_back(elves[i]);
      i += 2;
    }

    std::swap(elves, new_elves);
  }
  return elves[0];
}

/// A circle of elves as a doubly linked list,
/// each node living at its own index for the whole run
struct Elf {
  usize id;
  usize prev;
  usize next;
};

/// The elf that ends up with all of the presents,
/// when each elf takes from the one across the circle
fn solve_case2(Input const& num_elves) -> usize {
  auto elves = Range{0uz, num_elves} |
               stdv::transform([=](usize index) {
                 return Elf{
                     .id = index + 1,
                     .prev = (index + num_elves - 1) % num_elves,
                     .next = (index + 1) % num_elves,
                 };
               }) |
               aoc::collect_vec<Elf>();
  auto current = 0uz;
  // The elf directly across the circle, `num_left / 2` elves ahead
  auto across = num_elves / 2;
  auto num_left = num_elves;
  while (num_left > 1) {
    let[before, after] = std::pair{elves[across].prev, elves[across].next};
    elves[after].prev = before;
    elves[before].next = after;
    // Walking the whole half circle again every time would be quadratic,
    // so the elf across is carried along instead:
    // losing one elf pulls it one step closer,
    // and the circle shrinking to an even size pushes it one step away
    across = elves[across].next;
    if ((num_left % 2) != 0) {
      across = elves[across].next;
    }
    num_left -= 1;
    current = elves[current].next;
  }
  return elves[current].id;
}

int main() {
  std::println("Part 1");
  let example = parse("day19.example");
  AOC_EXPECT_RESULT(3, solve_case1(example));
  let input = parse("day19.input");
  AOC_EXPECT_RESULT(1830117, solve_case1(input));

  std::println("Part 2");
  AOC_EXPECT_RESULT(2, solve_case2(example));
  AOC_EXPECT_RESULT(1417887, solve_case2(input));

  AOC_RETURN_CHECK_RESULT();
}
