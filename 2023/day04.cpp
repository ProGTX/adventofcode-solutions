// https://adventofcode.com/2023/day/4

#include "../common/common.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <optional>
#include <ostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

template <int num_winning, int num_actual>
struct card_type {
  using winning_t = std::array<int, num_winning>;
  using actual_t = std::array<int, num_actual>;

  constexpr card_type(winning_t winning_, actual_t actual_)
      : winning{winning_}, actual{actual_} {
    // Cards need to be sorted in order for intersection to work
    std::ranges::sort(winning);
    std::ranges::sort(actual);
  }

  winning_t winning;
  actual_t actual;
};

// Requires the card to be sorted
template <int num_winning, int num_actual>
constexpr int card_value(const card_type<num_winning, num_actual>& card) {
  typename card_type<num_winning, num_actual>::winning_t intersection{0, 0, 0,
                                                                      0, 0};
  std::ranges::set_intersection(card.winning, card.actual,
                                std::begin(intersection));
  auto num_winning_numbers =
      std::ranges::count_if(intersection, std::identity{});
  if (num_winning_numbers == 0) {
    return 0;
  }
  return pown(2, (num_winning_numbers - 1));
}

static_assert(8 == card_value(card_type<5, 8>{{41, 48, 83, 86, 17},
                                              {83, 86, 6, 31, 17, 9, 48, 53}}));

template <int num_winning, int num_actual>
int solve_case(const std::string& filename) {
  int sum = 0;
  auto solver = [&](std::string_view line) {
    auto [card_id, numbers] = split<std::array<std::string_view, 2>>(line, ':');
    auto [winning_str, actual_str] =
        split<std::array<std::string_view, 2>>(numbers, '|');
    using card_t = card_type<num_winning, num_actual>;
    card_t current_card{
        split<typename card_t::winning_t, true>(winning_str, ' '),
        split<typename card_t::actual_t, true>(actual_str, ' ')};
    sum += card_value(current_card);
  };

  readfile_op(filename, solver);

  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(13, (solve_case<5, 8>("day04.example")));
  AOC_EXPECT_RESULT(32609, (solve_case<10, 25>("day04.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(467835, solve_case<true>("day04.example"));
  // AOC_EXPECT_RESULT(84883664, solve_case<true>("day04.input"));
  AOC_RETURN_CHECK_RESULT();
}
