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

inline constexpr int joker_card_number = 1;
inline constexpr int ace_card_number = 14;

template <bool joker>
constexpr int card_to_number(char c) {
  if (is_number(c)) {
    int number = static_cast<int>(c - '0');
    AOC_ASSERT((number > joker_card_number) && (number < 10),
               "Invalid card number");
    return number;
  }
  switch (c) {
    case 'T':
      return 10;
    case 'J':
      if constexpr (joker) {
        return joker_card_number;
      } else {
        return 11;
      }
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

static_assert(7 == card_to_number<false>('7'));
static_assert(13 == card_to_number<false>('K'));
static_assert(11 == card_to_number<false>('J'));
static_assert(1 == card_to_number<true>('J'));

enum class type_t {
  high_card,
  one_pair,
  two_pair,
  three_kind,
  full_house,
  four_kind,
  five_kind,
};

template <bool joker>
struct hand_type {
  using cards_t = std::array<int, 5>;

  cards_t cards;

  constexpr hand_type(std::string_view cards_str) {
    std::ranges::transform(cards_str, cards.begin(), &card_to_number<joker>);
  }

  constexpr hand_type(const cards_t& cards_) : cards{cards_} {}

  constexpr type_t type() const {
    std::array<int, ace_card_number> card_map{};
    int num_jokers = 0;
    for (auto c : cards) {
      if (c == joker_card_number) {
        if constexpr (joker) {
          ++num_jokers;
        } else {
          // This shouldn't actually be triggered,
          // we're just preventing code from being generated unnecessarily
        }
      } else {
        // Minus 2 to exclude 0 and 1
        ++card_map[c - 2];
      }
    }
    std::ranges::sort(card_map, std::greater<>{});
    if constexpr (joker) {
      card_map[0] += num_jokers;
    }
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

  constexpr std::strong_ordering operator<=>(const hand_type& other) const {
    auto type_comp = (type() <=> other.type());
    if (type_comp != std::strong_ordering::equal) {
      return type_comp;
    }
    return std::strong_order(cards, other.cards);
  }

  constexpr bool operator==(const hand_type& other) const = default;
};

static_assert(hand_type<false>{"AAAAA"}.type() == type_t::five_kind);
static_assert(hand_type<false>{"AA8AA"}.type() == type_t::four_kind);
static_assert(hand_type<false>{"23332"}.type() == type_t::full_house);
static_assert(hand_type<false>{"TTT98"}.type() == type_t::three_kind);
static_assert(hand_type<false>{"23432"}.type() == type_t::two_pair);
static_assert(hand_type<false>{"A23A4"}.type() == type_t::one_pair);
static_assert(hand_type<false>{"23456"}.type() == type_t::high_card);

static_assert(hand_type<false>{"T55J5"}.type() == type_t::three_kind);
static_assert(hand_type<true>{"T55J5"}.type() == type_t::four_kind);

static_assert(hand_type<false>{"23332"} > hand_type<false>{"TTT98"});
static_assert(hand_type<false>{"33332"} > hand_type<false>{"2AAAA"});
static_assert(hand_type<false>{"77888"} > hand_type<false>{"77788"});

static_assert(hand_type<false>{"KTJJT"} < hand_type<false>{"KK677"});
static_assert(hand_type<true>{"KTJJT"} > hand_type<true>{"KK677"});

template <bool joker>
using single_bid_type = std::pair<hand_type<joker>, int>;

template <bool joker>
using bids_type = std::vector<single_bid_type<joker>>;

template <bool joker>
constexpr int total_winnings(const bids_type<joker>& bids) {
  bids_type<joker> sorted_bids = ranges::sorted(bids);
  int sum = 0;
  for (int i = 0; i < sorted_bids.size(); ++i) {
    sum += (i + 1) * sorted_bids[i].second;
  }
  return sum;
}

static_assert((765 * 1 + 12 * 2) ==
              total_winnings<false>({single_bid_type<false>{"KTJJT", 765},
                                     single_bid_type<false>{"QQQJA", 12}}));
static_assert((765 * 2 + 12 * 1) ==
              total_winnings<true>({single_bid_type<true>{"KTJJT", 765},
                                    single_bid_type<true>{"QQQJA", 12}}));

template <bool joker>
int solve_case(const std::string& filename) {
  bids_type<joker> bids;
  auto read_values = [&](std::string_view line) {
    auto [hand_str, bid_str] =
        split<std::array<std::string_view, 2>>(line, ' ');
    bids.emplace_back(hand_str, to_number<int>(bid_str));
  };
  readfile_op(filename, read_values);

  int sum = total_winnings<joker>(bids);
  std::cout << filename << " -> " << sum << std::endl;
  return sum;
}

int main() {
  std::cout << "Part 1" << std::endl;
  AOC_EXPECT_RESULT(6440, (solve_case<false>("day07.example")));
  AOC_EXPECT_RESULT(249638405, (solve_case<false>("day07.input")));
  std::cout << "Part 2" << std::endl;
  AOC_EXPECT_RESULT(5905, (solve_case<true>("day07.example")));
  AOC_EXPECT_RESULT(249776650, (solve_case<true>("day07.input")));
  AOC_RETURN_CHECK_RESULT();
}
