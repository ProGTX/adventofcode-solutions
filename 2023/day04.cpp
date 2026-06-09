// https://adventofcode.com/2023/day/4

#include "../common/common.h"
#include "../common/rust.h"

#include <algorithm>
#include <array>
#include <print>
#include <ranges>

using CardStrings = std::pair<String, String>;

template <int num_winning, int num_actual>
struct card_type {
  using winning_t = std::array<int, num_winning>;
  using actual_t = std::array<int, num_actual>;

  constexpr card_type(winning_t winning_, actual_t actual_)
      : winning{winning_}, actual{actual_} {
    // Cards need to be sorted in order for intersection to work
    stdr::sort(winning);
    stdr::sort(actual);
  }

  winning_t winning;
  actual_t actual;
};

auto parse(String const& filename) -> Vec<CardStrings> {
  return aoc::views::read_lines(filename) |
         stdv::transform([](str line) -> CardStrings {
           let[_, numbers] = aoc::split_once(line, ':');
           let[winning_str, actual_str] = aoc::split_once(numbers, '|');
           return {String{winning_str}, String{actual_str}};
         }) |
         aoc::collect_vec<CardStrings>();
}

// Requires the card to be sorted
template <i32 num_winning, i32 num_actual>
fn num_matching(card_type<num_winning, num_actual> const& card) -> i32 {
  using intersection_t = typename card_type<num_winning, num_actual>::winning_t;
  intersection_t intersection{};
  stdr::fill(intersection, 0);
  stdr::set_intersection(card.winning, card.actual, std::begin(intersection));
  return stdr::count_if(intersection, std::identity{});
}

template <i32 num_winning, i32 num_actual>
fn card_value(card_type<num_winning, num_actual> const& card) -> i32 {
  auto matching = num_matching(card);
  if (matching == 0) {
    return 0;
  }
  return aoc::pown(2, (matching - 1));
}

static_assert(8 == card_value(card_type<5, 8>{{41, 48, 83, 86, 17},
                                              {83, 86, 6, 31, 17, 9, 48, 53}}));

template <i32 num_winning, i32 num_actual, bool count_scratchcards>
fn solve_case(Vec<CardStrings> const& cards) -> i32 {
  auto num_scratchcards = Vec<i32>(1, 0);
  auto sum = i32{};

  for (let[id, card_strings] : cards | stdv::enumerate) {
    let[winning_str, actual_str] = card_strings;
    using card_t = card_type<num_winning, num_actual>;
    let current_card =
        card_t{aoc::split<typename card_t::winning_t, true>(winning_str, ' '),
               aoc::split<typename card_t::actual_t, true>(actual_str, ' ')};

    if constexpr (!count_scratchcards) {
      sum += card_value(current_card);
    } else {
      // Add the original card
      ++num_scratchcards[id];
      let current_num = num_scratchcards[id];
      sum += current_num;

      let matching = num_matching(current_card);
      // Create copies of next cards
      let required_scratchcards_size = static_cast<usize>(id + matching) + 2;
      if (num_scratchcards.size() < required_scratchcards_size) {
        num_scratchcards.resize(required_scratchcards_size);
      }
      for (let i : Range{1, matching + 1}) {
        num_scratchcards[id + i] += current_num;
      }
    }
  }

  return sum;
}

int main() {
  std::println("Part 1");
  let example = parse("day04.example");
  AOC_EXPECT_RESULT(13, (solve_case<5, 8, false>(example)));
  let input = parse("day04.input");
  AOC_EXPECT_RESULT(32609, (solve_case<10, 25, false>(input)));

  std::println("Part 2");
  AOC_EXPECT_RESULT(30, (solve_case<5, 8, true>(example)));
  AOC_EXPECT_RESULT(14624680, (solve_case<10, 25, true>(input)));
  AOC_RETURN_CHECK_RESULT();
}
