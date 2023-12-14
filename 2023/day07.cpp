// https://adventofcode.com/2023/day/7

#include "../common/common.h"

#include <algorithm>
#include <compare>
#include <iostream>
#include <map>
#include <ostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

inline constexpr int ace_card_number = 14;

constexpr int card_to_number(char c) {
  if (is_number(c)) {
    int number = static_cast<int>(c - '0');
    AOC_ASSERT((number >= 2) && (number < 10), "Invalid card number");
    return number;
  }
  switch (c) {
    case 'T':
      return 10;
    case 'J':
      return 11;
    case 'Q':
      return 12;
    case 'K':
      return 13;
    case 'A':
      return ace_card_number;
    default:
      throw std::runtime_error("Invalid card number");
  }
}

static_assert(7 == card_to_number('7'));
static_assert(13 == card_to_number('K'));

enum class type_t {
  high_card,
  one_pair,
  two_pair,
  three_kind,
  full_house,
  four_kind,
  five_kind,
};

struct hand_t {
  using cards_t = std::array<int, 5>;

  cards_t cards;

  constexpr hand_t(std::string_view cards_str) {
    std::ranges::transform(cards_str, cards.begin(), &card_to_number);
  }

  constexpr hand_t(const cards_t& cards_) : cards{cards_} {}

  constexpr type_t type() const {
    std::array<int, ace_card_number + 1> card_map{};
    for (auto c : cards) {
      ++card_map[c];
    }
    std::ranges::sort(card_map, std::greater<>{});
    switch (card_map[0]) {
      case 5:
        return type_t::five_kind;
      case 4:
        return type_t::four_kind;
      case 3:
        if (card_map[1] == 2) {
          return type_t::full_house;
        } else {
          return type_t::three_kind;
        }
      case 2:
        if (card_map[1] == 2) {
          return type_t::two_pair;
        } else {
          return type_t::one_pair;
        }
      default:
        return type_t::high_card;
    }
  }

  constexpr std::strong_ordering operator<=>(const hand_t& other) {
    auto type_comp = (type() <=> other.type());
    if (type_comp != std::strong_ordering::equal) {
      return type_comp;
    }
    return std::strong_order(cards, other.cards);
  }

  constexpr bool operator==(const hand_t& other) const = default;
};

static_assert(hand_t{"AAAAA"}.type() == type_t::five_kind);
static_assert(hand_t{"AA8AA"}.type() == type_t::four_kind);
static_assert(hand_t{"23332"}.type() == type_t::full_house);
static_assert(hand_t{"TTT98"}.type() == type_t::three_kind);
static_assert(hand_t{"23432"}.type() == type_t::two_pair);
static_assert(hand_t{"A23A4"}.type() == type_t::one_pair);
static_assert(hand_t{"23456"}.type() == type_t::high_card);

static_assert(hand_t{"23332"} > hand_t{"TTT98"});
static_assert(hand_t{"33332"} > hand_t{"2AAAA"});
static_assert(hand_t{"77888"} > hand_t{"77788"});

using single_bid_t = std::pair<hand_t, int>;
using bids_t = std::vector<single_bid_t>;

constexpr int total_winnings(const bids_t& bids) {
  bids_t sorted_bids{bids};
  std::ranges::sort(sorted_bids, std::less<>{}, &single_bid_t::first);
  int sum = 0;
  for (int i = 0; i < sorted_bids.size(); ++i) {
    sum += (i + 1) * sorted_bids[i].second;
  }
  return sum;
}

static_assert((765 + 2 * 684) == total_winnings({single_bid_t{"32T3K", 765},
                                                 single_bid_t{"T55J5", 684}}));

template <bool>
int solve_case(const std::string& filename) {
  bids_t bids;
  auto read_values = [&](std::string_view line) {
    auto [hand_str, bid_str] =
        split<std::array<std::string_view, 2>>(line, ' ');
    bids.emplace_back(hand_str, to_number<int>(bid_str));
  };
  readfile_op(filename, read_values);

  int sum = total_winnings(bids);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(6440, (solve_case<false>("day07.example")));
  AOC_EXPECT_RESULT(249638405, (solve_case<false>("day07.input")));
  // std::cout << "Part 2" << std::endl;
  // AOC_EXPECT_RESULT(71503, (solve_case<true>("day07.example")));
  // AOC_EXPECT_RESULT(32583852, (solve_case<true>("day07.input")));
  AOC_RETURN_CHECK_RESULT();
}
