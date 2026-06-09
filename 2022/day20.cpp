// https://adventofcode.com/2022/day/20

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <print>
#include <ranges>

using Input = Vec<i64>;

fn parse(String const& filename) -> Input {
  return aoc::views::read_numbers<i64>(filename) | aoc::ranges::to<Input>();
}

struct IndexedNum {
  isize index;
  i64 value;
};

template <usize num_mixings, class OGList>
fn reorder_list(OGList&& original_list) -> Vec<i64> {
  let numbers = std::forward<OGList>(original_list) |
                stdv::enumerate |
                stdv::transform([](auto pair) -> IndexedNum {
                  let[i, v] = pair;
                  return {i, v};
                }) |
                aoc::collect_vec<IndexedNum>();
  auto mixed = numbers;

  for (let _ : Range{0uz, num_mixings}) {
    for (let& indexed_number : numbers) {
      let[orig_index, orig_value] = indexed_number;
      if (orig_value == 0) {
        continue;
      }

      let current_it = stdr::find(mixed, orig_index, &IndexedNum::index);

      let next_it = [&] {
        let begin = stdr::begin(mixed);
        let size = std::ssize(mixed) - 1;
        auto pos = std::distance(begin, current_it);
        pos = (size + (pos + (orig_value % size))) % size;
        return begin + pos;
      }();

      let it_distance = std::distance(current_it, next_it);
      if (it_distance < 0) {
        std::shift_right(next_it, current_it + 1, 1);
      } else {
        std::shift_left(current_it, next_it + 1, 1);
      }
      *next_it = indexed_number;
    }
  }

  return mixed | stdv::transform(&IndexedNum::value) | aoc::collect_vec<i64>();
}

fn sum_numbers(Vec<i64> const& list) -> i64 {
  let n = std::ssize(list);
  let zero_pos = stdr::distance(stdr::begin(list), stdr::find(list, i64{0}));
  let at = [&](isize offset) -> i64 {
    return list[static_cast<usize>((zero_pos + offset) % n)];
  };
  return at(1000) + at(2000) + at(3000);
}

template <usize num_mixings, i64 decryption_key>
fn solve_case(Input const& input) -> i64 {
  let mixed = reorder_list<num_mixings>(
      input | stdv::transform([](i64 v) { return v * decryption_key; }));
  return sum_numbers(mixed);
}

int main() {
  std::println("Part 1");
  let example = parse("day20.example");
  AOC_EXPECT_RESULT(3, (solve_case<1, 1>(example)));
  let input = parse("day20.input");
  AOC_EXPECT_RESULT(1591, (solve_case<1, 1>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(1623178306, (solve_case<10, 811589153>(example)));
  AOC_EXPECT_RESULT(14579387544492, (solve_case<10, 811589153>(input)));

  AOC_RETURN_CHECK_RESULT();
}
